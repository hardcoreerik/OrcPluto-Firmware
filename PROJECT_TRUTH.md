# PROJECT_TRUTH — OrcPluto-Firmware

**Integration snapshot:** 2026-11-21. This document defines what is accepted as project direction versus what has actually been demonstrated. Research files are evidence, not firmware releases.

## Purpose and ownership

OrcPluto-Firmware is Oregon Radio Company's independent, board-qualified Linux/FPGA firmware distribution for supported Pluto-compatible radios, starting with the candidate **Fishball/PlutoSky R1, XC7Z020 + physical AD9361**. It is an **optional reference implementation** of the independent [OrcStream](https://github.com/hardcoreerik/OrcStream) protocol, not a prerequisite or protocol owner.

- **OrcStream:** portable source-independent protocol, codec, SDK, frame and EVENT semantics, and conformance vectors.
- **OrcPluto-Firmware:** board support, safe boot/recovery, standard IIO/iiod compatibility, acquisition owner, optional DSP/ML and OrcStream producer.
- **esp-pluto-sdr:** independent ESP32-P4 host driver; existing IIOD transport remains; future OrcStream transport is additive.
- **OrcSDR:** a driver consumer, not a dependency of these projects.

## Evidence vocabulary

- `VERIFIED_ON_TARGET`: observed on owner's exact board, with recorded revision, command, logs and artifacts.
- `REPRODUCED_OFF_TARGET`: executed on a host/simulator with named inputs, version and captured output.
- `UPSTREAM_DOCUMENTED`: source, datasheet or independent matching-board claim, **not** remeasured on this unit.
- `PROPOSED`: architecture/recommendation awaiting implementation or validation.
- `BLOCKED`: unmet dependency or unavailable hardware/toolchain.

Never turn mathematical throughput or published upstream synthesis utilization into own-hardware measurements.

## Present state

- **UPSTREAM_DOCUMENTED:** Agent 1's independent [hardware branch](https://github.com/hardcoreerik/REA-Results/tree/research/orcpluto-fishball-hardware-20261121) describes the XC7Z020/AD9361 R1 architecture, 1 GiB nominal DDR, USB2 high-speed, RTL8211F Ethernet, published Linux 6.12/Debian 13 devkit and source-specific FPGA resource/timing reports. Exact owner-unit identification and remaining FPGA headroom are **BLOCKED** on inventory and a reproducible build.
- **UPSTREAM_DOCUMENTED:** [Agent 2's six reports](research/agent2/README.md) compare development distributions, ARMv7 inference, PL DSP, signal classification and measurable acceptance. This is research only; Agent 2 did no target benchmarks, bitstream synthesis or flash.
- **REPRODUCED_OFF_TARGET (external branch only):** Agent 1 reports deterministic 12-bit IQ packing math tests (8 passed), not native firmware, line-rate transport or an FPGA codec.
- **REPRODUCED_OFF_TARGET (this integration branch):** a C++17 CS16LE offline window-energy/DFT reference detector, deterministic synthetic fixture and 11 host assertions passed on Linux x86-64; one ASan/UBSan replay returned zero. This is not a live RF measurement, target ARM inference or OrcStream wire producer. See [experiment evidence](experiments/edge-detector/OFFTARGET_TEST_EVIDENCE.md).
- **REPORTED FROM A SEPARATE PROJECT:** [esp-pluto-sdr](https://github.com/hardcoreerik/esp-pluto-sdr) already receives IIO/IIOD IQ from a Fishball-firmware radio on an ESP32-P4. The existing OrcSDR Tab5 integration has genuine receiver/audio tests but unresolved transport lifecycle and power conditions. None is an OrcStream native firmware producer.
- **PROPOSED:** OrcPluto bootable image, native producer, streaming fanout, side-channel FPGA feature engine, ARM detector/ML classifier and classification EVENT packets.
- **BLOCKED:** own board inventory, recovery rehearsal, known-good image hashes, exact-bitstream resource utilization, target ABI/performance, full-rate fidelity and 24-hour integrated soak.
- **OrcStream main:** research/docs only at commit `66c3a294d36c2ad1b38243b9146952cabb5aa22f` (checked on 2026-11-21); experimental C99 codec branches are unmerged. No stable EVENT, JOB, compression or subscriber wire contract is assumed. Agent 3 compression results were not located at this snapshot.

## Non-negotiable controls

1. RX-first: no firmware flash, QSPI rewrite, FPGA reload, external clock change, GPIO/PTT action or RF TX without exact-board recovery and separate on-device authorization. TX must stay physically safe even through boot, reset and process death.
2. Original native signed 12-bit RX values in their verified 16-bit IIO containers remain available with exact scan format. Native IQ is never silently quantized, compressed lossily, decimated or replaced by inference.
3. Keep ordinary IIO/iiod and external libiio tools operational; OrcStream is opt-in. Never assume simultaneous independent IIO DMA readers work.
4. Native acquisition has one authoritative owner/resource arbitration model. AI/feature backlog may drop **features with counters**, never quietly block source acquisition. Receiver tuning and sample epochs are explicitly controlled.
5. Classification must distinguish measured RF properties, modulation hypotheses, validated protocol decodes and **unknown**. An unanchored sample counter is not UTC.
6. Portable OrcStream data types remain hardware-agnostic; do not define permanent packet IDs or codec numbers inside this repository.
7. Source/firmware/model/FPGA licenses and dataset rights require provenance review prior to distribution.

Canonical roadmap: [ROADMAP.md](ROADMAP.md); actual evidence/status: [STATUS.md](STATUS.md); integration decisions: [docs/architecture/EDGE_AI_INTEGRATION.md](docs/architecture/EDGE_AI_INTEGRATION.md).
