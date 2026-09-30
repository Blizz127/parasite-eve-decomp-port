# PE-SYS0 — Day 1 playable-slice acceptance contract

```text
contract_id=PE-SYS0
day1_contract_status=DEFINED
documentation_only=yes
production_implementation=no
gameplay_changes=no
visual_fidelity_reopened=no
```

This file defines, before further implementation, what it means for:

```text
"Parasite Eve Day 1 / Part 1 is properly playable"
```

It is the promotion gate for Python research/oracle lanes, the native
clean runtime, the UE5 runtime, and future PT builds. Outcome-faithful
shims, temporary presentation approximations, and partially-proven
systems must not silently become production authority.

Companion files:

| File | Role |
|---|---|
| `FIDELITY_PROMOTION_POLICY.md` | debt classes and stage promotions |
| `DAY1_SYSTEM_GATE_MATRIX.csv` | gates A–L |
| `DEBT_REGISTRY_SCHEMA.md` | debt schema + currently-evidenced seed |
| `UE_NATIVE_PARITY_POLICY.md` | retail-state parity, traces, Python demotion |

## 1. Authority hierarchy

```text
Retail Disc
  = content authority

Retail EXE / ASM / script bytecode
  = behavior + interpretation authority

Blizz127/Parasite-Eve-Decompilation
  = retail / matching-decomp / native gameplay authority

Native runtime in this repo (pc_port/)
  = promoted gameplay semantics, once oracles pass

Blizz127/parasite-eve-ue5
  = presentation consumer of promoted native behavior

Python clean runtime
  = research + executable oracle + trace generator
```

Old PC ports are **not** fidelity authority. UE5 does not own
gameplay research.

Generated or licensed retail payloads remain untracked and gitignored.

Unknown semantics remain explicitly unknown. They are not guessed into
production.

Registered USA Disc 1 / `SLUS-00662` facts used by this contract:

| Input | SHA-256 | Source |
|---|---|---|
| Disc 1 raw Mode2/2352 | `7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4` | PE-RD0 |
| `SLUS_006.62;1` | `5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b` | PE-RD0 / matching rebuild |
| `PE.IMG` logical Form1 | `c3709b2d29d3a0a344d6da1ef134288aff360b08e27bf034c303f02ee3eb4bb4` | PE-RD0 |

The matching-decomp / native-bootstrap lane on this checkout is a
parallel behavior-oracle lane. It is **not** the Day 1 playable runtime
and does not confer playable-slice acceptance.

## 2. Frozen visual contract (do not reopen in SYS0)

PT1 visual freeze commit: `3e4c65d`.

| Layer | Commit | Status |
|---|---|---|
| CAM-B | `1bf3832` | FROZEN CORRECT |
| VIS-B | `ac62411` | FROZEN CORRECT |
| VIS-C | `cad4598` | FIDELITY VERIFIED |

Freeze includes:

- native 320×224 field backgrounds
- retail layer/pan behavior
- retail 52-byte field view `MATRIX`
- GTE-style H/SZ projection
- NCLIP `MAC0 > 0`
- 4096-entry ordering table
- retail OTZ
- `addPrim` bucket-head behavior
- far → near drawing

Aya scaling hacks are forbidden.

The player 23×16 auxiliary table is bounding spheres `(cx, cy, cz, r)`,
**not** bone translations.

SYS0 does not reopen visual fidelity. Visual work is out of scope here
unless new retail bytes contradict the freeze.

## 3. What "Day 1 complete" means

Day 1 acceptance is a **bounded first-play human route**, not a room
count and not "the whole game."

A build may be called Day 1 complete only when a human can play the
retail first-play chain from cold boot through a **provenance-backed**
Day 1 completion boundary, using retail-authoritative behavior for every
critical-path system named in §5–§12.

Story-room count is no longer the primary acceptance blocker. The
current proven field prefix is already multi-room. The remaining
acceptance blockers are unproven later first-play identity (from
`m0377i` onward), text presentation, combat, FMV playback, persist
promotion, and save/load if the route requires them.

### 3.1 Proven first-play prefix (current story floor)

Canonical current route, playable in the Python research runtime:

```text
m0002i  ->  m0003i  ->  m0372i  ->  m0004i  ->  m0378i
```

| Rung | Commit | Claim |
|---|---|---|
| RD5-C2 | `1e7f0df` | m0004i reel matches reconciled dialogue-close + cut-record 1 |
| RD6-A | `65446c8` | m0004i north exit loads playable m0378i |
| RD6-B | `a0baebc` | first-play forward dest from m0378i is m0377i |

Next forward destination:

```text
m0377i
token=0xA80673C8
table_index=376
package=PE.IMG [0x155D0,0x15633)
sha256=eb9830559cd44802af33d81267704a6eda2067ad418fb8e21e3c2548fd8730a4
research=PE-RD7-R pending
```

This prefix is **not** Day 1 complete.

### 3.2 Day 1 acceptance route (bounded)

The acceptance route is the retail first-play chain:

```text
cold boot
  -> opening FMV (critical-path identity RESEARCH_REQUIRED)
  -> curb / limousine arrival + dialogue
       observed in PE-RD0; m0001i as predecessor is TENTATIVE
  -> m0002i Carnegie Hall sidewalk (first proven controllable field)
  -> m0003i Carnegie Hall lobby
  -> event-driven BGM (0xC8/0xC9 handles)
  -> m0372i first-play cutscene + messages 0x14..0x20
  -> FMV003 (opcode 0x35 3; proven on this arm)
  -> m0004i messages 0x21..0x23 + control restore
  -> m0378i
  -> m0377i (identity proven; destination contract RESEARCH_REQUIRED)
  -> every subsequent first-play field / event / combat node
       that retail actually reaches before the Day 1 completion marker
  -> Day 1 boss (identity RESEARCH_REQUIRED)
  -> post-battle progression (identity RESEARCH_REQUIRED)
  -> Day 1 completion boundary (identity RESEARCH_REQUIRED)
```

Do **not** invent later Carnegie Hall rooms, first-combat identities,
boss identity, or a Day 2 start room from memory or from old PC ports.
Those nodes stay `RESEARCH_REQUIRED` until a retail package token,
script arm, or EXE consumer proves them.

### 3.3 Scene / event identity table

Descriptive English names are observational, not retail identifiers.

| Node | Packed token | Proven role | Route status |
|---|---|---|---|
| Opening FMV | RESEARCH_REQUIRED | RD0 observed opening/limousine material before curb dialogue | RESEARCH_REQUIRED (file not opcode-mapped) |
| `m0001i` | `0xA80000C8` | lobby/m0378i later-game south dest; RD3-R labels a curb back-link | TENTATIVE as first-play predecessor; PROVEN as dest token |
| `m0002i` | table[1]; return token `0xA8000148` | Carnegie Hall snowy sidewalk; first proven controllable field | PROVEN playable |
| `m0003i` | `0xA80001C8` | Carnegie Hall lobby | PROVEN playable |
| `m0372i` | `0xA8067148` table[371] | first-play cutscene; messages `0x14..0x20` | PROVEN cutscene; text IDs only |
| FMV003 | ISO `FMV1/FMV003.STR;1` via `0x35 3` | m0372i `0x0F` arm, after seq27 start | PROVEN event; playback not implemented |
| `m0004i` | `0xA8000248` | post-cutscene field; messages `0x21..0x23`; then playable | PROVEN playable after RD5-C2 |
| `m0378i` | `0xA8067448` table[377] | m0004i north first-play dest | PROVEN playable |
| `m0377i` | `0xA80673C8` table[376] | m0378i south first-play dest (`persist[0x4A] < 0x30`) | PROVEN identity; dest contract RESEARCH_REQUIRED |
| First combat encounter(s) | RESEARCH_REQUIRED | none of the proven prefix rooms contain battle evidence | RESEARCH_REQUIRED |
| Day 1 boss | RESEARCH_REQUIRED | no provenance-backed identity | RESEARCH_REQUIRED |
| Day 1 completion marker | RESEARCH_REQUIRED | no provenance-backed persist/script/map end | RESEARCH_REQUIRED |

Optional / not first-play-critical (proven as authored, not required
to accept the first-play forward route):

| Node | Status |
|---|---|
| m0003i type-2 talk (`0x0D`/`0x22` ids `0x11`/`0x12`) | optional NPC; STRONG, not first-play mandatory |
| m0003i / m0378i return volumes to prior maps | return, not forward |
| m0378i / m0003i south dest `m0001i` when `persist[0x4A] >= 0x30` | later-game arm; not first-play |

## 4. Current floors (not acceptance)

### 4.1 Field

Playable Python research prefix through `m0378i` with retail collision
meshes, authored volume hops, inhibit/`0x3F` restore, and RD2M
pad/velocity/yaw. Mailbox/task machinery on m0004i / m0378i is
**outcome-faithful**, not task-state faithful
(`NONBLOCKING_FIDELITY`; see promotion policy).

### 4.2 Text

Message **IDs** are proven (`0x14..0x20` on m0372i; `0x21..0x23` on
m0004i; optional lobby `0x11`/`0x12`). Retail string bytes, encoding,
glyph/font decode, window layout, and wrapping are **unresolved**.
m0004i close rule is proven (terminal `0xFF` then newly-pressed
`D_8009D1F4 & 0x100`). m0372i uses authored `0x02` waits, not `0x22`
close. Day 1 **cannot PASS** on IDs only.

### 4.3 Audio

AUD1-D `40b7c3f`:

```text
C8 1E -> seq26 start   (m0003i +0x0444)
C9 1E -> seq26 stop    (m0372i)
C8 1F -> seq27 start   (m0372i +0x06FC; one-shot)
C9 1F -> seq27 stop
0x199 0x88 -> sound-worker command; NOT a BGM sequence selector
m0004i / m0378i -> no invented AKAO
```

Known fidelity debt: dry/incomplete reverb; linear interpolation;
incomplete articulation/collection selection; SFX/BGM mix; FMV/XA
pipeline incomplete. Human speaker verification still pending on AUD1-D.

### 4.4 Battle / menus / save

No current-route room has battle evidence. PT1 (`e5d568c` packaging)
explicitly has no save/load, no combat, and no menus. Native
`pc_port/platform/pe_save.c` is libcard save-manager bring-up, not
game-persist serialization.

## 5. System gates (summaries)

Full rows live in `DAY1_SYSTEM_GATE_MATRIX.csv`.

### A. FIELD

Minimum: load each critical-path field from the retail package table;
draw native 320×224 backgrounds under the frozen camera; place Aya on
the retail collision mesh; walk/run with retail pad→yaw→velocity;
honor authored `0x77` volumes and `0x31` tokens; inhibit on hop;
restore via opcode `0x3F`; do not leak one frame of pad across a
transition.

Unacceptable: background-derived collision; invented tokens; Aya
scale hacks; skipping inhibit/restore.

### B. TEXT

Minimum: retail string bytes, retail text encoding, glyph/font decode,
window layout, line wrapping, message close/advance, speaker/name
behavior where retail presentation requires a rendered name.

Unknown speaker names may remain IDs only if retail itself does not
require a rendered name.

Day 1 cannot PASS with message IDs only.

### C. BATTLE

Day 1 cannot PASS without actual combat. See §7.

### D. MENUS

Only surfaces proven necessary to complete the Day 1 route. See §8.

### E. PERSISTENCE

`persist[]` promotion policy in §9. No semantic name becomes
authoritative merely because a value appears to work.

### F. SAVE/LOAD

If the proven route requires save or load, serialize
retail-authoritative state, not clean-runtime convenience booleans.
Whether Day 1's critical path requires a save point is
`RESEARCH_REQUIRED`.

### G. AUDIO

Event-driven BGM from retail `0xC8`/`0xC9` handles. Do not autostart
a sequence on scene load. Do not treat `0x199 0x88` as a sequence
selector. Do not invent AKAO for rooms whose packages have none.

### H. FMV

Critical-path FMVs must play from retail STR (demux + MDEC + XA where
present) with synchronization and a return to field/script state.
No prerendered replacement video unless marked non-production.

### I. INPUT

Retail processed pad words (`D_8009D26C` held, `D_8009D1F4` newly
pressed). Field bits and the message-advance mask `0x100` are proven.
Battle/menu button bindings remain `RESEARCH_REQUIRED` until those
surfaces are.

### J. CAMERA / RENDER

The PT1 freeze. Native 320×224. No Aya scaling. No CAM-A zoom.

### K. DETERMINISM

Accepted oracle traces must replay 3/3 with a stable SHA-256. A test
pass on approximate semantics is not proof.

### L. UE5 / NATIVE PARITY

Retail-derived state equality, not visual similarity. See
`UE_NATIVE_PARITY_POLICY.md`.

## 6. Text acceptance

Required for Day 1 PASS:

| Requirement | Current evidence |
|---|---|
| Retail string bytes | UNRESOLVED (slot7 streams hashed; IDs not mapped to glyphs) |
| Retail text encoding | UNRESOLVED |
| Glyph / font decode | UNRESOLVED (`0x21`/`0x22`/`0x23` glyph bytes explicit debt) |
| Window layout | UNRESOLVED |
| Line wrapping | UNRESOLVED |
| Message close / advance | PROVEN on m0004i `0x21..0x23`: parser `1→2` at terminal `0xFF`, then newly-pressed `D_8009D1F4 & 0x100` clears state. Held input does not close. m0372i `0x14..0x20` use authored `0x02` waits |
| Speaker / name | m0004i `a1=0`; no invented speakers. Render a name only if retail presentation requires it |

Slot7 stream hashes (m0004i; both streams share the same terminal
structure for `0x21..0x23`):

| stream | package offset | size | SHA-256 |
|---|---:|---:|---|
| slot7[0] | `+0x9A618` | `0x1956` | `467bf214b60e2b6d2972d0ce8d8cd8898ec2908f70ed65ed63802b11ad88eba1` |
| slot7[1] | `+0x9BF70` | `0x2A25` | `231da6258a69d8c44e49509164973ef2236cabf4cd40bcce4c973ce7b91c262a` |

Selector: `D_800B0CD8 & 0x40000000`. Window records: four 56-byte
slots; state at `+0`, s16 id at `+0x10`. RD5-X places the table at
`D_800BCEA8` (not `0x800CCEA8`).

## 7. Battle acceptance

No battle feature may be required "because the game has it later."
Only what the proven Day 1 route actually exercises.

Identities of the first encounter(s) and the Day 1 boss are
`RESEARCH_REQUIRED`. The **gates** below still apply: Day 1 cannot
PASS by skipping combat or by substituting a scripted HP cut.

### 7.1 MUST_HAVE_FOR_DAY1

| Surface | Minimum production behavior | Identity status |
|---|---|---|
| Field → battle handoff | retail trigger / opcode / overlay load; inhibit field; enter battle with retail participant set | RESEARCH_REQUIRED |
| Participant / resource init | retail battle structs, not host convenience actors | RESEARCH_REQUIRED |
| ATB / timing | retail timing source and command-ready rule for the encounters on the route | RESEARCH_REQUIRED |
| Player command selection | the command surface those encounters actually offer | RESEARCH_REQUIRED |
| Range / target | retail range/target rules used by those encounters | RESEARCH_REQUIRED |
| Normal attack | executable and damage-dealing | RESEARCH_REQUIRED |
| Required PE commands | only PE commands the Day 1 route requires | which PE: RESEARCH_REQUIRED |
| Enemy behavior | the specific enemy programs on that route | RESEARCH_REQUIRED |
| Damage | retail formula for those attacks, not a host approximation that "feels close" | RESEARCH_REQUIRED |
| HP / death | retail HP fields; death removes or ends the unit as retail does | RESEARCH_REQUIRED |
| Battle completion | retail win/lose/escape outcomes that the route uses | RESEARCH_REQUIRED |
| EXP / BP / reward | if retail awards them on those fights, apply the retail grants to persist/actor state | RESEARCH_REQUIRED |
| Return to field | restore the correct map, persist, pose, inhibit, and camera | RESEARCH_REQUIRED |
| Day 1 boss | actual boss fight, not a skip or cinematic substitute | identity RESEARCH_REQUIRED |

### 7.2 MAY_DEFER_AFTER_DAY1

- PE commands not used on the Day 1 route
- later-game enemy programs and status systems not exercised
- extra battle camera / presentation polish beyond playable retail
  command/resolution
- tool/item/armor systems not required to finish those fights
- party members not present on the proven Day 1 route
- any battle feature whose retail use on this route is unproven

### 7.3 Unacceptable shortcuts

- "press X to win" or auto-resolve
- imported stats from an old PC port
- outcome-faithful HP numbers with no command/ATB/target loop, once
  this gate is claimed
- skipping the boss because the next field token can be forced

## 8. Menu acceptance

Do not automatically require every later-game menu.

| Surface | Day 1 requirement | Evidence |
|---|---|---|
| Field item | RESEARCH_REQUIRED | no proven first-play prefix use |
| Equipment | RESEARCH_REQUIRED | no proven first-play prefix use |
| Status | RESEARCH_REQUIRED | no proven first-play prefix use |
| PE (field) | RESEARCH_REQUIRED as a field menu; battle PE is §7 | no proven first-play prefix use |
| Options | RESEARCH_REQUIRED | not required to complete the proven prefix |
| Save / load UI | required **only if** a proven Day 1 critical-path save/load site exists | PT1 has none; site identity RESEARCH_REQUIRED |
| Battle command menu | MUST_HAVE_FOR_DAY1 once combat is on the route | battle identity RESEARCH_REQUIRED |
| Optional lobby talk | not a menu; field `0x0D`/`0x22` | STRONG; not mandatory |

A menu becomes mandatory only when a first-play script arm, EXE
consumer, or retail observation proves the route cannot complete
without it.

## 9. Persistence acceptance

`persist[]` is retail script-visible state. The research runtime models
indices `0..0x4A` as 32-bit words because those are the indices read
on the current prefix. That model width/length is **not** a promoted
retail array layout.

### 9.1 Promotion rule

No semantic name may become authoritative merely because a value
appears to work.

Every promoted entry must record:

| Field | Meaning |
|---|---|
| `index` | persist subscript |
| `width` | proven access width |
| `proven_readers` | exact consumers (scene + PC / handler) |
| `proven_writers` | exact producers |
| `known_values` | values observed with comparands |
| `reset_behavior` | boot / scene-entry / hop |
| `save_persistence` | present in retail save block? |
| `semantic_confidence` | PROVEN / STRONG / INTERPRETED / RESEARCH_REQUIRED |
| `provenance` | evidence commit + document |

Until `save_persistence` is proven, a named convenience boolean must
not replace the raw word.

### 9.2 Currently evidenced entries (not a complete map)

| index | width | proven readers | proven writers | known values | reset | save | confidence | provenance |
|---:|---|---|---|---|---|---|---|---|
| 0 | 32-bit word (bit tests) | m0004i first-play `persist[0] & 2` | m0004i `0x09` `& ~2`; `0x0A` `= 0` | `0` after reel; bit 2 must be clear for first-play arm | not re-zeroed by hops that carry the bank | RESEARCH_REQUIRED | PROVEN as flag word; **no English name promoted** | RD5-A/B/C/X |
| 1 | 32-bit word | dest startup pose arms (`==2`, `==3`, `==4`, `==0x17A`, `==0x179`) | hop `0x0A` writes `2`, `3`, `4`, `0x17A` | `2` sidewalk entrance; `3` lobby leftover into m0372i/m0004i; `4` m0004i→m0378i; `0x179` other m0378i door; `0x17A` = 378 | **not** zeroed across hops (RD4-A / RD6-A) | RESEARCH_REQUIRED | PROVEN as source-map numeric id | RD3-B, RD4-A, RD6-A/B |
| `0x19` | 32-bit word | m0003i `+0x03E4` `& 0x08000000` | RESEARCH_REQUIRED | RESEARCH_REQUIRED | RESEARCH_REQUIRED | RESEARCH_REQUIRED | reader only | RD4-A `LIVE_SCRIPT_PATH.md` |
| `0x4A` | 32-bit word | `< 9`, `< 0x11`, `< 0x18`, `< 0x30`, `> 0x78` | m0002i first-control `9`; m0372i `9→0x11→0x12`; m0004i `0x12→0x18` | `9`, `0x11` (required mid-cutscene checkpoint), `0x12`, `0x18`; later comparand `0x30` | not zeroed across hops | RESEARCH_REQUIRED | PROVEN first-play checkpoint word; **no English name promoted** | RD3-B through RD6-B |

`D_800B6A80[0x14]` is a related bank-4 flag used by m0003i north
progression. It is **not** `persist[]` and must not be collapsed into
it.

### 9.3 Mailbox / task state

Mailbox records are 12-byte entries at `D_800A3180` (type, id,
payload, extra, sender serial). Delivery creates a task
(`task+0x14 = payload`). This is **not** persist.

Current m0004i / m0378i implementations may be outcome-faithful in
research/PT. That class **cannot** promote to Native/UE production or
Day1 Accepted once battle or save/load share the same task/mailbox
state. Exact task-state becomes mandatory at that boundary.

## 10. FMV acceptance

Critical-path FMVs must not remain timing placeholders.

Minimum for each critical-path FMV:

1. STR demux of the retail ISO file
2. MDEC video
3. XA audio where the STR contains it
4. Synchronization with the authored script wait / overlay return
5. Transition back to field/script state without dropping persist,
   mailbox, or inhibit

Proven critical-path event on the current prefix:

```text
m0372i module1 +0x0478  opcode 0x35 3
  -> overlay table index 3
  -> \FMV003.STR;1
  -> XA lives inside the STR, not PE.IMG
```

Opening FMV is RD0-observed on the cold-boot path. The exact STR file
for that opening is `RESEARCH_REQUIRED` (ISO contains `FMV000` through
`FMV017A/B`; no SYS0 opcode map for cold boot).

No prerendered replacement video unless the artifact is explicitly
marked non-production. Recording `0x35` as a trace row is research
telemetry, not playback.

## 11. Input acceptance

Proven field contract (PE-RD2A `INPUT_PATH.md`):

- libpad `PadInitDirect` / `StartPAD`, not `PadRead()`
- processed held `D_8009D26C`; newly pressed `D_8009D1F4`;
  Circle/Cross swapped
- field bits: run `0x01`, up `0x08`, right `0x10`, down `0x20`,
  left `0x40`
- inhibit `D_8009D2E8` bit 0 (opcode `0x40` set / `0x3F` clear)
- digital 8-way snap to camera heading; analog replaces direction bits

Message advance uses processed mask `0x100`. The physical controller
label is not required for the VM contract.

Battle and menu bindings are `RESEARCH_REQUIRED`.

## 12. Determinism and human verification

Every promoted scene/system needs:

- a documented oracle trace
- 3/3 identical SHA-256 on the declared column set
- the command that produced it

Human verification is required for field traversal, readable text,
audio, FMV, combat, and any required menu/save UI. It is **not** a
substitute for the oracle. Visual presentation of Aya/camera is
already frozen and is not re-litigated by SYS0.

## 13. Python / UE / native

Once a surface is implemented natively (UE5 or native runtime) and
passes the parity contract, Python becomes oracle/test infrastructure
for that surface.

Production **must not** fall back to Python rendering, field
simulation, collision, script execution, or story behavior.

Python may still generate traces, decode evidence, generate fixtures,
and perform independent comparisons.

No split simulation authority.

Details: `UE_NATIVE_PARITY_POLICY.md`,
`FIDELITY_PROMOTION_POLICY.md`.

## 14. Day 1 PASS / FAIL

Day 1 PASS requires **all** of:

1. Every `RESEARCH_REQUIRED` node on §3.2 is either proven or proven
   absent from the first-play route.
2. Gates A–L are `Day1 Accepted` under the promotion policy.
3. Text is not IDs-only.
4. Combat actually happens, including the Day 1 boss.
5. Critical-path FMVs play from retail STR.
6. Persist used by the route is promoted; save/load (if required)
   serializes that state.
7. UE/native (or the then-declared production runtime) matches
   accepted oracle traces row-for-row.
8. Python is not production authority for any accepted surface.

Current result: **FAIL** (`day1_acceptance_ready_now=no`).

---

## FINAL REPORT

```text
day1_contract_status=DEFINED

field_gate=DEFINED
text_gate=DEFINED
battle_gate=DEFINED
menu_gate=DEFINED
persistence_gate=DEFINED
save_load_gate=DEFINED
audio_gate=DEFINED
fmv_gate=DEFINED
input_gate=DEFINED
render_gate=DEFINED
determinism_gate=DEFINED
ue_native_parity_gate=DEFINED

current_field_status=PROVEN_PREFIX_M0002I_TO_M0378I
current_text_status=RECORD_IDS_ONLY
current_battle_status=RESEARCH_REQUIRED
current_menu_status=NO_PROVEN_REQUIRED_FIELD_MENU
current_persistence_status=PARTIAL_UNPROMOTED
current_audio_status=HANDLE_TIMELINE_PROVEN_WITH_FIDELITY_DEBT
current_fmv_status=EVENT_RECORDED_NOT_PLAYED

largest_current_blocker=day1_combat_boss_and_completion_unproven
next_system_research_priority=PE-RD7-R_m0377i_first_play_destination_contract

python_production_fallback_allowed=no
retail_bytecode_runtime_authority=yes

day1_acceptance_ready_now=no

hard_blockers=day1_combat_identity_and_runtime_absent; day1_boss_identity_absent; day1_completion_boundary_absent; text_ids_only; fmv003_not_played; opening_fmv_identity_unmapped; persist_save_mapping_unknown; m0377i_destination_contract_incomplete; mailbox_task_state_not_exact
warnings=do_not_reopen_pt1_visual_freeze; aya_scaling_forbidden; 23x16_aux_is_bounding_spheres_not_bone_T; do_not_invent_bgm_or_story_rooms; outcome_faithful_mailbox_cannot_promote_once_battle_or_save_share_task_state; persist_names_are_not_authority; 0x199_0x88_is_not_a_sequence_selector; 0x3C_is_not_fov; pt1_has_no_save_load_or_combat; aud1d_human_speaker_verification_pending; m0001i_to_m0002i_writer_tentative

SUCCESS
PE-SYS0 SUCCESS — DAY 1 ACCEPTANCE AND FIDELITY PROMOTION CONTRACT FROZEN
```
