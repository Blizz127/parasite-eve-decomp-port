# Disc 1 gameplay regression baseline

Pinned live-run contract for the first real disc-1 play path. This is a
**native port** baseline, not a matching-C count. Re-run the harness; do
not treat this file as a substitute for the log.

## Harness

```
PE_ROUTE_REWARD_PILOT=1 ./pc_port/build/pe-route-boot-day2-tests
```

Requires a legally supplied Disc 1 BIN (`PE_DISC1_BIN` or `local/pe_disc1.path`).
The harness **rejects** a Disc 2 image (`PE_Disc_BootKind==2`). Movie and
opening menu are HOST_ADAPTED skips (`PE_Port_SetSkipMovie(1)`,
`PE_Port_SetSkipOpeningMenu(1)`). Frame cap: 62000.

`PE_ROUTE_REWARD_PILOT=1` is required to reach the sewer milestones: without it
the plain run never drives the two mandatory battles and ends at 45/57. The
plain run's stop reason moved down the field-menu stack as pages were
translated: `func_8004AD9C` (the field-menu page) → `PE_MenuDrawCallback` (the
`0x8004FF30` list callback, same frame 61593 and milestone count) →
`PE_MenuInputCallback` (the `0x8004AE1C` input handler, frame 61623, same
story/persist/token and 45/57 milestones). The whole `0x8004AE1C` input tree is
now native — including the Equipment draw/input pair (`0x8004B214`/`0x8004B394`),
the modal input (`0x8004B650`), the Equipment per-cell draw (`0x80050438`) and the
case-4/5 close page (`0x8005D994` + `0x8005247C`) — so that last stub is gone. The
tree then closed completely: the modal window's own draw (`0x8004B5DC`) and its two
leaves (`func_8005FCAC`, `func_8005ED18`) are native too, so **no named boundary
remains anywhere in the field-menu tree**. The plain run ends
at `frames=62000 stop=frame-limit story=0x00000009 persist1=0x0000000A
token=0xA8000148`, 45/57, with only the four documented HOST_ADAPTED movie/menu
skips and **no unresolved boundary**. Plain runs are recorded in
`{SCRATCH}/route_baseline.log` and `{SCRATCH}/route_baseline2.log`
(pre-`4FF30`), `{SCRATCH}/route_4ff30.log` (post-`4FF30`),
`{SCRATCH}/route_menu2.log` (input tree) and `{SCRATCH}/route_tree_plain.log`
(tree closed); pilot in `{SCRATCH}/route_baseline_pilot.log`,
`{SCRATCH}/route_4ff30_pilot.log`, `{SCRATCH}/route_menu_pilot.log` (identical
62000-frame, **55/57 pre-re-pin**, all-HOST_ADAPTED) and
`{SCRATCH}/route_tree_pilot.log` (57/57). `{SCRATCH}` = `/tmp/pe-2nd`.

With the pilot the last recorded run is `frames=62000 stop=frame-limit
story=0x68 token=0xA80031C8`, **57/57 milestones**, and **no unresolved boundary
stub at all** (the only invoked stubs are the four documented HOST_ADAPTED
skips). The m0027i pin below (persist[74]=0x68, at least one sewer victory) is
inside that pass set.

Two milestone observations and the endpoint pin were re-expressed on
2026-09-18 because they encoded route-specific values from an older m0031i run:

- Milestone 55 "normal item use restores 45 HP" compared Aya's HP against the
  absolute 45, which was her max HP in the run that authored the row. The route
  now wins both sewer battles and reaches m0031i at max HP 53
  (`func_80023E14` heals item7 by 90 then clamps HP to `record+28`), so the
  route-independent invariant — **HP is full after the Items/Use** — is asserted
  instead (strictly stronger). Traced live: `LOOT_PILOT_HEAL_DONE 58044 hp=53
  item4=0`, through the native field chain `func_80044B0C → func_80057834 →
  func_800516B4 → func_80023E14` + `func_80057D30`.
- Milestone 56 "equipment command407 selects carried slot0" sampled the gun slot
  only on the item-use frame; the field item use (m0031i, 58044, gun=2) always
  precedes the equip (m0032i fight, 58917, `command=407 slot=0`), so the
  co-occurrence could never happen. It now observes the command itself.
- The endpoint pin dropped the frozen m0031i token/PC, the absolute
  HP/status-copy 45, and the room-local `persist[1]=0x14E` and Aya `+0x98` bit
  `0x200`: the pilot advances into the m0032i fight (where milestone 56's
  equipment command is issued) and cannot park at m0031i. The durable end state
  is still asserted (persist[74]=0x68, persist[24] bits, C8/C9 retained, item7
  consumed, chest/switch flags, pistol slot 0, two victories, field control, no
  unresolved boundary) and the observed endpoint is printed on the
  `route: endpoint …` line.

Pilot log for the 57/57 PASS: `{SCRATCH}/route_m55c.log`.

## What “first real gameplay” means here

Control-flow / story-state traversal through:

| Pin | Value | Meaning |
| --- | --- | --- |
| token | `0xA80023C8` | `m0027i` first sewer hallway |
| persist[1] | `0x1A` | m0027i entry |
| persist[74] | `0x68` | sewers story |
| sewer victories | ≥1 | three enemies retired, Aya alive |

Evidence: `docs/evidence/boot-day2-route-harness/REPORT.md`,
`docs/ai_context/DAY1_DAY2_TRANSITIONS.md`. A run that does not reach
`m0027i` is a **regression of the playable path**, even if matching-C
counts went up.

## Disc 2

Not this baseline. Disc 2 (`SLUS_006.68`) shares the EXE; `PE.IMG` and
FMV tracks diverge. `func_800698D4` sets `D_800B0DCD` bit 2 when
`\FMV2\PEDISC02.IDF;1` is present. Add a disc-2 route file when that
path is driven.
