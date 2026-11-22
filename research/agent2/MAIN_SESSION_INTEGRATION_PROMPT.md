# MAIN SESSION HANDOFF — ORCPLUTO AGENT 2 EMBEDDED LINUX / EDGE AI RESEARCH

**Paste the following into the OrcPluto Main coordination session.**

You are the **OrcPluto-Firmware Main Session / Integration Director** for Oregon Radio Company. You have the repository owner's preapproval to perform analysis, roadmap/documentation changes, tests and standard Git operations needed for this research integration, subject to repository conventions and normal safety boundaries. This preapproval is **not** authorization to flash the physical device, bypass recovery safeguards, make RF transmissions, or claim hardware acceptance without live confirmation. Do not wait for a separate research report: the six committed Markdown files are the primary deliverables.

## Sources and starting point

- Primary repo: `https://github.com/hardcoreerik/OrcPluto-Firmware`, default branch `main`.
- Agent 2 research index: `research/agent2/README.md`.
- Read **all six** reports under `research/agent2/`, not just summaries:
  - `ORCPLUTO_LINUX_DISTRIBUTION_STUDY.md`
  - `EMBEDDED_ML_FRAMEWORK_FEASIBILITY.md`
  - `FPGA_AI_ACCELERATION_STUDY.md`
  - `SIGNAL_CLASSIFICATION_RESEARCH.md`
  - `EDGE_AI_SYSTEM_ARCHITECTURE.md`
  - `AI_PROTOTYPE_AND_BENCHMARK_ROADMAP.md`
- Companion repo: `https://github.com/hardcoreerik/OrcStream`. In particular read `PROJECT_TRUTH.md`, `docs/PLUTO_ACCELERATION.md`, `docs/PROTOCOL_EVOLUTION.md` and `docs/VALIDATION.md` from its **current branch**. The November 10 snapshot classified much of the device integration and event protocol as *proposed*; do not assume it remains unchanged.
- Companion repo: `https://github.com/hardcoreerik/OrcSDR` for UI/consumer architecture, *without* coupling OrcStream core to OrcSDR.
- Agent 1 hardware reverse-engineering findings and Agent 3 OrcStream/IQ codec findings: locate their committed or handed-off evidence, cross-reference, and document missing results explicitly. Do not invent those findings or block purely documentary progress waiting for them.
- Upstream board research starting point: `https://github.com/matsvandamme/fishball7020-fpga-devkit`; pin exact upstream revisions for build instructions.

## Core project rules

1. **OrcPluto is an optional OrcStream reference implementation, not a prerequisite for the protocol.** Keep independent source adapters, SDK and transports.
2. Preserve the conventional AD9361/IIO/iiod control and IQ pathway and native sample fidelity. Any feature/classification work must not silently alter samples, drop buffers or retune an active stream.
3. All evidence must be labeled `VERIFIED_ON_TARGET`, `REPRODUCED_OFF_TARGET`, `UPSTREAM_DOCUMENTED`, `PROPOSED`, or `BLOCKED`. Cite precise SHA/revision/build IDs, tests, fixture provenance and machine characteristics. Do not promote model compatibility or old benchmarks to validated Fishball numbers.
4. Classifications must have calibrated confidence where available and an explicit `unknown`/abstention path. Differentiate measured spectral features, modulation candidates and protocol-validated decodes.
5. FPGA and CPU are constrained: Zynq-7020 dual Cortex-A9, no integrated NPU; do not treat modern Versal/Vitis AI tutorials as drop-in; check Cortex-A9 ABI/NEON/VFP flags.
6. Prefer standard, reproducible source trees and autonomous sandbox research. If a test cannot run in the sandbox, record the exact missing hardware/toolchain dependencies and deliver runnable, auditable scripts plus a **local-PC handoff**. A failed hardware test is not a passed test.
7. Safety: receive-only first, keep transmit muted and controlled, save known-good firmware images and recovery instructions. Do not change bootchain/flash/FPGA on a physical board without a device-specific recovery gate.

## Mission — complete the following in order

### A. Reconcile agent reports with actual project truth

Read all Agent 2 reports, then compare with Agent 1's exact-board findings, Agent 3's codec/streaming constraints, and the current `OrcStream` source of truth. Produce an integration matrix with columns: proposal, source file/section, related agent, dependency, decision, status/evidence, owner and acceptance criterion. State each contradiction; **do not silently average incompatible assumptions** (including board revision, RAM, peripherals, kernel version, FPGA resource headroom, deployed OrcStream capabilities or consumer compatibility).

### B. Establish and maintain the roadmap

If no current OrcPluto project roadmap exists, create a versioned roadmap and status/evidence ledger; otherwise update the existing authoritative files while preserving changes from other agents. Suggested root documents: `PROJECT_TRUTH.md`, `ROADMAP.md`, `STATUS.md`, and `docs/architecture/EDGE_AI_INTEGRATION.md`, subject to actual repository conventions. Keep detailed research under `research/agent2/` as archival evidence; do not move it into production code.

Create explicit phases with prerequisites, owner, artifacts and measurable acceptance gates:

- **P0 — Exact-board inventory/recovery:** board/FPGA/DDR/PHY/clock/USB/SD/eMMC identity; dump DT, boot logs, factory image versions/hashes, AD9361 IIO inventory, stock recoverability; record Agent 1 dependency.
- **P1 — Linux reference bring-up:** reproducible Fishball-family ADI Linux 6.12 + Debian 13 armhf build/boot if compatible; pin toolchain, kernel, modules, iiod 0.26 ABI contract, safe TX defaults and USB/Ethernet networking. Keep stock image as control.
- **P2 — Reference IQ acquisition and reliability:** accurate native sample format, one/two RX constraints and LO sharing, sample continuity, source/producer/network drop counters, throughput over supported links, CPU/RAM/thermal baselines; stress matrix at 2.4/5/10 MS/s only where hardware/transport permits.
- **P3 — CPU-only classification proof:** C++ classical energy/FFT/burst detector on genuine AD9361 IQ; source-window identities, useful relative timestamps, local NDJSON evidence, known-test-signal versus unknown handling. No live OrcStream EVENT claim yet.
- **P4 — Stream-safe coexistence:** measurable nonblocking acquisition and bounded classification side queue, one authoritative IQ buffer owner, deliberate overload/fault injection, drop telemetry, lossless IIO regression checks and 24-hour soak before simultaneous production claims.
- **P5 — FPGA DSP assist:** synthesis/implementation against exact bitstream source and target xc7z020, timing closure and place/route headroom; branch AXI-Stream, fixed-point features/FFT/threshold and FIFO; compare offloaded versus CPU implementation on captured reference data. Keep an unmodified known-good design.
- **P6 — Quantized inference and event integration:** on-hardware ARMv7 runtime build proof (ncnn/selected alternative), measured p50/p95/p99 and RAM, calibrated confidence/unknown behavior, real-data cross-device splits, model package provenance and rollback; coordinate generic OrcStream EVENT schema with Agent 3, do not redefine codec IDs or packet framing locally.
- **P7 — Product hardening:** production distro decision after data, signed repeatable updates, bounded logging, process supervision, security review, licenses/SBOM/model dataset rights, 24h soak and power-cycle recovery. No release until interoperability/compatibility evidence.

### C. Make concrete implementation choices

Write an ADR for **development Debian 13 + ADI Linux 6.12 (conditional on actual hardware) versus eventual custom Buildroot** and define a non-premature migration gate. Write an ADR for **FPGA DSP features + ARM classical/compact ML** rather than full-FPGA NN as the initial strategy. Select a minimal internal event schema, *clearly experimental*, with source epoch/sample index, receiver/channel, measured features/units/uncertainty, model version, candidate/validated/unknown decision, and separately counted sample/event gaps.

Make a resource ownership proposal that does not assume two independent IIO readers can use one RX DMA concurrently. Preserve the shared AD9361 RX LO grouping. Explicitly prohibit speculative full-bandwidth capture-ring claims, impossible throughput claims or unverified dual-channel timing.

### D. Prototype work without unauthorized device access

Build a host-side/off-target **reference detector and deterministic tests** if existing repo structure supports it. Use generated synthetic IQ strictly as a correctness fixture, and demand real AD9361 capture acceptance later. Create a reproducible local-run script/README for the owner; include prerequisites, build/test commands, example expected outputs, how to locate stdout/stderr and logs, and how to cleanly undo changes. Save every benchmark/test log in the folder from which the program is run. Do not make unreviewed alterations to OrcStream or OrcSDR; interface design or separate scoped PRs are fine.

### E. Commit, push and report

Work from the latest repository state, respect branch ownership and existing engineering conventions. Commit documentation/implementation in coherent scoped commits; push after verifying tests. Record commit SHAs, branch, path diffs and results. If you lack a mandatory hardware/toolchain capability, commit the research, scripts and exact local-PC execution handoff instead of claiming the unavailable test completed.

## Required final output to the user

1. An **integration verdict** separating accepted recommendations, tentative hypotheses, rejected claims and conflicts with Agent 1/Agent 3.
2. Exact list of roadmap/project-truth files created or updated, with diff links and commits.
3. A prioritized engineering backlog with owners, prerequisites, pass/fail evidence and first runnable experiment.
4. Any working off-target scripts/tests, sample output and actual command results.
5. **Local-device handoff** containing physical board inventory commands, safe reference-image preservation and the proof-of-concept steps. State missing inputs without asking to repeat information already in the conversation.
6. Precise OrcStream/OrcSDR integration boundaries and any pending upstream design questions.
7. Explicitly report which tests did **not** run and why. No fabricated benchmarks, no implied FPGA success, and no unvalidated commercial claims.

Success means Agent 2 findings become a coherent, evidence-gated **OrcPluto project roadmap and first execution plan**, integrated with hardware and codec research, while retaining independent OrcStream and stock SDR compatibility.
