# TopGunShooting 문서 홈

Tags: #index #topgunshooting #ko

## 현재 집중 영역

- [[enemy-visibility-ko|Enemy Visibility]]
- [[fog-of-war-ko|Fog Of War]]
- [[code-review-2026-05-12-ko|최신 코드 리뷰]]

## 시스템 문서

- [[fog-of-war-ko]]
- [[enemy-visibility-ko]]
- [[character-base-ko]]
- [[peek-door-top-down-cqb-ko]]

## 리뷰

- [[code-review-2026-05-12-ko]]

## 결정 기록

- [[adr-2026-05-12-visible-mask-enemy-rendering-ko]]

## 핸드오프

- [[fog-of-war-handoff-ko]]

## 작성 규칙

- 주요 문서는 항상 한국어 `-ko`와 영어 `-en` 두 파일로 작성한다.
- 리뷰 문서는 `01-reviews/`에 둔다.
- 살아있는 시스템 설명서는 `02-systems/`에 둔다.
- 설계 결정은 `03-decisions/`에 ADR로 기록한다.
- 인수인계성 메모는 `04-handoff/`에 둔다.
- 이미지와 첨부 파일은 `assets/`에 둔다.

## 리뷰 백로그

- [ ] `AFogOfWarManager::RevealTriangle`의 FogWorld 경계 clipping 수정
- [ ] `ABaseEnemy`에서 `AFogOfWarManager` 캐싱
- [ ] PIE에서 Visible Mask 기반 Enemy 렌더링 확인
- [ ] Enemy visibility를 단일 지점으로 볼지 bounds 기반으로 볼지 결정
