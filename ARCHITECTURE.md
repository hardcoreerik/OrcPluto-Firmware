# ARCHITECTURE — OrcPluto-Firmware

**System architecture, integrated from all tracked Markdown research and integration notes | 2026-11-22.** This is a **design contract**. The only implemented program in this repository is the finite-file off-target reference detector; most blocks below remain planned. For reality versus plan see [PROJECT_TRUTH.md](PROJECT_TRUTH.md) and [STATUS.md](STATUS.md).

## 1. Architectural mission and trust boundary

Build a **safe, recoverable, compatible radio firmware appliance** around a board-qualified Linux/programmable-logic system. Original AD9361/IIO receive fidelity is the invariant; edge spectrum intelligence, adaptive detection, compression and open network streaming are optional enhancements.

Three independent layers own their respective contracts:

1. **Hardware/firmware, this repository:** exact board boot, Linux/kernel/FPGA/DT, IIO, sample acquisition, hardware-specific features, local management/health, optional producer integration.
2. **Portable transport/protocol, [OrcStream](https://github.com/hardcoreerik/OrcStream):** device-neutral packet definitions, codec identifiers, negotiated capabilities, timestamp/loss/event/JOB semantics, SDK and conformance.
3. **Clients, e.g. OrcSDR and esp-pluto-sdr:** applications, decoders, dashboards and host adapters. Neither application nor ESP32 is a mandatory firmware dependency.

## 2. Candidate board model and qualification

**Candidate:** Fishball/PlutoSky **R1 XC7Z020 + physical AD9361**; dual Cortex-A9, FPGA PL, externally documented nominal ~1 GiB DDR, USB2 and RTL8211F network PHY. The external Fishball project reports two RX/two TX AD9361 chains with RX0/RX1 sharing an LO and 16-bit I/Q receive containers for the AD9361's native signed ADC values. These are **upstream-documented**, not an exact owner-unit acceptance.

A versioned board profile must include:

- PCB revision, device package/speed grade, AD9361/clock/LO path, RF connector and physically fitted PA details.
- Original boot mode/partition map, FSBL/U-Boot/DT/XSA and bitstream hashes; safe default TX behavior and recovery path.
- Exact IIO devices, scan layouts, valid bits, packing/endianness, RX0/RX1 latency, DMA/IRQ, GPIO, Ethernet/USB device roles.
- Known-good toolchain, vendor/ADI source commit, build flags, timing constraints, full post-route utilization, power/thermal envelope and licensing.

**Profiles are not interchangeable by name.** R1/R2 or an authentic Pluto are different validation targets until proven otherwise. Device inventory and recovery are a hard gate before any new firmware/PL write.

## 3. Receive data plane and proposed edge-side branch

```text
                             [physical R1; verify every interface]
   RF RX0/RX1 (shared LO)
              |
          AD9361 LVDS
              |
      axi_ad9361 (aligned I/Q + valid strobes)
              |
   symmetric RX FIR / cpack (exact deployed layout)
              |
       original AXI RX DMA ---------------> IIO / iiod --------> stock clients
              |                                  |
              |                             [future validated single-owner
              |                              IQ fanout; not a second raw reader]
              |                                  +----> optional OrcStream RAW IQ
              |                                  +----> bounded CPU DSP/features
              |
              +---- [future passive PL RX mirror ONLY, if timed + byte-safe]
                                      |
                                  power/counter/FFT
                                      |
                               safe CDC + bounded FIFO
                                      |
                              Cortex-A9 feature worker
                                      |
                               small model / decision
                                      |
                         local versioned events / telemetry
                                      |
                     [future OrcStream EVENT adapter after wire approval]
```

The original RX DMA/IIO path always wins. A PL side branch may drop counted **feature records** at capacity; it may not insert backpressure into AD9361 acquisition or change the native IIO bitstream. A second IIO Linux buffer reader is **not** a validated mirror; test single-owner fanout or PL-side independence explicitly.

### Process candidates (not existing services)

| Responsibility | Proposed module | Failure/isolation requirement |
|---|---|---|
| Hardware+mode supervision | `orcp-supervisor` | Safe RX default, child restart/backoff, privilege separation, model rollback |
| Exclusive acquisition and optional fanout | `orcp-capture` | One RF configuration authority, epoch changes on retune/restart, no hidden sample conversion |
| Continuous IIO/iiod compatibility | pinned standard services | Never gated on AI availability or OrcStream |
| Efficient spectral measurements | `orcp-features` | Bounded windows/queues, explicit units, gain/LO context and drop counters |
| Optional classifier/anomaly detector | `orcp-classify` | CPU budget and timeout, unknown/abstain, never block native IQ |
| Local record/diagnostic store | `orcp-events` | Internal schema/version, no fabricated UTC, bounded retention, replay provenance |
| Optional OrcStream adapter | `orcp-orcstream` | Independent protocol version/negotiation, raw fallback, truthfully qualified codecs |

Names are logical responsibilities, not a committed daemon ABI or user-visible command set.

## 4. IQ fidelity contract

The source records **native** converter valid bits, IIO scan type (alignment/signedness), endian, channel interleaving, center/LO Hz, requested and **measured** sample rate, gain/RF bandwidth, filters/decimation, source/firmware/PL identity and stream epoch. An explicit requested transform must be recorded as a parent→derived stream relationship, including sample count and exact-versus-approximate nature.

- **Original IQ:** unchanged sample values for the verified native scan format; available whenever hardware permits.
- **Reversible packing:** e.g. verified sign-preserving 12-bit packing, *only* if sample-exact reconstructible from source values with independent vectors. Not the same as floating-point normalization.
- **Decimated/channelized/low-bit IQ:** useful **derived** data; never advertised as the original wideband signal.
- **Feature/event only:** metadata, not retrievable IQ unless an actual bounded capture buffer still contains the evidence.
- **Triggered IQ:** cap memory, specify pre/post window/expiry, partial/missing spans, source epoch and capture provenance.

Each path needs **independent** loss counters: ADC/source/DMA overrun, source owner queue, transformed output, feature FIFO/skips, transport, subscriber. Reset/wrap scopes and unobservable loss must be declared. No falsely perfect 0-drop claim.

### Throughput arithmetic, not measurements

`CS16_payload_bytes_per_second = sample_rate_complex_sps × 4 × enabled_rx_channels`.

| Complex sample rate | 1 RX raw payload | 2 RX raw payload |
|---|---:|---:|
| 2.4 MS/s | 9.6 MB/s | 19.2 MB/s |
| 5 MS/s | 20 MB/s | 40 MB/s |
| 10 MS/s | 40 MB/s | 80 MB/s |
| 61.44 MS/s | 245.76 MB/s | 491.52 MB/s |

These numbers exclude framing/copies/USB/Ethernet overhead and are **not validated sustained output**. The external upstream devkit's USB observations apply to a different environment and cannot substantiate this firmware's speeds.

## 5. Clock, indexing, dual receiver and retune semantics

A `source_epoch` changes whenever sample continuity, tuner configuration, FPGA/DMAs or model contract cannot be related reliably. A monotonic source index identifies windows **within** the declared source/timebase; per-transport packet sequence is separate. Do not infer receiver UTC from a simple FPGA counter or host packet-arrival timestamp. A verified time anchor, uncertainty and drift model are necessary to claim wall-clock or coherent multi-device timing.

Both RX receive channels may share one RF LO but have separate I/Q valid and routing behavior. Align channels by verified sample-enable timing and golden vectors; equality of sample-rate register values does not guarantee phase coherence.

Changing LO, gain, filters, RX channel mask, rate, bitstream or capture owner invalidates or explicitly regenerates affected spectrum baselines and inference windows. Reject conflicting operations with a typed busy/not-supported response; never silently retune an IIO subscriber.

## 6. Spectrum intelligence ladder

**First: deterministic measurements.** FFT/windowed power, robust noise estimate, peak/occupied bandwidth, burst onset/duration, occupancy and saturation/clipping flags with units and resolution. Current [finite-file reference](experiments/edge-detector/README.md) implements only basic energy+O(N²) DFT and is off-target.

**Second: adaptive context.** Compare fixed threshold, CFAR-inspired local threshold and **context-keyed robust spectral memory** grouped by verified LO/rate/gain/bandwidth/channel/profile/receiver state. Track resets, stale baseline, interference contamination, warm-up and classifier disable. No secret moving baseline that hides active transmitters. Preserve feature evidence and counterfactual fixed-threshold results for evaluation.

**Third: careful classification.** A small decision tree/logistic classifier on physics-grounded features before an INT8 ncnn/LiteRT candidate. Output the graded decision:

- `measurement` — observable power/FFT/burst/bandwidth with uncertainty.
- `morphology_candidate` — FSK-like/chirp-like/bursty shape; *not a named protocol*.
- `modulation_candidate` — calibrated family hypothesis with false-accept/unknown testing.
- `protocol_validated` — actual correct framing/sync/FEC/CRC evidence from a decoder.
- `unknown` — acceptable and often the right answer; uncalibrated confidence is `null`.

**Fourth, only if justified: hardware assist.** Passive PL power/valid counters, then decimation/FFT/event FIFO after original IIO bytes and post-route timing remain unaffected. Full FPGA neural inference is a separate high-risk study, not the starter architecture.

Train/test on **disjoint recording sessions, radios and sites**, with negative/OOD cases, interference/gain variation and honest dataset rights. A synthetic-only result does not prove real-world classification.

## 7. Internal event envelope — NOT OrcStream standard

For experiments, the [edge integration contract](docs/architecture/EDGE_AI_INTEGRATION.md) defines illustrative local NDJSON `orcp.internal.measurement.v0`. It should include source/firmware/PL/model identity; channel and config epoch; first sample/count and timebase; native scan/transform; measured properties, unit/calibration flags and method; evidence grade, unknown/decision and confidence or null; source vs feature/event loss counters; and capture existence where actually retained.

Its field names **do not reserve or freeze OrcStream wire fields, numeric IDs or binary frames**. A translator may be developed only after portable OrcStream protocol owners accept and version an interoperable EVENT/CAPABILITIES/CAPTURE contract with independent send/receive/conformance vectors. Security requires authenticated, authorized management independently of CRC.

## 8. OS, build provenance and security architecture

Initial development candidate: pinned Fishball-family Linux 6.12 ADI-derived kernel and Debian 13 `armhf`, exact FSBL/U-Boot/XSA/DT, correct IIO/iiod, on a **separate spare SD only after P0 recovery**. Harden, strip unneeded services, and benchmark against the stock preserved image. Buildroot is a **possible** production packaging path after same-board memory/boot/update/security A/B; Yocto is a future alternative if multi-variant management justifies it. None is already booted as OrcPluto.

Reproducible manifest must include boot image, kernel, rootfs, FPGA project/synthesis constraints, DT, compiler/libiio/iiod, package SBOM, ML models, dataset provenance and source hashes. OTA requires authentication, power-loss-safe update transaction and rollback with a verified hardware rescue path; do not implement first through ad hoc QSPI writes. Remote shell/iiod, API control, TX/GPIO and FPGA reload are distinct permission boundaries.

## 9. Acceptance harness and runtime observability

Gate every enhancement against a paired baseline **on the exact same hardware**:

- **Correctness:** IIO scan/channel/format, original value preservation, epoch/loss accounting, same Golden IQ before/after optional feature tap.
- **Realtime:** requested vs delivered IQ and detected gaps, queue watermarks, p50/p95/p99 *full window-to-event* latency, CPU per core, PSS, bus contention, temperatures and power if measured.
- **FPGA:** pinned toolchain, LUT/FF/DSP/BRAM **delta**, WNS/WHS, complete timing + CDC reports and full RX1/RX2 hardware regression.
- **ML:** real capture-disjoint confusion matrix and false positives/hour, calibration and unknown rejection by SNR/gain/device/site, INT8 exact build/operator checks and actual armhf runtime.
- **Reliability:** classifier crash/hang/backpressure, device reboot/reconnect, rollback, separate source and feature drops, 24-hour soak and controlled power interruption.
- **Interoperability:** stock libiio, independent ESP32-P4 receiver, and if OrcStream is used at all, a non-OrcSDR reference client capable of reproducing sample and event semantics.

Log files and run manifests must be saved in the **directory from which the test was launched**, with exact command, build IDs, machine/platform, fixture hashes, pass/fail and unexecuted gates. See [local handoff](docs/architecture/LOCAL_DEVICE_HANDOFF.md).

## 10. References and controlled decisions

- [Original Agent 2 technical reports](research/agent2/README.md) — Linux, ARM ML, PL, classification, system architecture, benchmarks.
- [ADR-0001: development OS](docs/architecture/ADR-0001-development-linux.md) — conditional Debian, deferred Buildroot decision.
- [ADR-0002: hybrid inference](docs/architecture/ADR-0002-hybrid-edge-inference.md) — deterministic PL features + bounded ARM processing.
- [Integration matrix](docs/architecture/INTEGRATION_MATRIX.md) — issue-by-issue cross-agent decisions and acceptance.
- [EDGE_AI_INTEGRATION.md](docs/architecture/EDGE_AI_INTEGRATION.md) — source ownership and experimental event details.
- [AI RF archive index](research/ai-rf-spectrum-intelligence/README.md) — staged proposals, binary details unverified in this audit.
- [OrcStream project truth](https://github.com/hardcoreerik/OrcStream/blob/main/PROJECT_TRUTH.md) and [protocol evolution](https://github.com/hardcoreerik/OrcStream/blob/main/docs/PROTOCOL_EVOLUTION.md) — source-neutral wire authority.
- [Full existing Markdown reconciliation](docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md) — all 21 existing `.md` paths, decisions and caveats.
