# OrcPluto-Firmware — Status and Evidence Ledger

**Updated:** 2026-11-21; scope: integration of Agent 2 research. Treat this as current work status, not firmware release notes.

| Item | Evidence label | Reproducible support | Next proof |
|---|---|---|---|
| Offline C++ CS16 detector | REPRODUCED_OFF_TARGET | [11 host assertions + ASan/UBSan replay](experiments/edge-detector/OFFTARGET_TEST_EVIDENCE.md), Linux x86-64 g++ 14.2.0; generated logs saved in launch folder | ARMv7+real AD9361 input, end-to-end latency, false-alarm and coexistence proof |
| Agent 2 six studies | UPSTREAM_DOCUMENTED | [research/agent2/README.md](research/agent2/README.md), published on `main` at `b9c69caa3bea28f7ef758f79185fe38c959b3618`; no target tests | Compare executable feasibility with actual R1 |
| Agent 1 exact-board reconnaissance | UPSTREAM_DOCUMENTED | [Independent REA research branch](https://github.com/hardcoreerik/REA-Results/tree/research/orcpluto-fishball-hardware-20261121), pinned upstream `c5611717e2144a978ab22cf63a20171eea675557` | Actual unit inventory, firmware/board/PA/boot hashes |
| Published FPGA reference | UPSTREAM_DOCUMENTED | 94/220 DSP48, 12,521/53,200 LUTs, +0.215ns WNS **reported by other R1 build**; see Agent 1 resource matrix | Fresh full route on exact checkout/XSA; baseline+mod comparisons |
| Existing USB Pluto driver | UPSTREAM_DOCUMENTED / REPORTED | [esp-pluto-sdr](https://github.com/hardcoreerik/esp-pluto-sdr) 2.4–5 MS/s class tests on ESP32-P4; Fishball firmware used | Our firmware compatibility and error/restart regression |
| OrcStream protocol implementation | PROPOSED / EXPERIMENTAL IN OTHER BRANCH | [OrcStream PROJECT_TRUTH](https://github.com/hardcoreerik/OrcStream/blob/main/PROJECT_TRUTH.md), main is docs, C99 drafts unmerged | Independent event/codec contract, test vectors and producers |
| Agent 3 compression research | BLOCKED | No compression-named OrcStream research branch located on inspection date | Read actual Agent 3 commit/handoff when available |
| Our firmware image and live recovery | BLOCKED | No bootable OrcPluto image or own board validation | P0/P1 |
| Target classifier/ncnn/TFLite | BLOCKED | No ARMv7 target binary or runtime traces in Agent 2 report | P3/P6 |
| FPGA feature engine | PROPOSED | Side-tap diagram, source-referenced patch analysis only | P5 synthesis, timing/CDC and live IQ proof |
| IQ+feature concurrent service | PROPOSED | Competing independent IIO readers **not validated** | P4 single owner or actual PL tap |
| Native OrcStream EVENT or IQ producer | BLOCKED | No stable wire EVENT nor native producer | OrcStream conformance and source proof |

## Cross-agent conflicts to resolve

1. **Board identification:** 'Fishball R1' and XC7Z020-CLG400 are strong upstream matches, but the actual owner's package, speed grade, PA population and USB/clock routing remain unverified. R2 artifacts are incompatible.
2. **Remaining FPGA resources:** nominal 126 DSP and 40,679 LUT *arithmetic residual* is not usable headroom; the published +0.215 ns WNS is slender. Agent 2 must not claim a classifier fits from these counts.
3. **Distribution:** Agent 2 prefers Debian 13 + ADI 6.12 for development and Buildroot as a *candidate* for production. Agent 1 confirms a matching published family build, not our own target acceptance. No premature distro migration.
4. **Timing:** matching rates and a 64-bit FPGA counter do not establish accurate UTC or inter-board RF phase coherence; both RX chains share RX LO.
5. **Capture ownership:** parallel IIO clients may contend for one DMA buffer; no claimed simultaneous live IIO+ML until a single fanout owner or verified passive FPGA tap exists.
6. **Codec and EVENT version:** Agent 3 final results absent; current OrcStream main has no registered EVENT IDs or Smart IQ bitstreams. Local NDJSON is experimental, not an OrcStream schema.
7. **Throughput:** 20–23 MB/s USB host observations are not maximum guaranteed on our firmware. Raw 1RX CS16 at 61.44 MS/s = 245.76 MB/s and cannot fit USB2 without substantial data reduction, which may be lossy or selective.
8. **TX safety and licenses:** PA-equipped hardware may emit at boot before software mute; source packages and HDL have different licensing requirements. No OTA or unverified flash.

## Test execution table

| Test | Result | Explanation |
|---|---|---|
| Agent 2 live board run | NOT EXECUTED | Agent 2 performed research only |
| RTL synthesis/place-route | NOT EXECUTED | No Xilinx toolchain and board HDL checkout in this session |
| ARMv7 ncnn or LiteRT on R1 | NOT EXECUTED | No target-ABI execution |
| IIO acquisition + AI coexistence | NOT EXECUTED | Single owner not implemented; no lab radio access |
| Firmware image build, SD boot, flash | NOT EXECUTED | Device-specific recovery gate incomplete |
| OrcStream EVENT wire integration | NOT EXECUTED | No frozen standard or native producer |
