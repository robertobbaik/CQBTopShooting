# Fog Of War Handoff

Date: 2026-05-09

## Current State

- `AFogOfWarManager` is placed in the level.
- `UFogOfWarRevealComponent` is attached to the player character and enabled from `BeginPlay`.
- Post-process fog path is now roughly working.
- Debug colors should be disabled for the final post-process path.
- Fog texture alpha is the important channel:
  - `FogTexture.A = fog alpha`
  - `VisibleMask = 1.0 - FogTexture.A`

## C++ Manager Features

`AFogOfWarManager` currently supports:

- Runtime transient fog texture.
- Optional debug texture color mode.
- Optional unbound post-process component.
- Reveal alpha feathering before post-process blur.
- Performance clamps:
  - Max texture dimension.
  - Minimum update interval.
  - Max reveal ray count.
  - Bilinear fog texture filtering.

Post-process material parameters pushed from C++:

- `FogTexture`
- `FogWorldOrigin`
- `FogWorldSize`
- `FogTextureSize`

Potential cleanup:

- Consider replacing `FogTextureSize` with `FogTexelSize`.
- C++ would pass:

```cpp
FogPostProcessMaterialInstance->SetVectorParameterValue(
    FogTexelSizeParameterName,
    FLinearColor(
        1.0f / FogTextureSize.X,
        1.0f / FogTextureSize.Y,
        0.0f,
        0.0f));
```

Then HLSL can use:

```hlsl
float2 offset = FogTexelSize * FogBlurTexels;
```

## Material Parameters

Current recommended parameter names:

- `FogTexture` - Texture Object Parameter, not Texture Sample Parameter2D.
- `FogWorldOrigin` - Vector.
- `FogWorldSize` - Vector.
- `FogTextureSize` - Vector or Float2 input into Custom node.
- `FogBlurTexels` - Scalar.
- `FogContrast` - Scalar.
- `HiddenDarkness` - Scalar.

Suggested starting values:

- `FogBlurTexels`: `1.5` to `3.0`
- `FogContrast`: `0.8` to `1.4`
- `HiddenDarkness`: `0.05` to `0.2`

## Reveal Alpha Feathering

`AFogOfWarManager` now softens reveal edges while writing `FogAlpha`.

Runtime setting:

- `RevealFeatherWorldDistance`
  - `0.0` disables source-side feathering.
  - `75.0` to `150.0` is a reasonable starting range.
  - Current default: `100.0`.

Behavior:

- Center/currently visible area writes toward `VisibleAlpha`.
- Reveal edge blends toward `ExploredAlpha`.
- Previous visible and feathered pixels decay back to `ExploredAlpha` on the next update.
- Overlapping reveal sources keep the lower alpha, so the most visible source wins.

The post-process material should still keep the light 3x3 blur. The C++ feathering reduces the hard source edge; the material blur smooths final screen sampling.

## Line Of Sight Debug

`UFogOfWarRevealComponent` supports cone occlusion through line traces.

Relevant settings:

- `bUseLineOfSight`
- `LineOfSightChannel`
- `bLineOfSightTraceComplex`
- `TraceHeight`
- `bDrawLineOfSightDebug`
- `bLogLineOfSightHits`

Debug colors:

- Clear ray segment: `LineOfSightClearDebugColor`
- Blocked ray segment and hit sphere: `LineOfSightBlockedDebugColor`

If cone occlusion does not react to walls or props:

1. Enable `bDrawLineOfSightDebug` on the player's `FogOfWarRevealComponent`.
2. Confirm rays turn red at blockers.
3. If rays stay green through objects, set those meshes to block `LineOfSightChannel`.
4. If mesh collision is too coarse, enable `bLineOfSightTraceComplex` temporarily for testing.

## Current Custom Node Plan

Custom Node name:

```text
FogVisibleMask
```

Output Type:

```text
CMOT Float 1
```

Inputs:

```text
FogUV           float2
FogTextureSize  float2
FogBlurTexels   float
FogContrast     float
```

Texture Object input:

```text
FogTexture
```

Important Unreal Custom node rule:

- If the Texture Object input is named `FogTexture`, Unreal exposes:
  - `FogTexture`
  - `FogTextureSampler`

HLSL:

```hlsl
float2 safeTextureSize = max(FogTextureSize, float2(1.0, 1.0));
float2 texel = 1.0 / safeTextureSize;
float2 offset = texel * FogBlurTexels;

float2 fogUV = saturate(FogUV);

float center = Texture2DSample(FogTexture, FogTextureSampler, fogUV).a;

float right = Texture2DSample(FogTexture, FogTextureSampler, fogUV + float2(offset.x, 0.0)).a;
float left = Texture2DSample(FogTexture, FogTextureSampler, fogUV - float2(offset.x, 0.0)).a;
float up = Texture2DSample(FogTexture, FogTextureSampler, fogUV + float2(0.0, offset.y)).a;
float down = Texture2DSample(FogTexture, FogTextureSampler, fogUV - float2(0.0, offset.y)).a;

float upRight = Texture2DSample(FogTexture, FogTextureSampler, fogUV + float2(offset.x, offset.y)).a;
float upLeft = Texture2DSample(FogTexture, FogTextureSampler, fogUV + float2(-offset.x, offset.y)).a;
float downRight = Texture2DSample(FogTexture, FogTextureSampler, fogUV + float2(offset.x, -offset.y)).a;
float downLeft = Texture2DSample(FogTexture, FogTextureSampler, fogUV + float2(-offset.x, -offset.y)).a;

float fogAlpha =
    center * 4.0 +
    (right + left + up + down) * 2.0 +
    (upRight + upLeft + downRight + downLeft);

fogAlpha *= 0.0625;

float visibleMask = 1.0 - fogAlpha;
visibleMask = saturate(visibleMask);
visibleMask = smoothstep(0.05, 0.95, visibleMask);
visibleMask = pow(visibleMask, max(FogContrast, 0.001));
visibleMask = saturate(visibleMask);

return visibleMask;
```

Material output wiring:

```text
SceneTexture(PostProcessInput0) * HiddenDarkness -> Lerp A
SceneTexture(PostProcessInput0)                  -> Lerp B
FogVisibleMask Custom Node                       -> Lerp Alpha
Lerp Result                                      -> Emissive Color
```

## Next Step

Continue by reducing the material graph further:

1. Move `FogUV` calculation into HLSL.
2. Custom node inputs become:
   - `WorldPositionXY`
   - `FogWorldOrigin`
   - `FogWorldSize`
   - `FogTextureSize` or `FogTexelSize`
   - `FogBlurTexels`
   - `FogContrast`
   - `FogTexture`
3. Custom node returns final `VisibleMask`.
4. Keep only these graph pieces outside HLSL:
   - `SceneTexture(PostProcessInput0)`
   - `AbsoluteWorldPosition`
   - `HiddenDarkness`
   - `Lerp`

After that, tune visual quality:

- Blur radius.
- Contrast curve.
- Hidden darkness.
- Previously seen versus never seen state, if needed.
