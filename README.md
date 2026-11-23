# OrcPluto-Firmware

**Open, board-qualified Linux + FPGA firmware for Pluto-class software-defined radios — with trustworthy IQ, optional intelligent spectrum processing, and an interoperable streaming future.**

**Maintained for Oregon Radio Company.** OrcPluto-Firmware is intended to turn supported Zynq/AD9361 radio hardware into a reliable, developer-friendly, network-capable SDR appliance, without surrendering standard Pluto/IIO compatibility or pretending research prototypes are finished products.

> **Current project state (2026-11-22):** **research and host-side prototype**, not a released/validated OrcPluto firmware image. The initial hardware candidate is the **Fishball/PlutoSky R1, XC7Z020 + physical AD9361**. Its exact owner-unit PCB/PA/boot/clock configuration and recovery procedure still require direct verification. The offline CS16LE detector has previously recorded host tests; no OrcPluto on-device AI, FPGA feature engine, native OrcStream producer, or production image is verified. Read [PROJECT_TRUTH.md](PROJECT_TRUTH.md) and [STATUS.md](STATUS.md) before claiming functionality.

## Why we are building it

A Pluto-compatible SDR should be more than a remote IQ faucet. The long-term vision is an **open, repeatable radio-computing platform** that can:

- Receive and expose **unaltered native IQ** with declared sample format, rate, channels, tuning, provenance, timing uncertainty, and loss.
- Continue working with **ordinary AD9361/IIO/iiod and libiio clients**. Existing tools must not be sacrificed for new features.
- Serve **multiple authorized consumers** through a deliberately designed single-owner capture/fanout service, rather than competing uncoordinated DMA readers.
- Offer **opt-in OrcStream** IQ and, when OrcStream defines stable negotiated semantics, compressed/derived streams, spectrum, events, and bounded processing requests.
- Discover activity via efficient **spectrum occupancy, burst detection, noise-floor tracking, and useful signal measurements**. Add modulation-family hypotheses and *validated* protocol identification only when evidence supports them.
- Use the **Zynq FPGA for deterministically timed DSP and feature extraction**, the ARM Cortex-A9 for control and small classifiers, and a desktop/cloud system for training or heavyweight analysis when useful.
- Support **event-triggered IQ clips**, traceable SigMF-style capture metadata, and reproducible signal datasets, subject to actual buffer limits and permissions.
- Explain receiver conditions — gain/AGC effects, front-end saturation indicators, blockers, baseline drift, feature drops, and uncertainty — rather than making unexplained AI declarations.
- Eventually ship as a **safe, maintainable appliance**: secure controls, reproducible board-specific builds, signed release artifacts, inventory, upgrades, rollback, diagnostics, and hardware compatibility tests.
- Extend to **additional Pluto-class boards through independent board profiles**; an R1 image is never presumed safe on R2 or another Zynq board.

These are product goals, **not a statement that the listed functions exist today**.

## What makes OrcPluto different

**Truth before spectacle.** IQ may be transformed only by an explicit user-selected profile. Lossless packing must restore exact native sample values. Filtering, lower-bit-depth conversion, event-only output, or selective capture are different products of the source data and must be labeled accordingly. Relative sample counters are not UTC by themselves.

**A useful edge-AI ladder.** Start with measurable FFT/power/burst and a robust baseline. Compare fixed thresholds with CFAR-inspired/adaptive-memory detection on separated real-world test captures. Only graduate to calibrated, unknown-aware classifiers when they beat simpler methods. Consider FPGA feature offload only when it demonstrably helps and does not impair the receiver.

**Standards and portability.** OrcPluto should remain useful to conventional IIO software even when OrcStream and the AI services are disabled. OrcStream stays independent of OrcPluto; its protocol, codec IDs, EVENT semantics and SDK live in the separate [OrcStream repository](https://github.com/hardcoreerik/OrcStream).

**Reproducible evidence.** A published bandwidth, latency, inference, FPGA, or decoder claim needs the exact source/build/model identity, device revision, command, logs, workload, comparator, and observed result. Simulator math and upstream third-party benchmarks are not our own board measurements.

## Initial hardware and software direction

| Layer | Current candidate or policy | Evidence |
|---|---|---|
| Radio | Candidate Fishball/PlutoSky **R1** with XC7Z020 and AD9361; one common RX LO for its two RX paths | Upstream-documented; specific owner device not fully inventoried |
| Boot and board support | Exact-board FSBL, U-Boot, DT, FPGA image, safe boot/recovery; preserve factory media | Unvalidated on owner device |
| Development OS | Pinned **ADI-derived Linux 6.12 + Debian 13 armhf** Fishball-family baseline after hardware/recovery checks | Conditional development choice, not our boot result |
| Production OS | Debian hardened for trials; Buildroot or Yocto only if A/B evidence supports the migration | Deferred |
| Existing radio API | Native **libiio / iiod** and correct AD9361 scan/channel/control interface | Mandatory compatibility target |
| Compute | Dual Cortex-A9 for orchestration and bounded DSP/classification; FPGA passive feature-side tap after measured timing/resource proof | Proposed |
| Intelligence | FFT/energy/occupancy → adaptive baseline → tiny classical model → optional verified armhf INT8 runtime | Only a standalone offline detector has host-test evidence |
| Streaming | Existing IIO first; optional OrcStream adapter once ownership and wire protocol are separately validated | No native producer demonstrated |
| Security and operation | RX-first, independent TX safety, authenticated control, audited updates, recovery and diagnostics | Release requirements, not current implementation |

### Intended data path

```text
     AD9361 RX0/RX1 (shared RX LO; exact board profile required)
                       |
             existing FPGA receive chain
                       |
             original RX DMA / IIO ----> iiod ----> existing libiio users
                       |
              [negotiated sample owner]
                       +----> optional OrcStream IQ producer [FUTURE]
                       +----> bounded CPU feature service [FUTURE]
                                   |
                                   +--> measured spectrum / bursts
                                   +--> unknown-aware classifier
                                   +--> experimental local events
                                   +--> OrcStream EVENT adapter [FUTURE, protocol-owned]
       [future independent, passive FPGA feature tap -- only after RTL proof]
```

The branches in square brackets are **not deployed claims**. Do not assume an extra IIO reader can consume the same DMA stream without changing the acquisition service. A future PL feature side channel must never backpressure the original IQ path.

## Intended user experiences

- **Ordinary SDR receiver:** plug in the radio, discover IIO, tune and receive original samples exactly as with compatible firmware.
- **Spectrum explorer:** monitor meaningful signal activity and see activity/bandwidth/noise/burst evidence, plus explicit unknowns.
- **Research instrument:** reproducible captures, source/model identifiers, sample-index-domain metadata, loss counters and offline replay.
- **Edge signal scout:** run efficient low-power local detection with optional targeted IQ capture or host-side follow-up.
- **Development platform:** open reference architecture for FPGA DSP experiments, ARM inference, conditional OrcStream integration and variant-safe image building.

No one of these experiences should require OrcSDR. OrcSDR, ESP32-P4 clients and other receivers are potential **consumers**, not core firmware dependencies.

## Ownership across repositories

| Project | Primary authority |
|---|---|
| **OrcPluto-Firmware** (this repo) | Board profiles, boot/Linux/PL integration, IIO correctness, capture ownership, optional feature and stream producer |
| [OrcStream](https://github.com/hardcoreerik/OrcStream) | Portable wire protocol, common codecs, negotiation, event/jobs semantics, test vectors and independent SDK |
| [esp-pluto-sdr](https://github.com/hardcoreerik/esp-pluto-sdr) | Independent ESP32-P4 Pluto/IIOD host driver; OrcStream support may be additive |
| [OrcSDR](https://github.com/hardcoreerik/OrcSDR) | SDR application and UI/decoder consumer, not required to use this firmware |
| [REA-Results](https://github.com/hardcoreerik/REA-Results) | Independent hardware/FPGA evidence and proposed safe validation, not automatic verification of the owner's unit |

## Engineering path

1. **Verify the exact device and recoverability** (board/PA/clock/boot/SD image and safe TX behavior) before writing hardware.
2. **Bring up a pinned reference OS on a separate, recoverable medium** and verify stock IIO/iiod interoperability.
3. **Prove original IQ fidelity, true delivery rates, sample/epoch accounting and single-owner capture.**
4. **Run the off-target detector, then a controlled receive-only detector on the actual Cortex-A9** with labeled physical captures and explicit unknown behavior.
5. **Prove reliable IQ + bounded feature coexistence** and benchmark feature drops under stress.
6. **Evaluate passive FPGA DSP, compression, model inference and OrcStream adapters separately**, requiring their own correctness, compatibility, cost and interoperability gates.
7. **Harden for a device product:** reproducible release, hardware matrix, security, update rollback and long-running endurance.

Details and release gates are in [ROADMAP.md](ROADMAP.md). Prioritized current limitations and run states are in [STATUS.md](STATUS.md).

## Research and reproducible offline test

Original research is preserved under [research/](research/README.md):

- [Agent 2: Linux, ARM ML, FPGA features, signal classification, architecture and benchmarks](research/agent2/README.md).
- [AI RF spectrum intelligence archive index](research/ai-rf-spectrum-intelligence/README.md), exploring context-keyed spectrum memory, CFAR-style adaptation, temporal fingerprints and receiver health.
- [Full Markdown audit and reconciliation](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md), including the provenance/limits of what was inspected.

The only current code experiment is [`experiments/edge-detector/`](experiments/edge-detector/README.md), an **offline**, deliberately slow O(N²) DFT/energy reference for finite CS16LE files. It is **not firmware**, and its synthetic fixtures are **not real RF test data**.

From the checkout root with Python 3 and a C++17 GCC/Clang compiler available:

```sh
python3 experiments/edge-detector/run_tests.py
# Windows PowerShell: py -3 experiments/edge-detector/run_tests.py
```

Expected from the previously documented host test: `PASS: 11 assertions; 2 burst windows; 0 inferred modulation labels`. A new run must be judged by **its own** exit code and log. The script writes `edge_detector_test.log` and `edge_detector_test_artifacts/` in the **directory you launch it from**. It does not touch any radio. See its README for prerequisites, manual replay, failure evidence, cleanup and future device tests.

## Device safety and contributions

**No generic flashing instructions.** A Fishball R1, R2, genuine Pluto and other clones may differ in FSBL, QSPI/SD layout, FPGA fabric, clocks, power amps, TX port behavior and rescue path. Do not flash, reload PL, run unsafe DFU, change clock/TX/GPIO or transmit based on an upstream-family match. First follow the [read-only board inventory and local handoff](docs/architecture/LOCAL_DEVICE_HANDOFF.md), then establish an exact-board backup and demonstrated recovery with separate operator authorization.

Contributions should be small, independently testable and linked to a roadmap gate. Include build inputs/SHAs, test commands, log location, expected versus observed outcomes, licensing provenance and explicit untested limitations. Keep original research intact; publish experimental measurements honestly. Security, RF transmission and third-party datasets require their own permissions and rights review.

## Canonical documentation

| Document | Role |
|---|---|
| [PROJECT_TRUTH.md](PROJECT_TRUTH.md) | What is real, what is proposed, and non-negotiable contracts |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Full system partition, fidelity/timing/ownership and integration boundaries |
| [ROADMAP.md](ROADMAP.md) | Milestones, dependencies and go/no-go acceptance |
| [STATUS.md](STATUS.md) | Evidence ledger, blockers, work completed and next reproducible step |
| [docs/architecture/](docs/architecture/EDGE_AI_INTEGRATION.md) | Focused ADRs, edge-AI integration and exact-board handoff |
| [docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md) | File-by-file audit across both branches |

**Built out of spite. Engineered to be undeniable.** That means interoperable hardware, faithful signal data and published tests — not unverified marketing performance.
