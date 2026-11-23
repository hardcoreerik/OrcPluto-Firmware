# Complete Markdown review — OrcPluto-Firmware (2026-11-22)

**Scope:** `hardcoreerik/OrcPluto-Firmware`, `main` at `b9c69caa3bea28f7ef758f79185fe38c959b3618`, and `integration/agent2-edge-ai-roadmap-20261121` at `d0f9658335a298952734d7183695a60fda7b714c`. The branch comparison was **26 ahead, zero behind** at inspection. This document covers **every existing tracked `.md` file at those snapshots**, not only those named `research`. The integration tree was not truncated.

**Method:** enumerate GitHub recursive tree, compare branches and fetch each tracked Markdown file by path/ref; inspect section-level technical content, evidence claims, acceptance gates and repo links. Also inspect the two off-target source files, and read OrcStream `main` project truth and protocol evolution as an external authority cross-check. This is a **documentation/source audit**, not a reproducible execution or independent scientific verification of every external citation.

**Inventory:** `main` has **10 Markdown files**; all 10 are identical and present in the integration tree. The integration branch has **21 Markdown files**, including **11 additions**. No tracked Markdown was deleted or changed by the existing integration branch relative to main. The new canonical documents produced by this consolidation were **not** part of the 21-file pre-consolidation census.

## A. All 10 Markdown files already on `main`

| Path | Review / retained decision |
|---|---|
| [`research/README.md`](../../research/README.md) | Research isolation and evidence rules are correct; expand index for AI RF archive and review trail |
| [`research/agent2/README.md`](../../research/agent2/README.md) | Honest research-only scope, six source reports, unresolved physical validation; preserve |
| [`research/agent2/ORCPLUTO_LINUX_DISTRIBUTION_STUDY.md`](../../research/agent2/ORCPLUTO_LINUX_DISTRIBUTION_STUDY.md) | Conditional development **Linux 6.12 + Debian 13 armhf**, Buildroot potential production, Yocto as alternative. Board-specific FSBL/DT and recovery first. Preserve research; reflect conditional choice in ADR and roadmap |
| [`research/agent2/EMBEDDED_ML_FRAMEWORK_FEASIBILITY.md`](../../research/agent2/EMBEDDED_ML_FRAMEWORK_FEASIBILITY.md) | Cortex-A9 ARMv7hf, hand-coded classical model first, ncnn vs LiteRT after ABI/run; no modern NPU assumption. Published 1 GiB DDR is not free allocatable RAM |
| [`research/agent2/FPGA_AI_ACCELERATION_STUDY.md`](../../research/agent2/FPGA_AI_ACCELERATION_STUDY.md) | Passive PL feature branch before FPGA NN; preserve FIR/cpack/DMA, validate RX channel strobes, clocks, CDC, routed timing and DSP/BRAM delta; old FINN/PYNQ example not plug-and-play |
| [`research/agent2/SIGNAL_CLASSIFICATION_RESEARCH.md`](../../research/agent2/SIGNAL_CLASSIFICATION_RESEARCH.md) | Four levels of evidence; actual modulation/protocol identification requires stronger verification; session/device-disjoint real captures, calibrated unknown, dataset rights. Never turn RadioML synthetic scores into product performance |
| [`research/agent2/EDGE_AI_SYSTEM_ARCHITECTURE.md`](../../research/agent2/EDGE_AI_SYSTEM_ARCHITECTURE.md) | Hybrid CPU+eventual PL path, bounded overload, source ownership, distinct IQ/feature/event losses, optional generic OrcStream adapter. Contains an **illustrative internal** event example, not official OrcStream fields |
| [`research/agent2/AI_PROTOTYPE_AND_BENCHMARK_ROADMAP.md`](../../research/agent2/AI_PROTOTYPE_AND_BENCHMARK_ROADMAP.md) | Exact-board P0/G0, baseline IIO, offline then ARM detector, provenance/false alarms and p99; 24h and post-route gates. Targets remain unmeasured, never inherited from matching family |
| [`research/agent2/FOLLOW_UP_ACTIONS.md`](../../research/agent2/FOLLOW_UP_ACTIONS.md) | Handoff checklist, evidence labeling, owner assignments and local launch-folder logs. Older to-do snapshot; consolidated into root ROADMAP/STATUS without changing archival text |
| [`research/agent2/MAIN_SESSION_INTEGRATION_PROMPT.md`](../../research/agent2/MAIN_SESSION_INTEGRATION_PROMPT.md) | Historical integration instructions and scope boundaries, not living project truth. Previous tasks partly executed in branch and remaining device gates still blocked |

## B. All 11 Markdown files added only on the integration branch

| Path | Review / retained decision |
|---|---|
| [`PROJECT_TRUTH.md`](../../PROJECT_TRUTH.md) | Correct five-grade vocabulary, no unearned hardware claims, standard IIO/raw IQ priority and protocol independence. Expand into canonical whole-project truth |
| [`ROADMAP.md`](../../ROADMAP.md) | Correct P0–P7 gates; integrate later adaptive memory, compression and productization without marking them implemented |
| [`STATUS.md`](../../STATUS.md) | Good proof ledger; refresh explicit tested/not-tested rows, archival zip limit, C++ static review findings and two-branch reconciliation |
| [`docs/architecture/ADR-0001-development-linux.md`](../architecture/ADR-0001-development-linux.md) | Sound conditional Debian first, Buildroot evidence gate; preserve as decision record |
| [`docs/architecture/ADR-0002-hybrid-edge-inference.md`](../architecture/ADR-0002-hybrid-edge-inference.md) | Sound passive PL + bounded ARM work, unknown handling; preserve as decision record |
| [`docs/architecture/EDGE_AI_INTEGRATION.md`](../architecture/EDGE_AI_INTEGRATION.md) | Detailed source owner, internal NDJSON, sample/epoch/overload and read-only inventory. This remains the practical edge-AI engineering contract |
| [`docs/architecture/INTEGRATION_MATRIX.md`](../architecture/INTEGRATION_MATRIX.md) | Cross-agent conflicts and conditional dispositions are explicit. Keep as technical trace matrix; update only when new test evidence arrives |
| [`docs/architecture/LOCAL_DEVICE_HANDOFF.md`](../architecture/LOCAL_DEVICE_HANDOFF.md) | Correct read-only-first and verified SD recovery sequence; operator-specific device writes remain blocked |
| [`experiments/edge-detector/README.md`](../../experiments/edge-detector/README.md) | Accurate finite-file CS16LE reference warning, run commands, log-in-launch-folder policy and hardware gates; preserve |
| [`experiments/edge-detector/OFFTARGET_TEST_EVIDENCE.md`](../../experiments/edge-detector/OFFTARGET_TEST_EVIDENCE.md) | Reports host Linux x86-64 correctness + sanitizer evidence; no assumption of ARM, RF or streaming acceptance. Preserve as dated evidence, not new-run assertion |
| [`research/ai-rf-spectrum-intelligence/README.md`](../../research/ai-rf-spectrum-intelligence/README.md) | Index to historical AI RF ZIP and its claimed checksum. Concept prompts include spectrum memory, receiver health and temporal fingerprints; review detailed binary contents separately before claiming every embedded study was audited |

## C. Additional non-Markdown artifacts checked

- `experiments/edge-detector/detect.cpp`: reads finite CS16LE interleaved I/Q, uses window mean counts-squared power and O(N²) DFT for triggered windows; emits explicit offline source and unknown classifier status. **Static issues to fix before untrusted files**: output alias check compares only path strings (hard links/symlink aliases may bypass); `std::stoul`/`std::stod` may accept suffix junk; an error following partial output should never be mistaken for a valid run. It is not calibrated dBm/FFT throughput proof.
- `experiments/edge-detector/run_tests.py`: deterministic 8 × 64-sample fixture and 11 scripted checks; uses `Path.cwd()` for log and artifacts, so the launch-folder contract is honored. Host tests were reported in existing evidence, **not re-executed in this audit**. More corrupt-path, malformed input and exhaustion tests are appropriate.
- `research/Artifical_Intelligence_RF_research.zip`: 29,305-byte tracked binary in Git tree; the index lists **13 files** and SHA256 `a10ff4f0c4b2beb2e797c07cb9d732b3e7585dbd0676c1336563bff9b940ee24`. **Limitation:** the GitHub text connector failed to decode the ZIP bytes, so the internal documents and digest were not independently opened/verified. The ZIP is preserved unmodified, and its concepts are classified as proposals from the accompanying README, not reviewed scientific evidence.
- `.gitignore`: correctly ignores generated detector log/artifacts in the checkout/run folder.

## D. Cross-document disputes resolved

| Issue | Resolution and proof condition |
|---|---|
| R1 hardware identity vs published family | Candidate only; exact owner PCB/PA/clock/boot/FSBL/FPGA revision and recovery must be checked before write |
| Debian vs Buildroot | **Pinned Debian 13 armhf + ADI 6.12 only as conditional development candidate**; Buildroot production is a measured future option |
| 94/220 DSP, LUT leftovers and WNS | Arithmetic remainder is **not fitted/timed FPGA margin**; the published 2RX FIR and older WBFM figures cannot be summed; full exact-bitstream place/route mandatory |
| Dual RX timing and sample counters | Shared RX LO is not phase/UTC proof; validate lane strobes, clock domain, anchor and error before coherent/time claims |
| Standard IIO vs independent ML reader | No guarantee two buffers can own the DMA. Require a single source owner or independently proven passive PL tap; native IQ wins backpressure |
| AI label vs real protocol identity | Measurements, morphology, modulation hypothesis and decoder validation have distinct evidence; explicit unknown and calibrated or null confidence |
| Compression vs data reduction | Lossless native 12-bit packing needs exact source-value vectors. Low-bit IQ, decimation, selected clips and event-only streams are not original IQ. Actual payload and link-rate accounting is required |
| OrcPluto vs OrcStream governance | OrcStream defines portable wire packet/codec/EVENT/JOB semantics; local NDJSON remains deliberately internal/experimental |
| Useful AI research vs current proof | Adaptive noise/spectrum memory, receiver health, temporal fingerprints and feedback are **experiments on the roadmap**, not implemented firmware or measured classifier improvements |
| Productization vs lab firmware | Safe boot/recovery, authentication, reproducible images, SBOM/license and rollback are release gates, not already delivered |

## E. Reconciled root-document decisions

- **`README.md`** — newly authored **whole vision** and navigation: who OrcPluto serves, standard/client interoperability, product strategy, staged AI/PL/compression/OrcStream, provenance and safe reproduction.
- **`PROJECT_TRUTH.md`** — current actual/project boundary, five evidence labels, no fabricated on-target claims, protocols and hardware constraints.
- **`ROADMAP.md`** — accepted P0–P7 sequence plus *parallel, clearly non-release* spectrum-intelligence, compression, ML and appliance lanes.
- **`STATUS.md`** — what was reviewed and what was not run, with active blockers and immediate evidence work.
- **`ARCHITECTURE.md`** — canonical architecture; consolidate existing edge ADRs and provenance without overwriting them.
- **`architectured.md`** — filename compatibility pointer to canonical `ARCHITECTURE.md`; do not maintain conflicting copies.
- **`research/README.md`** — updated navigation for both the original six reports and supplemental AI RF archive; no alteration to original research.
- **No firmware, FPGA, OrcStream, esp-pluto-sdr or OrcSDR source changes** are part of this documentation consolidation.

## F. Exact unexecuted proof and next review

**Not done here:** binary ZIP unpack/hash; fresh host compiler or sanitizer replay; actual board inventory/SD backup/boot; ARM model runtime; PL synthesis/place/route/CDC; real IIO captures; simultaneous IQ+AI; OrcStream protocol implementation; 24h soak. Those require separately retained logs, code/binary artifacts and often physical access. The next engineering action is a fresh offline detector run with launch-folder logs, then read-only hardware identification and verified recovery per [local handoff](../architecture/LOCAL_DEVICE_HANDOFF.md). No flashing or transmitting is authorized by this review.
