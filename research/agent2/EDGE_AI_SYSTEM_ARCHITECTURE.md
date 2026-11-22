# OrcPluto Edge AI System Architecture

**Agent 2 | 2026-11-21 | Proposed architecture; not an implemented component or released OrcStream protocol**

## System goal and invariants

Transform the **Fishball Rev.A / XC7Z020 + AD9361** into an intelligent radio source **without regressing standard libiio/iiod IQ streaming**. OrcPluto is a reference source and optional producer of OrcStream metadata; OrcStream itself must remain source/device/FPGA independent. The native RX path is the product's primary real-time obligation, while optional AI processing is best-effort, bounded and transparent. [F1][O1]

**Non-negotiable:** standard IIO clients still work; original IQ is preserved on request, not silently substituted with lossy IQ, transformed features or classification results; stream continuity, loss and timebase domains remain honest; remote controls require authorization; RX-first and transmitter muted by default. The current OrcStream `main` is a **research/architecture project**, while its C99 codec experiment is on a separate unmerged branch—not a stable published event wire format. [O1][O2]

## Recommended target architecture: incremental hybrid B → D

```text
                     +---------------------- Native reference path --------------------+
AD9361 RX0/RX1 ---> axi_ad9361 --> verified FIR/cpack --> existing AXI-DMA --> IIO --> iiod --> standard clients
        |                    (unchanged loss/fidelity requirements)               |
        |                                                                         +--> optional OrcStream IQ adapter
        +--> future passive FPGA feature branch [PROPOSED]                                   |
                  | FFT/power/burst/counters --> bounded FIFO --> ARM feature/event service   |
                  |                                                      |                     |
                  +--> optional bounded IQ excerpt/ring                  +--> classifier       |
                                                                             |                 |
                                                                    versioned events --------+
                                                                             |
                                                              optional host/OrcSDR/OrcNode
                                                        extended ML/decoder visualization
```

**Phase 0** can run entirely in the Linux A9 user process using a sampled IIO branch or a **single owner of capture buffers**. Native iiod and a second local consumer may contend for the same IIO buffers; a transparent multi-client *sample fanout* is a design requirement, not an existing feature. Do not advertise simultaneous full-rate IIO and full-rate AI until the IIO ownership/DMA architecture has been tested. A passive FPGA tap makes the cleanest future separation.

## Compare four architectural options

The columns below contain **engineering estimates and design implications**, not measured throughput, latency or energy. Payload calculations assume complex signed 16-bit IQ, **4 bytes per sample per RX**; actual driver scan layout/filters must be read back. [F1]

| Option | Data route | Compute/memory effect | Latency class (unmeasured) | Risks | Role |
|---|---|---|---|---|---|
| **A: ARM only** | AD9361→existing FPGA DMA→A9 DSP/features→A9 classifier | IQ buffers and CPU FFT directly compete with acquisition, iiod and networking; NN memory can grow | Block window + software queue + CPU inference | Single device/buffer ownership, 2-core starvation, high sample rate | **First proof** |
| **B: PL DSP + A9 ML** | ADC mirror→PL power/FFT/features→small FPGA FIFO→A9 model | Minimal feature bus traffic, bounded A9 work; FPGA DSP/BRAM and clock complexity | Window + feature transfer + tiny inference | Timing closure, CDC, false alarm pipeline | **Preferred deployed target** |
| **C: PL DSP + PL NN** | ADC mirror→PL features→quantized NN→ARM metadata | Small event bandwidth, high PL DSP/BRAM use, less CPU | Fixed pipeline + transfer (theoretical) | Quantization, integration, tools, validation and timing | Research-only later |
| **D: Hybrid device/host** | Device detector/coarse label→events + optional IQ→client host decoder/ML | Device keeps only small model/ring; heavy classification on PC/P4/server | Network/USB + host compute | Link outages, resource arbitration, privacy, transport | **Preferred extensibility** |

**Choose A for the minimum demonstration**, B for reliable always-on edge detection and D to scale expensive workloads. C is only justified by measured need and sufficient synthesis margin.

## Quantitative buffer planning

By arithmetic: `CS16_rate_bytes/s = sample_rate × 4 × N_rx`.

| Rate | 1 RX CS16 | 2 RX CS16 | 250 ms at 1 RX | 1 s at 2 RX |
|---|---:|---:|---:|---:|
| 2.4 MS/s | 9.6 MB/s | 19.2 MB/s | 2.4 MB | 19.2 MB |
| 5 MS/s | 20 MB/s | 40 MB/s | 5 MB | 40 MB |
| 10 MS/s | 40 MB/s | 80 MB/s | 10 MB | 80 MB |

These are **raw payload budgets**, not real delivered transport or measured DDR bandwidth. Upstream one-board Fishball measurements report USB carrying ~20 MB/s with single RX 5 MS/s complete and higher set rates showing shortfall; high-rate feature computation should therefore not be attempted as a second full raw stream over USB. [F2]

**Suggested starting design caps (pending board measurements):** small DMA refills (tune based on source alignment), 2.4 MS/s single RX, 8 Ki–16 Ki complex-sample analysis windows, 16–32 sample-window queue entries or summarized features, 2–4 seconds *selective narrowband/decimated* sample excerpts rather than a 2-channel full-rate raw IQ ring, preallocated heap and model loaded once. At 2.4 MS/s a 4 MiB CS16 ring retains ~0.437 seconds for a single RX. At 10 MS/s it retains ~0.105 seconds. A 32 MiB raw ring on 2 RX at 5 MS/s is ~0.8 s—ring memory is never free.

The model process must not touch DMA buffers after the IIO owner releases them; avoid broad pointer sharing across process lifetimes. Prefer a bounded copy of *decimated* sample blocks/feature arrays and strict ownership; compare its extra memory bandwidth with zero-copy under real load.

## Process and thread architecture (proposed)

- `orcp-edge-supervisor`: launches services, health/identity, privileges, crash backoff, model selection/rollback and safe default configurations. No direct TX privileges in ordinary ML process.
- `orcp-capture-adapter`: **one negotiated source owner**, based on current Linux IIO limits. Exports timestamped feature jobs and optional IQ excerpts, records native scan format and IIO drops. Initial adapter may own a single radio session in benchmark mode; do not claim normal iiod and second capture consumer coexist until proven.
- `orcp-feature`: window/FFT/noise/peak/burst estimator using integer/NEON or future PL feature FIFO. Emits measurement records independent of model decision.
- `orcp-classify`: model/vocabulary hash and dataset/training provenance; bounded work queue, source epoch and abstain logic. It is never allowed to retune or block native sample acquisition.
- `orcp-publish`: writes events to a local audited log, then optional OrcStream adapter (only after protocol evolves), or temporary versioned JSON export for prototype visualization. Network backpressure drops feature messages with counters; raw IQ remains separate.

Thread policy: acquisition highest, then bounded feature extraction, then ML and event delivery. Treat A9 dual-core affinity as a benchmark variable, not a guarantee. Avoid memory allocation in hot callbacks, expensive JSON serialization per IQ block, and model loading on the acquisition thread. Add CPU-time budget (e.g. classifier gets at most 10–20% of aggregate capacity initially—**a trial policy not a measured available margin**); fall back to detector-only on stress.

## Feature model and provenance

A signal record should carry:

| Field family | Required semantics |
|---|---|
| Identity | source ID, hardware revision if known, firmware and PL git/hash identity, model and algorithm ID/version, RX channel |
| Acquisition | LO Hz, configured and effective sample rate, RF bandwidth, gain and mode, filter/DDC/decimation chain, native IQ scan format |
| Time | source epoch, first and end sample index, sample-counter validity, clock domain (monotonic/device/UTC) and uncertainty |
| Measurement | peak/mean power in declared units, noise estimate and estimator, SNR method + uncertainty, spectral center/BW estimates and resolution |
| Classification | class taxonomy, level (`morphology_candidate`, `modulation_candidate`, `protocol_validated`), calibrated confidence or `null`, unknown/rejection reason, evidence pointers |
| Quality | IQ gaps, feature FIFO overruns, model timeout/skip, output queue drops, receiver saturation, AGC changes, partial/truncated window |
| Payload | optional IQ-excerpt locator, retention expiry/availability, SigMF capture metadata hash |
| Resource health | CPU%, model PSS, PL build and utilization as build metadata, temperature, classification time histogram, uptime |

A 64-bit FPGA sample counter does **not** equal UTC; if no calibrated clock is present, publish sample index/relative time with declared uncertainty, and optionally host wall-clock observation as a *separate* timestamp. Dual channels share RX LO on AD9361; `RX1` and `RX2` are separate observations, not independently tunable arbitrary-band radios. [O2]

### Illustrative event JSON **for internal prototype only**

This is **NOT** a committed OrcStream message type/schema and should not be treated as protocol standard.

```json
{
  "schema": "orcp.internal.measurement.v0",
  "source_epoch": "boot-0007",
  "rx_channel": 0,
  "first_sample_index": "1048576",
  "sample_count": 8192,
  "sample_timebase": "device_counter_unanchored",
  "lo_hz": 433920000,
  "sample_rate_sps": 2400000,
  "features": {
    "occupied_bandwidth_hz": 12500,
    "relative_power_dbfs": -38.2,
    "noise_floor_dbfs": -59.1,
    "measurement_status": "illustrative_values_not_measured"
  },
  "classification": {
    "label": "fsk_family_candidate",
    "probability": null,
    "decision": "unknown",
    "evidence_level": "signal_morphology_only",
    "model_id": "illustration-not-built"
  },
  "quality": {"iq_gap_count": 0, "feature_drop_count": 0, "metrics_illustrative": true}
}
```

## OrcStream and OrcSDR integration boundaries

Current upstream OrcStream documents `main` as research/architecture, with experimental C99 v0.1 in **unmerged branch** `chatgpt/protocol-c99-v0.1-20261104`; features like subscriptions, events and device jobs remain proposals. Therefore OrcPluto Agent 2 defines a **semantic data contract only**, not numeric packet types, field offsets, encryption, packet lengths or codec IDs. Do not silently change existing C99 header or invent reserved official constants. [O1][O3]

**Integration phases:**

1. **Local evidence:** append newline-delimited internal measurement/event JSON with schema version and source sample windows; OrcSDR dashboard may consume through an explicitly experimental adapter.
2. **Generic OrcStream coordination:** coordinate labels, feature units, loss, uncertainty, model identity and capability negotiation with Agent 3 / OrcStream maintainers; no Pluto-only fields in common core.
3. **Opt-in transport:** OrcPluto may run both conventional IIO/iiod and optional source-native OrcStream producer over Ethernet/USB-IP *once actual multi-client and raw IQ behavior are proven*.
4. **Portable clients:** identical data semantics in Linux/Windows/macOS/ESP32-P4/OrcSDR and non-OrcSDR tools; event consumers need no FPGA knowledge.
5. **Advanced offload:** optional host decoder inference using IQ excerpts; capability negotiation, resource authorization and timeouts must be protocol-level portable, not an OrcPluto hard dependency.

Do **not** report prototype classification JSON as native OrcStream until a protocol implementation, spec and cross-client conformance exist. Do not attach misleading 'raw IQ' fidelity semantics to feature-only or lossy results.

## Resource ownership and safety state machine

```text
BOOT/SAFE_RX -> DISCOVER -> IDLE/BASELINE
   -> REQUEST feature job -> RESERVE -> ENABLE -> RUN
      -> STOP/DRAIN -> RELEASE -> IDLE
                    |       |
                  TIMEOUT  OVERRUN -> DEGRADED (detector-off)
                    |       |
                    +--> recovery with IIO unaffected
```

Capabilities include FPGA bitstream version, free resources currently reserved, supported sample rates, channel masks and whether feature-only monitoring is possible concurrently with standard iiod. Reject conflicting retune, RX FIR, DMA or bitstream changes with explicit `BUSY`/unsupported status instead of forceful takeover. Security: read-only metadata can be exposed broadly only when configured; writing radio, GPIO, model, firmware or TX controls requires authentication and authorization. CRC is *not* an authentication mechanism. [O2]

## Fault and overload behavior

| Failure | Safe behavior |
|---|---|
| Classifier hung/slow | watchdog, timeout and drop classification, keep iiod/IQ alive |
| Feature FIFO overflow | account exact discarded feature windows; no ADC stall; mark sequence discontinuity |
| IQ DMA/iiod overrun | surface source-level IQ loss separately from event output loss; never conceal |
| Host disconnect | no unbounded event queue; require fresh negotiation/session epoch if continuity invalid |
| Model update incompatible | atomic rollback to last validated model, detector-only fallback |
| FPGA selftest fails | disable new logic where possible, restore known-good bitstream/SD recovery; TX safe |
| Gain/LO/sampling change | flush old classifier windows; new capture epoch, update metadata |
| Two readers demand same device buffer | negotiated sharing or fail clearly; never assume independent buffers |

## What must be measured before deployment

- A9 utilization and deadline miss rate for original IIO alone and each classifier option; FPGAs' actual resource overhead and timing margin.
- Sustained delivered-vs-nominal IQ samples under simultaneous feature extraction, `iiod` and optional event/OrcStream publication at 2.4, 5 and 10 MS/s (mark impossible transport rates as such).
- Linux scheduling jitter, memory/PSS, DDR/AXI contention, networking/USB drop rates, CPU temperature, boot and failover.
- p50/p95/p99 detection+classification latency including sample window acquisition and event transport. p99 **not** just net forward-pass time.
- Independent two-channel valid timing, forced feature drop, triggered IQ alignment, 24-hour soak and power cycling.

## Sources and project contracts

- [F1] Fishball stock design and IIO map: https://matsvandamme.github.io/fishball7020-fpga-devkit/block-design/
- [F2] Fishball throughput measurements: https://matsvandamme.github.io/fishball7020-fpga-devkit/measured-performance/
- [O1] OrcStream current main/experimental status: https://github.com/hardcoreerik/OrcStream/blob/main/PROJECT_TRUTH.md
- [O2] OrcStream Pluto acceleration plan: https://github.com/hardcoreerik/OrcStream/blob/main/docs/PLUTO_ACCELERATION.md
- [O3] OrcStream future event/protocol proposal: https://github.com/hardcoreerik/OrcStream/blob/main/docs/PROTOCOL_EVOLUTION.md
- [S1] SigMF capture metadata: https://sigmf.org/
- [F3] Fishball modern IIO and RF safety contract: https://matsvandamme.github.io/fishball7020-fpga-devkit/debian-rootfs/