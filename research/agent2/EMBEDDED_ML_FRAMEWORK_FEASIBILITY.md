# Embedded ML Framework Feasibility — Zynq-7020 Cortex-A9

**Agent 2 | 2026-11-21 | Research-only; zero OrcPluto model benchmarks or builds performed**

## Executive recommendation

Run **feature-driven, classical inference first** (threshold/state machine, logistic regression, shallow decision tree, small random forest); these can be implemented in a few hundred lines of portable C/C++ and have predictable bounded runtime. For neural inference, **evaluate ncnn first for compact production C++ models**, with **LiteRT/TensorFlow Lite Interpreter** as the reference comparison and potential better choice for `.tflite` interoperability. This is a deployment *preference*, not a measured ncnn win. **ONNX Runtime arm32 is experimental** in current build documentation; do not commit to it as the sole runtime without reproducing a build. [N1][T1][O1]

The chip is Zynq-7020 with dual ARM Cortex-A9 (ARMv7-A, `armhf`) and NEON/VFPv3, not a Cortex-A53/aarch64 and not a Versal NPU. Debian 13 explicitly supports ARMv7 hard-float. **Important compiler trap:** Google's LiteRT ARMv7 *example* selects `-mfpu=neon-vfpv4`, but AMD's UG585 says this Zynq Cortex-A9 has VFPv3. The build must target **`-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard`** (or an explicitly validated equivalent) instead of blindly copying a Raspberry Pi 3/4 VFPv4 example. Validate `readelf -A`, `file`, `ldd`, `/proc/cpuinfo`, CPU feature flags and an actual executable launch. [A1][T1]

## Candidate viability matrix

`Documented target` means source/build instructions support a relevant architecture. It **does not** mean tested on Fishball.

| Candidate | ARMv7 evidence | Inference fit | Dependencies / risk | Disposition |
|---|---|---|---|---|
| Native C/C++ heuristics, trees, linear model | ARMv7 general C ABI | Best for first-stage detection, simple modulation features | Very small; model code generation possible; must prevent overfitting | **Use now (proposed)** |
| **ncnn** | project supplies `arm-linux-gnueabihf` CMake toolchain (`-march=armv7-a -mfpu=neon`) and embedded build guide [N1] | Tiny CNN/MLP in C++, int8 pipelines | Model conversion/op support, exact size and latency unmeasured | **First neural candidate** |
| **LiteRT / TensorFlow Lite Interpreter** | Google publishes ARMv7 cross-compilation instructions [T1] | Popular INT8 `.tflite` models | Some optional XNNPACK/ops/toolchain features vary by version; built binaries unverified | **Comparison / fallback** |
| ONNX Runtime | official build table calls ARM32v7 *experimental* [O1] | ONNX deployment and rich graph ops | ABI/dependency footprint and reproducibility risk | Evaluate after simpler runtimes |
| TFLite Micro | embeddable interpreter, static arena design | Narrow tiny models, potentially no syscalls | Not designed to exploit Linux A9 scheduling or NEON by default; operator coverage | Niche option |
| CMSIS-NN | explicitly optimized for Cortex-M [C1] | Quantized microcontroller kernels | **Wrong optimization target** for Cortex-A9 | Do not recommend as default |
| PyTorch / desktop TensorFlow | no practical native A9 production evidence | Off-device training only | Package/compute/memory cost | Exclude from device |
| Custom fixed-point generated C | portable, builds with cross compiler | Very small dense nets or logistic/tree models | Development burden; tests for overflows and saturation | Strong fallback |

## Deployment envelope and memory analysis

The Fishball family has **approximately 1 GiB DDR** as documented from two Micron DRAM devices, with `MemTotal: 1027848 kB` on one tested board. This is **not** a 1 GiB free heap: Linux, iiod, CMA/DMA, page cache, USB gadget, stacks and stream buffers compete for it. Query both `MemAvailable` and process proportional set size on the *actual OrcPluto board*. [F1]

Useful *calculated*, not benchmarked, data sizes:

| Input format | 2.4 MS/s, 1 RX | 5 MS/s, 1 RX | 10 MS/s, 1 RX | 10 MS/s, 2 RX |
|---|---:|---:|---:|---:|
| CS16: 4 bytes / complex sample / RX | 9.6 MB/s | 20 MB/s | 40 MB/s | 80 MB/s |
| 100 ms buffer at 10 MS/s | — | — | 4 MB | 8 MB |
| 1-second 10 MS/s ring | — | — | 40 MB | 80 MB |
| Packed 12-bit I/Q, **only if exact** | 7.2 MB/s | 15 MB/s | 30 MB/s | 60 MB/s |

These are payload-only calculations, not observed USB/ARM throughput or Codec performance. In upstream one-board measurements, USB reaches approximately **20 MB/s delivered**, with complete data near 5 MS/s on a single 16-bit IQ receive stream; higher configured rates exhibited sample delivery shortfalls. No assumption that 10 MS/s raw IQ can reach the host reliably. [F2]

Suggested **initial engineering limits** (to validate, not measured): one 2.4 MS/s receiver, 4096- or 8192-point analysis blocks, one 32–128-frame feature queue, classifier invoked **after** event prefiltering; process memory target below 64 MiB PSS for detector+runtime+model, resident IQ ring capped at 32–64 MiB. Increase only when CPU, memory and overrun measurements permit.

## Suggested DSP → features → inference contract

Input feature vector can be produced on host or CPU from finite windows: `[-]` noise-power estimate, peak-to-average ratio, spectral occupancy and edges, integrated bandpower, normalized spectral entropy, cyclostationary/correlation peaks (expensive), envelope variance, phase-difference histogram, occupied bandwidth, burst duty cycle and timing. Each feature must carry an input sample range, transform parameters and quality flags; avoid confusing raw FFT magnitude with calibrated dBm. Optional GPU is absent.

**Initial baseline pipeline:**

```text
IIO / DMA IQ -> bounded capture queue -> decimated window or coarse FFT
              -> noise + energy detector -> candidate region / burst
              -> small feature vector -> tree/logistic or ncnn INT8 model
              -> class probabilities + reject/unknown gate -> event log
```

Separate *signal-present detection* from modulation classification, then from validated protocol decoding. A DMR symbol-rate spectral signature can be a **candidate**, not proof of a DMR packet; a valid frame sync and error-correcting decode are stronger evidence.

### Quantization workflow

1. Train FP32 on PC/workstation (and optionally GPU/cloud). Record sample rates, channel impairments, class taxonomy and fixed splits.
2. Quantize to INT8 weights and activations (calibrated PTQ or QAT); compare accuracy by SNR and per class, not only pooled average.
3. Convert to ncnn or `.tflite`; keep a reference FP32 implementation off-device.
4. Verify operator compatibility, static shapes, saturation rules, quantization zero-points, and model file/version checksums.
5. For any accelerator, calculate **end-to-end** overhead including DSP, copies, feature preparation, inference and postprocessing. Never equate model latency alone with radio-event latency.
6. Keep a deterministic C reference for very small models. If ncnn and LiteRT differ materially, inspect operator semantics and numerical stability.

### Threading priorities

- Highest service priority: radio DMA/iiod acquisition; application classification must not stall IIO buffer refill or interfere with the standard IIO service. Side-channel DMA is not yet present; do not imply it exists.
- Feature extraction runs with bounded CPU, optional affinity to one A9; neural inference runs only after CPU-time tokens are available. Test 1 vs 2 threads—dual core does not imply speedup when the memory bus is also carrying IQ.
- Preallocate state, use bounded SPSC/MPMC queues, non-blocking drop policy with drop counter, watchdog and deterministic fallback to detector-only mode.
- Explicit failure: no silent change to sample rate, IIO format or loss reporting. If model misses the deadline, emit `classification_skipped` and preserve IQ streaming.

## Quantitative capacity estimates (mathematical examples only)

A quantized dense layer with input dimension 32 and output 16 takes **512 multiply-accumulate terms** plus bias/activation overhead. A 3-layer 32→16→8→4 MLP takes `512 + 128 + 32 = 672` MACs per inference (weights ~672 bytes if INT8 excluding biases/alignment). At 10 classifications/s this is only 6,720 MAC/s, plus preprocessing. That is a strong architectural argument for trying classical/tiny models before a hardware NPU; **not a measured A9 benchmark**.

An input of 1024 IQ samples with 2 channels of INT8 features is ~2048 bytes, but a 1-D convolution chain can produce much larger intermediate tensors. Account for worst-case activation arena and stride/padding. Avoid streaming raw 10 MS/s samples directly through a large neural net; event-gated classification or a low-rate analysis branch is the intended path.

## Required reproducible smoke tests

On the **Linux x86 development host**: pin exact runtime git tag+commit, `arm-linux-gnueabihf` cross-toolchain version, sysroot from the chosen target image, build flags and operators. Cross-compile a deterministic tiny model, link dynamically or statically with an SBOM, inspect ELF:

```sh
file ./model_smoke
readelf -h ./model_smoke
readelf -A ./model_smoke
arm-linux-gnueabihf-readelf -d ./model_smoke
sha256sum ./model_smoke ./model.bin
```

On the **actual board**, use its native `ldd` and CPU features, then a representative performance runner that outputs a JSON line per run with `model_sha`, `runtime_sha`, target build ID, latency p50/p95/p99, RSS/PSS, CPU% for both cores, model throughput and `iiod` continuity. Input fixtures must include zero, constant, white noise, legitimate bursts, mixed transmissions and adversarial out-of-distribution signals. A `file`/`readelf` match is **not** runtime proof.

## Model benchmarking table to fill on hardware

| Framework/model | Build/ABI | Binary stripped size | Model bytes | Peak PSS | p50/p95/p99 inference | CPU% while IIO active | Delivery rate | Decision |
|---|---|---:|---:|---:|---|---|---|---|
| Tiny hand-coded tree | NOT BUILT | — | — | — | — | — | — | pending |
| ncnn INT8 MLP | NOT BUILT | — | — | — | — | — | — | pending |
| LiteRT INT8 MLP | NOT BUILT | — | — | — | — | — | — | pending |
| ONNX Runtime minimized | NOT BUILT | — | — | — | — | — | — | pending |

## Unresolved technical and licensing risks

- CPU clock, thermal throttling and the memory pressure under **simultaneous** iiod+classification not measured.
- Google cross-compile notes must be adapted to **A9 VFPv3**; do not use ARM64, VFPv4, dot-product, FP16, SVE or other newer ISA codepaths.
- `armhf` wheels for Python APIs are not assured. Use C++ entry points first.
- `ncnn` conversion may not support all ONNX/PTQ nodes; model must be defined by the practical target operator set.
- ONNX Runtime's experimental ARM32 status and runtime footprint need actual build tests.
- Model/runtime licenses may permit redistribution while training data do not. Evaluate each dataset separately (RadioML historical sets have CC BY-NC-SA terms).
- No superiority claims without exact same model/splits and measured deployed latency+quality.

## Primary sources

- [A1] AMD Zynq-7000 TRM Cortex-A9 NEON/VFPv3: https://docs.amd.com/r/en-US/ug585-zynq-7000-SoC-TRM/NEON
- [T1] Google LiteRT CMake cross-compilation ARMv7: https://developers.google.com/edge/litert/build/cmake_arm
- [N1] ncnn ARMv7 hard-float toolchain: https://github.com/Tencent/ncnn/blob/master/toolchains/arm-linux-gnueabihf.toolchain.cmake
- [N2] ncnn build guide: https://ncnn.readthedocs.io/en/latest/how-to-build/how-to-build.html
- [O1] ONNX Runtime build guide (ARM32v7 experimental): https://onnxruntime.ai/docs/build/inferencing.html
- [C1] Arm CMSIS-NN Cortex-M scope: https://arm-software.github.io/CMSIS_6/latest/NN/index.html
- [D1] Debian trixie armhf: https://www.debian.org/releases/trixie/armhf/
- [F1] Fishball board DDR inventory: https://matsvandamme.github.io/fishball7020-fpga-devkit/hardware/
- [F2] Fishball one-board performance: https://matsvandamme.github.io/fishball7020-fpga-devkit/measured-performance/
- [R1] DeepSig datasets/license: https://www.deepsig.ai/datasets/
