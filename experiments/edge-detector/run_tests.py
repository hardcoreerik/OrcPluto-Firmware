#!/usr/bin/env python3
"""Offline deterministic CS16LE burst tests. Writes log in *launch* directory."""
import json
import os
from pathlib import Path
import random
import shutil
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
CWD = Path.cwd()
LOG = CWD / 'edge_detector_test.log'
ART = CWD / 'edge_detector_test_artifacts'
ART.mkdir(exist_ok=True)
lines = []
def run(cmd):
    p = subprocess.run(cmd, text=True, capture_output=True, check=False)
    lines.append(f"$ {' '.join(map(str,cmd))}\nexit={p.returncode}\nstdout:\n{p.stdout}\nstderr:\n{p.stderr}")
    return p

def must(ok, message):
    lines.append(('PASS ' if ok else 'FAIL ') + message)
    if not ok:
        raise AssertionError(message)

try:
    compiler = os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++')
    must(bool(compiler), 'C++17 compiler available')
    exe = ART / ('edge_detect.exe' if os.name == 'nt' else 'edge_detect')
    p = run([compiler, '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror', str(ROOT / 'detect.cpp'), '-o', str(exe)])
    must(p.returncode == 0, 'build without warnings')
    window = 64
    random.seed(8675309)
    data = bytearray()
    for index in range(8):
        for j in range(window):
            if index in (3,4):
                # Deterministic 4-bin complex sinusoid. No floating generation dependencies.
                i,q = [(700,0),(0,700),(-700,0),(0,-700)][j%4]
            else:
                i, q = random.randint(-3,3), random.randint(-3,3)
            data += struct.pack('<hh', i, q)
    iq = ART / 'synthetic_cs16le.bin'
    iq.write_bytes(data)
    out = ART / 'events.ndjson'
    p = run([str(exe),str(iq),str(window),'10000',str(out)])
    must(p.returncode == 0, 'reads all complete CS16 IQ windows')
    rows = [json.loads(s) for s in out.read_text().splitlines()]
    must([x['first_sample_index'] for x in rows] == [192,256], 'exact burst window positions')
    must(all(x['classification']['status']=='unknown' and x['classification']['confidence'] is None for x in rows), 'never guesses modulation')
    must(all(x['measured']['peak_dft_bin']==16 for x in rows), 'DFT peak at known bin')
    must(all(x['sample_index_domain']=='file_relative_not_device_time' for x in rows), 'no fabricated device timestamp')
    bad = ART / 'truncated_cs16le.bin'
    bad.write_bytes(bytes(data)+b'\x00')
    p = run([str(exe),str(bad),str(window),'10000',str(ART/'truncated_events.ndjson')])
    must(p.returncode == 4 and 'truncated' in p.stderr, 'rejects truncated IQ, no silent trailing drop')
    p = run([str(exe),str(iq),'1','10000',str(out)])
    must(p.returncode == 2, 'rejects invalid window')
    p = run([str(exe),str(iq),str(window),'10000',str(iq)])
    must(p.returncode == 2 and len(iq.read_bytes()) == len(data), 'rejects output aliasing input file')
    print('PASS: 8 assertions; 2 burst windows; 0 inferred modulation labels')
except Exception as exc:
    lines.append(f'FAIL exception: {exc!r}')
    print(f'FAIL: {exc}', file=sys.stderr)
    raise
finally:
    LOG.write_text('\n\n'.join(lines)+'\n', encoding='utf-8')
    print('log:', LOG)
