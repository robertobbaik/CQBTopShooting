# ADR 2026-05-12: Visible Mask Based Enemy Rendering

Tags: #adr #enemy #fog-of-war #rendering #en

Korean: [[adr-2026-05-12-visible-mask-enemy-rendering-ko]]

## Status

Accepted for current prototype.

## Context

Enemies were still rendered even when they were outside the Fog Of War visible mask. The post-process fog darkened the screen, but it did not prevent enemy meshes from being drawn.

## Decision

`ABaseEnemy` controls its render visibility by querying `AFogOfWarManager::IsWorldLocationCurrentlyVisible`.

The Enemy actor is hidden while its sample point is outside the current visible mask and shown while inside it.

## Consequences

- The implementation is simple and works with existing Fog data.
- AI and collision continue running while the enemy is hidden.
- Actor-wide hidden state hides child visual components too.
- Enemy count may expose a manager lookup cost until manager caching is added.
- Single-point sampling can flicker near Fog boundaries.

## Follow-Up

- [ ] Cache the Fog manager in `ABaseEnemy`.
- [ ] Evaluate bounds-based sampling.
- [ ] Decide whether hidden enemies should continue AI perception updates.
