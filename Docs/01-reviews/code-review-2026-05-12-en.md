---
tags:
  - review
  - fog-of-war
  - enemy
  - character
  - en
---

# Code Review - Character / Enemy / Fog Of War Structure

Korean: [[code-review-2026-05-12-ko]]

Date: 2026-05-12  
Target state: current working tree

## Related Docs

- [[fog-of-war-en]]
- [[enemy-visibility-en]]
- [[character-base-en]]
- [[adr-2026-05-12-visible-mask-enemy-rendering-en]]

## Scope

- `AFogOfWarManager`
- `UFogOfWarRevealComponent`
- `ABaseCharacter`
- `ABaseEnemy`
- `ANormalEnemy`
- `TopGunShooting.Build.cs`

## Overall Assessment

The direction is sound. Fog reveal calculation, post-process rendering, and Enemy visibility control are separated enough to support future extension. The main risk is that the current implementation relies on full CPU texture uploads and per-Enemy tick/query work, which can become a bottleneck as map size and Enemy count grow.

## Findings

### Medium: Cone reveal can be dropped near FogWorld boundaries

Related locations:

- `Source/TopGunShooting/FogOfWarManager.cpp:449`
- `Source/TopGunShooting/FogOfWarManager.cpp:454`

`RevealTriangle` drops the entire triangle when any point fails `WorldToFogPixel`. Near FogWorld boundaries, this can leave visible regions unrevealed, and Enemy rendering based on the visible mask inherits the same error.

Recommended fix:

- Short term: clamp outside points into the FogWorld rectangle.
- Medium term: clip triangles against the FogWorld rect before rasterizing.

### Medium: Full fog texture upload can become a performance bottleneck

Related locations:

- `Source/TopGunShooting/FogOfWarManager.cpp:530`
- `Source/TopGunShooting/FogOfWarManager.cpp:550`

`UploadTexture` converts all `FogAlpha` data into `TexturePixels` and uploads the whole texture every update. Even at 1024x1024, this refreshes roughly 4MB of pixel data multiple times per second.

Recommended fix:

- Track changed pixel ranges as dirty rects.
- Upload only changed rects.
- Profile with Unreal Insights or stats before raising texture resolution.

### Medium: Each Enemy repeatedly searches for the Fog manager

Related locations:

- `Source/TopGunShooting/BaseEnemy.cpp:31`
- `Source/TopGunShooting/BaseEnemy.cpp:37`

`ABaseEnemy::UpdateFogRenderVisibility` calls `AFogOfWarManager::Get(this)` every update. Since that path searches actors, it adds avoidable cost as Enemy count grows.

Recommended fix:

- Cache the manager as `TWeakObjectPtr<AFogOfWarManager>` in `BeginPlay`.
- Re-query only when the cached pointer becomes invalid.

### Medium: Enemy visibility uses a single sample point

Related locations:

- `Source/TopGunShooting/BaseEnemy.cpp:39`
- `Source/TopGunShooting/BaseEnemy.cpp:50`
- `Source/TopGunShooting/BaseEnemy.h:48`

Enemy render visibility is based on one sampled point. Large enemies or boundary cases may flicker or remain hidden while part of the mesh should be visible.

Recommended fix:

- Short term: tune `FogVisibilitySampleOffset` in Blueprint.
- Medium term: add bounds-based multi-point sampling.

### Low: `SetActorHiddenInGame` hides the whole Actor

Related location:

- `Source/TopGunShooting/BaseEnemy.cpp:46`

The current behavior is simple and matches the request, but it also hides child widgets, decals, and VFX components.

Recommended fix:

- If UI/VFX needs separate behavior later, switch to component-level visibility control.

### Low: InputAction asset paths are hard-coded in C++

Related locations:

- `Source/TopGunShooting/BaseCharacter.cpp:49`
- `Source/TopGunShooting/BaseCharacter.cpp:55`
- `Source/TopGunShooting/BaseCharacter.cpp:61`
- `Source/TopGunShooting/BaseCharacter.cpp:67`

If InputAction assets move or get renamed, default input can silently stop binding.

Recommended fix:

- Add warning logs for asset load failures.
- Move character-specific input defaults to Blueprint defaults or a DataAsset if variants grow.

## Verification

Last build command:

```text
MSBuild.exe ..\TopGunShooting.sln /t:Build /p:Configuration="Development Editor" /p:Platform=Win64
```

Result:

```text
0 warnings
0 errors
```

## Next Work

- [ ] Fix `RevealTriangle` behavior at FogWorld boundaries.
- [ ] Cache the Fog manager in `ABaseEnemy`.
- [ ] Verify Enemy visible-mask rendering in PIE.
- [ ] Evaluate bounds-based Enemy visibility sampling.
