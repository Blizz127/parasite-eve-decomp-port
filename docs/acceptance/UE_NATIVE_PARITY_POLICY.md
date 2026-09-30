# PE-SYS0 — UE5 / native parity policy

```text
policy_id=PE-SYS0-UE-NATIVE
documentation_only=yes
acceptance_basis=retail_derived_state_equality
visual_similarity_is_not_acceptance=yes
python_production_fallback_allowed=no
```

UE5 and the native clean runtime are accepted on **retail-derived
state equality**, not on whether a frame "looks like" Parasite Eve.

The PT1 visual stack is already frozen. This policy does not reopen
it. It defines how production runtimes prove they implement the same
state the accepted oracle already hashed.

## 1. Authority

Same hierarchy as `PE_DAY1_ACCEPTANCE_CONTRACT.md`.

```text
Retail disc / EXE / bytecode     content + behavior authority
Blizz127/Parasite-Eve-Decompilation
                                 retail / decomp / native gameplay authority
Accepted oracle trace            comparison artifact
Python research runtime          oracle generator (not production)
Native runtime (this repo)       promoted gameplay semantics
Blizz127/parasite-eve-ue5        presentation consumer of promoted native behavior
```

Old PC ports are not a parity target. Do not duplicate gameplay
research into the UE5 tree.

The matching-decomp SHA-1 on this checkout proves C leaves equal
retail words. That is not scene parity and not Day 1 acceptance.

## 2. Python demotion (no split authority)

When a scene or system is implemented in UE5 or native and passes
this contract, Python becomes oracle/test infrastructure for that
surface.

Production **must not** fall back to:

- Python rendering
- Python field simulation
- Python collision
- Python script execution
- Python story behavior

Python **may** still:

- generate traces
- decode evidence
- generate fixtures
- perform independent comparisons

Two live simulators for one surface are a `BLOCKER`. There is no
"UE draws, Python decides the hop" split.

## 3. Trace contract

Where an accepted oracle trace exists, the UE/native trace must match
**row-for-row** unless a documented representation-only difference is
proven harmless.

### 3.1 Required columns (minimum union)

Field / traversal traces (RD3-B and later playable rungs):

```text
frame
scene
transition_state
x
y
z
yaw
triangle
clip_id
clip_frame
control_inhibit
```

Cutscene / dialogue / hop traces (RD4-E, RD5-C, RD6-A supersets):

```text
frame
scene
phase
x
y
z
yaw
triangle
clip_id
clip_frame
persist_0x4A
mailbox
message_id
camera_state
control_inhibit
```

A production runtime that claims a later rung must emit **at least**
that rung's accepted column set. Extra host-only columns are allowed
if they are ignored by the comparator.

### 3.2 Encoding

| Field | Production encoding |
|---|---|
| `frame` | integer, same epoch as the oracle (route-local, not wall time) |
| `scene` | packed retail name (`m0002i`, …), not a descriptive alias |
| `x,y,z` | retail signed world units used by the oracle (16.16 halves where the oracle used integer halves of `actor+0x28/+0x2C/+0x30`) |
| `yaw` | `0x1000`/turn, masked `0xFFF` |
| `triangle` | retail mesh index, or empty only when the oracle row is empty |
| `clip_id` / `clip_frame` | retail clip id and clock |
| `persist_0x4A` | raw word, hex or decimal matching the oracle file |
| `mailbox` | payload byte or accepted token (`0xFF`, `1`, `2`, `3`, empty) |
| `message_id` | raw id or empty |
| `control_inhibit` | retail inhibit bit, not a host bool with inverted polarity |
| `camera_state` | the oracle's existing encoding for that rung; do not invent a new schema in the same file |

If UE stores floats internally, the traced value is the retail
integer (or the exact conversion documented in a representation
diff). Silent float rounding that changes a cell is a mismatch.

### 3.3 Hash policy

```text
hash = SHA-256 of the CSV bytes that the oracle hashed
```

Rules:

1. Same header, same row count, same cell text as the accepted file,
   unless a representation diff file lists an exact substitution.
2. 3/3 identical production runs.
3. An extended route must preserve an earlier accepted trace as a
   **row-for-row prefix** (RD6-A already does this to RD5-C2
   `87b3425e7194cc0e3812e6692eee2ddccd33639ac8ea315877608c39943c8bc3`).
4. Do not hash PRESENTATION_ONLY PNGs as if they were state traces.
5. Audio PCM oracles stay on their own SHA-256 (seq26
   `cca775b1…`, seq27 `43b3074c…`, audio event trace
   `6bc5ff15…`) and are compared separately from field CSV.
6. Changing a column's meaning without a new oracle commit is a
   failed parity run, not a "format bump."

### 3.4 Representation-only differences

A difference may be ignored only if a checked-in note proves all of:

- it cannot change persist, mailbox, pose, clip, inhibit, dest token,
  HP, or message state
- it is host ABI / endian display / path punctuation / debug HUD
- both sides still hash equal after the documented normalization

Examples that are **not** representation-only:

- `y` as plane height vs `height<<16` on the wrong mesh format
- mailbox omitted because the native task table is empty
- message auto-closed so the row count shrinks
- float `H/SZ` that moves a later volume hit

## 4. Surface parity checklist

A surface reaches Native/UE Parity only when:

| Check | Required |
|---|---|
| Accepted oracle exists | yes |
| Production emits the same columns | yes |
| Row-for-row match or listed harmless diffs | yes |
| Disc/package SHA gates still fail-closed | yes |
| Python is not on the production call path | yes |
| Visual freeze inherited (no Aya scale, native 320×224) | yes |
| Persist words are raw, not convenience booleans | yes |
| Mailbox/task is `PROVEN_EXACT` if battle/save share it | yes |
| FMV/text/battle rows exist when that gate claims PASS | yes |

## 5. Current production status

| Runtime | Playable-slice status |
|---|---|
| Python research (`tools/retail_scene`, `tools/retail_audio` in RD/PT worktrees) | current executable oracle; **not** production |
| Native matching/bootstrap on this checkout (`pc_port/`) | boot/GPU/CD frontier; **not** a Day 1 field runtime |
| UE5 | no playable-slice implementation in-tree |

Therefore `ue_native_parity_gate` is DEFINED as policy and
`NO_UE_PLAYABLE_RUNTIME` as status. There is nothing to accept yet.

## 6. Required status block for a future parity claim

```text
surface=
oracle_commit=
oracle_trace_sha256=
production_trace_sha256=
row_equal=yes
representation_diffs=
python_on_production_path=no
visual_freeze_inherited=yes
persist_raw_words=yes
mailbox_class=
human_smoke=recorded_or_not_required
parity_status=ACCEPTED|REJECTED
```
