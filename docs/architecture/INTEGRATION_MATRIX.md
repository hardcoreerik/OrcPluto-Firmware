# Agent 2 integration matrix (2026-11-21)

**Scope:** six Agent 2 reports, Agent 1 hardware branch, OrcStream main project truth. All feature rows are proposals unless otherwise stated. Status vocabulary: `VERIFIED_ON_TARGET`, `REPRODUCED_OFF_TARGET`, `UPSTREAM_DOCUMENTED`, `PROPOSED`, `BLOCKED`.

| Proposal | Primary Agent 2 section | Other evidence / conflict | Decision | Status / owner | Acceptance |
|---|---|---|---|---|---|
| Debian 13 armhf + ADI 6.12 development | `ORCPLUTO_LINUX_DISTRIBUTION_STUDY` / Executive decision | Agent 1 firmware lineage confirms published devkit board family, **not our live exact unit** | **ACCEPT CONDITIONALLY** | UPSTREAM_DOCUMENTED, board owner + firmware | Identified board, SD rollback, fresh version-pinned boot, IIO/iiod/USB/Ethernet and TX-default regression |
| Buildroot production firmware | Linux study / C | No proven cost/security/size or performance advantage on our target | **DEFER decision** | PROPOSED, release lead | A/B benchmark + SBOM, security/update and reproducible image |
| FPGA feature tap and counters | `FPGA_AI_ACCELERATION_STUDY` / Where to tap | Agent 1 nominal resource remainder **not a fitted design**, +0.215 ns published WNS; old 96-DSP WBFM not additive to 94-DSP 2RX | **ACCEPT FOR SYNTHESIS R&D** | BLOCKED, Agent 1 FPGA + Agent 2 | No-op mirror, CDC proof, full route WNS/headroom and IIO/2RX original-IQ regression |
| ARM classical detector | `SIGNAL_CLASSIFICATION_RESEARCH` / Summary and `AI_PROTOTYPE_AND_BENCHMARK_ROADMAP` / CPU-only | Does not establish sample access concurrent with iiod; no real capture yet | **ACCEPT OFF-TARGET FIRST** | REPRODUCED_OFF_TARGET for synthetic window detector; TARGET BLOCKED, Agent 2 | 11 host checks and ASan/UBSan, then independently labeled AD9361 known/unknown captures and false alarms |
| Quantized ncnn or LiteRT | `EMBEDDED_ML_FRAMEWORK_FEASIBILITY` / Candidate matrix | Cortex-A9 ARMv7hf NEON/VFPv3; no NPU; neither framework actually built/run | **EVALUATE, DO NOT SELECT VENDOR YET** | BLOCKED, Agent 2 | Cross-compiled valid ELF, run on board, PSS+CPU, p50/p95/p99, held-out accuracy |
| Full FPGA neural net | FPGA study / Candidate approaches | FINN board-shell support retired; resource/timing proof absent | **REJECT as first implementation**, retain optional R&D | PROPOSED, FPGA+ML | Clear CPU bottleneck, proved timing/resource advantage |
| Single source owner / bounded AI side queue | `EDGE_AI_SYSTEM_ARCHITECTURE` / Resource ownership | Independent IIOD and AI IIO readers are not known safe | **ACCEPT as invariant** | BLOCKED, firmware architecture owner | Backpressure/fault injection, no new raw-IQ loss, explicit feature gaps |
| RX1/RX2 common RX LO and timing truth | Edge AI / Feature model | Agent 1 confirms shared LO and unanchored samples ≠ UTC | **ACCEPT** | UPSTREAM_DOCUMENTED, firmware + Agent 1 | Hardware channel alignment and calibrated clock/timebase claims |
| Versioned internal NDJSON events | Edge AI / Illustrative event and `SIGNAL_CLASSIFICATION_RESEARCH` | OrcStream main has no stable EVENT type and Agent 3 final report absent | **ACCEPT INTERNAL ONLY** | PROPOSED, Agent 2 | Source window, units, relative time, model/algorithm ID, unknown, separate drop counters |
| OrcStream EVENT or codec IDs | Edge AI / OrcStream boundary | OrcStream `PROJECT_TRUTH.md` and `docs/PROTOCOL_EVOLUTION.md` reserve portable ownership; Agent 3 report absent | **DO NOT DEFINE HERE** | BLOCKED, OrcStream/Agent 3 | Independent sender/receiver conformance and versioned wire vectors |
| Triggered IQ capture | Edge AI and Benchmark Roadmap | 1 GiB DDR is not free RAM; multi-second full-wideband ring unsupported | **LIMIT TO BOUNDED BUFFER PLAN** | BLOCKED, acquisition owner | Measured memory/DDR and exact pre/post window with missing-span flags |
| Signal/modulation labels and calibration | `SIGNAL_CLASSIFICATION_RESEARCH` / Taxonomy and evaluation | Synthetic RadioML may be noncommercial and generalize poorly; protocol evidence differs from spectral guess | **ACCEPT STRICT EVIDENCE LEVELS** | PROPOSED, Agent 2 | Session/device-held-out real IQ, OOD, false-alarm/calibration and verified decoder evidence |
| USB compression beyond packed12 | Benchmark roadmap and Agent 1 throughput report | Original 61.44 MS/s CS16 1RX = 245.76 MB/s; USB2 cannot transport that uncompressed; Agent 3 results missing | **RESEARCH WITHOUT PROMISE** | BLOCKED, Agent 3 | Sample-exact vs quantified lossy fidelity, host/FPGA time, worst-case expansion and sustainable delivered rate |
| Stock client fallback and TX-safe startup | Linux study / safety and Edge AI / invariants | Agent 1 documented PA population uncertainty, brief boot emission risk and version-specific patches | **MANDATORY** | BLOCKED, board owner + firmware | RX-only board-safe cold boot, iiod/esp-pluto regression, no accidental TX |

## Critical unresolved contradictions

1. **Board identity:** public Fishball R1 component list and a previously reported IIO model string do not prove this exact owner's physical 7020 speed grade, PA, external-clock routing or boot image. Do not use R2 bitstreams or FSBL.
2. **FPGA resource:** Agent 2's 94/220 DSP figure is externally synthesized on a matching published design. No free-route headroom claim or guaranteed ncnn/PL model fit.
3. **Distribution/ABI:** a vendor Debian 13 armhf image and iiod 0.26 are not target-compatible proof until matched live; no assumption modern ARM AI binaries work.
4. **Capture:** simultaneous `iiod` and local IIO capture without a fanout contract is explicitly unsupported.
5. **Wire protocol:** no Agent 3 result was available, and OrcStream C99/event work remains experimental. Internal events cannot silently become standardized packets.
6. **Timing/throughput:** UTC, phase coherence and full-rate USB IQ cannot be inferred from a configured ADC rate or the presence of a 1GbE PHY.

Original research has not been modified; this is a review synthesis, not new hardware evidence.
