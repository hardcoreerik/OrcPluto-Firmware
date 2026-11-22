# OrcPluto Signal Classification Research

**Agent 2 | 2026-11-21 | Candidate algorithms, research datasets, evidence and acceptance; no new training performed**

## Summary: prioritize measurements, not exotic model claims

The first commercial-grade feature should be a **truthful spectrum and activity detector**: identify occupied channels, bursts, bandwidth, relative power and time intervals, issue confidence/quality flags and optionally retain IQ. Next, add a **known-set coarse modulation family classifier**, calibrated to reject unrecognized signals. Third, implement **protocol candidate discovery** (DMR/NXDN/P25/LoRa etc.) as a staged detector that can become a definitive protocol identification **only after matching demodulation or frame validation**. This is more useful and defensible than claiming arbitrary modulation recognition from an isolated neural softmax score.

A classifier trained only on historical synthetic RadioML 2016 data is **not production-validated**: DeepSig explicitly calls its open datasets historical, does not recommend them for new real-world work, and licenses them **CC BY-NC-SA 4.0**, requiring legal review for commercial training/reuse. DeepSig recommends collecting real data. [D1]

## Taxonomy: separate four evidence levels

| Level | Example output | What it proves | What it does NOT prove |
|---|---|---|---|
| 1. **Measurement** | elevated energy in bins, 12 kHz estimated occupied bandwidth | A detection under specified threshold and calibration | A specific modulation or protocol |
| 2. **Signal morphology** | bursty signal, constant envelope, two/ four-level frequency transitions, chirp-like sweep | Empirical descriptors and a candidate family | DMR, LoRa, P25 identity |
| 3. **Classification hypothesis** | `FSK-family, probability/calibrated confidence X`, with OOD rejection | Model's limited judgment on a known trained class set | Protocol decode, encryption, sender identity |
| 4. **Validated protocol** | DMR sync + valid FEC/frame; LoRa header/CRC; NXDN frame structure | A particular protocol/frame observed, under decoder's declared limits | All fields correct, device identity or lack of interference |

Many signals overlap spectrally and share air-interface shapes. For example DMR and certain other 4FSK/CPFSK systems can resemble each other under distortion; a waterfall image is not definitive protocol evidence. Report **`unknown`** or `ambiguous` when the evidence is weak.

## High-value algorithms by capability

| Capability | First implementation | Optional ML/advanced DSP | False positives / caveats |
|---|---|---|---|
| Energy detection / occupancy | Windowed FFT, robust noise-floor estimator, hysteresis and dwell | Anomaly scoring on history | Gain changes, blockers, nonstationary interference; calibration of thresholds |
| Center and bandwidth | Adjacent-bin clustering, integrated occupied bandwidth, PSD threshold crossings | Spectrogram object detector on host | Window/FFT resolution, roll-off and overlap significantly affect edges |
| Burst and timing | Adaptive threshold and connected activity intervals | HMM or temporal classifier | Short sporadic bursts need high-rate enough feature cadence |
| CW | Narrow-band peak tracking, chirp/tonal stability | Small sequence model | Beacons/telemetry tones may resemble CW; decode Morse to confirm |
| AM/analog FM | Envelope variance, phase derivative/discriminator, spectral shape | Compact tree/MLP | AM vs other amplitude-varying waveforms needs sufficient SNR |
| FSK/GFSK/MSK | Instantaneous frequency histogram, zero-crossings, symbol-rate candidate | Shallow CNN on phase difference | Multipath/clock offset; 2FSK/4FSK split requires adequate resampling |
| PSK/QAM | Moments/cumulants, cyclostationarity, phase and amplitude histograms | Quantized CNN with constellation features | Carrier sync, baud estimator and channel impairments are essential |
| OFDM | Cyclic-prefix correlation, subcarrier comb, spectral flatness | Classifier of spectral/correlation features | Bursty wideband interferers and short captures mislead |
| Chirp/LoRa candidate | Dechirp FFT peaks, linear chirp slope, chirp periodicity | SF/BW candidate model | LoRa identity needs preamble/header and CRC decoder when present |
| Digital voice families | Bandwidth/symbol rate/candidate synchronizer | Candidate ranking model | Protocol naming must wait for actual sync+FEC/frame validation |
| Unknown/anomaly | Thresholded low maximum calibrated probability, OOD distance and energy gate | One-class detector, ensembles | Must evaluate on held-out protocols, devices and sites |
| RF fingerprints | Stable cross-session RF impairment features | Metric-learning embedding | Highly sensitive to receiver, LO, gain, channel and spoofing; not identity proof |

## Why classical DSP is frequently a better first product

Energy/occupied-bandwidth measurement is inherently tied to an interpretable signal-processing chain; neural confidence is neither a calibrated power measurement nor guaranteed stable across receivers. FFT + threshold is cheap in streaming PL, while CPU runs event logic. Symbol-rate or preamble correlation for named protocols can be more specific than a generic CNN. The meaningful ML opportunities are ranking ambiguous families, adapting scan priority from long-term occupancy, and anomaly detection on quality-controlled historical features.

Recommended first classes: `noise/idle`, `continuous_narrowband`, `intermittent_narrowband`, `wideband`, `chirp_candidate`, `FSK_family`, `AM_family`, `FM_family`, `PSK_QAM_family`, `OFDM_candidate`, `unknown`. Explicitly distinguish *morphology label* from *modulation family label*. Start with fewer classes if local data do not support reliable differentiation.

## Datasets and their role

| Source | Data type / labels | Suitable use | Licensing & generalization judgement |
|---|---|---|---|
| **RadioML 2016.10A** | GNU Radio **synthetic**, 11 modulations, varied SNR | Baseline reproduction and smoke tests | DeepSig's **CC BY-NC-SA 4.0**; not a commercial production dataset without rights clearance [D1] |
| **RadioML 2018.01A** | Historical synthetic/channel simulation data with 24 modulations; ~2M examples × 1024 complex samples, HDF5, per DeepSig description | Research comparison at varied SNR | Same DeepSig noncommercial license; do not assume all captures are physically over-the-air merely because the associated paper's title uses the phrase [D1] |
| **TorchSig** | Open source synthetic RF generation / impairments and benchmark pipeline | Generate controlled training and negative cases, varying channels/interference | Verify exact library/dataset version and generated asset license; synthetic → live generalization remains unproven [D2] |
| **SigMF data from OrcPluto AD9361** | **Real** over-the-air IQ, metadata with captures and annotations | Core domain-specific validation & training | Own data if legally collected; permission, capture context, sensitive content and provenance must be governed [S1] |
| **LoRaIQ (2026 experimental dataset)** | IQ frames with SigMF metadata and labels (reported) | Targeted chirp candidate evaluation | Need independent license, annotation method and sample rate audit; not vetted here [D3] |
| **Public GitHub pretrained AMC models** | Often RadioML-derived networks | Off-device architecture comparison only | Code, model weights, training-data license and hardware-generalization requirements differ; do not ship unvetted weights |

DeepSig's public page says its 2018.01A collection contains simulated channel effects and 24 modulation types, with approximately 2 million examples of length 1024; it does **not** establish that every item came from a distinct real transmitter/physical capture. It also explicitly discourages reliance on those old datasets for new production work. [D1]

## Dataset collection specification for OrcPluto

**Ground truth** should come from a controlled test transmitter/known SDR frame generation, validated receiver logs, or human audited captures, not from the algorithm under evaluation. For every capture record:

- Source board model, firmware/FPGA SHA256, AD9361 serial/config, `hw_model`, receiver index (RX1/RX2), antenna/front-end, gain, LO, nominal and measured sample rate, clock source and synchronization uncertainty.
- IQ format (`CS16`, true effective bits and alignment, signedness, endian), scale, DC/IQ imbalance, known filter/decimation, source first-sample counter and continuity/overrun counters.
- RF environment, center frequency, approximate occupancy and SNR method, relative/absolute power calibration, LO drift, noise floor and AGC/gain mode.
- Signal source provenance (generator/hardware family, **not just a protocol label**), generator version, mode, burst repetition, symbol-rate/BW, distance and interference context, and whether the capture is synthetic or real.
- Consent/legal constraints as applicable, capture timestamp **with timebase validity**; do not encode a monotonic counter as UTC.
- Annotation window start/length, class label, validation basis and uncertainty. Use SigMF standard `global`/`captures`/`annotations` and extensions where needed. [S1]

**Collection stages:** (1) controlled signal generator at several gain/noise settings; (2) same signals over cable with safe attenuation; (3) multiple real transmitters under controlled permissions; (4) field recordings across multiple environments; (5) negative and overlapping-signal mixes; (6) regression corpus under multiple Fishball revisions and firmware variants. Avoid TX→RX loops without proper attenuation—the upstream Fishball guide warns of high TX power and sensitive RX front ends. [F1]

### Leakage prevention and validation splits

Do **not** randomly split nearby windows from the same recording across train and test. Define `recording_group_id` using continuous-capture identity; hold out entire capture sessions, transmitters, days/sites and hardware devices. Run separate test sets for (a) seen device/new session; (b) new hardware/receiver; (c) new RF environment; (d) lower SNR/interference; (e) unseen modulation/protocol. Generate intentionally confusing pairs to characterize failure, not to optimize averaged leaderboard performance.

### SNR and interference tests

Report performance in bins (illustrative test plan: −15, −10, −5, 0, +5, +10, +20 dB **as achievable**, each with empirical SNR calculation). Not all labels are physically recoverable at every low SNR: inability to distinguish can be correct `unknown`. Include CW nearby tones, adjacent-channel FM, narrowband blockers, IQ imbalance, CFO, sample-rate offset, phase noise, multipath, bursts shorter than one FFT frame and simultaneous transmissions. Do not rely on RF receiver's reported RSSI alone for ground truth SNR.

## ML model recommendations

1. **Tree/logistic model on FFT/burst features**: small, auditable and easy to compare across bit-exact C and Python. Calibrate using held-out *real* Fishball samples.
2. **Compact 1D CNN on derived phase/amplitude** after strong baselines: deploy int8 using ncnn/LiteRT on event-gated sample windows, with constrained CPU.
3. **Quantized spectral model** for closely related families if additional data justify it; quantization-aware training, pruning and distillation on workstation (user's GPU/cloud resources can accelerate offline training, not imply on-device support).
4. **Tiny FPGA MLP** only if an existing CPU classifier's p95 time or CPU load actually threatens streaming and a feature model demonstrably retains accuracy.

No uncalibrated softmax output should be labelled '92% certain' without a calibration experiment. Compare ECE, reliability curves, Brier score or equivalent, and tune reject thresholds based on false-alarm cost. A successful model must explicitly support **unknown / abstain** as a class or control decision.

## Evaluation plan and acceptance

### Metrics

- **Detection:** probability of detection at fixed false alarms/hour and fixed test SNR, time-to-detect, occupied-bandwidth error (Hz), center-frequency error (Hz), burst onset/offset error (samples/ms), noise estimation error (dB against calibrated reference).
- **Classification:** precision/recall/F1 per family, balanced accuracy, full confusion matrix, calibration/reliability, unknown rejection (AUROC/AUPRC/false acceptance), performance across device, site, SNR, overlap and gain strata.
- **Protocol candidates:** time-to-first-sync, fraction of valid frames, FEC/CRC failure rates and specific false identity rate on unrecognized captures.
- **System:** classifications/s and p50/p95/p99 latency; CPU/PSS; FPGA utilization; IQ delivered-vs-requested samples, DMA/feature overflows and event loss under stress. Separate pipeline measurement from classifier microbenchmark.

### Recommended release gates (targets, not achieved)

- Every published event includes measurement/interpretation distinction, valid signal timebase, source/frequency/bandwidth, build/model ID and `unknown` behavior.
- A user can turn off all classification with **no change** to standard IIO IQ output and tuning/USB behavior.
- Prototype detector catches a reproducible injected burst with explicit SNR and measured sample windows, and logs false alarms over a separate no-signal capture.
- Protocol name is shown as *candidate* unless validated by matching sync, framing/FEC/CRC; decoder limitations surfaced.
- Out-of-distribution test set is mandatory; no deployed confidence thresholds chosen solely on training/test overlap.

## Proposed highest-value product experiences

1. **Live signal discovery:** responsive spectrum occupancy/burst events with meaningful center/bandwidth/time uncertainty, optionally guiding OrcSDR scan priority.
2. **Event-triggered IQ clips:** pre/post windows with explicit sample indices, drop/expired flags, and SigMF export for later training or host review.
3. **Protocol-aware candidates:** DMR/NXDN/P25/LoRa candidates with progressive evidence status; use existing OrcSDR software decoders as *optional consumers/services* rather than hard dependency of the core detector.

## Sources

- [D1] DeepSig historical datasets, provenance and CC BY-NC-SA: https://www.deepsig.ai/datasets/
- [D2] TorchSig overview and synthetic dataset pipeline: https://torchsig.com/ and https://torchsig.readthedocs.io/latest/datasets.html
- [D3] LoRaIQ: Experimental LoRa Dataset (license unverified): https://zenodo.org/records/20341802
- [S1] SigMF format/specification: https://sigmf.org/
- [F1] Fishball published board safety and measurements: https://matsvandamme.github.io/fishball7020-fpga-devkit/
- [R1] RadioML generation code, historical/unmaintained: https://github.com/radioML/dataset
- [R2] 2024 peer-reviewed real-vs-synthetic robustness study: https://link.springer.com/article/10.1186/s13634-024-01176-6
- [R3] 2025 TorchSig 2.0 research: https://events.gnuradio.org/event/26/contributions/752/attachments/220/586/TorchSig_GRCon2025.pdf
