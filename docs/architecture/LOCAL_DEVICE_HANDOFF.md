# Local OrcPluto hardware continuation — read-only first

**Status:** plans and safe read-only commands, **not executed on owner's board**. P0 actual unit identity/recovery is the blocking dependency. Reference: [Agent 1 safe validation](https://github.com/hardcoreerik/REA-Results/blob/research/orcpluto-fishball-hardware-20261121/SAFE_HARDWARE_VALIDATION_PLAN.md).

## Safety and ownership

1. Verify the physical R1/PA revision, SMA port identity and power-off termination of **both TX ports** before power-on. The matching public board reportedly emits a brief calibration signal at boot; software mute alone is not a substitute for physical loads. Initial tests are receive-only.
2. Identify exact serial/network host **on the actual bench** rather than assuming `192.168.2.1`, `fishball.local` or any COM port is stable.
3. Do not enable transmit, run calibration/reset commands, reinitialize AD9361, change clock or antenna routing, manipulate GPIO, rewrite QSPI, use generic Pluto DFU or install a new boot image under this handoff.
4. Don't compete with existing `iiod` acquisition. The first live CS16 capture must use an explicitly exclusive session or a proven fanout, not an unverified second reader.

## Operator prerequisite software

- On Windows: Python 3 for the offline test, GCC/Clang C++17 (e.g. a verified MSYS2 MinGW-w64 install), existing libiio/iio-utils path and a trusted read-only SD-card imaging utility. No credentials scripted.
- On Linux host/WSL with devices explicitly attached: libiio utilities `iio_attr`, `iio_info`, `iio_readdev` if present; `lsusb`, `ethtool`, `dtc`, `sha256sum`, Git, C++17 compiler.
- On the candidate device: authorized shell session with read-only access; no assumption `root` is always available or safe. Source trees/version pin known from Agent 1, not guessed remote images.

Record `python --version`, compiler `--version`, `iio_info -V` if available and exact tool locations in the local log. A missing tool is a recorded `BLOCKED`, not an implicit pass.

## Step 1 — run off-target correctness proof (no radio connected)

Open a terminal in your OrcPluto-Firmware checkout root and run:

```sh
python experiments/edge-detector/run_tests.py
# Or, on Windows PowerShell:
py -3 experiments/edge-detector/run_tests.py
```

**Expected:** `PASS: 8 assertions; 2 burst windows; 0 inferred modulation labels`. The command saves `edge_detector_test.log` in your current directory and fixture binaries/events in `edge_detector_test_artifacts/`. Failure: retain *full* `edge_detector_test.log`, compiler version and generated fixtures; do not report a pass. Clean undo: remove only generated log and artifacts folder after archiving them. No installation or device action occurs.

## Step 2 — collect actual unit inventory without modifying the radio

On the verified authorized **radio shell**, first choose a working directory with enough storage; copy and run the exact read-only inventory command in [EDGE_AI_INTEGRATION.md](EDGE_AI_INTEGRATION.md#device-read-only-inventory-run-on-the-already-verified-radio-terminal). It writes `orcpluto-inventory-YYYYMMDDTHHMMSSZ.log` in **that command's current directory**. Copy the log to the PC; redact unique serials, MAC addresses and credentials before any public upload.

On host, record USB and IIO discovery:

```sh
iio_attr -S
iio_info -h
iio_readdev -h
# Linux host only, if utilities exist:
lsusb -t
```

**Stop if** device identity doesn't match R1, TX ports aren't physically understood/terminated, key boot hashes are unavailable, or the live IIO scan/channel layout is unknown. No command above sets frequency/gain/TX.

## Step 3 — preserve images and establish recovery BEFORE writes

Power down radio; remove the original SD card. Use a trusted host **read device -> image file** path; double-check source physical disk. On Linux, after manually identifying the removed card's actual `/dev/sdX`, never use it as an output:

```sh
lsblk -o NAME,SIZE,MODEL,TYPE,MOUNTPOINTS
# Example only; replace /dev/sdX only after physically identifying the original card:
sudo dd if=/dev/sdX of=./orcpluto-original-sd.img bs=4M iflag=fullblock status=progress conv=noerror,sync
sha256sum ./orcpluto-original-sd.img > ./orcpluto-original-sd.img.sha256
```

Logs and hashes belong in the working folder **on the host**, never on the card. Capture exact boot mode, partition geometry, DT and bitstream hashes. Clone the image onto a *separate* SD after explicit operator authorization and demonstrate rollback without changing original media. If SD boot mode/FSBL compatibility is unclear, report `BLOCKED`; do not touch QSPI.

## Step 4 — prepare an exclusive, receive-only real-capture trial (not executed)

1. Confirm `iio_info`-reported devices, enabled channels, signed/scale/bits/endianness and actual sample rate. Check `iio_readdev -h` for the **installed CLI version**; do not rely on invented flags.
2. Ensure no simultaneous stream or second independent IIO capture owner; a deliberate local test must record whether `iiod` was paused or hardware capture mediated through its own service. Do not stop the existing OrcSDR session without operator agreement.
3. With approved receive-only RF stimulus and validated antenna/input protection, capture finite I/Q with its scan information, lo/rate/gain/filter, source epoch, count, timestamps and SHA256. Save as separate raw file plus sidecar manifest; do **not** claim conversion to CS16LE until alignment and lane order are verified. Commands for acquisition remain `BLOCKED` until the real tool/device API and ownership path are confirmed.
4. Convert verified native IQ explicitly to CS16LE when necessary, then run `edge_detect <file> 64 10000 <output.ndjson>` only as an illustrative threshold/detector; physical sensitivity cannot be inferred from the synthetic threshold. Press Ctrl-C to stop any optional local reader **only after** it has a specified cleanup/close path. Keep stdout/stderr and error metadata.
5. Compare detections with independently labeled signal and unknown capture regions; record missing windows, false alarms, CPU/PSS, p50/p95/p99 full pipeline, ADC and producer losses, USB/network path, power/temperature if actually measured.

## Status for this handoff

- Off-target correctness: reproducible script, source and actual local run recorded.
- Exact board inventory, backup, spare-SD boot, target model runtime, IIO live capture, FPGA timing, stream coexistence and 24h soak: **NOT EXECUTED in director sandbox**.
- No OrcStream or OrcSDR files changed. No physical device accessed.

Attach logs, board/firmware/FPGA hashes, fixture SHA256 and a command history when returning results. Store raw binary IQ only in an authorized workspace; do not publish potentially sensitive capture data automatically.
