# Character Base

Tags: #system #character #input #camera #ko

English: [[character-base-en]]

## 목적

`ABaseCharacter`는 player character의 공통 동작을 제공한다. 이동, aim rotation, camera setup, input binding, Fog reveal 지원을 담당한다.

## 현재 구현

- `UFogOfWarRevealComponent`가 character에 붙어 있다.
- `USpringArmComponent`와 `UCameraComponent`가 top-down camera 동작을 정의한다.
- Enhanced Input action은 `/Game/Input/IA_*`에서 로드한다.
- 이동은 default, precision, fast 상태를 지원한다.
- Aim은 cursor deprojection 또는 look input으로 갱신된다.

## 관련 코드

- `Source/TopGunShooting/BaseCharacter.h`
- `Source/TopGunShooting/BaseCharacter.cpp`
- `Source/TopGunShooting/PointManCharacter.h`
- `Source/TopGunShooting/PointManCharacter.cpp`

## 관련 문서

- [[fog-of-war-ko]]
- [[code-review-2026-05-12-ko]]

## 알려진 리스크

- InputAction asset path가 C++에 hard-code되어 있다.
- Aim은 활성화되어 있으면 매 tick 갱신된다.
- 향후 AI character subclass가 생기면 player 전용 aim 동작을 명확히 꺼야 한다.

## 다음 작업

- [x] Input asset load 실패 시 warning log를 추가한다.
- [x] Input 기본값을 Blueprint default나 DataAsset으로 옮길지 결정한다.
