# Enemy Visibility

Tags: #system #enemy #fog-of-war #rendering #ko

English: [[enemy-visibility-en]]

## 목적

Enemy actor는 Fog Of War visible mask 안에 있을 때만 렌더링되어야 한다. 이 처리는 post-process fog 뒤로 enemy mesh가 보이는 문제를 막기 위한 것이다.

## 현재 구현

- `ABaseEnemy`가 주기적으로 `AFogOfWarManager`에 현재 가시성을 질의한다.
- 샘플 위치는 `GetActorLocation() + FogVisibilitySampleOffset`이다.
- 샘플 지점이 visible이면 `SetActorHiddenInGame(false)`를 적용한다.
- 샘플 지점이 visible이 아니면 `SetActorHiddenInGame(true)`를 적용한다.
- Fog manager가 없을 때는 `bRenderWhenFogManagerMissing`이 fallback 동작을 결정한다.

## 관련 코드

- `Source/TopGunShooting/BaseEnemy.h`
- `Source/TopGunShooting/BaseEnemy.cpp`
- `Source/TopGunShooting/FogOfWarManager.cpp`

## 관련 문서

- [[fog-of-war-ko]]
- [[adr-2026-05-12-visible-mask-enemy-rendering-ko]]
- [[code-review-2026-05-12-ko]]

## 설계 메모

- 현재 처리는 렌더링만 숨긴다. AI, collision, gameplay state는 계속 동작한다.
- Visible 판정은 `FogAlpha < ExploredAlpha`를 사용하므로 feather edge도 render-visible로 본다.
- Actor 전체 hidden state를 쓰기 때문에 widget, VFX 같은 child visual component도 같이 숨겨질 수 있다.

## 알려진 리스크

- 현재는 update마다 Fog manager를 검색한다.
- 단일 샘플 지점은 visibility edge 근처에서 깜빡임을 만들 수 있다.
- 큰 Enemy는 몸의 일부가 보여야 하는데 sample point가 hidden이라 계속 숨겨질 수 있다.

## 다음 작업

- [ ] `ABaseEnemy`에서 `AFogOfWarManager`를 캐싱한다.
- [ ] Bounds 기반 multi-point sampling 옵션을 추가한다.
- [ ] UI/VFX를 actor와 함께 숨길지 별도로 제어할지 결정한다.
