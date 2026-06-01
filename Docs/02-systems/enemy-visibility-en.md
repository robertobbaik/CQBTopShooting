# Enemy Visibility

Tags: #system #enemy #fog-of-war #rendering #en

Korean: [[enemy-visibility-ko]]

## Purpose

Enemy actors should render only while they are inside the Fog Of War visible mask. This prevents enemy meshes from being drawn through the post-process fog.

## Current Implementation

- `ABaseEnemy` periodically checks its visibility against `AFogOfWarManager`.
- The sampled position is `GetActorLocation() + FogVisibilitySampleOffset`.
- If the sampled point is visible, `SetActorHiddenInGame(false)` is applied.
- If the sampled point is not visible, `SetActorHiddenInGame(true)` is applied.
- If no Fog manager exists, `bRenderWhenFogManagerMissing` controls fallback behavior.

## Related Code

- `Source/TopGunShooting/BaseEnemy.h`
- `Source/TopGunShooting/BaseEnemy.cpp`
- `Source/TopGunShooting/FogOfWarManager.cpp`

## Related Docs

- [[fog-of-war-en]]
- [[adr-2026-05-12-visible-mask-enemy-rendering-en]]
- [[code-review-2026-05-12-en]]

## Design Notes

- This currently hides rendering only. AI, collision, and gameplay state continue running.
- The visible check uses `FogAlpha < ExploredAlpha`, so feathered visible edges count as render-visible.
- Actor-wide hidden state also hides child visual components such as widgets or VFX.

## Known Risks

- Each update currently searches for the Fog manager.
- One sample point can flicker near a visibility edge.
- Large enemies may be partially visible while the sample point is still hidden.

## Next Work

- [ ] Cache `AFogOfWarManager` in `ABaseEnemy`.
- [ ] Add optional bounds-based multi-point sampling.
- [ ] Decide whether UI/VFX should hide with the actor or separately.
