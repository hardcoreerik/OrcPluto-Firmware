# PROJECT_TRUTH — OrcPluto-Firmware

**Canonical project truth. Reviewed 2026-11-22.** This file is the claim boundary: no other roadmap, presentation, research note or archived handoff upgrades untested work to implemented firmware. Sources are linked below; [STATUS.md](STATUS.md) lists individual proof obligations.

## Mission and boundary

OrcPluto-Firmware is Oregon Radio Company's independent, board-qualified Linux/FPGA firmware platform for Pluto-compatible SDR devices, beginning with the **candidate Fishball/PlutoSky R1, Zynq XC7Z020 + physical AD9361**. The intended product is an interoperable, capable, maintainable **SDR appliance**, not merely an AI experiment.

Its authority covers **device-specific boot, PL design, Linux services, safe hardware control, original-IQ acquisition, optional source-local DSP/intelligence, and an optional OrcStream producer**. It is **not** the OrcStream portable protocol, an OrcSDR application, or an ESP32 host driver.

- [OrcStream](https://github.com/hardcoreerik/OrcStream): packet/codec/event/capability semantics and cross-source SDK.
- [esp-pluto-sdr](https://github.com/hardcoreerik/esp-pluto-sdr): independent ESP32-P4 IIO/IIOD host source; future OrcStream support additive.
- [OrcSDR](https://github.com/hardcoreerik/OrcSDR): a possible receiver/decoder/UI consumer, not a dependency.
- [REA-Results](https://github.com/hardcoreerik/REA-Results/tree/research/orcpluto-fishball-hardware-20261121): independent research on the candidate board, not direct owner-unit proof.

## Evidence vocabulary

| Label | Meaning |
|---|---|
| `VERIFIED_ON_TARGET` | Reproduced on **this owner's exact verified board revision**, with logs, build and instrument/command evidence |
| `REPRODUCED_OFF_TARGET` | Executed with named fixtures and recorded output in a host/simulator environment, not on the radio |
| `UPSTREAM_DOCUMENTED` | Matching-board documentation, third-party reported measurements, specifications or external project observations |
| `PROPOSED` | Design, policy, or hypothesis awaiting implementation/test |
| `BLOCKED` | Mandatory verification, hardware, source contract, toolchain or recovery dependency absent |

A feature may have multiple grades for different parts. For example, an offline detector can be `REPRODUCED_OFF_TARGET` while its on-device classifier remains `BLOCKED`. `UPSTREAM_DOCUMENTED` never means owner-unit validated.

## What has actually been done

1. **Original research on `main`:** ten Markdown files (research index, Agent 2 index, six engineering studies and two follow-up/handoff notes) were preserved intact. They cover Linux distribution, ARMv7 ML runtime viability, FPGA feature acceleration, signal-classification evidence, edge system architecture and a staged benchmark plan. **Research-only, no physical radio validation by Agent 2.**
2. **Integration branch:** eleven more tracked Markdown files include three project-governance docs, two ADRs, an integrated edge-AI contract and matrix, a local hardware handoff, the detector README/evidence, and an AI RF archive index. The branch also adds the offline C++ source/runner and `research/Artifical_Intelligence_RF_research.zip`.
3. **Offline detector:** the dated [test evidence](experiments/edge-detector/OFFTARGET_TEST_EVIDENCE.md) reports g++ 14.2.0 on Linux x86-64, 11 scripted assertions for deterministic synthetic CS16LE windows and one ASan/UBSan replay returning zero. This is **`REPRODUCED_OFF_TARGET` only**. It is an O(N²) DFT + energy demonstration; no continuous radio, classifier, timing or end-to-end transport result.
4. **Candidate hardware:** [Agent 1 REA work](https://github.com/hardcoreerik/REA-Results/tree/research/orcpluto-fishball-hardware-20261121) documents R1/AD9361 architecture, nominal 1 GiB DDR, Ethernet/USB2, compatible board-specific Linux/HDL sources, and external FPGA resource/timing reports. This evidence is **`UPSTREAM_DOCUMENTED`**; exact owner-unit PCB/part grade, TX PA population, clocks, image and live recovery are **`BLOCKED`**.
5. **Related Pluto client observations:** `esp-pluto-sdr` and OrcSDR project reports describe successful IIO/IIOD IQ reception on an ESP32-P4 path. These are **separate-project reports**, not OrcPluto firmware acceptance or a native OrcStream producer.
6. **OrcStream:** its [2026-11-10 main truth](https://github.com/hardcoreerik/OrcStream/blob/main/PROJECT_TRUTH.md), inspected during this 2026-11-22 review, says `main` remains research/architecture; a separate experimental C99 codec exists, but stable native producer, EVENT, JOB, compression, subscription and security contracts have not been approved. Do not invent numeric packet/codec IDs here.
7. **AI RF archive:** [the archive index](research/ai-rf-spectrum-intelligence/README.md) identifies a 13-file research handoff and a SHA256 checksum. The archive is binary and was **not independently unpacked or hash-verified in this GitHub-text-only review**; its detailed contents cannot be upgraded to accepted engineering evidence without that review.

## Non-negotiable engineering invariants

**RF safety and board identity**

- Receive first. No flash, QSPI writes, FPGA reload, external clock change, GPIO/PTT action, or RF TX on an unverified board. TX-safe power-up must be addressed physically, including any populated amplifier; software mute alone may not cover boot transients.
- The original storage/boot assets must be backed up, hashed, recoverable and preserved. Board-family compatibility never substitutes for exact R1/PA/PCB/package/FSBL/DT verification. No generic Pluto DFU on this candidate.
- Any transmit, persistent write, or other disruptive on-device operation requires separate exact-board authorization and a working recovery plan.

**Data fidelity and interoperability**

- The original signed receive sample values (typically native 12-bit ADC data carried in the verified 16-bit IIO scan representation) must remain accessible **without silent quantization, downsampling, clipping, compression loss or substitution**. Inspect actual scan format on hardware rather than assuming fixed CS16LE.
- Standard libiio/IIO/iiod remains a supported route and compatibility regression target. OrcStream is optional and opt-in.
- Reversible packing has to round-trip exact native values. Decimated, selected, compressed-lossy, feature-only and triggered-capture outputs must have separate labels and declared transforms; do not call them original IQ.
- Explicit epochs and source/sample counters, reset causes, metadata and separate source/DMA, producer queue, feature, transport and subscriber losses; unknown losses are `not_measured`, not invented zero.
- A configured ADC sample rate and PHY or USB nominal speed do **not** prove delivered streaming throughput. For one RX, CS16 has 4 payload bytes per complex sample: 2.4 MS/s = 9.6 MB/s; 5 MS/s = 20 MB/s; 10 MS/s = 40 MB/s; 61.44 MS/s = 245.76 MB/s, prior to overhead.

**Correct resource ownership**

- Exactly one explicit authority controls radio tuning/capture/session changes; do not presume two independently opened Linux IIO buffers safely share one DMA source. Either implement a negotiated fanout or verify a truly passive PL-side feature tap.
- Inference/feature workers are bounded, lower priority and fail-open with respect to original IQ acquisition. Count and report skipped windows/events; do not silently block capture, retune, or hide IQ loss.
- RX0/RX1 share an LO on the candidate AD9361 configuration; equal sample-rate settings do not prove phase alignment or UTC. Sample-index clocks need a verified anchor and uncertainty before assigning wall time.

**Classification truth**

- Distinguish **measured properties** from **signal morphology**, **modulation candidates** and **protocol-validated outputs**. A spectral or neural guess cannot be promoted to DMR/NXDN/P25/LoRa identity without real protocol sync/frame evidence.
- Explicit `unknown`/abstain behavior; confidence is `null` until independently calibrated. Include capture window, frequency/rate/gain, declared units, model/algorithm version, quality flags and provenance.
- Use real, holdout-separated AD9361 sessions and negative/OOD cases before product accuracy claims. Synthetic RadioML is a research fixture and can have noncommercial license restrictions; preserve dataset/model rights.

**Security and product reliability**

- Diagnostic data read permissions and write/control permissions must be separately authorized. CRC does not authenticate packets. Plan authenticated remote management, rollback, startup safe states, process supervision, observability, SBOM and pinned build inputs.
- Do not infer proprietary AMD tool/IP, third-party firmware, training datasets or model weights are freely redistributable; each release needs a rights audit.

## Accepted direction vs implementation state

| Area | Decision | State |
|---|---|---|
| Development Linux | Conditionally pin Fishball-compatible ADI Linux 6.12 + Debian 13 armhf **after recovery/identity** | `PROPOSED` from upstream docs |
| Production distro | Compare hardened Debian with Buildroot and possibly Yocto after measurements | `PROPOSED`; migration deferred |
| First intelligence | Offline/controlled energy-FFT-burst detection with honest units and unknowns | Offline implementation `REPRODUCED_OFF_TARGET`; device `BLOCKED` |
| Adaptive spectrum memory / CFAR-inspired baselines | Evaluate against fixed-threshold controls on grouped capture fixtures | `PROPOSED` |
| First FPGA edge feature | Passive mirrored sample count/power/threshold then bounded FIFO; protect original DMA/IIO | `PROPOSED`; synthesis/on-board `BLOCKED` |
| Neural inference | First test tiny C++ tree/logistic model; compare ncnn vs LiteRT only after armhf ABI checks | `PROPOSED` |
| Original-IQ fanout and optional OrcStream | Preserve stock IIO, build exclusive/negotiated source ownership; don't define wire schema locally | `PROPOSED` / protocol dependency `BLOCKED` |
| Firmware appliance, OTA, multi-board image matrix, production security | Product direction | `PROPOSED`; no release |

## Explicitly NOT verified today

No owner-board inventory/backup/recovery rehearsal; no bootable OrcPluto OS image; no fresh exact-bitstream place/route/timing or independent feature PL; no ARMv7 model binary/device execution; no real IQ target detection and false-alarm data; no continuous independent IIO + AI coexistence; no native OrcStream source or stable events/compression; no 24-hour integrated soak; no TX safety qualification. Published upstream utilization such as 94/220 DSP and +0.215 ns WNS are **other-build data**, never actual remaining usable headroom.

See [ARCHITECTURE.md](ARCHITECTURE.md), [ROADMAP.md](ROADMAP.md), [STATUS.md](STATUS.md), and the [21-file evidence review](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md). Original research stays archival, not overwritten by this synthesis.
