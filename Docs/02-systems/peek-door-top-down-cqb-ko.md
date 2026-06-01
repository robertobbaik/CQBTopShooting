# Peek Door - Top-Down CQB

Tags: #system #interaction #cqb #ko

English: [[peek-door-top-down-cqb-en]]

## 목적

Peek Door 시스템은 플레이어가 문을 완전히 열지 않고 살짝 열어 내부를 제한적으로 확인하게 만드는 CQB 상호작용이다.

## 플레이 흐름

1. 플레이어가 문에 접근한다.
2. Peek 입력을 누른다.
3. 문이 약 10~20도 열린다.
4. 좁은 cone 시야가 공개된다.
5. 주변 AI는 소리나 시야 변화에 반응할 수 있다.

## 구현 방향

- 문은 hinge 기준으로 작은 각도만 회전한다.
- Reveal은 좁은 cone으로 제한한다.
- Peek 중 이동 속도를 줄일 수 있다.
- Peek 시작 소리는 AI hearing 이벤트로 연결할 수 있다.

## 관련 시스템

- [[fog-of-war-ko]]
- [[enemy-visibility-ko]]
- [[character-base-ko]]

## 튜닝 값

| 항목 | 권장값 |
|---|---|
| 문 회전 각도 | 10~20도 |
| Reveal 각도 | 30~45도 |
| AI 의심 반경 | 8~12m |
| Peek 중 이동 속도 | 30~50% 감소 |

## 다음 작업

- [ ] Door actor interface 정의
- [ ] Peek 입력 추가
- [ ] Peek reveal source를 Fog 시스템과 연결
- [ ] AI hearing reaction 설계
