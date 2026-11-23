# ADR-0001 — Development Linux and production distribution gate

**Status:** ACCEPTED FOR EXPERIMENT, TARGET BLOCKED | 2026-11-21
**Sources:** [Agent 2 Linux study](../../research/agent2/ORCPLUTO_LINUX_DISTRIBUTION_STUDY.md), [Agent 1 firmware lineage](https://github.com/hardcoreerik/REA-Results/blob/research/orcpluto-fishball-hardware-20261121/FIRMWARE_LINEAGE_AND_COMPATIBILITY.md)

## Context

The candidate R1 has XC7Z020/AD9361 and board-specific FSBL, FPGA bitstream, device tree, ADI DMA/IIO and safe TX controls. Upstream devkit documents Debian 13 armhf with ADI-derived Linux 6.12 and pinned iiod 0.26 for **matching published hardware**, not necessarily our exact unit. Factory Buildroot/5.15 is a control, not a destination or safely interchangeable bitstream.

## Decision

1. For development, **conditionally adopt** the exact-board *reconstructed* Linux 6.12 + Debian 13 armhf stack, pinned at devkit commit `c5611717e2144a978ab22cf63a20171eea675557` and its exact XSA/FSBL/U-Boot/kernel/DT/package hashes after inventory. Use a **separate SD card** only after recovery gate. Do not copy the live vendor rootfs or rely on generic `zynq_pluto_defconfig`.
2. Maintain standard libiio/iiod + USB gadget, RX/TX attribute ABI, network access and safe TX boot defaults, with continuous regression against baseline and the existing `esp-pluto-sdr` host driver.
3. Buildroot is a **possible production distribution**, not an approved implementation. Do not initiate the migration before P1–P4 CPU/DDR/stream measurements, persistent state/update rollback design, security/SBOM/lifecycle analysis and evidence of why Debian is insufficient.
4. Yocto/meta-xilinx may be reconsidered when multi-SKU fleet maintenance outweighs BSP/recipe overhead. Neither Buildroot nor Yocto is assumed more deterministic until measured.

## Required release gate

- Exact physical PCB/package/PA population/boot mode and recovery evidence.
- Cold-boot proven device-tree, DMA/2RX scan layout, iiod 0.26, Ethernet and USB with matching board FPGA image.
- A/B results against baseline on the **same board**, at identical sample/format conditions, with retained logs and safe TX checks.
- License audit: devkit GPL, ADI mixed HDL, Linux/U-Boot, package SBOM and installer permissions.

**State:** upstream-documented, proposed for our hardware; no target build or boot has been executed here.
