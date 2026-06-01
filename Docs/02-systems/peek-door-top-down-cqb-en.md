# Peek Door - Top-Down CQB

Tags: #system #interaction #cqb #en

Korean: [[peek-door-top-down-cqb-ko]]

## Purpose

The Peek Door system lets the player slightly open a door and inspect the room without fully entering or exposing the whole area.

## Player Flow

1. The player approaches a door.
2. The player holds the peek input.
3. The door opens by roughly 10 to 20 degrees.
4. A narrow cone reveal exposes limited vision.
5. Nearby AI can react to sound or visibility changes.

## Implementation Direction

- The door rotates around its hinge by a small angle.
- Reveal is limited to a narrow cone.
- Movement speed can be reduced while peeking.
- Peek start audio can feed AI hearing events.

## Related Systems

- [[fog-of-war-en]]
- [[enemy-visibility-en]]
- [[character-base-en]]

## Tuning Values

| Item | Suggested Value |
|---|---|
| Door rotation angle | 10 to 20 degrees |
| Reveal angle | 30 to 45 degrees |
| AI suspicion radius | 8 to 12m |
| Movement speed while peeking | 30 to 50% reduction |

## Next Work

- [ ] Define the door actor interface.
- [ ] Add peek input.
- [ ] Connect peek reveal source to the Fog system.
- [ ] Design AI hearing reactions.
