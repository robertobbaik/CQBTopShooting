# Fog Of War

Tags: #system #fog-of-war #rendering #en

Korean: [[fog-of-war-ko]]

## Purpose

Fog Of War hides unrevealed map areas and exposes only the player's current sight region. The current implementation uses a CPU-written alpha texture and a post-process material.

## Current Implementation

- `AFogOfWarManager` owns the runtime transient `FogTexture`.
- `UFogOfWarRevealComponent` registers reveal sources with the manager.
- `FogAlpha` stores visibility state:
  - `HiddenAlpha`: never visible
  - `ExploredAlpha`: previously visible
  - `VisibleAlpha`: currently visible
- The post-process material samples the alpha texture and computes `VisibleMask = 1.0 - FogAlpha`.
- Cone reveal supports line-of-sight traces and feathered edges.

## Related Code

- `Source/TopGunShooting/FogOfWarManager.h`
- `Source/TopGunShooting/FogOfWarManager.cpp`
- `Source/TopGunShooting/FogOfWarRevealComponent.h`
- `Source/TopGunShooting/FogOfWarRevealComponent.cpp`

## Related Docs

- [[fog-of-war-handoff-en]]
- [[enemy-visibility-en]]
- [[code-review-2026-05-12-en]]

## Known Risks

- `RevealTriangle` drops the whole triangle when any vertex is outside FogWorld.
- The manager uploads the full fog texture every update.
- Reveal source registration can fail if the manager is not present during `BeginPlay`.

## Next Work

- [ ] Clamp or clip cone triangles at FogWorld bounds.
- [ ] Profile full texture uploads before increasing texture size.
- [ ] Consider dirty rect upload after gameplay behavior is stable.
