# Day 1 port audit against the matched decomp C (2026-09-28)

Purpose: preservation — the port must run retail logic 1:1. This audit reads
every port function that stands in for a byte-matched decomp function and
records where it diverges. Method: static read of each hand port next to its
matched C (`src/`, `src/overlays/`); nothing here is inferred from
behaviour alone. Raw tables: `build/lanes/shadow/` (git-ignored);
per-system counts: [`audit-systems-summary.tsv`](audit-systems-summary.tsv).
Open divergences are tracked in [`KNOWN_DIVERGENCES.md`](KNOWN_DIVERGENCES.md).

## How much of the EXE path runs matched C

All 2241 matched EXE leaves:

| Port status | Leaves |
|---|---|
| Generated from the matched C | 946 |
| `decomp_hand` adapter (faithful port of the matched C) | 467 |
| Hand port (pre-matching translation) | 692 |
| Hand port with a bootstrap/boundary marker | 37 |
| Cut helpers / file-local copies only | 20 |
| Absent | 79 (51 Psy-Q) |

Hand definitions: 1437 covering 1320 matched addresses. Classification:
123 replaceable by the generated TU now; 222 Psy-Q (stay host-side,
PsyCross migration); 31 host I/O/hardware; 830 need an adapter; 129 are
**partial cuts** (drop part of the retail function).

Overlays: rooms 3194 leaves (1151 generated; most of the rest are identical
bodies credited elsewhere), subsystem overlays `ovl_03C5/03C9/03D2/0457/0700`
partly generated. No battle overlay code is involved (battle logic is in the
EXE).

## Confirmed divergences on the Day 1 path (top 30)

Combat: (1) `func_800299CC` Battle_Update assembled from hand cuts — drops
`func_8003495C`, the `D_8009D23C` script commands, `func_80033430`;
(2) mode-3 cut drops item-0x12 consume, countdowns, per-enemy walk;
(3) `func_80024A3C` phases 4/6 deferred; (4) a duplicate of `func_8001A680`
at 71 call sites. Checked **faithful**: enemy damage formula `func_80028574`,
contact damage, item effects, command queue, death phases, victory init,
equip/stat pipeline.

Death / Game Over: (5) op 0x31 runs a cut `func_80017BB4` that drops the
continue/reset branch; (6) `func_8006A25C` keeps 3 stores, no audio
stop/reset; (7) op 0xED cut drops ~10 keys; (8) `func_8006FC18` skips unknown
destroy callbacks and returns 0.

Sound: (9) `func_80015DAC` default path drops ~25 command keys incl. a second
music-start label; (10) `func_8006A0E8` stubbed; (11) `func_8006F044` stub;
(12) `func_80085644` never sets reverb mode/enable/voice attributes;
(13) nested stream starts never start (`func_8008A92C` boundary,
`func_8008A750` absent); (14) `func_8008CA84` private callback copies.

Field VM / rooms / CD: (15) the VM dispatcher `func_80017018` is a hand port
(47 inline opcodes, no fallback); (16) `func_8006C5BC` state 13 skips field
inits; (17) `func_8006D60C` drops an SFX pair, timer latches, calls, and passes
0 instead of `D_8009D188`; (18) analog-stick walk path returns early;
(19) `func_800698D4` wrong return when the drive is not ready;
(20) `func_8006D078` returns instead of looping (a frame late);
(21) `func_8006B4F8` still cut helpers (room SFX tail fixed, rest re-check).

Rendering: (22) `func_8003F074` cut drops GTE lighting setup,
SetGeomScreen, a call and a flag (generated TU suppressed);
(23) `func_8006CC68` never calls `func_8003AC90`/`func_8003AF14`;
(24) `func_8003A088` mode-0 no-op each frame; (25) `func_80068B94` drops
SetGeomScreen and the camera-seed projection; (26) `func_80069B08` stub of a
10-state disc-read/title machine.

Menus / items / memory card: (27) `func_8005D2B4` 16 of 21 script commands
are no-ops (inventory capacity, slots, item/key-item removal, counters);
(28) `func_80043DA4` commands 7/8 stop the port; (29) `func_8004C608`
memory-card help ids stop the port; (30) memory-card state machine
`func_80041108`/`func_80040F80` mostly unimplemented.

## Behaviour checks (numbers, not reading)

- Combat damage (owner: "enemies die in one shot"): the damage chain matches
  the matched C; the retail video's first rat (part 01 @ ~15:53–16:03) dies to
  one attack command, the port's needs two — not easier. **Still open:** the
  owner's exact binary in normal mode, Eve's damage on Aya without aids, sewer
  rats vs part 02.
- Movies: every Day 1 movie plays its own file with retail length
  (FMV002 68.15 s vs 68.1 s; FMV003 76.8 s vs ~76.3 s; FMV004 37.45 s vs 37.1 s).
- Field SFX: footstep cadence and loudness match part 01 @ 5:16–5:28.

## Fix order

1. Wire op 0x31 / 0xED to the complete ports; fix `func_8006A25C` (Game Over).
2. Replace the battle tick with the matched C (`func_800299CC`, `func_80024A3C`).
3. Renderer primitive root cause (particle streaks, rats, gun); items 22–26.
4. Sound items 9–14; pause; Eve 2 vocals; audio sweep.
5. Menus/items/memory card 27–30; save persistence.
6. Field VM dispatcher and the remaining partial cuts; then the 123
   replaceable hand ports, route-tested in batches.

No build goes to the owner until the combat and Game Over fixes are verified.
