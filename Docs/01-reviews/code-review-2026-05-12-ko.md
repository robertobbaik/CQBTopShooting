---
tags:
  - review
  - fog-of-war
  - enemy
  - character
  - ko
---

# 코드 리뷰 - 캐릭터 / Enemy / Fog Of War 구조

English: [[code-review-2026-05-12-en]]

작성일: 2026-05-12  
대상 상태: 현재 워킹트리 기준

## 관련 문서

- [[fog-of-war-ko]]
- [[enemy-visibility-ko]]
- [[character-base-ko]]
- [[adr-2026-05-12-visible-mask-enemy-rendering-ko]]

## 리뷰 범위

- `AFogOfWarManager`
- `UFogOfWarRevealComponent`
- `ABaseCharacter`
- `ABaseEnemy`
- `ANormalEnemy`
- `TopGunShooting.Build.cs`

## 전체 판단

구조 방향은 적절하다. Fog reveal 계산, post-process 표시, Enemy 렌더링 제어가 분리되어 있어 확장하기 쉽다. 다만 현재 구현은 CPU texture 전체 업로드와 Enemy 단위 tick/query에 의존하므로 Enemy 수와 맵 크기가 늘어나면 병목이 생길 수 있다.

## 주요 발견 사항

### Medium: FogWorld 경계 근처에서 Cone reveal이 누락될 수 있음

관련 위치:

- `Source/TopGunShooting/FogOfWarManager.cpp:449`
- `Source/TopGunShooting/FogOfWarManager.cpp:454`

`RevealTriangle`은 삼각형 점 하나라도 `WorldToFogPixel` 변환에 실패하면 전체 삼각형을 버린다. 플레이어가 FogWorld 경계 근처에 있을 때 실제 시야 안의 영역이 reveal되지 않을 수 있고, Visible Mask 기반 Enemy 렌더링도 이 영향을 받는다.

권장 수정:

- 단기적으로 FogWorld 밖 좌표를 사각형 내부로 clamp한다.
- 중기적으로 삼각형을 FogWorld rect로 clipping한 뒤 rasterize한다.

### Medium: Fog texture 전체 업로드가 성능 병목이 될 수 있음

관련 위치:

- `Source/TopGunShooting/FogOfWarManager.cpp:530`
- `Source/TopGunShooting/FogOfWarManager.cpp:550`

현재 `UploadTexture`는 매 갱신마다 `FogAlpha` 전체를 `TexturePixels`로 변환하고 texture 전체를 업로드한다. 기본 1024x1024에서도 초당 여러 번 4MB 수준의 데이터를 갱신한다.

권장 수정:

- 변경된 pixel 범위를 dirty rect로 추적한다.
- 전체 texture 대신 변경된 rect만 업로드한다.
- 해상도 상향 전 Unreal Insights 또는 stat으로 측정한다.

### Medium: Enemy마다 Fog manager를 반복 검색함

관련 위치:

- `Source/TopGunShooting/BaseEnemy.cpp:31`
- `Source/TopGunShooting/BaseEnemy.cpp:37`

`ABaseEnemy::UpdateFogRenderVisibility`는 갱신마다 `AFogOfWarManager::Get(this)`를 호출한다. 내부적으로 actor 검색이 발생하므로 Enemy 수가 늘면 불필요한 비용이 된다.

권장 수정:

- `BeginPlay`에서 `TWeakObjectPtr<AFogOfWarManager>`로 캐싱한다.
- invalid 상태일 때만 재검색한다.

### Medium: Enemy visibility가 단일 샘플 포인트 기반임

관련 위치:

- `Source/TopGunShooting/BaseEnemy.cpp:39`
- `Source/TopGunShooting/BaseEnemy.cpp:50`
- `Source/TopGunShooting/BaseEnemy.h:48`

Enemy 렌더링 판정은 한 지점만 샘플링한다. 큰 Enemy나 Fog 경계에서는 몸 일부가 보여야 하는데도 숨겨지거나, 경계에서 깜빡일 수 있다.

권장 수정:

- 단기적으로 `FogVisibilitySampleOffset`을 Blueprint에서 조정한다.
- 중기적으로 bounds 기반 multi-point sampling을 추가한다.

### Low: `SetActorHiddenInGame`은 Actor 전체를 숨김

관련 위치:

- `Source/TopGunShooting/BaseEnemy.cpp:46`

현재 방식은 간단하고 요구사항에는 맞다. 다만 widget, decal, VFX component도 함께 숨겨질 수 있다.

권장 수정:

- UI/VFX가 분리될 때는 component별 visibility 제어로 확장한다.

### Low: InputAction asset path가 C++에 hard-code되어 있음

관련 위치:

- `Source/TopGunShooting/BaseCharacter.cpp:49`
- `Source/TopGunShooting/BaseCharacter.cpp:55`
- `Source/TopGunShooting/BaseCharacter.cpp:61`
- `Source/TopGunShooting/BaseCharacter.cpp:67`

InputAction asset 이름이나 경로가 바뀌면 기본 입력이 조용히 비활성화될 수 있다.

권장 수정:

- asset load 실패 시 warning log를 추가한다.
- 캐릭터별 입력 차이가 생기면 Blueprint default 또는 DataAsset으로 분리한다.

## 검증 상태

마지막 확인 빌드:

```text
MSBuild.exe ..\TopGunShooting.sln /t:Build /p:Configuration="Development Editor" /p:Platform=Win64
```

결과:

```text
경고 0개
오류 0개
```

## 다음 작업

- [ ] `RevealTriangle` FogWorld 경계 누락 수정
- [ ] `ABaseEnemy`에서 Fog manager 캐싱
- [ ] PIE에서 Enemy Visible Mask 렌더링 확인
- [ ] Enemy bounds 기반 visibility sampling 검토
