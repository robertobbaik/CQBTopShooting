# Character Base

Tags: #system #character #input #camera #en

Korean: [[character-base-ko]]

## Purpose

`ABaseCharacter` provides shared player-character behavior: movement, aim rotation, camera setup, input binding, and Fog reveal support.

## Current Implementation

- `UFogOfWarRevealComponent` is attached to the character.
- `USpringArmComponent` and `UCameraComponent` define top-down camera behavior.
- Enhanced Input actions are loaded from `/Game/Input/IA_*`.
- Movement supports default, precision, and fast states.
- Aim can be driven by cursor deprojection or look input.

## Related Code

- `Source/TopGunShooting/BaseCharacter.h`
- `Source/TopGunShooting/BaseCharacter.cpp`
- `Source/TopGunShooting/PointManCharacter.h`
- `Source/TopGunShooting/PointManCharacter.cpp`

## Related Docs

- [[fog-of-war-en]]
- [[code-review-2026-05-12-en]]

## Known Risks

- InputAction asset paths are hard-coded in C++.
- Aim updates every tick while enabled.
- Future AI character subclasses should explicitly disable player-only aim behavior.

## Next Work

- [ ] Add warning logs when input asset loads fail.
- [ ] Decide whether input defaults should move to Blueprint defaults or a DataAsset.
