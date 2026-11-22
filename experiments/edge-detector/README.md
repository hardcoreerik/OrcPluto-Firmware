# Off-target CS16 energy + DFT reference detector

**Evidence label:** `REPRODUCED_OFF_TARGET` (2026-11-21 Linux x86-64). This is not firmware, native IIO, a production FFT, an ML classifier or an OrcStream EVENT wire format.

## What it does

Reads interleaved *little-endian signed 16-bit I then Q* from a finite file. Windows are complete, non-overlapping and fixed length. Calculates mean `I*I+Q*Q` in **ADC container counts squared**. For windows exceeding a configured threshold, calculates a deliberately simple O(N²) complex DFT for peak-bin evidence and writes local `orcp.internal.measurement.v0` NDJSON. Reported `unknown` is an honest absence of modulation classification, and `file_relative_not_device_time` prevents false UTC/FPGA claims.

There is no calibrated dBm, receiver frequency, real sample counter, timing accuracy or sample-gap detection in this standalone replay. Never use this CPU-intensive DFT as proof that real-time 2.4 MS/s acquisition is feasible.

## Run/test (Windows, Linux or macOS)

Prerequisites: Python 3, a C++17 compiler `g++` or `clang++` in PATH (on Windows, use a suitable MinGW-w64/MSYS2 compiler and matching Python; Python's subprocess must be able to run compiled executables).

From the **folder in which you want the log saved** (for example the checkout root):

```sh
python3 experiments/edge-detector/run_tests.py
# Windows PowerShell: py -3 experiments/edge-detector/run_tests.py
```

The test runner builds under `./edge_detector_test_artifacts/`, generates a reproducible fixture (seed `8675309`), runs positive and negative tests and saves **`./edge_detector_test.log` in the exact working directory you launched from**. The log captures compiler command/output and each test's stdout/stderr; generated IQ/events are also under `./edge_detector_test_artifacts/`.

Actual off-target run (2026-11-21, g++ C++17, x86 Linux):

```text
PASS: 7 assertions; 2 burst windows; 0 inferred modulation labels
log: .../edge_detector_test.log
```

The runner asserts sample windows `192` and `256` (64 samples each), expected DFT bin `16`, an explicit `unknown` decision, no invented device clock, rejection of a truncated IQ file (exit 4) and rejection of invalid window size (exit 2). No hardware was connected; no radio settings were changed.

### Manual replay after building

```sh
./edge_detector_test_artifacts/edge_detect ./edge_detector_test_artifacts/synthetic_cs16le.bin 64 10000 ./edge_detector_test_artifacts/manual_events.ndjson
```

On Windows substitute `edge_detect.exe`. The CLI expects `input window_samples power_threshold_counts2 output_ndjson`; its values have **no RF calibration or assurance of detection sensitivity**.

### Cleanup / rollback

To remove **only generated artifacts** after preserving evidence, delete `edge_detector_test.log` and the `edge_detector_test_artifacts` folder from the same working directory. Source files are unchanged. The detector doesn't modify any device, IIO configuration, FPGA or firmware.

## What target proof still needs

1. Verify actual AD9361 IIO scan layout/endianness, rate and signed data. Convert captures explicitly (never assume they match this file format).
2. Establish *exclusive* test-source ownership before reading IIO; do not add a second uncoordinated DMA reader alongside iiod.
3. Run on the actual Cortex-A9 after an ARMv7hf cross-compile; compare sample-index evidence against independently labeled RF and unknown captures.
4. Measure per-window processing, end-to-end delay, per-core load, PSS, thermal, source IQ losses, feature skip counts and false alarms by signal condition.
5. Replace reference DFT with validated bounded FFT/feature implementation; only after that evaluate continuous streaming and future FPGA assist.

See [integrated roadmap](../../ROADMAP.md) and [architecture](../../docs/architecture/EDGE_AI_INTEGRATION.md).
