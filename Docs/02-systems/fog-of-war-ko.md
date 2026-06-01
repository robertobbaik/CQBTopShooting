# Fog Of War

Tags: #system #fog-of-war #rendering #ko

English: [[fog-of-war-en]]

## 목적

Fog Of War는 아직 공개되지 않은 맵 영역을 숨기고, 플레이어의 현재 시야 영역만 보여준다. 현재 구현은 CPU에서 갱신하는 alpha texture와 post-process material을 사용한다.

## 현재 구현

- `AFogOfWarManager`가 runtime transient `FogTexture`를 소유한다.
- `UFogOfWarRevealComponent`가 reveal source로 manager에 등록된다.
- `FogAlpha`는 가시 상태를 저장한다.
  - `HiddenAlpha`: 한 번도 보이지 않은 영역
  - `ExploredAlpha`: 이전에 보였던 영역
  - `VisibleAlpha`: 현재 보이는 영역
- Post-process material은 alpha texture를 샘플링해 `VisibleMask = 1.0 - FogAlpha`를 계산한다.
- Cone reveal은 line-of-sight trace와 feather edge를 지원한다.

## 관련 코드

- `Source/TopGunShooting/FogOfWarManager.h`
- `Source/TopGunShooting/FogOfWarManager.cpp`
- `Source/TopGunShooting/FogOfWarRevealComponent.h`
- `Source/TopGunShooting/FogOfWarRevealComponent.cpp`

## 관련 문서

- [[fog-of-war-handoff-ko]]
- [[enemy-visibility-ko]]
- [[code-review-2026-05-12-ko]]

## 알려진 리스크

- `RevealTriangle`은 꼭짓점 하나라도 FogWorld 밖이면 삼각형 전체를 버린다.
- Manager는 Fog texture 전체를 매 update마다 업로드한다.
- `BeginPlay` 시점에 manager가 없으면 reveal source 등록이 누락될 수 있다.

## 다음 작업

- [ ] Cone triangle을 FogWorld 경계에서 clamp 또는 clip한다.
- [ ] Fog texture 크기를 키우기 전에 전체 업로드 비용을 profile한다.
- [ ] Gameplay 동작이 안정된 뒤 dirty rect upload 도입을 검토한다.
