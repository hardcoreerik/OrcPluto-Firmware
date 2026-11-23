# ROADMAP — OrcPluto-Firmware

**Evidence-gated development plan | Consolidated 2026-11-22.** This is the intended sequence, **not a release checklist already passed**. Current truth is [PROJECT_TRUTH.md](PROJECT_TRUTH.md), current proof is [STATUS.md](STATUS.md), and technical allocation is [ARCHITECTURE.md](ARCHITECTURE.md).

## Shipping strategy

Create a **safe, compatible, reproducible RX appliance first**. Add trustworthy local detection before neural-network branding. Add FPGA offload, smart IQ, multi-consumer streaming and remote management only when each improvement passes a baseline-versus-change test on the owner's exact unit.

Priority order: **device identification / rollback → standard radio compatibility → original-IQ fidelity / source ownership → classical detector → sustained coexistence → justified FPGA/ML/OrcStream work → release hardening**.

## Release phases (all device gates pending)

| Phase | Owner / dependency | Required deliverables | Pass / fail gate |
|---|---|---|---|
| **P0 — Board truth & recovery** | Owner + hardware/REA agent; **first** | Exact R1 SKU/PCB/XC7Z020 speed grade, DDR, PHY, PA/TX port/clock/USB/SD/QSPI/boot switch inventory; live read-only device tree, IIO and image hashes; untouched original media; spare card/rescue drill | Exact identity and a rehearsed path to recover without unsafe writes; if missing, **STOP/BLOCKED** |
| **P1 — Known-good Linux/IIO baseline** | Firmware lead, after P0 | Pinned Fishball-appropriate FSBL/U-Boot/XSA/bitstream/DT, ADI-derived Linux 6.12/Debian 13 armhf trial image on **separate recoverable SD**; build manifest | Safe cold boot, RX-only/default TX proof, 1RX and supported 2RX format/channel verification, libiio/iiod 0.26 where compatible, Ethernet/USB discovery/reconnect, esp-pluto-sdr regression; original media never modified |
| **P2 — Native IQ fidelity & source ownership** | Firmware + hardware; P1 | One authoritative tune/acquire owner; scan-format proof, epoch/timebase metadata, separate losses, link and CPU/memory/thermal benchmarks | Byte/sample equivalence vs reference and recorded delivered-versus-requested rates (2.4, 5, 10 MS/s **when path supports them**); loss distinguishable; no hidden format conversion |
| **P3 — Truthful signal presence baseline** | Edge/DSP agent; offline fixture then P2 | Port/replace O(N²) reference with measured bounded FFT/power/burst detector on A9; known/unknown controlled captures, no-signal negatives, local versioned events | Host golden vectors + armhf ABI/device execution + independent known-signal/noise capture, false alarms/hour, measurement units, source-index spans, full window→event p50/p95/p99; unknown rather than fabricated class |
| **P4 — Bounded concurrent services** | Firmware acquisition owner + DSP; P2/P3 | Explicit fanout or independent proven passive PL tap, bounded IQ/feature/event queues, overload and crash supervision, telemetry | Active IIO/iiod and feature service coexist under stress with **no new unexplained source IQ loss**; feature skip/drop accounting and epoch resets; 24h soak before continuous claim |
| **P5 — FPGA passive feature engine** | FPGA/REA + DSP; P0/P2/P4 | No-op RX mirror → valid-sample counter/power/threshold → CDC/bounded FIFO, optional FFT/decimation only after proven value | Pinned RTL/Vivado/XSA, simulation/golden vectors, post-route WNS/WHS, CDC and BRAM/DSP/LUT delta, 1RX/2RX scan integrity and native IIO non-regression on owner board |
| **P6 — ML and portable event/stream adapters** | Edge AI + OrcStream protocol owner; P3/P4, P5 optional | Tiny tree/logistic baseline, armhf INT8 ncnn/LiteRT candidate A/B, grouped real-session model evaluation, internal event adapter and only **then** negotiated portable OrcStream integration | Device-executed ELF, calibrated rejection/OOD, latency/power/PSS/CPU, IIO continuity; conformance tests for independently approved OrcStream wire contracts and an unrelated client |
| **P7 — Product/release hardening** | Release/security + hardware; preceding device gates | Reproducible pinned images and hardware matrix, SBOM/license audit, defaults/authentication, signed artifact manifest, update/recovery/rollback, lifecycle diagnostics | Independent rebuild, recovery/power-cycle exercise, 24h stable RX+services, source/client compatibility, zero unexplained sample loss, safe TX/controls and public evidence manifest |

**No schedule is promised.** Phases are gated by results, not dates; P3 off-target experiments can progress while device P0/P1 is blocked, but cannot be advertised as on-device success.

## Immediate execution backlog

| Priority | Task | Expected output / definition of done |
|---|---|---|
| 1 | Run [read-only physical-board inventory](docs/architecture/LOCAL_DEVICE_HANDOFF.md) and compare with [REA Agent 1](https://github.com/hardcoreerik/REA-Results/tree/research/orcpluto-fishball-hardware-20261121) | Board/PA/boot/clock identification and redacted log in execution folder; no write |
| 2 | Preserve exact original SD and record recoverable separate-card procedure **before** any image experiment | Images/sha256 and operator-validated rollback, or `BLOCKED` |
| 3 | Reproduce [offline detector tests](experiments/edge-detector/README.md) in the working checkout | New `edge_detector_test.log`, fixture hashes and compiler version; previously reported 11 checks are not a substitute |
| 4 | Tighten detector input/error semantics before real capture | Canonical-path/inode output-alias defense, strict integer/threshold parsing, no ambiguous partial-result success, malformed/empty cases in tests; continue to reject incomplete windows |
| 5 | Establish portable native-scan conversion/golden IQ corpus | Explicit real source scan layout, little-endian validation, sample alignment, known antenna/RF stimulus and negative windows |
| 6 | Prototype adaptive-memory detector entirely off-target | Fixed threshold vs CFAR-inspired vs context-keyed robust baseline; grouped holdouts, controlled drift/gain/blocker changes, fixed false-alarm budget |
| 7 | Confirm current OrcStream wire and Agent 3 codec status independently before adapter work | Explicit version/feature negotiation and sample-exact compression vectors; no local wire packet IDs |
| 8 | Capture reproducible Vivado and Linux build closure when hardware/tools become available | Full source/toolchain hashes, original data path constraints, post-route report and safe image plan |

## Parallel R&D lanes — experiments, not shipped features

### A. Spectrum intelligence and evidence-rich UX

The [AI RF research index](research/ai-rf-spectrum-intelligence/README.md) motivates **context-keyed spectrum memory**, temporal signal fingerprints, receiver health explanations, and protocol-confirmed feedback. The priority experiment is **adaptive noise baseline with controlled forget/reset behavior**. Compare against static thresholds and CFAR-inspired methods under gain/LO/sample-rate changes, adjacent interferers and changing noise. Log detection probability, false alarms per hour, onset/bandwidth error and compute load. A learned baseline must never silently classify an unseen signal as known or erase a real narrowband transmission.

Future optional features: occupancy map and dwell trends; burst signatures useful for ranking, not identity; bounded, explicitly optional pre/post-event IQ; and a UI/host API showing which observation caused an alert. RF sensing, not active jamming or interference generation.

### B. Compression and bandwidth efficiency

The source sample count/format and exact 12-bit useful values matter more than flashy ratios. Work with [OrcStream](https://github.com/hardcoreerik/OrcStream) ownership for portable codec negotiation and independent vectors. Evaluate **lossless native-12 packing**, entropy/predictive schemes and **explicit opt-in approximate representations** separately. Benchmark across actual RF recordings by: exact roundtrip or quantified distortion, compression worst-case expansion, byte/s *delivered* across transport, CPU/FPGA cycles, copies, recovery from gaps, and independent host decoder. No claim that 61.44 MS/s native CS16 can traverse USB2 continuously.

### C. Heterogeneous edge computing

Baseline with C++ FFT/handcrafted features, then a small interpretable tree. Compare ncnn and LiteRT **only after** Cortex-A9 armhf/NEON ELF/run success. FPGA candidate order: no-op side tap → counters/power → optional filter/channelizer/FFT → experimental INT8 PL inference **only if** measured CPU bottleneck and timing/resource savings justify it. Legacy FINN or Vitis AI examples do not make a deployable Fishball NPU.

### D. Open clients and radio standards

Maintain **IIO/iiod first**. OrcStream is additive and source-neutral; OrcSDR/ESP32-P4/PC clients test the compatibility contract but are not exclusive consumers. Explore capture export via SigMF, receiver diagnostics and independent event consumers. Do not invent EVENT, JOB or compression packet semantics in this repository.

### E. Appliance productization and scale

Reproducible Linux/HDL builds and exact hardware profiles, user-facing setup/health pages, secure remote administration, provisioning, release manifests, signed update and rollback, multi-board support gates, SBOM/license and dataset audit, long-running observability. **Buildroot vs hardened Debian remains an evidence-driven decision**, not an ideological requirement.

## Research-to-decision rules

Every proposal must enter through the [review matrix](docs/architecture/INTEGRATION_MATRIX.md) or a new ADR. It must state the baseline, source artifact, on-target/off-target grade, changed resource/path, failure modes, independent metric, stop condition, owner and integration boundary.

Research and previous session prompts in `research/agent2/` are historical records. The [full Markdown review](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md) documents which files were reconciled, what was preserved and which speculative claims remain blocked.

## First runnable experiment (no hardware required)

```sh
python3 experiments/edge-detector/run_tests.py
# Windows: py -3 experiments/edge-detector/run_tests.py
```

Requires Python 3, GCC/Clang C++17. Saves `edge_detector_test.log` plus `edge_detector_test_artifacts/` **in the launch directory**, not in an inaccessible temporary path. Expected historical reference: `PASS: 11 assertions; 2 burst windows; 0 inferred modulation labels`; check the actual log before claiming a new PASS. Runbook, failure modes and cleanup: [experiment README](experiments/edge-detector/README.md).

On-device continuation is **blocked** until exact-board inventory, TX physical safety and original-image recovery are established. Do not interpret this roadmap as permission to flash or transmit.
