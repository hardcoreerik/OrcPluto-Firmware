# Offline detector execution evidence

**Evidence grade:** `REPRODUCED_OFF_TARGET`
**UTC run:** 2026-11-22 01:32
**Host:** Linux x86_64 kernel 6.18.57
**Compiler:** g++ (Debian 14.2.0-19) 14.2.0
**Repo source:** `experiments/edge-detector/detect.cpp` and `run_tests.py` on this integration branch (checked against uploaded Git blobs).

## Commands actually executed

From the working directory:

```sh
python experiments/edge-detector/run_tests.py
g++ -std=c++17 -O1 -g -fsanitize=undefined,address -fno-omit-frame-pointer -Wall -Wextra -Werror experiments/edge-detector/detect.cpp -o edge_detector_test_artifacts/edge_detect_asan
ASAN_OPTIONS=detect_leaks=1 ./edge_detector_test_artifacts/edge_detect_asan ./edge_detector_test_artifacts/synthetic_cs16le.bin 64 10000 ./edge_detector_test_artifacts/sanitizer_events.ndjson
```

## Observed stdout

```text
PASS: 11 assertions; 2 burst windows; 0 inferred modulation labels
log: /mnt/data/orcpluto-integration/edge_detector_test.log

windows=8 events=2 samples=512
runner exit:0
```

The pure-Python fixture generator uses deterministic PRNG seed `8675309`; CS16LE I/Q samples, 8 windows of 64 samples, tone windows 3 and 4, expected first sample indices 192 and 256. The tests check the explicit offline source epoch, expected DFT bin 16, explicit `unknown` class and null confidence, file-relative index domain, malformed/truncated data rejection and input/output alias refusal. Compilation with warnings-as-errors and ASan/UBSan returned zero on this host; sanitizer log showed no errors.

## Interpretation

- **PASS:** one non-target x86 Linux execution of the offline reference; eleven scripted assertions and one sanitizer replay.
- **NOT TESTED:** ARMv7 cross-build/runtime; live IIO or Pluto capture; simultaneous IIO+AI; real-world detector sensitivity, false-alarm rate, latency, sample continuity, FPGA synthesis, deployed firmware, OrcStream EVENT framing.
- **WARNING:** reference O(N²) DFT is not designed for real-time raw 2.4 MS/s. This is a correctness fixture, not throughput research.

Generated logs were saved in the **launch directory**, as required: `edge_detector_test.log` and `edge_detector_sanitizer.log`. Source-backed GitHub report records the commands; untracked local log contents were not represented as production CI artifacts.
