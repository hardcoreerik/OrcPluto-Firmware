# Agent 2 — Follow-Up Actions & Acceptance Gates

**Status:** tasks to schedule; no physical-board acceptance has occurred in this research pass.  
**Primary owner:** OrcPluto Main coordination session, delegating hardware/FPGA to Agent 1, codec/wire semantics to Agent 3, CPU/FPGA ML to Agent 2 and local operation to the repo owner.

| Priority | Task | Owner / dependency | Output | Acceptance gate |
|---|---|---|---|---|
| P0 | Pin exact Fishball Rev.A/PlutoSky Zynq part, DDR size, USB/Ethernet/microSD wiring and any variant discrepancies | Agent 1 + physical device | Inventory and photos/boot logs/version manifest | Device identity and hardware capabilities backed by live data |
| P0 | Secure stock firmware, `BOOT.bin`, DT and SD partitions; document known-good recovery | Agent 1 + owner | Hashes and restoration procedure | Recovery rehearsed or explicitly blocked before risky changes |
| P0 | Confirm stock IIO service, exact scan format and sample-rate/LO/rate constraints | Agent 1 / local | IIO inventory and safe RX capture trace | Stock baseline reproducible; TX muted |
| P1 | Reproduce ADI 6.12 + Debian 13 armhf board-specific boot and compatibility | Main + Agent 2 | Locked build manifest, kernel/DT/libiio/iiod versions, smoke test | Native IIO RX and standard host compatibility verified |
| P1 | Log current source-sample fidelity and streaming baselines | Main / local | Test log, source/net-loss distinction, CPU/RAM/thermal numbers | Repeatable sample test at feasible rates, without hidden decimation |
| P1 | Make receive-only ARM detector against captured data first, then real-time | Agent 2 | C++ code, fixtures, local log and versioned event schema | Genuine AD9361 known source detected and unknown classified as unknown |
| P1 | Record first sensor event with sample-index evidence window | Agent 2 | NDJSON event and SigMF snippet if retained | No invented UTC timestamp; explicit units and certainty |
| P2 | Establish simultaneous IQ + features architecture | Agent 1 + 2 | DMA/AXI ownership diagram and loss/overload matrix | Original IIO remains stable; backpressure sheds features first |
| P2 | Synthesize passive FPGA DSP tap for exact bitstream toolchain | Agent 1 + 2 | Vivado reports, utilization and timing slack | Timing closure, IIO unchanged, bit-exact reference comparison |
| P2 | Port/test selected quantized ARMv7 inference runtime | Agent 2 | Compiler flags, linked runtime, model test and CPU/PSS/latency logs | Runs on Cortex-A9; p50/p95/p99 with raw IQ still acquired |
| P2 | Evaluate real-world datasets and confidence calibration | Agent 2 | Capture-disjoint test splits, unknown/fp curves and dataset license record | Unknown classes not forced into known protocol labels |
| P2 | Define generic events in OrcStream without hard-coding Pluto | Agent 3 + Main | ADR, schema fixture, conformance plan | Receiver independent of OrcPluto and OrcSDR |
| P3 | Validate 24-hour streaming/classification soak and process-fault recovery | Main + local | Logs, thermal/power results and continuity report | No silent sample loss; faults observable and recoverable |
| P3 | Production Linux, security/update/license decision | Main | ADR, BOM/SBOM and update/rollback design | Maintenance and recovery demonstrated; not just a smaller image |

## Preexisting findings not to overstate

- Documented **vendor/devkit** observations are not measurements on the owner's exact board.
- The Fishball upstream FPGA DSP usage is a **starting resource estimate**, not uncommitted LUT/BRAM/timing headroom for a new accelerator.
- FINN/hls4ml builds may be synthetically feasible but lack an integrated, timed, independently tested Fishball design.
- ARMv7 Debian package support does not imply a prebuilt working ARMv7 LiteRT/ONNX/ncnn target package.
- Models trained on RadioML/synthetic captures cannot be advertised as real-world protocol recognition without live, capture-disjoint validation.
- Current OrcStream capability must be checked at merge time; as of the cited November 10 project truth, events, jobs and native source integration were proposals.

## Minimal runnable experiment (proposed, not performed)

1. On the physical board, gather read-only identity and known-good IIO baseline (save command outputs with time/version and source file provenance).
2. Under a **controlled receive-only test**, acquire a short IQ recording from one channel; preserve native scan format and source rate.
3. Run an FFT/power estimate and threshold/burst detector **on the ARM**, with 8192- or another benchmarked sample window, outputting versioned NDJSON plus an explicit `unknown` option. The window size is a test input, not an RF protocol guarantee.
4. Compare detector event sample indices, true injected test windows, false positives, CPU/PSS and baseline acquisition continuity. Record p50/p95/p99 of full window-to-event latency, not forward-pass alone.
5. Repeat with ongoing IQ transport and assess contention only **after** single-owner capture architecture is demonstrated. Stop and record `BLOCKED` if two readers conflict; do not silently consume or overwrite IIO buffers.
6. Place logs, build manifest and hash-tagged IQ fixtures next to the run directory; submit them to Main and Agent 1 for FPGA acceleration planning.

## Local PC / target hardware handoff requirements

The Main session should provide concrete commands *adapted to the owner's verified board and OS*, with no guessed device addresses, GPIO pin numbers or boot partitions. Its run/test instructions must specify: toolchain/version installation; read-only inventory; backup and recovery; compiling or installing binaries on target; precise capture/launch/stop commands; expected output and log filenames in the directory the program is run from; abnormal failure handling; how to bundle logs/IQ provenance for review; and an uninstall/rollback procedure.