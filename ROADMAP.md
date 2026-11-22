# OrcPluto-Firmware — Evidence-Gated Roadmap

**Version:** research-integration v0.1 (2026-11-21); phases are **planned**, not already implemented. Current evidence: [STATUS.md](STATUS.md). The existing [Agent 2 research](research/agent2/README.md) remains archival source material.

| Phase | Principal owner / prerequisite | Deliverable | Measurable acceptance gate |
|---|---|---|---|
| **P0: Board identity, backups, recovery** | Agent 1 + owner; read-only actual unit | Exact SKU/PCB/PA/DDR/PHY/clock/USB/SD/QSPI inventory, live DT/IIO/boot logs, hashes of original boot/SD/QSPI artifacts, verified spare-card strategy | Board/profile matches actual hardware; recovery path demonstrably viable **before any write**, otherwise BLOCKED |
| **P1: Linux reference image** | Firmware lead + Agent 2, depends P0 | Version-pinned ADI Linux 6.12 + Debian 13 armhf candidate with exact R1 FSBL/U-Boot/XSA/DT, libiio/iiod 0.26 compatibility and safe TX defaults | Cold boot on spare card, native IIO 1RX and 2RX tests where supported, Ethernet/USB discovery/reconnect, existing esp-pluto-sdr client smoke; factory image remains untouched |
| **P2: Accurate IQ and reliability** | Firmware + Agent 1; P1, sample-format inventory | Single-owner acquisition contract, rate/format/epoch record, independent loss counters and measured CPU/RAM/thermal/link baselines | Byte and continuity evidence for declared 2.4, 5, 10 MS/s cases **only where link/device supports them**; all losses distinguished, no silent sample mutation |
| **P3: Classical ARM detector** | Agent 2; P2 or isolated offline fixture | Portable C++ energy/FFT/window/burst detector, explicit `unknown`, local NDJSON and deterministic fixtures | Synthetic off-target tests pass; target requires real AD9361 capture with recorded sample windows, detection error and false-alarm rate; no OrcStream EVENT claim |
| **P4: Safe simultaneous services** | Firmware acquisition owner + Agent 2; P2/P3 | One IIO sample owner or proven PL-side independent feature tap, bounded queues/overload rules and telemetry | Stress and fault injection; feature overflow only loses counted features, original-IQ delivery not worsened; 24h soak required for continuous claim |
| **P5: FPGA DSP acceleration** | Agent 1 FPGA + Agent 2; P0/P2/P4 | No-op IQ mirror then fixed-point power/threshold/counter branch and optional FFT/feature FIFO | Same pinned Vivado/RTL/XSA, simulation vectors, *post-route* WNS/CDC/BRAM/DSP reports and RX1/RX2 IIO byte integrity; hardware regression before claiming deployed |
| **P6: Quantized ML + generic event interoperability** | Agent 2 inference + OrcStream/Agent 3; P3/P4, validated codec/event negotiation | ARMv7-hf build of small ncnn or measured alternative, real-capture-disjoint classifier, versioned internal events; negotiated generic EVENT adapter only once OrcStream defines it | Actual Cortex-A9 ELF/runtime and latency p50/p95/p99, PSS, end-to-end stream continuity, calibrated/abstain/OOD tests; independent non-Pluto event consumer |
| **P7: Product hardening** | Firmware/security/release owner; all preceding gates | Choose production distro on evidence; signed artifact manifest, SBOM/licenses, secure auth/update/rollback, controlled operations | 24h acquisition+classification soak, power-cycle/recovery, independently reproduced builds, no unexplained IQ loss, demonstrated vendor-standard compatibility |

## Parallel R&D, not premature release claims

- **Compression and Smart IQ:** Agent 3 owns codec semantics/bit-exact vectors in OrcStream; Agent 1 estimates available FPGA timing/headroom; firmware only implements negotiated codecs. Benchmark lossless 12-bit packing, entropy/predictive codecs and opt-in controlled-loss representations against identical real IQ. Keep link overhead, CPU, stream fidelity and worst-case expansion visible. 61.44 MS/s full native IQ through USB2 is **not promised**.
- **On-device intelligence:** Agent 2 runs ARM reference first, FPGA feature assist second, quantized inference third; no automatic presumption that modern Vitis AI or PYNQ shells support this exact XC7Z020.
- **Interoperability:** preserve IIOD, add native OrcStream as optional, and evaluate SigMF/VITA-49 bridges independently of proprietary radio-app UI.
- **Other variants:** each board requires independently validated device-tree, boot, clock, PA and FPGA profile; no cross-variant flash.

## First runnable experiment

Build and test the off-target detector in `experiments/edge-detector/`, if present, before asking the owner for any live-board change. Then run the **read-only** inventory procedure in [docs/architecture/EDGE_AI_INTEGRATION.md](docs/architecture/EDGE_AI_INTEGRATION.md) on the verified radio, archive the original SD card *offline*, and obtain distinct authorization for experimental boot/SD writes.

## Stage verdicts

The initial software prototype is **REPRODUCED_OFF_TARGET** only after host logs are actually attached. Every hardware acceptance cell remains **BLOCKED**, not PASS by inheritance from the upstream Fishball devkit.

Review owners: Main integration director coordinates Agent 1 hardware, Agent 2 ML and Agent 3 codecs. Cross-repository changes require separate scoped review.
