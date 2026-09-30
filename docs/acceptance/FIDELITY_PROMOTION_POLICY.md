# PE-SYS0 — Fidelity promotion policy

```text
policy_id=PE-SYS0-PROMOTION
applies_to=python_research_runtime, pt_builds, native_runtime, ue5_runtime
documentation_only=yes
```

This policy stops outcome-faithful shims, temporary presentation
approximations, and partially-proven systems from becoming production
authority.

Authority remains the hierarchy in `PE_DAY1_ACCEPTANCE_CONTRACT.md`.
A unit-test pass is not proof if the tested semantics are approximate.

## 1. Debt classes

| Class | Meaning | Research runtime | PT build | UE / native production | Day1 acceptance |
|---|---|---|---|---|---|
| `PROVEN_EXACT` | Retail bytes, handlers, and observable state match the claimed contract. Independent oracle agrees. | yes | yes | yes | yes |
| `PROVEN_OUTCOME` | Retail-visible end state is correct (token, persist word, clip id, control bit) but intermediate machinery is not shown to be retail-shaped. | yes | yes, labeled | no, if the approximate surface is shared with another production system; otherwise no for Day1 | no, unless the class is later promoted to `PROVEN_EXACT` |
| `NONBLOCKING_FIDELITY` | Proven behavior with a documented waveform / presentation / unused-path gap that does not change route-authoritative state. | yes | yes, labeled | no for the indebted surface if Day1 claims that surface exact; yes as residual only if the gate explicitly allows it | allowed only on surfaces whose Day1 gate lists that debt as acceptable |
| `PRESENTATION_ONLY` | Host-window, debug overlay, or inspection framing that is not claimed as retail pixels. | yes | yes, if not substituted for retail presentation | no as production presentation | no as production presentation |
| `RESEARCH_REQUIRED` | Identity, handler, or layout is not provenance-backed. | yes as an explicit unknown | no as a shipped behavior | no | no |
| `BLOCKER` | Missing or approximate behavior that the Day1 route cannot complete without. | yes only as a named stop | no | no | no |

Rules:

- Unknown stays `RESEARCH_REQUIRED`. It is never quietly stored as
  `PROVEN_OUTCOME` because a guessed implementation "worked."
- `PROVEN_OUTCOME` is a research/PT holding class, not a production
  destination.
- `NONBLOCKING_FIDELITY` must name the exact gap (example: dry type-5
  reverb). An unnamed "audio is close" is a `BLOCKER`.
- `PRESENTATION_ONLY` artifacts must not be hashed as retail frames.

## 2. Promotion stages

```text
Research
  -> Oracle Accepted
  -> PT Candidate
  -> Native/UE Parity
  -> Day1 Candidate
  -> Day1 Accepted
```

A surface may skip PT Candidate if it is never packaged as a playtest,
but it may not skip Oracle Accepted or Native/UE Parity on the way to
Day1 Accepted.

### 2.1 Research

Allowed: decode, probes, bounded Python simulation, explicit unknowns,
`PROVEN_OUTCOME` machinery.

Required to leave: a written evidence report, hashes, and a named
unknowns list.

### 2.2 Oracle Accepted

Required evidence:

- retail source (EXE range and/or package range + SHA-256)
- handler or bytecode citation
- independent oracle or 3/3 deterministic trace
- column set declared
- no invented story/audio/geometry

A test pass on a host convenience model does **not** accept the oracle.

### 2.3 PT Candidate

Allowed in a research PT:

- `PROVEN_EXACT` surfaces
- `NONBLOCKING_FIDELITY` listed in the PT notes
- `PROVEN_OUTCOME` mailbox/task **only while** battle and save/load
  do not share that state (see §4)

Not allowed: unmarked approximations, Aya scale hacks, invented BGM,
ID-only text presented as "dialogue done," prerendered replacement
FMV claimed as retail.

PT packaging does not promote a surface.

### 2.4 Native / UE Parity

Required evidence:

- the same oracle trace columns as the accepted Python/retail oracle
- row-for-row equality, or a documented representation-only difference
  proven harmless (`UE_NATIVE_PARITY_POLICY.md`)
- no Python fallback on that surface

### 2.5 Day1 Candidate

All gates A–L have an owner implementation on the production runtime.
Every `RESEARCH_REQUIRED` node on the Day 1 route is either proven or
proven absent. Residual debt is only `NONBLOCKING_FIDELITY` that the
gate matrix explicitly allows.

### 2.6 Day1 Accepted

Human verification of the full §3.2 route **plus** all oracle hashes
green on the production runtime. Python is oracle-only for every
accepted surface. `day1_acceptance_ready_now` may flip to `yes` only
in a later contract revision that cites those hashes.

## 3. Evidence that is not enough

| Artifact | Why it is not enough |
|---|---|
| Green unit tests of a shim | tests the shim, not retail |
| Visual similarity | not state equality |
| "It plays through" | can hide forced tokens / auto-close / skipped FMV |
| Old PC port behavior | not authority |
| A persist value that "unlocks the door" | not a semantic promotion |
| One emulator screenshot | observation, not a runtime contract |
| Matching-decomp leaf SHA-1 | proves C==ASM for that leaf; does not accept a scene |

## 4. Mailbox / task promotion (special ruling)

Current research implementations of m0004i / m0378i mailbox delivery
are `PROVEN_OUTCOME` / `NONBLOCKING_FIDELITY`: they reproduce the
retail-visible clip / restore / hop results without claiming exact
`D_800A3180` records and task-block fields.

That is acceptable **temporarily** in Research and in a research PT.

It is **not** promotable to Native/UE Parity, Day1 Candidate, or
Day1 Accepted once any of the following is true:

- battle init or battle return reads or writes the same task/mailbox
  tables
- save/load serializes task, actor, or mailbox state
- a later first-play script depends on task re-arm, `task+0x08`
  flags, or sender serial rather than on the payload byte alone

At that boundary the mailbox path must be reclassified:

```text
PROVEN_OUTCOME  ->  BLOCKER
```

until task records, `func_800653B8` / `func_80065400` / `func_80012700`
fields, and re-arm PCs are `PROVEN_EXACT`.

RD5-X already proved the 12-byte mailbox record and the payload-1
no-op / payload-2 clip-`0x0A` outcomes. The remaining gap is
implementation fidelity of the task machine, not payload mapping.

## 5. Persist promotion

Follow `PE_DAY1_ACCEPTANCE_CONTRACT.md` §9.

Promotion of an index from research use to production authority
requires the full field set (index, width, readers, writers, values,
reset, save persistence, confidence, provenance).

A convenience boolean such as `saw_m0372i_cutscene=true` must never
replace `persist[0x4A]`. If a higher-level name is later proven, it
is an alias for the raw word, not a second store.

## 6. Python demotion

When Native/UE Parity is accepted for a surface:

```text
Python  :=  oracle / decoder / fixture / comparator
Production  :=  UE5 or native only
```

Production must not call Python to render, simulate the field, run
collision, execute scripts, or decide story. Two live simulators for
the same surface are a `BLOCKER`.

Python may remain the **generator** of the oracle trace that native
code is measured against.

## 7. Visual freeze vs this policy

PT1 visual commits (`3e4c65d`, CAM-B `1bf3832`, VIS-B `ac62411`,
VIS-C `cad4598`) are already `PROVEN_EXACT` for the frozen field
presentation stack. SYS0 must not reopen them. New rooms inherit the
same camera/OT/NCLIP/background rules; they do not get a second
"make Aya look better" campaign.

## 8. Stage checklist (copy per surface)

```text
surface=
debt_class=
stage=
retail_authority=
oracle_trace_sha256=
oracle_runs=
representation_diffs=
python_still_production=no
battle_or_save_shares_this_state=
human_verification=
blocker_if_any=
```
