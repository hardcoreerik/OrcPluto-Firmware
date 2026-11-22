# ADR-0002 — FPGA DSP, ARM classification and truthful inference

**Status:** ACCEPTED AS INITIAL ARCHITECTURE; implementation blocked | 2026-11-21
**Sources:** [Agent 2 FPGA study](../../research/agent2/FPGA_AI_ACCELERATION_STUDY.md), [ML runtimes](../../research/agent2/EMBEDDED_ML_FRAMEWORK_FEASIBILITY.md), [classification](../../research/agent2/SIGNAL_CLASSIFICATION_RESEARCH.md), [Agent 1 FPGA resource matrix](https://github.com/hardcoreerik/REA-Results/blob/research/orcpluto-fishball-hardware-20261121/FPGA_RESOURCE_AND_OPPORTUNITY_MATRIX.md)

## Decision

**First proof:** offline and single-owner real-IQ C/C++ energy/FFT/power/burst detection on ARMv7. The classifier emits feature measurements, event windows and `unknown`; it must not fabricate named protocols or confidence percentages.

**First FPGA acceleration:** an **independently gated passive receive-side tap**, initially a no-op mirror then sample counters, power/threshold features and bounded feature FIFO, with original AD9361 → symmetric FIR → cpack → DMA → IIO path preserved. Feature FIFO overflow increments an observable counter and never backpressures or retunes native acquisition. Exact RX1/RX2 lane validity, dual shared RX LO, 250 MHz RX-fabric / 100 MHz AXI clock domains and CDC shall be documented from implemented source.

**Inference candidates:** test a tiny interpretable tree/logistic model first; try ncnn INT8 or LiteRT built for Cortex-A9 ARMv7 hard-float NEON/VFPv3 only after smoke/ABI checks. ONNX Runtime ARM32 support is experimental; modern Vitis AI/DPU is not available here as a drop-in NPU. FINN/hls4ml IP is *optional future R&D*, subject to full Vivado synthesis, routing, memory and timing margins.

## Reliability and truth contract

- One authoritative acquisition owner or demonstrated PL-side independent feature tap. Two separate Linux IIO read buffers must not be presumed safe concurrently.
- A feature/classification worker has bounded queues, CPU budgets and process supervision. Saturation may skip classification with separate counters; IQ remains the priority. Preserve original CS16 scan format and sample index/epoch.
- Each internal experimental event records sample-index domain, channel, measured features/units, evidence window, source and model version, decision grade (measurement, modulation_candidate, protocol_validated, unknown), calibrated confidence or null, and separate IQ/source and event loss.
- A sample counter is **relative** until externally anchored and calibrated; it is not UTC. No permanent wire IDs: OrcStream owns the interoperable event/codec schema.
- Never assign a protocol from spectral appearance alone. Train/test splits must hold out **whole capture sessions/devices**, include real AD9361 samples and OOD/unknown negative cases. Respect dataset licensing; RadioML historical sets are not unconditionally commercially redistributable.

## Evidence before deployment

Require `file`/`readelf` for actual armhf binary, executed target inference, p50/p95/p99 full pipeline, model/runtime hashes, PSS/CPU per core, IIO drops, confidence calibration, and 24h soak. For FPGA require same-base synthesis, utilization, WNS, CDC, dual-RX alias and data integrity. All currently **not executed**.
