# TopGunShooting Docs Home

Tags: #index #topgunshooting #en

## Current Focus

- [[enemy-visibility-en|Enemy Visibility]]
- [[fog-of-war-en|Fog Of War]]
- [[code-review-2026-05-12-en|Latest Code Review]]

## Systems

- [[fog-of-war-en]]
- [[enemy-visibility-en]]
- [[character-base-en]]
- [[peek-door-top-down-cqb-en]]

## Reviews

- [[code-review-2026-05-12-en]]

## Decisions

- [[adr-2026-05-12-visible-mask-enemy-rendering-en]]

## Handoff

- [[fog-of-war-handoff-en]]

## Writing Rules

- Every major document has a Korean `-ko` file and an English `-en` file.
- Date-based review notes go in `01-reviews/`.
- Living system explanations go in `02-systems/`.
- Design decisions go in `03-decisions/` as ADRs.
- Temporary handoff notes go in `04-handoff/`.
- Images and binary attachments go in `assets/`.

## Review Backlog

- [ ] Fix FogWorld boundary clipping in `AFogOfWarManager::RevealTriangle`.
- [ ] Cache `AFogOfWarManager` inside `ABaseEnemy`.
- [ ] Verify Enemy rendering in PIE against the visible mask.
- [ ] Decide whether Enemy visibility should sample one point or bounds.
