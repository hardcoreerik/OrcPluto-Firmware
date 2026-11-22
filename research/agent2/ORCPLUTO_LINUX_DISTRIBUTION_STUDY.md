# OrcPluto Linux Distribution Study

**Agent 2 research | 2026-11-21 | Status: research/recommendation, NOT tested on OrcPluto target**

## Executive decision

- **Development image:** **Fishball devkit's Linux 6.12 (Analog Devices-derived) + Debian 13 `armhf`**. It already runs on the precise *family* of Fishball7020/PlutoSky R1 hardware and gives a practical SDK/package environment. Pin the devkit revision, kernel, userspace package manifest, `.xsa`/bitstream and device tree.
- **First prototype/field trial:** retain the Debian baseline (minimal installed services and pinned packages). Avoid prematurely porting to a new distribution while hardware identity, IQ continuity and AI budget are unknown.
- **Production candidate:** **custom, reproducible Buildroot appliance** with the *validated* modern ADI kernel/device tree, libiio/iiod, C++ detector/classifier, limited network services, read-only root and supervised recovery. This is a recommendation, **not an existing supported Fishball image**. Build the migration only after a successful Debian prototype and evaluate whether its lifecycle cost is worthwhile.
- **Alternative at commercial scale:** Yocto/OpenEmbedded with AMD `meta-xilinx`, especially if BSP variants, CVE tracking, package-level licensing/SBOM and fleet update mechanics justify its greater engineering burden. No off-the-shelf Yocto machine configuration specifically for this board has been verified.

**Hardware/fork identity gate:** the upstream devkit explicitly covers `FISH Ball PlutoSDR Rev.A (Z7020-AD9361)`, *not* every Zynq Pluto clone. Its makers identify PlutoSky R2 as a different board. Before trusting any build, capture `iio_attr -S`, `/proc/cpuinfo`, `/proc/meminfo`, `/etc/os-release`, kernel and DT hashes, `/opt/VERSIONS`, storage partitions, and the boot `uEnv.txt`/`BOOT.bin` provenance. [F1][F2]

## Verified starting points and non-equivalence

| Aspect | Factory reconstruction | Modern devkit |
|---|---|---|
| Kernel | vendor-derived 5.15 | ADI-derived Linux 6.12 LTS |
| Root filesystem | Buildroot + BusyBox, RAM-backed | Debian 13 (`trixie`) `armhf`, `systemd`, SD-card ext4 |
| Boot partition | SD boot assets | FAT boot partition plus ext4 root (`p2`) |
| FPGA | Vivado-buildable reconstructed design | existing pinned `.xsa` bitstream; same FPGA design |
| IIO endpoint | libiio/iiod, Pluto-style | iiod 0.26 pinned by devkit compatibility logic |
| Software iteration | rebuild image; small footprint | apt + cross-compilation; persistent storage |
| Risk | old kernel, ephemeral root, constrained packages | SD wear, systemd/apt complexity, broader attack surface |

The project describes BootROM → FSBL (sets up DDR, loads PL) → U-Boot → kernel and DT → rootfs; `BOOT.bin` contains a critical bitstream/boot-chain component. Modern U-Boot is **still 2016.07**: the project explicitly notes no native A/B rollback for the ROM's fixed `BOOT.bin`. The modern kernel requires `CONFIG_NAMESPACES` and `CONFIG_AUTOFS_FS` for systemd. [F1][F2][F3]

**Do not migrate iiod carelessly.** The devkit preserves cyclic transmit via the high-speed libiio path and pins Debian libiio 0.26; its documented compatibility contract includes TCP 30431, USB gadget behavior, identity attributes, transmitter safety, and self-tests. The factory's old `iiod` is not a safe binary copy into Debian 13 because of `libaio`/time64 ABI differences. [F3]

**Boot and storage:** published modern images use a 128 MB FAT partition containing `BOOT.bin`, kernel, DT and U-Boot environment, plus ext4 root on the remaining SD card; exact card size and flash layout on the owner's unit remain unverified. The vendor Buildroot root filesystem runs from RAM and loses ordinary writes at reboot. [F4][F5]

## Ranked decision matrix

Scoring: 1=poor, 5=strong; subjective *architectural assessment*, not benchmark. Columns: board readiness (B), package/ML convenience (M), stream determinism potential (S), maintainability (L), reproducibility (R), production footprint (P). Scores are not actual hardware measurements.

| Rank for **development** | Strategy | B | M | S | L | R | P | Notes |
|---:|---|---:|---:|---:|---:|---:|---:|---|
| **1** | Existing devkit: 6.12 + Debian 13 armhf | 5 | 5 | 3 | 4 | 4 | 2 | Already documented for matching Fishball family; practical iterative experimentation |
| 2 | Custom Buildroot + tested ADI 6.12 | 2 | 2 | 5 | 3 | 5 | 5 | Best lean target, but port/packaging work not yet done |
| 3 | Yocto/meta-xilinx + custom machine | 2 | 3 | 4 | 3 | 5 | 4 | Strong long-term process, highest initial integration cost |
| 4 | Factory 5.15 + RAM Buildroot | 5 | 1 | 4 | 2 | 3 | 5 | Reference/recovery only; do not confuse factory fidelity with maintainability |
| 5 | Another ARMv7 distro (e.g., generic Debian image) | 1 | 4 | 2 | 2 | 2 | 2 | No verified Fishball DT, FPGA, IIO or recovery advantage |

**Production ranking conditional on prototype success:** (1) custom Buildroot; (2) Yocto; (3) locked-down Debian; (4) factory Buildroot. This is not an assertion that Buildroot automatically improves stability; it only reduces variables if the team actually maintains updates and tests.

## Distribution-specific engineering assessment

### A. Existing vendor Buildroot 5.15

Good as a recovery reference and to compare IIO, RF calibration, USB and startup behavior. Low process count and boot from RAM may simplify testing. Poor rapid development UX: no apt, minimal tools, non-persistent filesystem; old security baseline. Preserve unmodified SD image and hash it. If the newer Linux 6.12 baseline changes behavior, use vendor target as a control rather than importing its binaries. [F1][F3]

### B. Debian 13 armhf + ADI Linux 6.12 — **selected development environment**

Debian supports ARMv7 hard-float, satisfying Cortex-A9's general ISA/ABI requirements, and the Fishball-specific reconstruction documents the board integration. Verify all installed Python/C++ and inference dependencies on `armhf`; support for Debian itself does **not** establish support for a particular ML wheel. Use native compilation only for small patches; cross-compile heavyweight runtimes from a capable x86-64 host. Keep capture and inference programs lean and reproducibly packaged; no obligatory Python in the real-time process. [F1][D1]

Potential production problems: root SD card writes, large dependency set, post-install drift, prolonged boot/daemon startup, general purpose service exposure. Countermeasures: pin repo snapshots and source hashes, minimal package list, turn off unneeded units, read-only service runtime, log rotation, atomic application bundle and known-good SD image.

### C. Custom Buildroot 6.12 — **proposed production environment**

Use an external Buildroot tree (`BR2_EXTERNAL`) with Fishball board-specific defconfig and version-pinned kernel/bootloader/DT and package recipes. Decide if rootfs should be ext4 on SD (with separate writable data/log partition) or squashfs/readonly; the factory's RAM root is not required for the new image. Build a complete reproducible image, not `apt install` steps. Buildroot is a root-filesystem **build system**, not a binary-package update distribution; security updates require rebuilding/redeploying whole images. [B1]

Minimum packages: busybox, dropbear/sshd only for debug images, libiio/iiod, AD9361 related libraries as required, C++ runtime, selected NN runtime or tiny predictor, a supervised `orcsdr-edge` service, network/USB gadget support, safe RF quiesce, metrics and a signed updater. `systemd` not essential if simpler init/supervision meets operational needs.

### D. Yocto / PetaLinux-style

AMD's `meta-xilinx` supports Zynq-7000 reference machines including ZC702, and AMD documentation states a shift toward native Yocto tooling rather than relying on PetaLinux BSPs. Fishball still needs its own machine, device tree, boot-chain and ADI/iiod compatibility validation. Yocto can excel at multi-board product lines, reproducible layers and SPDX/SBOM workflows, but can be operationally costly for a first device. [Y1][Y2]

### E. Other distributions

Generic `armhf` Debian-derived systems may run in principle but lack a verified board-specific boot chain and device tree. Ubuntu's contemporary ARM focus is commonly arm64, not a compelling Fishball/A9 route. Do not treat Alpine, Arch ARM or PYNQ images as ready without evidence of Zynq-7000/this-board kernel support, package ABI and IIO datapath; no candidate surpassed the existing Fishball reconstruction in this research pass.

## FPGA, low-latency DSP, and kernel policy

- Retain ADI's `ad9361`, IIO buffer/DMA and PL binding configuration. A kernel package alone cannot replace the correct DT and live bitstream.
- Program PL through the existing FSBL/`BOOT.bin` for the initial milestone; runtime FPGA Manager with DT overlays is *possible on Zynq-7000* but dangerous if an acquisition device still owns live DMA mappings. Design controlled stop/unbind, bitstream load, overlay, probe, self-test, new stream epoch and rollback, before enabling it. [X1]
- Prefer bounded C/C++ acquisition queues, pinned/affined threads, memory preallocation, standard Linux `SCHED_FIFO` only with measured starvation protection, and benchmark kernel scheduling jitter. `PREEMPT_RT` is **not yet evaluated** for this ADI 6.12 fork and should not be assumed compatible. Avoid GPU/NPU promises.
- Snapshot `iiod` and USB gadget compatibility before replacing services. Preserve default transmitter mute and existing safety order.

## Security, maintenance, licenses and update design

1. Maintain SBOM with exact git revisions for kernel, ADI HDL, devkit patches, toolchains, libiio, runtime, models and rootfs packages.
2. Review GPL-2.0 devkit patches/scripts separately from upstream kernel GPL, libiio licensing, proprietary AMD tools/IP and model/dataset licenses. Keep OrcStream generic SDK licensing independent; comply with derived firmware/HDL terms. [F1]
3. Default RX-only with TX muted; protect retune/reconfigure/transmit behind authorization. Disable or secure demo credentials, unsecured iiod/control ports and SSH before network deployment.
4. Signed release manifest covering `BOOT.bin`, DT, kernel, rootfs, app and model; compatibility identifier must be checked before applying. Do not claim existing U-Boot provides atomic rollback.
5. Stage SD image update to inactive location with physical recovery card or verified alternate boot arrangement. Boot-health gate requires IIO readback, RF safety, continuous IQ and model daemon watchdog. If a single FAT `BOOT.bin` remains the boot source, preserve an offline rescue path and avoid unsafe in-place overwrite.
6. Validate USB+Ethernet+IIO interoperability, dual-RX, identity persistence, thermal stability and all built images against board revisions.

## Decision gates and missing numbers

**Must measure on owner's hardware:** actual revision, DDR size, kernel memory reserve/CMA, `MemAvailable` under full RX load, image size, boot-to-IIO-ready, boot-to-AI-ready, CPU headroom at 2.4/5/10 MS/s, storage write rate/lifetime, jitter, RX overrun and USB/Ethernet service reliability. No verified measurements were performed by this agent.

**Migration gate:** only adopt custom Buildroot after it meets or beats Debian on RF/IIO compatibility, code reproducibility, stream throughput, crash recovery and security update process. Otherwise a pinned Debian appliance may be the cheaper, safer release choice.

## Sources (accessed 2026-11-21)

- [F1] Fishball7020 devkit README: https://github.com/matsvandamme/fishball7020-fpga-devkit/blob/main/README.md
- [F2] Hardware inventory and 1 GiB DDR evidence: https://matsvandamme.github.io/fishball7020-fpga-devkit/hardware/
- [F3] Why Debian, libiio and boot contract: https://matsvandamme.github.io/fishball7020-fpga-devkit/debian-rootfs/
- [F4] SD partitions / rootfs build: https://matsvandamme.github.io/fishball7020-fpga-devkit/build/rebuild-the-debian-root/
- [F5] Boot chain: https://matsvandamme.github.io/fishball7020-fpga-devkit/how-it-works/
- [D1] Debian 13 armhf installation guide: https://www.debian.org/releases/trixie/armhf/
- [B1] Buildroot manual: https://buildroot.org/downloads/manual/manual.html
- [Y1] AMD Yocto machine support: https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/2907766785/Yocto+Project+Machine+Configuration+Support
- [Y2] AMD Yocto 2026 Zynq recipe guidance: https://edf.docs.amd.com/en/latest/yocto/minimal-yocto-project-configuration.html
- [X1] Zynq-7000 Linux FPGA Manager: https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18841645/Solution+Zynq+PL+Programming+With+FPGA+Manager