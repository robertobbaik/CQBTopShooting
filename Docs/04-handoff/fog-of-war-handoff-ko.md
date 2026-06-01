---
tags:
  - handoff
  - fog-of-war
  - rendering
  - ko
---

# Fog Of War Handoff

English: [[fog-of-war-handoff-en]]

작성일: 2026-05-09

## 관련 문서

- [[fog-of-war-ko]]
- [[enemy-visibility-ko]]
- [[code-review-2026-05-12-ko]]

## 현재 상태

- `AFogOfWarManager`가 level에 배치되어 있다.
- `UFogOfWarRevealComponent`가 player character에 붙어 있고 `BeginPlay`에서 활성화된다.
- Post-process fog 경로는 prototype 수준에서 동작한다.
- 최종 post-process 경로에서는 debug color를 꺼야 한다.
- Fog texture alpha가 핵심 channel이다.
  - `FogTexture.A = fog alpha`
  - `VisibleMask = 1.0 - FogTexture.A`

## Material Parameters

- `FogTexture`: Texture Object Parameter
- `FogWorldOrigin`: Vector
- `FogWorldSize`: Vector
- `FogTextureSize`: Vector 또는 Float2
- `FogBlurTexels`: Scalar
- `FogContrast`: Scalar
- `HiddenDarkness`: Scalar

추천 시작값:

- `FogBlurTexels`: `1.5` ~ `3.0`
- `FogContrast`: `0.8` ~ `1.4`
- `HiddenDarkness`: `0.05` ~ `0.2`

## Line Of Sight Debug

Cone occlusion이 벽이나 prop에 반응하지 않을 때:

1. Player의 `FogOfWarRevealComponent`에서 `bDrawLineOfSightDebug`를 켠다.
2. 차단 지점에서 ray가 빨간색으로 바뀌는지 확인한다.
3. Ray가 object를 통과하면 해당 mesh가 `LineOfSightChannel`을 block하도록 설정한다.
4. Collision이 너무 단순하면 테스트 중에만 `bLineOfSightTraceComplex`를 켠다.

## 다음 단계

- [ ] Material graph가 복잡해지면 Fog UV 계산을 HLSL로 더 이동한다.
- [ ] Blur radius, contrast curve, hidden darkness를 튜닝한다.
- [ ] 이전에 본 영역과 한 번도 보지 않은 영역을 다르게 표시할지 결정한다.
