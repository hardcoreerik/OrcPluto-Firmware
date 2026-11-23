# OrcPluto research — evidence preserved

Research is stored separately from firmware images, boot assets, RTL, portable OrcStream wire definitions and release artifacts. A research report or candidate benchmark **does not** establish working OrcPluto target firmware.

## Research workstreams

- **[Agent 2 — embedded Linux, Cortex-A9 ML, FPGA DSP and signal classification](agent2/README.md):** six in-depth studies, plus follow-up actions and historical integration handoff from 2026-11-21. Development Linux candidate: pinned ADI Linux 6.12 + Debian 13 armhf *after* the exact-board recovery gate. Buildroot remains a conditional production alternative.
- **[AI RF spectrum intelligence](ai-rf-spectrum-intelligence/README.md):** supplemental 2026-11-22 AI/RF brainstorming archive, proposing spectrum memory, context-aware detection, temporal features, receiver health and protocol-confirmed feedback. The historical binary bundle is [Artifical_Intelligence_RF_research.zip](Artifical_Intelligence_RF_research.zip); the spelling is retained for provenance. Its index lists 13 internal files and a checksum; the ZIP **was not independently unpacked or hash-verified in the 2026-11-22 GitHub text audit**.
- **[Complete 21-file Markdown reconciliation](../docs/reviews/ALL_MARKDOWN_REVIEW_2026-11-22.md):** records every existing tracked Markdown document on main and the integration branch, decisions, remaining contradictions and unread binary-archive limitations.

## What graduates from research

Proposals graduate only after explicit ownership, exact-board prerequisites, evidence grade, a reproducible baseline, test fixtures, pass/fail criteria, compatibility and safety regression, and artifact provenance. Current engineering authority is in [PROJECT_TRUTH.md](../PROJECT_TRUTH.md), [ARCHITECTURE.md](../ARCHITECTURE.md), [ROADMAP.md](../ROADMAP.md) and [STATUS.md](../STATUS.md). Retain historical reports intact; do not confuse prior session handoff prompts with the live architecture.

OrcStream owns portable protocol, EVENT and codec semantics; this firmware owns only actual board support and optional device-local implementations. Standard IIO/iiod, native IQ and RX-first recovery remain non-negotiable.
