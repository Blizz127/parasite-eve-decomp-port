# Design investigation — switch-table ownership & dispatch addressing

Status: **design for review — no implementation performed.** Read-only
evidence gathered on `tooling/maspsx-expand-div` (2026-08-22). Target
class: every leaf whose function contains a compiler-lowered `switch`
(jump table). Known members: `func_80012850` (parked, evidence report has
the full byte-level diff), `func_8001F814` (parked, same class per
CLAUDE.md), plus every still-asm unit among the **38 asm files that
reference a `jtbl_` dispatch** — this is the largest single blocker class
remaining.

## Q1 — Where does retail put the table?

**Answer: a pooled read-only section at the front of the image; ownership
is logical-per-function but physical-shared.**

Evidence:

- All disc-1 switch tables sit in the leading rodata blob (file
  `0x800..0x2A0C`, VRAM `0x80010000+`) *before all text*: 58 `jtbl_*`
  dlabels there, 4 more in the second pool (`data/818A0.rodata.s`,
  VRAM `0x800A3xxx+`). Table sizes match their owner's case count
  (`jtbl_80010000` = 24 words ↔ 12850's 24 cases; `jtbl_80010060` = 7
  words ↔ 12C20's 7 cases).
- No case-block address arrays exist adjacent to any owner inside .text;
  the only references are the dispatch loads against the pool label.
- The pool is dense and ordered roughly by owner function order
  (10000↔12850, 10060↔12C20, 106E4↔1F814, …), consistent with Psy-Q ld
  concatenating per-object `.rdata` contributions into one output section
  placed ahead of text — a link-layout artifact, not a source property.
- Our splat-generated pool already carries byte-correct contents: the
  absolutizer shipped on this branch converts label refs to literals
  (1374 sites), and full-tree builds with it are EXACT SHA-1.

Corollary: the pool copy needs no regeneration — any solution must simply
avoid *duplicating* it.

## Q2 — What is the dispatch addressing shape?

**Retail (all inspected sites): the 3-word indexed-symbol form.**

```
lui  $at, %hi(jtbl_80010000)
addu $at, $at, $idx
lw   $reg, %lo(jtbl_80010000)($at)
jr   $reg
```

**Era cc1 plain-switch lowering (empirical, this exact toolchain): the
4-word full-materialization form.**

```
lui   $at, %hi($Llocal)
addiu $at, $at, %lo($Llocal)
addu  $at, $at, $idx
lw    $reg, 0($at)
```

Consequences:

- The 3-word form is **unreachable from source phrasing** under era cc1:
  the choice is made by cc1's switch lowering for a TU-local table,
  before assembly. (Empirical: our `-O2 -G8` build of 12850.)
- Therefore either the original Psy-Q cc1 lowered differently (consistent
  with the cc1 archaeology null result — we do not have that cc1), or
  aspsx folded the pair post-assembly (unverifiable — no aspsx binary).
  Either way, only a translation-layer rewrite can reach parity.
- **Audit correction (post-review):** the missing tail `nop` is *not* an
  independent defect and needs no trim-policy change. Mechanism pinned by
  reconstruction: cc1 emits the epilogue's `j $31` inside a `.set reorder`
  region, so GAS auto-fills the delay slot (pre-trim `.text` = `0x3E0`,
  containing `nop` at `0x3D0` plus align pad); trimming to the retail
  target `0x3D0` then cuts exactly that gas-appended word. It sits one
  word past the target solely because the dispatch +1 shifted every later
  instruction. Once the Axis-A fold removes the +1, the nop lands back at
  its retail position *inside* the kept window and existing trim behavior
  preserves it. Audit of matched leaves found zero historical exposure:
  trim can only ever strip gcc-private excess beyond the retail-derived
  size (zero-tail verified), so any bite into retail content would have
  tripped the SHA gate; all committed leaves are EXACT post-trim. No park
  records were misattributed (12574 was under-emission; 12850's over-
  emission is fully accounted for by div guards + this displacement).

## Q3 — Should maspsx rewrite it, or should C/build produce it?

Key structural finding: **no single layer closes both halves.**

- Placement/dedup alone (build/linker side) fixes duplication but not the
  +1-word dispatch: the addressing mode is chosen at compile time.
- An addressing fold alone (maspsx) still leaves the duplicate table: cc1
  will always emit its local `$L` table into the object's `.rodata`.

So the design must pair one decision per axis:

### Axis A — addressing (maspsx, argued as expansion-parity)

New per-leaf env gate (pattern-matches the three existing patches):

```
MASPSX_DISPATCH_FOLD=<jtbl symbol name>
```

maspsx rewrites the exact 4-instruction sequence above into

```
lui  $at, %hi(<jtbl>)
addu $at, $at, $idx
lw   $reg, %lo(<jtbl>)($at)
```

when the referenced label is a local `.rodata` data label. Notes:

- This *is* in the established patch family after all: it produces the
  ASPSX-shaped output from the same cc1 input (expansion parity), is
  algebraically exact (%hi/%lo decomposition with variable index is
  identity), matches a narrow 4-line pattern, stays flag-off by default,
  and is unit-testable like `test_div.py`.
- No symbol-definition burden is created: pool `dlabel`s already emit
  `.globl` (verified: `include/macro.inc` `dlabel` macro,
  `visibility=global`), so the rewritten relocs resolve externally against
  `800.rodata.s.o`.
- The gate takes the pool symbol name because the fold must *retarget*
  away from the local label for Axis B to deduplicate.

### Axis B — placement/dedup (build side)

Extend the existing trim machinery for gated leaves:

Strip the C object's local table block from `.rodata` (the pool copy
already carries identical literal bytes). Scope: exactly the
`$L…:`-labeled word array the folded dispatch used to reference.

(The pad-strip policy fix originally listed here is withdrawn — the
audit above showed no trim bug exists; see the Q2 correction.)

## Rejected alternatives

| Alternative | Why rejected |
|---|---|
| Extern-table `goto *(tbl[op])` dispatch in C | Labels-as-values still emits an object-local table; addressing shape differs again; correctness would depend on case blocks landing at retail addresses (circular). |
| ld `/DISCARD/` special-case per object | Brittle per-carve script surgery in the linker script; doesn't touch addressing. |
| Accept 4-word form; keep leaf as asm | Defeats the carve program for the largest remaining class (~38 units). |
| Fold without retargeting (keep local table) | Leaves duplicate table bytes in the image; pool copy and object copy would both ship. |

## Validation ladder (proposed, once approved)

1. Vendored tests: fold-pattern cases incl. negative (non-matching
   sequences untouched), flag-off passthrough, retarget naming.
2. Flag-off full rebuild → EXACT SHA-1 (unchanged baseline).
3. Gate ON for `func_80012850` only → size gate `0x3D0` → verify →
   expected EXACT SHA-1. Per the audit correction, this single gate
   validates Axis A and Axis B together: folding the dispatch removes
   the +4 shift, which restores the gas-filled nop at its retail
   position automatically; the dedup strip then keeps the object clean.
4. Repeat for `func_8001F814`; then batch through remaining jtbl owners.

## Open items for review

- Confirm the fold should key on env-provided pool symbol name vs. an
  automatic local→pool mapping derived from the yaml (more magic, less
  wiring).
- Whether Axis B table-stripping should live in `$TRIM` or a dedicated
  filter next to the absolutize step.

## Audit addendum — pad-strip exposure (2026-08-22)

Ten-minute audit requested during review; result: **no trim-policy bug
exists and none is needed**.

- Semantics (tools/trim_elf_section_pad.py): refuse if target > current;
  if current > target, require all bytes beyond target to be zero, then
  truncate sh_size to target and lower alignment. Silent by design but
  bounded: it can never remove nonzero bytes.
- Reconstruction of func_80012850's untrimmed object under
  `--expand-div`: `.text` = `0x3E0` — cc1 emits the terminal `j $31`
  inside a `.set reorder` region, GAS appends the delay-slot `nop`
  (`0x3D0`) plus align pad (`0x3D4..0x3E0`). Trimming to retail size
  `0x3D0` therefore consumes exactly the gas-appended nop — which is
  displaced one word past its retail position purely by the dispatch +1.
- Matched-leaf exposure: structurally impossible without a SHA failure
  (any stripped retail byte would mismatch). All committed leaves are
  EXACT post-trim.
- Park-record misattribution check: none found. 12574 = under-emission
  (0x60 vs 0x6C); 12850 over-emission fully explained (div guards −16
  words; nop displacement a consequence of dispatch shape).

Consequence for implementation order: the separately-gated "trim fix"
step is withdrawn. Axis A + Axis B together close func_80012850 in one
E2E gate; no trim changes required.
