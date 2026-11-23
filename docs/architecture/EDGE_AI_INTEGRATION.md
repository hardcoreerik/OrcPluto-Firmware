# OrcPluto Edge AI — Integrated Engineering Contract

**Director review:** 2026-11-21. Detailed original [Agent 2 reports](../../research/agent2/README.md), [Agent 1 hardware evidence](https://github.com/hardcoreerik/REA-Results/tree/research/orcpluto-fishball-hardware-20261121) and [OrcStream protocol truth](https://github.com/hardcoreerik/OrcStream/blob/main/PROJECT_TRUTH.md) take precedence over assumptions in this plan.

## Architecture

```text
AD9361 RX0/RX1 [shared RX LO]
  -> verified axi_ad9361 -> symmetric FIR/cpack -> original AXI DMA -> IIO/iiod -> standard clients
           |                                                     |
           | [FUTURE FPGA TAP; not yet synthesized]              +-> optional authorized IQ fanout/OrcStream adapter
           +-> aligned sample counter / power / trigger
                 -> CDC + bounded feature FIFO -> ARM feature service -> optional ARM classifier
                                                   -> local experimental NDJSON
                                                   -> future OrcStream EVENT adapter (schema owned by OrcStream)
```

**Owner of the underlying acquisition:** one RF configuration/session controller. The selected initial CPU-only experiment may use *exclusive* test-capture ownership. A second IIO reader is forbidden unless a real negotiated fanout implementation proves that the DMA and iiod can coexist. If IIO and IQ event subscribers are sharing samples, ownership, ref-count/release and backpressure semantics require explicit stress tests.

**Overload:** original IQ and IIO service win; if features/AI cannot keep up, drop feature records with monotonically counted `feature_drops`, `classification_skips` and `event_queue_drops`. Sample-source overruns, producer queue loss, transport loss and client delivery loss have **separate** counters and reset epochs. Never convert lossy data into apparent exact IQ.

## Internal event record — intentionally NOT an OrcStream wire standard

Use local NDJSON schema `orcp.internal.measurement.v0` for offline/early target experiments only. Minimum fields:

- `source_id`, `source_epoch`, `rx_channel`; source's native format/endian/valid converter bits and firmware+PL hash when known.
- `first_sample_index`, `sample_count`, `sample_timebase` (`file_relative_not_device_time`, `device_monotonic_unanchored` or verified clock with uncertainty); frequency/rate/gain/config epoch when known.
- `features` with literal units, DFT/window configuration, calibration validity and saturation/partial-window flags.
- `decision` as `unknown`, `morphology_candidate`, `modulation_candidate` or `protocol_validated` with basis; `confidence` only after calibration, otherwise null.
- `iq_loss` and `feature_loss` counters where observable (and `not_measured` rather than invented zero), `model_id` and `model_sha` if inference ran.
- Evidence window and IQ-excerpt retention only if the bytes really exist. Do not claim UTC from an FPGA count or classify named standards without independent sync/FEC/CRC evidence.

Do **not** reserve packet type, codec ID, JSON fields on the wire or auth semantics here. Agent 3/OrcStream owns it. A future translation layer will map these measurements into the **then-current, conformance-tested** OrcStream event schema. OrcSDR reads the public driver API; neither OrcSDR nor OrcStream common C99 code depends on OrcPluto.

## Resource/timing constraints (published on *another* public R1)

Agent 1 reports for the symmetric 2RX FIR build: 94/220 DSP48 and 12,521/53,200 LUTs, WNS +0.215 ns under the reported RX clock constraint. Those figures and their derived 126-DSP arithmetic remainder are **UPSTREAM_DOCUMENTED**, not available placed/routed budget on this hardware. The optional 96-DSP WBFM channelizer came from an older 72-DSP baseline and is **not** an additive, simultaneously functioning RX2 design. No new FPGA classifier feasibility claim before exact checkout, utilization/timing/CDC/BRAM and hardware tests.

At 2.4 MS/s CS16 1RX = 9.6 MB/s; 5 MS/s = 20 MB/s; 10 MS/s = 40 MB/s. These are mathematical payloads. Existing ESP32-P4 USB tests and public R1 reports are different setups. Maximum ADC rate 61.44 MS/s does not imply continuous USB/1GbE raw original IQ. Performance marketing requires explicit codec fidelity, exact sample-rate evidence and end-to-end measurements.

## Device-read-only inventory (run on the already verified radio terminal)

**Prerequisites:** verify exact physical board, known authorized SSH/session, no active disruptive work, both potential TX ports safely terminated per its verified PA population; no login secrets or unique serials in public logs. These commands gather information only and write one log to the working directory. They do not flash or retune.

```sh
LOG="$PWD/orcpluto-inventory-$(date -u +%Y%m%dT%H%M%SZ).log"
{
  date -u
  uname -a
  cat /etc/os-release 2>/dev/null || true
  cat /proc/cpuinfo
  head -n 14 /proc/meminfo
  cat /proc/device-tree/model 2>/dev/null || true
  cat /proc/mtd 2>/dev/null || true
  cat /proc/partitions
  cat /proc/cmdline
  mount
  command -v lsblk && lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINTS || true
  command -v iio_info && iio_info || true
  command -v ethtool && ethtool eth0 || true
  command -v fw_printenv && fw_printenv || true
  test ! -f /opt/VERSIONS || cat /opt/VERSIONS
  for f in /boot/BOOT.bin /boot/uImage /boot/devicetree.dtb /boot/uEnv.txt; do
    test ! -f "$f" || sha256sum "$f"
  done
} 2>&1 | tee "$LOG"
printf 'Inventory log: %s\n' "$LOG"
```

**Preservation and rollback:** Before ANY card write, power down, remove original SD, take and SHA256 a whole-card image via trusted host read-only imaging, record boot switch and exact partition/device identity, and verify that a *separate* replacement SD can boot a known-good image. Do not modify QSPI, use generic Pluto DFU, install startup actions, change FPGA or enable TX without separate board-specific authorization. An SD card is not inherently a guarantee of rescue from an incompatible FSBL. Preserve logs and hashes outside the original SD.

## Local test/handoff

Off-target: run `python3 experiments/edge-detector/run_tests.py` from the repo root (or `python` on Windows with GCC/Clang C++17 available). The runner saves `edge_detector_test.log` **in the invoking directory** and fixtures under `edge_detector_test_artifacts/`. Only deterministic synthetic signal/window correctness is tested, not physical signal-classification or streaming performance. See the experiment README for invocation, sample output and cleanup.

Next on target: verify source scan format and native rate via non-writing IIO query, arrange **exclusive test capture** (no competing reader), collect a short AD9361 CS16 capture with provenance, replay through detector using verified correct little-endian lanes, measure known-signal and no-signal decisions against independent labels. If capture ownership, RF safety or board identification is unclear, **stop and report BLOCKED**.

The first prototype does **not** require OrcStream changes. No OrcSDR, OrcStream or `esp-pluto-sdr` production files were altered here.
