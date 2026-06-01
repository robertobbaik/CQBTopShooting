---
tags:
  - handoff
  - fog-of-war
  - rendering
  - en
---

# Fog Of War Handoff

Korean: [[fog-of-war-handoff-ko]]

Date: 2026-05-09

## Related

- [[fog-of-war-en]]
- [[enemy-visibility-en]]
- [[code-review-2026-05-12-en]]

## Current State

- `AFogOfWarManager` is placed in the level.
- `UFogOfWarRevealComponent` is attached to the player character and enabled from `BeginPlay`.
- Post-process fog path is working at a prototype level.
- Debug colors should be disabled for the final post-process path.
- Fog texture alpha is the important channel:
  - `FogTexture.A = fog alpha`
  - `VisibleMask = 1.0 - FogTexture.A`

## Material Parameters

- `FogTexture`: Texture Object Parameter
- `FogWorldOrigin`: Vector
- `FogWorldSize`: Vector
- `FogTextureSize`: Vector or Float2
- `FogBlurTexels`: Scalar
- `FogContrast`: Scalar
- `HiddenDarkness`: Scalar

Suggested starting values:

- `FogBlurTexels`: `1.5` to `3.0`
- `FogContrast`: `0.8` to `1.4`
- `HiddenDarkness`: `0.05` to `0.2`

## Line Of Sight Debug

If cone occlusion does not react to walls or props:

1. Enable `bDrawLineOfSightDebug` on the player's `FogOfWarRevealComponent`.
2. Confirm rays turn red at blockers.
3. If rays stay green through objects, set those meshes to block `LineOfSightChannel`.
4. If mesh collision is too coarse, enable `bLineOfSightTraceComplex` temporarily for testing.

## Next Step

- [ ] Move more Fog UV calculation into HLSL if the material graph becomes hard to maintain.
- [ ] Tune blur radius, contrast curve, and hidden darkness.
- [ ] Decide whether previously seen and never seen areas should render differently.
