# STATUS — OrcPluto-Firmware

**Reviewed:** 2026-11-22. This ledger distinguishes documented work from actual target acceptance. The planned target board is a candidate Fishball/PlutoSky **R1 XC7Z020 + AD9361**. No new live hardware validation is implied by this document consolidation.

## At a glance

| Area | Evidence grade | Actual state | Required next proof |
|---|---|---|---|
| Original Agent 2 research | `UPSTREAM_DOCUMENTED` / research | Six technical studies plus index/follow-up/handoff on repository `main` | Convert into exact-board tests and artifact-based ADR decisions |
| Integration docs and architecture | `PROPOSED` / documented | Prior branch supplies truth, roadmap, status, 2 ADRs, matrix, handoff and edge contract; this audit consolidates all 21 existing tracked Markdown files | Merge docs; run acceptance tasks rather than assuming implementation |
| Host CS16LE detector | `REPRODUCED_OFF_TARGET` | Prior report: 11 assertions, two expected burst windows, ASan/UBSan replay passed on Linux x86-64, g++ 14.2.0; no new run in this docs review | Repeat test; tighten input/output edge cases; build ARMv7 and run actual recorded AD9361 IQ |
| Candidate board details | `UPSTREAM_DOCUMENTED` | REA/Fishball external documentation for R1 silicon/DDR/PHY/USB and firmware lineage | Read-only exact owner-unit PCB, part grade, PA, TX boot behavior, clock, boot/DT/IIO inventory |
| FPGA baseline utilization | `UPSTREAM_DOCUMENTED` | Matching *other* R1 published symmetric 2RX build: 94/220 DSP48, 12,521/53,200 LUT, +0.215 ns WNS; arithmetic remainder **is not routable headroom** | Rebuild same revision, compare post-route full report and native IIO output on owner board |
| Existing Pluto USB/IIOD client | Separate-project report | `esp-pluto-sdr` / OrcSDR have reported ESP32-P4 IIO receive paths | Independent regression against *our* new reference image, rates and failure recovery |
| AI RF research archive | Index verified as tracked text; ZIP **not independently unpacked** | Index describes 13-file bundle and checksum, including memory, receiver-health and temporal fingerprint concepts | Verify ZIP hash/contents in a binary-capable environment, reconcile each internal doc before treating as authoritative |
| Spectrum memory / adaptive detection | `PROPOSED` | No implemented CFAR/adaptive-memory tests in this repository | Deterministic controlled fixtures + grouped real-capture comparisons against fixed threshold |
| Linux 6.12 + Debian 13 armhf | `UPSTREAM_DOCUMENTED` candidate | Matching Fishball-family source and external recipe; no owner-board boot | Exact image/boot/DT hashes, spare SD, recovery rehearsal, full IIO/USB/Ethernet validation |
| Custom Buildroot/Yocto | `PROPOSED` / deferred | Production distro trade studies only | Same-board A/B footprint/boot/update/security/resource evidence |
| Live on-device classifier | `BLOCKED` | No completed ARMv7 ELF/model/runtime/benchmark | Device ABI, capture-accurate scan conversion, known/unknown labels, target CPU/PSS/latency |
| FPGA feature engine | `BLOCKED` | No integrated mirror/FFT/FIFO synthesis, CDC proof or flashed trial | Exact-source RTL, full route timing/resource delta, 2RX native-IQ integrity |
| IQ + AI concurrent owner/fanout | `BLOCKED` | No demonstrated safe simultaneous service; second IIO DMA reader not presumed supported | Single-owner/side-tap design, overload tests and separate source vs feature loss |
| OrcStream native producer/event/codec | `BLOCKED` | OrcStream main research-stage at last read; experimental C99 on separate branch | Portable protocol approval and independent conformance, then actual producer and generic consumer |
| Product firmware/recovery/security | `BLOCKED` | No bootable OrcPluto image or release/update/rollback validation | P0/P1/P7 gates and device-safe authorization |
| 24h soak and release claims | `BLOCKED` | No verified long-duration target service | Continuous IIO + feature soak, reconnect, power cycle, memory/thermal and explicit loss ledger |

## Documentation reconciliation

The `main` baseline SHA at comparison was `b9c69caa3bea28f7ef758f79185fe38c959b3618`; the integration branch head inspected was `d0f9658335a298952734d7183695a60fda7b714c`. That branch was **26 commits ahead / 0 behind** and contained **21 tracked Markdown files**, including the same **10** present on `main`. Its **11 branch-only** Markdown files capture integration, ADR, experiment and AI archive-index work. Original research files remained byte-for-byte common to both branch trees at the audit point.

Consolidation writes the canonical [README.md](README.md), [PROJECT_TRUTH.md](PROJECT_TRUTH.md), [ROADMAP.md](ROADMAP.md), [ARCHITECTURE.md](ARCHITECTURE.md), this ledger and a [file-by-file audit](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md). Original `research/agent2/` studies and technical ADRs remain archival supporting references.

The `research/Artifical_Intelligence_RF_research.zip` filename is historical and intentionally not renamed. The index lists SHA256 `a10ff4f0c4b2beb2e797c07cb9d732b3e7585dbd0676c1336563bff9b940ee24`, **not independently recalculated in this review**.

## Test execution truth

| Test or operation | In this documentation review | Prior reported result / source |
|---|---|---|
| Enumerate and inspect both repo trees / every existing tracked `.md` | **DONE** by GitHub connector; 21 unique Markdown files on integration branch | File-by-file [audit](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md) |
| Cross-check OrcStream protocol authority | **DONE** by GitHub text reads | Main project truth and protocol evolution docs dated 2026-11-10 |
| Review detector C++ and Python runner text | **DONE** (static only) | Local log, CWD fixture path and bounded O(N²) behavior confirmed in source |
| Run detector code in this session | **NOT EXECUTED** | Prior [recorded off-target test](experiments/edge-detector/OFFTARGET_TEST_EVIDENCE.md) reports 11 assertions and ASan/UBSan |
| Unpack/validate AI research ZIP binary | **NOT EXECUTED** | GitHub text connector could read index but not decode ZIP contents |
| Cross-compile or execute on Cortex-A9 | **NOT EXECUTED** | Hardware/toolchain not attached |
| Boot image, original SD backup and recovery | **NOT EXECUTED** | Exact hardware identity and owner-side physical procedure required |
| Synthesis/post-route FPGA report and board IQ comparison | **NOT EXECUTED** | Exact HDL/Vivado and physical board unavailable |
| Real RF capture, AI/IIO coexistence, 24h soak | **NOT EXECUTED** | P0/P1/P2 missing |
| Native OrcStream wire integration | **NOT EXECUTED** | Portable protocol negotiation and hardware source missing |

## Open issues requiring explicit decisions

1. **Board reality:** name, form factor and a prior IIO `hw_model` alone do not identify the actual owner unit's power amplifiers, speed grade and boot/clock topology. Do not use R2 boot assets or generic DFU.
2. **FPGA room:** 220 total DSP minus 94 reported use is arithmetic, not deployable available resources; timing slack in that external report is narrow. The old 96-DSP WBFM experiment and newer 2RX FIR build are **not automatically additive**.
3. **Native IQ and USB:** 1RX CS16 payload at 61.44 MS/s is 245.76 MB/s; upstream USB examples and 1GbE PHY names do not guarantee full-rate transport. Report actual sustainable bytes, samples and losses.
4. **Capture contention:** iiod plus independent IIO reader may compete for one RX DMA. Do not imply simultaneous real-time intelligence from a successful standalone offline file replay.
5. **Timing:** equal nominal samples and an FPGA counter do not establish true UTC, calibrated clock precision or cross-channel phase coherence.
6. **AI identity:** detecting frequency/burst features cannot validate DMR/NXDN/P25/LoRa; confidence absent calibration remains null and `unknown` must work.
7. **Protocol authority:** OrcStream owns source-neutral codec/EVENT/JOB IDs and tests. This firmware's internal NDJSON is **not** an interoperable wire standard.
8. **Detector robustness:** its Python runner compiles and logs in the launch directory, but C++ currently checks input/output aliases by pathname string, not canonical/inode identity, and numeric parsing may accept trailing characters. Harden before accepting untrusted inputs.
9. **Binary archive provenance:** the AI RF index describes concepts but the embedded 13 Markdown files are not exposed as tracked paths. Unpack/hash-review before final claim of full archive-content audit.

## Next safe actions

First run `python3 experiments/edge-detector/run_tests.py` from the directory where logs should be saved (Windows: `py -3 experiments/edge-detector/run_tests.py`); compare the real run to the documented expected `PASS: 11 assertions; 2 burst windows; 0 inferred modulation labels`. The runner saves `edge_detector_test.log` and artifacts in the **launch directory**. Full tool prerequisites, manual replay and cleanup: [experiment instructions](experiments/edge-detector/README.md).

Separately, perform the [read-only exact-board inventory](docs/architecture/LOCAL_DEVICE_HANDOFF.md), preserve and hash original SD offline, and stop until recovery and TX safety are established. Only then start P1 reference-image work. No document here grants permission for a disruptive device action.
