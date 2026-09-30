# PE-SYS0 — Debt registry schema

```text
schema_id=PE-SYS0-DEBT
documentation_only=yes
invented_claims=no
```

This file defines how fidelity debt is recorded. It seeds **only**
debts that already appear in accepted evidence. It is not a complete
inventory of the game, and empty cells stay empty.

A later registry CSV may instantiate this schema. Do not add rows
because a system "must exist" in Day 1. Add rows when a report names
the gap.

## 1. Schema

| Field | Required | Allowed values / notes |
|---|---|---|
| `debt_id` | yes | Stable id `DEBT-SYS0-NNN` or a rung-local id already used in evidence |
| `surface` | yes | `field` `text` `battle` `menu` `persist` `save` `audio` `fmv` `input` `render` `determinism` `mailbox` `other` |
| `class` | yes | `PROVEN_EXACT` `PROVEN_OUTCOME` `NONBLOCKING_FIDELITY` `PRESENTATION_ONLY` `RESEARCH_REQUIRED` `BLOCKER` |
| `summary` | yes | One sentence. No new story claims. |
| `retail_authority` | yes | EXE range, package range, opcode, or `unmapped` |
| `evidence` | yes | Commit + document path |
| `allowed_research` | yes | `yes` / `no` |
| `allowed_pt` | yes | `yes` / `yes_labeled` / `no` |
| `allowed_ue_native` | yes | `yes` / `no` |
| `allowed_day1` | yes | `yes` / `no` |
| `blocks_promotion_to` | no | Stage that cannot be entered while this row stays in this class |
| `promotion_condition` | no | What evidence flips the class |
| `notes` | no | Proven unknowns only |

Class meanings and stage permissions:
`FIDELITY_PROMOTION_POLICY.md`.

Persist rows that later enter this registry must also carry the persist
promotion fields from the Day 1 contract §9:

```text
index,width,proven_readers,proven_writers,known_values,
reset_behavior,save_persistence,semantic_confidence,provenance
```

Those fields are **not** filled here for unpromoted persist words.
The contract table is the current census, not a debt invention.

## 2. Seed (currently evidenced only)

| debt_id | surface | class | summary | retail_authority | evidence | allowed_research | allowed_pt | allowed_ue_native | allowed_day1 | blocks_promotion_to | promotion_condition |
|---|---|---|---|---|---|---|---|---|---|---|---|
| DEBT-SYS0-001 | audio | NONBLOCKING_FIDELITY | Seq26 declares reverb type 5 (`C2`/`FC 02`) but the bounded mixer emits a dry mix. | AKAO seq26 events; type-5 register program unresolved | AUD1-A `MIXER_CONTRACT.md`; AUD1-D `40b7c3f` | yes | yes_labeled | no | no | Native/UE Parity for mix-exact audio | Retail type-5 reverb program recovered and waveform-compared |
| DEBT-SYS0-002 | audio | NONBLOCKING_FIDELITY | Sample path uses linear interpolation; Gaussian interpolation is not claimed. | SPU voice path in AUD1-A mixer | AUD1-A `MIXER_CONTRACT.md`; AUD1-D warnings | yes | yes_labeled | no | no | Day1 Accepted for mix-exact audio | Proven SPU interpolator implemented and hashed |
| DEBT-SYS0-003 | audio | NONBLOCKING_FIDELITY | Articulation / collection selection incomplete; bank-0 fallback arts retained. | AKAO collection / art tables not fully bound | AUD1-D `40b7c3f` warnings | yes | yes_labeled | no | no | Day1 Accepted for mix-exact audio | Collection/art binding proven per used sequence |
| DEBT-SYS0-004 | audio | NONBLOCKING_FIDELITY | SFX / BGM mix fidelity incomplete. | SFX banks and mix bus unmapped | AUD1-D report; project audio floor | yes | yes_labeled | no | no | Day1 Accepted for mix-exact audio | SFX path and mix bus proven |
| DEBT-SYS0-005 | audio | RESEARCH_REQUIRED | `0x199 0x88` SDRV payload still unknown. Proven **not** a seq26/27 selector. | `0xEA 0x199 0x88` → mailbox `{0x88,0,0x88,0}` + `D_800B0DBE` | AUD1-C `WORKER` / AUD1-D `40b7c3f` | yes | yes_labeled as non-BGM | no as BGM | no as BGM | n/a for BGM timeline | Payload semantics recovered without changing the negative BGM ruling |
| DEBT-SYS0-006 | fmv | BLOCKER | FMV003 is a proven first-play event (`0x35 3`) but is only recorded, not played (no STR demux / MDEC / XA). | ISO `FMV1/FMV003.STR;1`; overlay index 3 | AUD1-C `SONG_TABLE.csv`; AUD1-D timeline; RD0 `DISC1_INVENTORY.csv` | yes as a named stop | no as production FMV | no | no | Day1 Candidate | STR/MDEC/XA playback + sync + return-state oracle |
| DEBT-SYS0-007 | fmv | RESEARCH_REQUIRED | Opening / limousine FMV observed on cold boot; exact STR file not opcode-mapped. | ISO `FMV000`–`FMV017B` exist; cold-boot consumer unmapped | PE-RD0 `REPORT.md` | yes | no as a specific file claim | no | no | Day1 Candidate | Opcode/overlay map from cold boot to a hashed STR |
| DEBT-SYS0-008 | text | BLOCKER | First-play messages are IDs only (`0x14..0x20`, `0x21..0x23`; optional `0x11`/`0x12`). Glyph bytes unresolved. | slot7 streams hashed; parser markers `F9/FF FE <id>` | RD4-E; RD5-C; RD5-X `MESSAGE_CLOSE_RULE.md`; RD5-C2 `1e7f0df` | yes | yes_labeled IDs-only | no | no | Day1 Candidate | Retail string bytes + encoding + glyph decode + layout + wrap |
| DEBT-SYS0-009 | text | PROVEN_EXACT | m0004i `0x22` close rule is proven (terminal then new-press `0x100`). Residual: physical button name for mask `0x100` is unnamed. | `func_80037870`; `func_8003EB04`; `D_8009D1F4` | RD5-X `MESSAGE_CLOSE_RULE.md`; RD5-C2 | yes | yes | yes (mask, not a guessed button label) | yes | n/a | Optional: name the Sony bit only if an EXE consumer proves it |
| DEBT-SYS0-010 | mailbox | PROVEN_OUTCOME | m0004i / m0378i mailbox/task machinery is outcome-faithful, not task-state faithful (re-arm / `+0x05B8` tail). | `D_800A3180` 12-byte records; `func_800653B8` / `func_80065400` / `func_80012700` | RD5-X `MAILBOX_PROOF.md`; RD5-C2; RD6-A `65446c8` | yes | yes_labeled | no once battle/save share task state | no once battle/save share task state | Native/UE Parity when battle or save share the tables | Exact task records + re-arm PCs |
| DEBT-SYS0-011 | field | RESEARCH_REQUIRED | m0377i first-play destination identity is proven; Y snap and full dest contract are not. | token `0xA80673C8`; package `PE.IMG [0x155D0,0x15633)` SHA `eb983055...` | RD6-B `a0baebc` `NEXT_DESTINATION.md` | yes | no as a playable room | no | no | Day1 Candidate | PE-RD7-R dest contract |
| DEBT-SYS0-012 | field | RESEARCH_REQUIRED | `m0001i → m0002i` curb-to-sidewalk writer remains TENTATIVE. | token `0xA80000C8`; RD3-R back-link label | RD07C `REPORT.md`; RD3-A `REPORT.md` | yes | no as a proven hop | no | no | Day1 Candidate if curb is on the critical path | Writer + persist/pose contract proven |
| DEBT-SYS0-013 | persist | RESEARCH_REQUIRED | Retail persist array base, length, and save-block mapping are unmapped. Research models `0..0x4A` only. | script `0x09`/`0x0A` consumers | RD4-A through RD6-B; contract §9 | yes | yes as raw words 0..0x4A | no as a named layout | no | Day1 Candidate if save is required; Native/UE Parity for any persist aliasing | Base/length/save map from EXE |
| DEBT-SYS0-014 | persist | RESEARCH_REQUIRED | `persist[0x19] & 0x08000000` is a proven reader; writer, values, reset, and save behavior are unknown. | m0003i `+0x03E4` | RD4-A `LIVE_SCRIPT_PATH.md` | yes | yes as an opaque test | no named | no named | n/a until written on the route | Reader/writer/value census |
| DEBT-SYS0-015 | save | RESEARCH_REQUIRED | No proven Day1 critical-path save/load site. PT1 has no save/load. Native `pe_save.c` is libcard bring-up only. | libcard / save-manager leaves exist; game block unmapped | PT1 `PE_PT1_FEEDBACK.md`; `pc_port/platform/pe_save.c` | yes | yes without save | no as game save | no as complete | Day1 Candidate if a site is later proven | Save-block map + round-trip oracle |
| DEBT-SYS0-016 | battle | BLOCKER | No first-combat encounter, Day1 boss, or Day1 completion marker is provenance-backed. No battle runtime. | unmapped | RD0 `BEHAVIOR_SURFACE.csv` (`battle=LATER`); RD4-E `battle=no`; PT1 no combat | yes as unknown | no | no | no | Day1 Candidate | Proven encounter/boss/completion nodes + combat oracles |
| DEBT-SYS0-017 | render | PRESENTATION_ONLY | Camera init-table **bytes** (16-byte entry layout) unproven on m0378i; OR-2 onto records 1 and 7 **outcome** is hard-gated. | `func_800655D4` OR-2 outcome | RD6-A `65446c8` warnings | yes | yes | no as a byte layout claim | no as a byte layout claim | n/a for layer outcome | Prove 16-byte entry fields |
| DEBT-SYS0-018 | field | RESEARCH_REQUIRED | m0378i unknowns: `0x9B`/`0xED` operands; `0x41` `actor+0x98` bit `0x40` gameplay meaning; type-2/3 clip-2 visual identity. | m0378i script | RD6-B `a0baebc` | yes | yes if unused on first-play forward | no guessed | no guessed | n/a until a later arm needs them | Operand/bit/clip census |
| DEBT-SYS0-019 | audio | PRESENTATION_ONLY | AUD1-D physical speaker verification is still pending. PCM oracles exist. | seq26/27 PCM SHAs | AUD1-D `40b7c3f` | yes | yes_labeled | yes after speaker check | yes after speaker check | Day1 Accepted for audio | Human speaker verification recorded |
| DEBT-SYS0-020 | audio | NONBLOCKING_FIDELITY | PT1 two-room playtest still has no Event-3, so lobby BGM stays silent in that packaged PT. | m0003i `0xC8 0x1E` is Event-3 | AUD1-D warnings | yes | yes_labeled on the old PT1 package | no if that package is claimed current | no | n/a for current AUD1-D runtime | PT package includes the handle timeline |

## 3. Explicitly not seeded

The following were **not** added, because they are not currently
evidenced as Day 1 debts (they would be invented):

- named Day 1 boss or first-enemy species
- a Day 2 start map
- a required field item / equipment / PE menu
- a required save point location
- a complete persist name map
- any visual reopen of CAM-B / VIS-B / VIS-C

## 4. How to add a row later

1. Cite the evidence commit and document.
2. Pick a class from the promotion policy. If identity is unproven,
   the class is `RESEARCH_REQUIRED`, not `PROVEN_OUTCOME`.
3. Fill every required schema field.
4. Do not promote a persist name in the same change unless §9 is
   satisfied.
5. Update `PE_DAY1_ACCEPTANCE_CONTRACT.md` current-status lines only
   when the class change is accepted.
