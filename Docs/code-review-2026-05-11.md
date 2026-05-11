# 코드 리뷰 - 캐릭터/FogOfWar 재구성

작성일: 2026-05-11

## 리뷰 범위

이번 리뷰는 최근 변경된 캐릭터 구조와 FogOfWar 구조를 기준으로 작성했다.

- `TopGunPlayerCharacter` -> `PointManCharacter` 변경
- `BaseCharacter`, `BaseEnemy` 도입
- 이동, 조준, 카메라, InputAction, Fog reveal 공통화
- FogOfWarManager의 StaticMesh 제거
- Fog post-process 기본 활성화
- Cone 시야 알파 페더링
- Cone 시야 Line Of Sight 차단 처리

## 발견 사항

### Medium: FogWorld 경계 근처에서 Cone reveal이 누락될 수 있음

`AFogOfWarManager::RevealTriangle`은 삼각형의 세 꼭짓점 중 하나라도 `FogWorld` 밖이면 해당 삼각형 전체를 처리하지 않고 반환한다.

관련 위치:

- `Source/TopGunShooting/FogOfWarManager.cpp:449`
- `Source/TopGunShooting/FogOfWarManager.cpp:454`

위험:

- 플레이어가 FogWorld 경계 근처에 있을 때, Cone 시야의 일부 ray 끝점이 경계 밖으로 나갈 수 있다.
- 이 경우 해당 삼각형 전체가 스킵되어 부채꼴 시야에 빈 조각이 생길 수 있다.
- LOS hit 지점이 가까우면 문제가 덜 보일 수 있지만, 일반적으로 긴 시야 ray에서는 재현될 가능성이 있다.

권장 수정:

- 삼각형 rasterize 전에 FogWorld 사각형 기준으로 clipping 처리한다.
- 단기적으로는 경계 밖 좌표를 가장 가까운 FogWorld 내부 좌표로 clamp하는 방식도 가능하다.
- 완전히 FogWorld 밖에 있는 삼각형만 스킵하도록 조건을 완화하는 것이 좋다.

### Medium: Fog texture 전체 업로드가 성능 병목이 될 수 있음

현재 `UploadTexture`는 Fog 갱신 때마다 전체 `FogTexture` 데이터를 다시 업로드한다.

관련 위치:

- `Source/TopGunShooting/FogOfWarManager.cpp:530`
- `Source/TopGunShooting/FogOfWarManager.cpp:550`

위험:

- `1024x1024` 텍스처 기준으로 매 업데이트마다 약 4MB 픽셀 데이터를 업로드한다.
- 업데이트 간격이 `0.033`초라면 부하가 있는 상황에서 비용이 눈에 띌 수 있다.
- 맵 크기, 텍스처 해상도, reveal source 수가 늘어나면 확장성이 좋지 않다.

권장 수정:

- reveal 처리 중 변경된 영역만 dirty rect로 추적한다.
- 전체 텍스처가 아니라 변경된 사각 영역만 업로드한다.
- 당장은 `MaxRuntimeTextureDimension`, `MinRuntimeUpdateInterval`, warning log 같은 방어 장치를 유지하는 것이 좋다.

### Medium: BaseCharacter가 InputAction asset 경로에 직접 의존함

`ABaseCharacter` 생성자에서 `/Game/Input/IA_*` 경로를 직접 로드하고 있다.

관련 위치:

- `Source/TopGunShooting/BaseCharacter.cpp:49`
- `Source/TopGunShooting/BaseCharacter.cpp:55`
- `Source/TopGunShooting/BaseCharacter.cpp:61`
- `Source/TopGunShooting/BaseCharacter.cpp:67`

위험:

- InputAction asset 이름이나 위치가 바뀌면 입력 바인딩이 깨질 수 있다.
- 모든 파생 캐릭터가 같은 입력 구성을 기본으로 상속한다.
- 이후 캐릭터 타입별 조작 방식이 달라질 경우 BaseCharacter 기본값이 오히려 제약이 될 수 있다.

권장 수정:

- 지금은 공통 기본값으로 유지해도 된다.
- 다만 `EditDefaultsOnly`로 열어두고, 파생 Blueprint에서 입력 asset을 교체할 수 있게 운용하는 것이 좋다.
- asset load 실패 시 warning log를 남기면 추적이 쉬워진다.

### Low: Fog post-process material 경로가 코드에 고정되어 있음

`AFogOfWarManager`가 `/Game/Materials/M_FogOfWar_PostProcess`를 기본 material로 로드한다.

관련 위치:

- `Source/TopGunShooting/FogOfWarManager.cpp:50`

위험:

- material 이름이나 위치가 변경되면 새로 배치한 FogOfWarManager의 기본 렌더링이 깨질 수 있다.
- 이미 배치된 actor는 property override로 동작할 수 있지만, 기본값 의존성이 남는다.

권장 수정:

- 현재 자동 로드는 기본 세팅 편의성 측면에서 유지할 만하다.
- 대신 material load 실패 시 warning log를 남겨야 한다.

### Low: LOS debug 옵션을 켜둔 채로 두면 비용이 커질 수 있음

Line Of Sight debug는 ray마다 debug line과 sphere를 그린다.

관련 위치:

- `Source/TopGunShooting/FogOfWarManager.cpp:341`
- `Source/TopGunShooting/FogOfWarManager.cpp:384`
- `Source/TopGunShooting/FogOfWarRevealComponent.h:60`

위험:

- `RayCount`가 192 수준이면 Fog 업데이트마다 많은 debug draw가 발생한다.
- 화면도 복잡해지고 CPU 비용도 늘어난다.

권장 수정:

- `bDrawLineOfSightDebug` 기본값은 계속 `false`로 유지한다.
- 충돌 채널과 장애물 차단 확인이 필요할 때만 일시적으로 켠다.

## 확인된 좋은 변경

- `TopGunPlayerCharacter`가 `PointManCharacter`로 정리되었다.
- 공통 캐릭터 기능이 `BaseCharacter`로 이동했다.
- `BaseCharacter`가 이동, 조준, 카메라, InputAction binding, Fog reveal component를 담당한다.
- 공격이나 액션은 `PrimaryAction`, `SecondaryAction` override hook으로 파생 클래스에서 재정의할 수 있다.
- `BaseEnemy`가 준비되었고, 기본적으로 Fog reveal과 aim rotation을 꺼둔다.
- `FogOfWarManager`에서 불필요한 debug static mesh plane이 제거되었다.
- Fog post-process가 기본 활성화되고, 기본 material도 자동 로드된다.
- Cone 시야에 알파 페더링이 적용되어 가장자리 경계가 덜 딱딱해졌다.
- Cone 시야가 Line Of Sight trace 결과에 따라 주변 오브젝트에 가려질 수 있는 기반이 들어갔다.

## 검증 상태

빌드 명령:

```text
MSBuild.exe ..\TopGunShooting.sln /t:Build /p:Configuration="Development Editor" /p:Platform=Win64
```

마지막 확인 결과:

```text
경고 0개, 오류 0개
```

맵 세팅 확인:

- `BP_FogOfWarManager_C_1`
- `bEnableFogPostProcess=True`
- `FogPostProcessMaterial=/Game/Materials/M_FogOfWar_PostProcess`

## 다음 리뷰 대상

1. PIE에서 `bDrawLineOfSightDebug=true`로 켜고 Cone 시야 차단이 의도대로 되는지 확인한다.
2. `Visibility` 채널을 계속 쓸지, 전용 `FogBlocker` trace channel을 추가할지 결정한다.
3. FogWorld 경계 근처 Cone clipping 문제를 수정한다.
4. 맵 크기나 Fog texture 해상도를 올리기 전에 dirty rect 업로드가 필요한지 판단한다.
5. 파생 캐릭터가 늘어나기 전에 `BaseCharacter`의 InputAction 기본값 운용 방식을 확정한다.
