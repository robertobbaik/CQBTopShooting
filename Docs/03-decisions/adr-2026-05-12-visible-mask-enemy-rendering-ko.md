# ADR 2026-05-12: Visible Mask 기반 Enemy 렌더링

Tags: #adr #enemy #fog-of-war #rendering #ko

English: [[adr-2026-05-12-visible-mask-enemy-rendering-en]]

## 상태

현재 prototype 기준으로 채택.

## 배경

Enemy가 Fog Of War visible mask 밖에 있어도 mesh가 렌더링되고 있었다. Post-process fog는 화면을 어둡게 만들지만, enemy mesh 자체가 그려지는 것을 막지는 못한다.

## 결정

`ABaseEnemy`가 `AFogOfWarManager::IsWorldLocationCurrentlyVisible`을 질의해서 render visibility를 제어한다.

Enemy actor는 sample point가 current visible mask 밖에 있으면 hidden 처리되고, visible mask 안에 있으면 다시 표시된다.

## 결과

- 기존 Fog data를 그대로 활용하므로 구현이 단순하다.
- Enemy가 hidden이어도 AI와 collision은 계속 동작한다.
- Actor 전체 hidden state를 쓰기 때문에 child visual component도 같이 숨겨진다.
- Fog manager caching이 들어가기 전까지 Enemy 수가 많아지면 lookup 비용이 생길 수 있다.
- 단일 지점 sampling은 Fog 경계에서 깜빡임을 만들 수 있다.

## 후속 작업

- [ ] `ABaseEnemy`에서 Fog manager를 캐싱한다.
- [ ] Bounds 기반 sampling을 검토한다.
- [ ] Hidden enemy의 AI perception update를 계속 유지할지 결정한다.
 