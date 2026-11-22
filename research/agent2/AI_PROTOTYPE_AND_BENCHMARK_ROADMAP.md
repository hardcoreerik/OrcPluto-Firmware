# OrcPluto AI Prototype and Benchmark Roadmap

**Agent 2 | 2026-11-21 | Engineering test plan; all OrcPluto local execution remains pending**

## Recommended simplest successful demonstration

**Proof goal:** receive a real known narrowband signal with the Fishball AD9361, identify a reproducible active spectral region or burst on the dual Cortex-A9 **on-device**, emit a timestamped/relative-indexed local event, and prove baseline native IQ acquisition was not silently interrupted. Use **CPU-only energy detection or a hand-coded tree first**; only add a learned classifier after the data/measurement contract works. A model on synthetic RadioML alone is not proof of an intelligent radio.

A minimal receive-only demonstration can avoid FPGA changes and does not require USB protocol changes. If the Linux IIO device cannot be safely shared with normal `iiod`, **stop the iiod capture session during initial controlled measurement** and state that the baseline is nonconcurrent; do *not* claim full-streaming coexistence until separately proven. A later passive PL feature tap is the intended solution for concurrency. [F1][F2]

**Best initial known signal:** a locally controlled **receive-only** RF source or an existing permitted broadcast/beacon at a known frequency. With an attenuated cabled test source, observe single-tone, simple AM/FM or controlled FSK bursts from an external generator. Never connect Fishball TX directly to Fishball RX: its upstream project warns of potentially damaging RF levels and requires proper attenuation. If no generator is available, use available over-the-air non-sensitive signals with verified legal/operational context, and label ground truth as uncertain until validated. [F3]

## Stage gates (do not skip)

| Gate | Objective | Artifact | Pass condition |
|---:|---|---|---|
| **G0** | Identify exact hardware and immutable baseline | Board inventory, DT/boot/bitstream hashes, tested SD backup, FPGA util/timing baseline | Hardware `hw_model` confirms expected Rev.A / Z7020+AD9361; known-good recovery path exists |
| **G1** | Reproduce upstream firmware/receiver behavior | `./devkit selftest`, `iio_attr`/`iio_info`, fixed-rate receive logs | IIO native format, receiver behavior, safety state and delivered rates documented |
| **G2** | CPU-only on-device detector | C/C++ detector CLI+tests, buffered IQ records, JSONL events | Detect known burst with window indices and compare against offline reference; explicit unknown/no-event case |
| **G3** | ML/heuristic comparison | ncnn/LiteRT/tree cross-builds and 3 representative model variants | Each runnable armhf and quantified accuracy/latency/PSS without exceeding acquisition CPU budget |
| **G4** | Prove concurrent stock-IIO + AI model | iiod and detector stress matrix, loss counters | No new IQ loss over baseline at validated rates; no silent contention |
| **G5** | Passive FPGA feature branch | HDL + simulation + Vivado/timing deltas + ARM driver | RTL test bit-exact; original IIO IQ and both RX unchanged; no clock path violation |
| **G6** | Triggered capture and standardized metadata | SigMF clips, event provenance, unknown policy | Sample windows correct, loss/overflow/clip expiration truthful |
| **G7** | Optional tiny HLS/FINN model | Model source+IP report+hardware acceptance | Full placed design timing met; actual speed/efficiency benefit exceeds cost |
| **G8** | Fleet-ready appliance hardening | reproducible distro, security/update/rollback tests and pilot logs | Boots safely, works with independent IIO clients, recovers on corruption and reports firmware identity |

No gate has been experimentally passed in this assignment; published third-party Fishball tests are a reference, not the user's own board results.

## Short-, medium-, and long-term milestones

### Short term — establish proof, no new FPGA bitstream

- Audit the exact board using `iio_attr -S` and `/opt/VERSIONS`; clone/pin the upstream Fishball devkit and make a byte-for-byte SD backup before changes. Determine if memory/CPU/receiver/USB details match expected Rev.A.
- Choose Debian 13 armhf, Linux 6.12 ADI image if board match is established; keep original SD as rollback. Maintain baseline hashes, network and iiod identity.
- Confirm receiver scanning, tune/readback, `iio_info`, `./devkit selftest` and single RX real IQ capture; log delivered samples per second against requested rate.
- Implement a small offline FFT energy detector with deterministic input fixtures on host, then cross-compile C/C++ (`-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard`) and run it on device.
- Demonstrate one known burst with event's source window, estimated bandwidth, relative power and noise floor (with explicit measurement method). The output should **not** be labelled DMR/LoRa/etc without protocol validation.
- Compare a deterministic tree, ncnn and LiteRT build for a compact **identical** feature model; pin exact runtime/toolchain versions and record cross-compile failures, if any.

### Medium term — continuous operation and FPGA feature export

- Develop ownership-safe feature fanout, per-path overrun accounting, bounded worker threads and failure isolation; verify stock IIO and detector concurrent operation at realistic sample rates.
- Create SigMF dataset pipeline, RF impairment test sets, unknown/reject calibration, held-out device/time/location splits and controlled labelled signals.
- Build a passive FPGA mirrored branch for power/FFT/burst/valid sample counting. Validate HDL simulation, dual RX and original IIO bitstream fidelity before testing a new image on actual hardware.
- Publish versioned feature-event semantics to OrcStream's protocol research team; integrate an **experimental** OrcSDR dashboard adapter without declaring an official OrcStream event message.
- Begin production Buildroot proof only after Debian behavior is known; include signed release manifest and recovery plan.

### Long term — specialized hardware and commercial appliance

- Benchmark FINN/hls4ml quantized accelerator versus ARM feature models on identical capture corpus and stress load. Do not deploy just because synthesis passes; require compelling p95/p99/energy/CPU advantage with acceptable FPGA margin.
- Add triggered IQ capture, optional host classification offload and capability-aware resource requests; evaluate encrypted, authenticated management and OTA/SD image update lifecycle.
- Generalize the feature producer and decoder adapters beyond the Fishball to other SDR hardware, keeping OrcStream protocol/codec implementation source-neutral.
- Iterate model versions via documented dataset collection, reliability and safety tests; publish reproducible product claims with firmware, FPGA, model and host identity.

## Before touching the radio: read-only inventory commands

This snippet is intended for a **terminal on the running board**, and saves a log in the directory from which the command is executed; it does not flash or transmit. Paths and utilities are checked at runtime. Run as an appropriate user after verifying the intended SSH host. Do not paste private serial numbers into public issue reports without redaction.

```bash
OUT="$PWD/orcpluto-inventory-$(date -u +%Y%m%dT%H%M%SZ).log"
{
  echo '### UTC and kernel'; date -u; uname -a
  echo '### OS'; cat /etc/os-release 2>/dev/null || true
  echo '### CPU'; cat /proc/cpuinfo
  echo '### RAM'; cat /proc/meminfo | head -n 12
  echo '### IIO'; command -v iio_info && iio_info || true
  echo '### Version manifest'; test -f /opt/VERSIONS && cat /opt/VERSIONS || true
  echo '### Root and mounts'; findmnt -T /; lsblk -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINTS 2>/dev/null || true
  echo '### Network'; ip -brief address 2>/dev/null || true
  echo '### Boot files (if present)'; for f in /boot/BOOT.bin /boot/uImage /boot/devicetree.dtb /boot/uEnv.txt; do test ! -f "$f" || sha256sum "$f"; done
  echo '### Available pressure sensors'; for f in /proc/pressure/{cpu,memory,io}; do test ! -r "$f" || { echo "$f"; cat "$f"; }; done
} 2>&1 | tee "$OUT"
printf 'Saved inventory log: %s\n' "$OUT"
```

**On an operator's Linux development host** (in a pinned clone of the Fishball devkit, **not** the OrcPluto repository):

```bash
# Verify upstream's documented helper commands; these may require a working board
./devkit doctor
./devkit net find
./devkit selftest --ssh  # receive/test interactions; review upstream instructions
```

The upstream project documents `./devkit setup` and `./devkit build`, but those are not required for read-only inventory. **Do not run `./devkit flash`, DFU, write-card, or TX self-tests** without explicit hardware authorization, a recovery image and any required attenuation. Fishball explicitly warns that DFU is unsafe on this board. [F3]

## CPU-only POC benchmark procedure

1. **Offline fixture:** create labelled CS16 IQ files for 1) silence/noise; 2) strong CW; 3) amplitude-varying signal; 4) frequency-varying FSK-like signal. Record generation parameters and deterministic seed. Compute reference FFT/PSD and known time windows on host. Store golden events as JSONL; compare sample-index and energy calculations, not just human visual output.
2. **Receiver capture:** use the matching board's IIO documentation/tool to capture native I/Q at a measured rate, without conflicting with another active capture owner. Record configured rate, native scan layout, gain, center, source epoch, complete sample count, overrun counters and byte count. `iiod` may need to be quiescent during direct buffer tests; document the state. **Don't assume fixed scan channels without querying them.**
3. **On-device detector:** process 8192 complex samples/window as the first trial, calculate magnitude power and short FFT, detect occupancy above a measured threshold with hysteresis. Convert output to typed JSONL local events. A 8192-sample window at 2.4 MS/s spans `8192/2,400,000 = 3.413... ms` of IQ, before CPU analysis/queueing. FFT processing cost and event latency are **unmeasured**.
4. **Validation:** compare offline Python or C host and ARM output for same IQ file; differences from fixed-point saturation/quantization are bounded and explained. Test zero IQ, alternating full-scale, disabled signal, contiguous burst, edge-crossing burst and interference.
5. **Streaming coexistence:** compare baseline single-user IIO delivery with model off/on at same configuration, same duration, buffer count and network/USB path; report delivered/requested samples, CPU%, PSS and drops. If capture cannot be shared, label `NOT_YET_SUPPORTED` rather than inventing success.
6. **Classifier swap:** run calibrated tree vs ncnn vs LiteRT against identical feature fixtures and weight structure; report F1/unknown false accept and p50/p95/p99. Repeat with A9 frequency, thermal and memory load recorded.

## Suggested acceptance targets (engineering targets, NOT observed results)

| Metric | Initial target | How to test | Action on failure |
|---|---|---|---|
| IQ continuity | **Zero incremental unexplained** IQ sample gaps compared with tested baseline rate | delivered/nominal samples and source DMA counters over 30 min, 2 h and 24 h | downgrade to detector-only or redesign fanout |
| Detector truth | known signal region and burst boundaries match reference within 1 configured FFT/window interval | golden IQ fixture and controlled RF test | inspect windowing/threshold/rate metadata |
| False alarms | published false positives/hour at fixed threshold for distinct no-signal runs | at least 1 h representative noise/adjacent-channel runs | recalibrate, add unknown or tighter detector |
| Latency | report p50/p95/p99 **including window + queue**, not a target until baseline measured | monotonic clocks, histograms across sustained stress | reduce duty/window/queue; PL prefilter |
| CPU headroom | choose a product-specific guardrail after idle and heavy IIO baseline | `pidstat`, `/proc/stat`, p95 CPU by core, watchdog | time-budget classifier and cap concurrency |
| FPGA timing | **all** constrained setup/hold paths meet timing | Vivado full placed design, STA and multiple build reports | re-pipeline/CDC/fewer features |
| Device integrity | source identity, DT and bitstream revisions match intended image | SHA256 manifest, RF/IIO selftest | refuse run or recovery |
| Unknown quality | measured false acceptance on **held-out** unseen signals | OOD set, per-SNR confusion/calibration | reject class hypotheses more aggressively |
| Restart | model failure/reload does not break IIO or enable TX | fault injection, kill -9 model process, monitored safe states | decouple processes and permissions |
| Product power/thermal | no untracked throttling or shutdown over 24h | temp/power logging, compare baseline and feature load | reduce model duty, cool or hardware redesign |

Avoid arbitrary claims like 99% classification accuracy or 100 MS/s: actual signal family/SNR and sample rate conditions must accompany any published number.

## Benchmark log file schema

Store all measurement artifacts under a uniquely named run directory in the **folder where the benchmark is launched**, plus `run.json`, `stdout.log`, `stderr.log`, `metrics.csv` and optionally SigMF snippets. A successful run should be reproducible from:

```text
run_id, date_utc, test_operator, board_model, board_rev, serial_redacted,
boot_sha, kernel_commit, bitstream_sha, dt_sha, rootfs_image_sha,
libiio_version, runtime_commit, model_sha, dataset_manifest_sha,
compiler_version, build_flags, rx_channels, requested_rate_sps,
measured_rate_sps, native_scan_format, gain, rf_bandwidth, lo_hz,
input_window_samples, fft_window, fft_overlap, cpu_affinity,
feature_drops, iio_dma_drops, network_drops, delivered_samples,
inference_count, classification_skips, latency_p50_ms,
latency_p95_ms, latency_p99_ms, cpu_core0_pct, cpu_core1_pct,
peak_pss_kib, thermal_c, power_w_if_measured, test_duration_s,
false_alarms_per_hour, recall_by_snr, notes, pass_fail
```

For FPGA runs, add `vivado_version`, `hls_version`, `WNS_ns`, `WHS_ns`, `LUT`, `FF`, `DSP48`, `BRAM18`, `BRAM36`, added AXI ports, clock-frequency assumptions, timing constraints version and full utilization/timing reports. **Missing metrics should be `not_measured`, not zero.**

## Experiment matrix

- **Rates:** 0.96, 2.4, 5 and higher if supported by measured link and firmware; control channels/format. Upstream reports 5 MS/s single RX over USB as near complete, so do not treat 10 MS/s USB as a validated stress point. [F4]
- **Paths:** offline IQ replay, direct on-device IIO, stock iiod to PC, iiod plus classifier (only if supported), FPGA side feature extraction; both RX channels if verified.
- **Sources:** CW/AM/FM/2FSK/4FSK/OFDM candidate and unknown, plus noise and two simultaneous signals.
- **Perturbations:** SNR, adjacent blockers, AGC/gain, CFO, sample-rate offset, multipath, burst length and duration.
- **Conditions:** cold boot, steady state, CPU load, network load, classifier crash/reload, feature FIFO saturation, prolonged temperature rise.

Cross-check source CPU throughput against the project's existing `esp-pluto-sdr` host-driver observations, but never assume those ESP32-P4/USB rates transfer unchanged to the A9 on-board classifier. [E1]

## Handoff to the local hardware / RSA-REA agent

Physical validation may require tools or hardware not present in this research sandbox. For any failed or inaccessible step, produce a **transfer manifest**, not an implicit claim of completion:

1. Toolchain versions and source SHAs (`git rev-parse HEAD` for exact repositories), cross-compiler/linker and sysroot hashes, model binaries SHA256, expected CPU ABI and dependent `.so` list.
2. Exact shell commands required on the local PC/board, board image and SD rollback prerequisites, privileged operations called out; default to RX-only.
3. Input fixture files and golden output hashes, expected vs observed with clear **NOT RUN** rows.
4. `logs/` includes complete compiler logs, `readelf`, selftest, `dmesg` excerpts, `metrics.csv`, JSONL events and reproduction metadata; no private serials or credentials in public artifacts.
5. Link relevant RSA-Sandbox and REA-Results artifacts **once paths and their repository rules are verified**; don't assume location/branch access. Agent 1 hardware results must be imported before final pin assignments/constraints.

## Release decision rubric

**Ship first:** reliable presence/burst/occupancy metrics and event-driven IQ capture with unknown handling. **Ship later:** modulation-family hypotheses with calibrated confidence and verified negative set. **Ship last:** protocol candidates promoted by real decoders, advanced PL model inference and generic remote compute offload. All features must be opt-in when they consume meaningful stream resources.

## Sources

- [F1] Fishball modern root and iiod compatibility: https://matsvandamme.github.io/fishball7020-fpga-devkit/debian-rootfs/
- [F2] Fishball block design and IIO DMA: https://matsvandamme.github.io/fishball7020-fpga-devkit/block-design/
- [F3] Fishball firmware getting started, safety and no DFU: https://matsvandamme.github.io/fishball7020-fpga-devkit/
- [F4] Measured one-board Fishball throughput: https://matsvandamme.github.io/fishball7020-fpga-devkit/measured-performance/
- [E1] Existing portable Pluto host driver status: https://github.com/hardcoreerik/esp-pluto-sdr
- [N1] ncnn ARM hardfloat toolchain: https://github.com/Tencent/ncnn/blob/master/toolchains/arm-linux-gnueabihf.toolchain.cmake
- [T1] LiteRT ARMv7 build: https://developers.google.com/edge/litert/build/cmake_arm
- [O1] OrcStream hardware/research boundary: https://github.com/hardcoreerik/OrcStream/blob/main/PROJECT_TRUTH.md

## Final recommendations — Agent 2 disposition

1. **Best development Linux:** known Fishball-compatible **ADI Linux 6.12 + Debian 13 `armhf`**. It is documented on matching boards; confirm actual revision and baseline before any use.
2. **Best planned deployment Linux:** **minimal custom Buildroot appliance** carrying the *tested* ADI kernel and original IIO contract. This recommendation remains conditional; if its migration and update burden exceeds the benefit, retain a pinned/locked-down Debian image for the first product.
3. **Best ARM inference framework:** **ncnn** is the first compact neural-runtime *candidate* for A9/NEON C++, pending side-by-side model/latency comparisons with the documented ARMv7 **LiteRT Interpreter**. For the very first detector use **plain C/C++ classical DSP + tree or logistic classifier**, not a neural framework.
4. **Best FPGA acceleration:** passive, fully decoupled **FPGA DSP feature branch plus ARM classifier**, preserving existing `axi_ad9361`/IIO/iiod. A full FINN/hls4ml neural accelerator is contingent on measured CPU pressure, actual timing/resource availability, and real end-to-end superiority.
5. **Top three features:** (i) spectrum occupancy / burst detection; (ii) event-triggered SigMF IQ excerpts with exact provenance and loss; (iii) coarse modulation + protocol **candidates** with calibrated unknown handling and decoder validation before definitive identity.
6. **Minimal POC:** receive 2.4 MS/s single-channel AD9361 IQ under controlled conditions, run a portable FFT/energy/burst detector on the Cortex-A9, write indexed local events, compare to a host golden reference, and report IQ continuity/CPU/PSS. No FPGA edits required.
7. **Milestones:** short term: read-only hardware inventory, Debian baseline, C++ on-device detection and ncnn/LiteRT builds; medium term: independent feature path, real SigMF corpus, calibrated classifier and continuity tests; long term: portable OrcStream event adapter, triggered hardware capture, optional FPGA AI/host offload and hardened appliance image.
8. **Unsupported assumptions / open risks:** exact owner's board revision and image not inspected; no live ML runtime compiled or benchmarked; dual RX concurrent delivery and iiod fanout not independently measured; no FPGA free BRAM and timing headroom characterized beyond upstream reference; ARMv7 VFPv3 ABI needs explicit build verification; training data license and domain-shift concerns; partial reconfiguration/OTA rollback not designed; model confidence/unknown behavior uncalibrated; no real hardware boot-to-ready, thermal, p99 latency, CVE and sustained-IQ tests from this session. None should be silently upgraded to a release claim.