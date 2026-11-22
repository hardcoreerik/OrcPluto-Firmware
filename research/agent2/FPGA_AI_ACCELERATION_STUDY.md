# FPGA AI Acceleration Study — Fishball7020 / AD9361

**Agent 2 | 2026-11-21 | Evidence: upstream Fishball design reports, vendor/tool docs; no new synthesis, simulation or live implementation**

## Engineering decision

**Use programmable logic first for *side-channel DSP feature extraction* (decimation/channel power, FFT, statistics, burst detection, timestamps), not as a full end-to-end CNN inference engine.** Keep the verified AD9361→IIO/DMA native IQ path functionally unchanged. On the A9, classify the reduced event/features with small classical or INT8 models. Only investigate FINN/hls4ml quantized inference if ordinary IQ streaming and the feature branch coexist at full continuous load with resource/timing margin.

This is not a judgement that FINN cannot generate IP for XC7Z020. FINN explicitly supports Vivado IP Integrator (AXI-Stream FIFO in/out) creation for Xilinx parts. The **shell-integrated PYNQ/Zynq-7000 flows were retired from official support at Vivado 2024.2**; Fishball needs a custom AXI integration, software driver and benchmark harness. [F1]

## Hardware/HDL facts from the Fishball reference

The reconstructed design exposes two RX and two TX AD9361 channels via `axi_ad9361`; ADI core deserializes the LVDS bus to four 16-bit RX components (`I0,Q0,I1,Q1`), followed by optional 4-channel FPGA FIR decimation, `cpack` and `axi_ad9361_adc_dma`. The Linux driver and DT rely on the precise address map and IRQ wiring. This is *not* a clean generic AXI-Stream pipe at every point. [B1]

| Item | Reference value / evidence |
|---|---|
| Silicon | AMD/Xilinx XC7Z020, dual ARM A9 and 7-series PL [H1] |
| DSP48E1 count | 220 device total [H1] |
| Slice LUT | 53,200 total device LUTs [H1] |
| BRAM | 4.9 Mb nominal device block RAM [H1] |
| Fishball default DSP | **94 / 220** used with both RX paths FIR-filtered [B1][M1] |
| Fishball default LUTs | approx. **12,521 / 53,200** [B1][M1] |
| Reference worst timing slack | **+0.215 ns** for a constrained **4.000 ns RX clock** (build-specific) [B1] |
| Alternative stock filter | 72 DSP, but **channel-1 aliasing risk on decimation**; do **not** reclaim DSP by breaking RX2 [B1] |
| Clock domains | RX fabric `l_clk` constrained 250 MHz; PS/AXI 100 MHz; delay clock 200 MHz [B1] |
| Existing RX DMAC register address | `0x7C400000` [B1] |
| AD9361 core register base | `0x79020000` [B1] |

**Arithmetic only, not a floorplan:** 220−94 = **126 remaining DSP instances nominal**, and 53,200−12,521 = **40,679 nominal LUTs**. Place-and-route, timing paths, BRAM count, DMA channels, memory bandwidth, clock-domain crossings and reserved routing determine *usable* headroom. **No current mapped BRAM consumption for the proposed AI logic** was established by this research. The FPGA's nominal 4.9 Mb RAM is ~612.5 KiB before ECC/wiring overhead—tiny compared with a raw multi-second IQ ring. [H1]

**Timing is the main red flag.** Only +0.215 ns of reported slack exists in the published default constrained build; new logic sharing the hot ADC clock domain can fail timing even if LUT/DSP percentages appear roomy. Prefer registering pipeline stages, retiming across domains with proven CDC FIFOs, and running analysis clocks only as high as required. Some constraints may be conservative relative to actual RX sample rate, but relaxing them without cross-checking the I/O timing requirements is not acceptable.

## Candidate FPGA approaches — ranked by risk/value

| Rank | Approach | Benefits | Costs / proof required |
|---:|---|---|---|
| **1** | Passive power/occupancy estimator + counters | Tiny feature output, negligible outbound metadata, low-risk first accelerator | Exact taps, scale, valid timing, saturation, input reference and anti-loss backpressure |
| **2** | Optional DDC/FIR + feature FFT | Lower ARM DSP load and automatic detection over channels | DSP/BRAM cost, complex timing, spectral calibration and decimation fidelity |
| **3** | Side-channel capture/event ring + small AXI DMA | Precise event windows; ARM reads only interesting samples | Competes for DDR+AXI bus; valid trigger sample index/epoch; race/recovery tests |
| **4** | hls4ml-generated compact MLP/CNN IP | Quick architecture exploration of fixed-size low-precision inference | HLS/AXI wrappers, BRAM/clock reports, tool-version tests, real board transport |
| **5** | FINN quantized streaming network | More comprehensive low-bit IP optimization | Quantization/export, unsupported board shell, manual IPI integration and memory fabric |
| **6** | Custom RTL BNN/CNN inference | Fine-grained control over resource and datapath | Large verification and maintenance burden; only worth it after measurable model need |

### Where to tap: preserve stock IIO

```text
 AD9361 -> axi_ad9361 -> existing FIR/cpack -> existing RX DMA -> IIO/iiod/raw IQ
                   |                 |
                   +-- mirror valid I/Q --+--> CDC FIFO --> decimate/window/FFT -->
                                                    event/feature FIFO --> AXI/IRQ --> ARM
```

The diagram is **proposed**. In the reference design, `valid` is channel-sensitive and `cpack` uses a common strobe. A branch must use aligned `I0/Q0` and `I1/Q1` with documented latency and exactly counted valid samples. **Do not make classifier backpressure stall the ADC DMA**. On feature FIFO full, drop *feature records* with visible counters while native IQ continues. Keep different clocks and synchronization explicit. The reference RX FIR patch treats all four components identically; do not reintroduce earlier channel-1 aliasing by tapping only one half of the path. [B1]

### Minimum RTL feature engine

- Windowed **total power**: sum of `I^2 + Q^2` in wide saturating accumulators, record valid sample count, exponent/scale, fixed-point overflow count and source first/last index.
- **Threshold occupancy**: noise floor as a tracked estimate, threshold crossings with hysteresis and minimum burst width; threshold requires configuration per bandwidth/gain and cannot promise universal sensitivity.
- **Coarse spectral features**: FFT option when benefits justify its cost. Windowing, bin order, CIC/FIR transfer function, accumulation and noncoherent magnitude scale must be documented. Provide exact `window_id`, bin width and rates.
- **Event FIFO**: fixed record with channel ID, sample-index boundaries, center frequency snapshot (latched by ARM at config epoch), measured energy and quality flags. If absolute UTC needed, add a calibrated reference and uncertainty; FPGA cycle counter itself is not UTC.
- **Control**: versioned AXI-Lite register block for read-only build identity, enable, thresholds, FIFO fill, overflow counters, IRQ masking and user reset. Permission and RF tuning remain under Linux service authorization.

## FPGA/AI toolchains and compatibility

### FINN + Brevitas + QONNX

Brevitas quantization-aware training can export QONNX, which FINN can transform to hardware-layer graphs [F2]. FINN's 2024.2-oriented documentation says Zynq-7000 PYNQ-Z1/Z2 support is **retired/not CI-tested**, but `xc7z020` can still be targeted by generic Vivado IPI output (stitch IP). The emitted accelerator has AXI-Stream FIFO ports, **not** a Fishball driver, DMA node, kernel overlay or deploy-ready bitstream. Must integrate manually into upstream Tcl-based block design. [F1]

Practical work: train/quantize → export QONNX → FINN transformations → single-layer/small MLP stitched IP → AXI-Stream testbench → add IP to Vivado design → match `l_clk` or introduce CDC → configure feature buffer and AXI DMA or AXI-Lite registers → update DT and driver → hardware selftest. Keep FINN host synthesis tool container version pinned. If IP out-of-context timing passes, **full radio design implementation must still pass**.

### hls4ml (Vitis HLS preferred backend)

hls4ml supports models from Keras/PyTorch/ONNX with quantization/pruning and generates HLS C++/IP. Its maintainers recommend **Vitis** HLS backend for newer Xilinx work; old Vivado HLS backend targets the discontinued Vivado HLS 2020.1 compiler. Backend releases have version-specific simulation and synthesis limitations (notably convolution/RNN patterns); pin a known-good combination and test. `ReuseFactor` adjusts DSP throughput vs latency. [H2][H3]

For Fishball, a fixed 16–32 feature scalar MLP may be better than a raw-IQ CNN: model input can arrive via 100 MHz AXI/control domain and remain independent of 250 MHz-constrained ADC paths. Low-bit integer math is mandatory to explore 7020 scale; neural model weights/storage and large input windows consume scarce BRAM.

### Vitis AI / DPU: not a default for this generation

Current Vitis AI docs focus DPUCZDX8G on **Zynq UltraScale+** and Versal/Alveo families, not an ordinary plug-and-play Zynq-7020 deployment [V1]. Historical Vitis AI 1.3 documentation mentioned **Zynq-7000 devices** in a DPU table [V2]; this demonstrates legacy research paths, not contemporary supported A9 deployment. Do not market Fishball as having a supported modern Vitis AI NPU. Use custom HDL/HLS or FINN IP if justified.

### Vivado version and license caveat

AMD docs confirm XC7Z020 remains in Vivado-supported devices and show 2026.1 tiered licensing [V3]. A project that builds with 2020.2/2024.2 may not reproduce in the newest tool unmodified; devkit HDL Tcl constraints, older ADI IP, available versions, and license need local verification. Pin the **whole** Vivado and IP dependency graph and record versions in reports. Do not assume an `.xsa` generated in one version will reproduce identically in another.

## Integration plan, in full

1. Read upstream `firmware/src/hdl/projects/pluto/` Tcl/HDL and patches from a pinned commit; identify actual module ports for ADC signals and registered bus widths.
2. Save upstream utilization and timing reports (LUT, FF, DSP, RAMB18/RAMB36, WNS, unconstrained paths). Confirm published +0.215 ns slack locally.
3. Add **no-op mirror** and counter in the RX domain. Simulate both channels at full rate and under decimator active/inactive, checking signed 12→16 representation, `valid` strobes, 64-bit sample epoch/wrap and original DMA bit-exactness.
4. Insert clock-domain FIFO to independent analyzer clock. Verify both clock edges, reset order, FIFO overflow, boundaries, and no backpressure to the original RX DMA.
5. Add simple power engine, run synth/implementation, check timing and resource delta, then read control/status registers from Linux via a controlled driver. No `devmem` in product service.
6. Update DT for register region/IRQ; add kernel module or UIO where safe; do not reuse existing IIO register addresses or DMA interrupts.
7. Verify unmodified iiod works with both RX channels and TX boot-mute safety; run devkit `selftest`, continuous host streaming and stress load while analyzer is enabled/disabled.
8. Add FFT/DDC *only if CPU/streaming measurements* show a need. Then trial hls4ml or FINN tiny network with a completely separate functional reference.
9. Promote only when FF/LUT/BRAM/DSP timing margins are sustainable across multiple synthesis runs/temperature/clock configurations; preserve a recovery image.

## Acceptance matrix (all pending)

| Test | Requirement / policy |
|---|---|
| Bitstream timing | No failing setup/hold paths at explicit validated clocks; report full-device WNS/WHS |
| FPGA resource budget | Record *delta* for LUT, FF, DSP48, BRAM, AXI DMA and timing, not just free % |
| IQ fidelity | Hash/compare native output fixtures before/after analyzer insertion when sample-synchronous fixtures available |
| Streaming | No statistically significant reduction of sustained IIO delivered rate/no new drops at tested rates |
| Dual RX | Both I/Q pairs aligned and alias behavior maintained under RX decimation |
| Analyzer correctness | Fixed-point bit-exact agreement with host reference, including overflow/zero/maximum values |
| Feature loss | Overflow/drop counters monotonically increase; no silent reset without epoch change |
| Recovery | Disabling/resetting feature engine leaves IIO unaffected; watchdog cannot latch TX on |
| Model accuracy | Train/test split and calibrated unknown handling; end-to-end rate/latency measured on live IQ |
| Reproducibility | pinned source commits, tool versions, release bitstream SHA256 and test logs |

## Key unknowns

No new Fishball bitstream was generated; no Vivado tools were executed; exact free BRAM/route congestion and AXI HP bus headroom not established; no FINN or hls4ml generated model was synthesized for this board; no FPGA deep network performance claims are warranted. The clock relationship and CDC behavior require hands-on HDL and board verification. FPGA dynamic partial reconfiguration is **not** recommended as a first project; compatibility and recovery are more important than load-time demos.

## References

- [B1] Fishball block design, clocks, DMA map and FIR warnings: https://matsvandamme.github.io/fishball7020-fpga-devkit/block-design/
- [M1] Reproducible Fishball measured performance: https://matsvandamme.github.io/fishball7020-fpga-devkit/measured-performance/
- [H1] AMD Zynq-7000 device resources: https://www.amd.com/en/products/adaptive-socs-and-fpgas/soc/zynq-7000.html
- [F1] FINN supported hardware/retired Zynq PYNQ: https://finn.readthedocs.io/en/latest/getting_started.html
- [F2] FINN Brevitas export: https://finn.readthedocs.io/en/latest/brevitas_export.html
- [H2] hls4ml FAQ: https://fastmachinelearning.org/hls4ml/intro/faq.html
- [H3] hls4ml Vitis backend version-specific notes: https://github.com/fastmachinelearning/hls4ml/blob/main/docs/backend/vitis.rst
- [V1] Vitis AI current DPU target docs: https://docs.amd.com/r/en-US/ug1414-vitis-ai/Compiling-for-DPU
- [V2] Historic Vitis AI 1.3 Zynq-7000 mention: https://docs.amd.com/r/1.3-English/ug1414-vitis-ai/Vitis-AI-Compiler
- [V3] Vivado 2026.1 licensing/support: https://docs.amd.com/r/2026.1-English/ug973-vivado-release-notes-install-license/Device-Availability-by-Subscription-Tier
- [X1] Zynq FPGA manager Linux path: https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841645/Solution+Zynq+PL+Programming+With+FPGA+Manager
