# Agent 2 — Embedded Linux, Edge AI & Signal Classification

**Research date:** 2026-11-21  
**Repository:** `hardcoreerik/OrcPluto-Firmware`  
**Path:** `research/agent2/`  
**State:** research-only; zero local OrcPluto hardware tests and zero flashed images reported by this agent.

## Reports

1. [ORCPLUTO_LINUX_DISTRIBUTION_STUDY.md](ORCPLUTO_LINUX_DISTRIBUTION_STUDY.md) — Fishball Debian 13/ADI 6.12 development baseline, Buildroot production candidate, Yocto alternative.
2. [EMBEDDED_ML_FRAMEWORK_FEASIBILITY.md](EMBEDDED_ML_FRAMEWORK_FEASIBILITY.md) — Cortex-A9/ARMv7 inference framework tradeoffs, quantization and target toolchain/ABI gates.
3. [FPGA_AI_ACCELERATION_STUDY.md](FPGA_AI_ACCELERATION_STUDY.md) — documented AD9361 datapath, FPGA resource and timing constraints, feature extraction before full NN acceleration.
4. [SIGNAL_CLASSIFICATION_RESEARCH.md](SIGNAL_CLASSIFICATION_RESEARCH.md) — measured DSP versus ML, dataset provenance, calibrated confidence and unknown-signal handling.
5. [EDGE_AI_SYSTEM_ARCHITECTURE.md](EDGE_AI_SYSTEM_ARCHITECTURE.md) — four alternative pipelines, nonblocking stream/data ownership, evidence/event metadata, OrcStream boundary.
6. [AI_PROTOTYPE_AND_BENCHMARK_ROADMAP.md](AI_PROTOTYPE_AND_BENCHMARK_ROADMAP.md) — practical receive-only proof of concept, tests, acceptance gates and local handoff.

## Working decision — subject to cross-agent and device verification

- Development: Fishball-family **ADI Linux 6.12 + Debian 13 armhf** *if hardware identity and boot compatibility pass*.
- First field prototype: streamline and pin that baseline; do **not** rebuild a new distro before the experiment works.
- Production candidate: **custom Buildroot** only after demonstrated need, reproducible builds and recovery/update design.
- Inference: **classical DSP/hand-coded features first**, then compare a compact **ncnn** model or other verified ARMv7-compatible runtime.
- FPGA: **branch/decimate/FFT/feature FIFO** as a passive side service; leave the original IIO IQ stream working. Full FPGA NN inference is research, not the first implementation.
- Event output: internal versioned measurements initially. Do **not** invent or freeze OrcStream wire fields; OrcStream is device-independent.

## Read-before-implementing gates

1. Agent 1 confirms the exact board revision, FPGA build/headroom, boot chain, DDR, interfaces, and safe recovery.
2. Agent 3 confirms the evolving OrcStream IQ/event semantics and ownership of codec/wire design.
3. Main session compares findings with `hardcoreerik/OrcStream` authoritative `PROJECT_TRUTH.md` and OrcSDR current development state.
4. Local Fishball/AD9361 measurements establish actual IQ baseline, CPU headroom, power and temperature, not estimates from other boards.
5. No speculative performance numbers become release claims. Keep stock IIO/iiod compatibility and a known-good rollback path.

## Handoff

The downloadable handoff bundle contains `handoff/MAIN_SESSION_INTEGRATION_PROMPT.md`, `handoff/FOLLOW_UP_ACTIONS.md`, and a file-integrity manifest. The handoff prompt is a task for the Main integration session, not an instruction that any engineering step has already run.
