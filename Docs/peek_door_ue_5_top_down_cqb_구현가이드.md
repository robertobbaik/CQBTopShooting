# Peek Door 시스템 구현 가이드 (UE5 Top-Down CQB)

문을 살짝 열어 내부를 확인하는 전술 행동을 UE5 탑뷰 슈팅 게임 기준으로 정리한 문서입니다.

---

## 1. 시스템 개요

플레이어가 문을 소폭 열어 좁은 시야로 방 내부를 확인합니다.

### 핵심 포인트
- 안전하게 정보를 얻을 수 있지만 완전하지 않음
- 좁은 시야(부채꼴 Reveal)만 공개
- 문 여는 소리로 적 AI가 반응 가능
- 진입 전 긴장감과 판단 재미 제공

---

## 2. 플레이어 흐름

1. 문 앞 접근  
2. Peek 입력 (홀드)  
3. 문이 10~20도 열림  
4. 좁은 시야 Reveal  
5. 적 AI가 소리/시야 반응 가능

---

## 3. 구현 세부

### 3-1. 문 회전 (Door Rotation)
- 힌지 축 기준 회전
- 회전 각도: 10° ~ 20°
- Timeline 또는 Lerp 사용
- 열리는 방향은 문 손잡이 반대 방향 우선

```cpp
FRotator StartRot = Door->GetActorRotation();
FRotator PeekRot = StartRot;
PeekRot.Yaw += PeekAngle;
```

### 3-2. 시야 Reveal (Vision Reveal)
- 플레이어 기준 부채꼴 Cone 공개
- 각도: 30° ~ 45°
- 거리: 짧게 제한
- 실시간 또는 짧은 주기 업데이트

### 3-3. 입력 및 상호작용
- `E 탭` : 일반 문 열기
- `E 홀드` : Peek 시작
- 입력 해제: 문 닫기 / 원위치
- Peek 중 이동 속도 감소 가능

---

## 4. 시야 Reveal 단계 예시

1. 기본 암전 상태  
2. Peek 시작 (문 열림)  
3. 문틈 방향 부채꼴 시야 공개  
4. Peek 종료 (문 닫힘)

---

## 5. AI 반응 설계 (State Machine)

### 1) Idle / Patrol
- 대기 또는 순찰 상태

### 2) Suspicious (의심)
- 문 여는 소리 감지
- 문 방향 주시
- 소리 반응 반경 예: 8~12m

### 3) Alert (경계)
- 문 근처 이동
- 무기 조준
- 경계 태세 유지

### 4) Investigate (탐색)
- 문 앞으로 접근
- 주변 확인

### 5) Combat (교전)
- 플레이어 발견 시 공격 전환

---

## 6. 소리 및 감지 처리

### Peek 시작 사운드 이벤트
- DoorPeekStartSound 재생
- AI Hearing Sense 전달

### 시야 감지
- 문이 열린 동안 AI도 틈새 시야 확보 가능
- 플레이어가 Cone 안에 들어오면 발각 가능

---

## 7. 밸런스 파라미터 예시

| 항목 | 값 |
|---|---|
| Peek 회전 각도 | 10° ~ 20° |
| Reveal 각도 | 30° ~ 45° |
| Reveal 시간 | 0.5초 ~ 지속 |
| AI 의심 시간 | 2 ~ 4초 |
| AI 탐색 시간 | 2 ~ 6초 |
| AI 시야 Cone | 90° ~ 120° |
| Peek 중 이동속도 | 30~50% 감소 |

---

## 8. UE5 구현 구조 (권장)

```text
PlayerCharacter
 └─ Peek 요청
InteractableDoor
 ├─ 조건 확인
 ├─ Peek 시작 / 종료
 └─ 문 회전 처리
VisionSystem
 ├─ Cone 계산
 └─ Limited Vision Reveal
AIController
 ├─ Hearing Sense
 ├─ Sight Sense
 └─ Blackboard / BT 상태 전환
```

---

## 9. 확장 아이디어

- 문 종류별 Peek 속도 차이
- 팀원이 반대편 문 동시 Peek
- 카메라 삽입 정찰과 연계
- 특수 장비로 소음 감소
- 숙련도 스킬로 Peek 속도 향상

---

## 10. 핵심 요약

Peek Door는 **정보 획득 vs 위험 노출** 사이의 긴장을 만드는 핵심 시스템입니다.

- 너무 강하면 정보 치트가 됨
- 너무 약하면 사용 가치가 없음
- 짧고 제한된 시야 + AI 반응이 가장 중요함
