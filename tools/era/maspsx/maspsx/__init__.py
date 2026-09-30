# LOCALLY MAINTAINED — tracked in the Parasite-Eve-Decompilation repo.
# Vendored from upstream: https://github.com/mkst/maspsx
# This file is un-ignored via .gitignore negations under tools/era/;
# scripts/setup_era.sh re-clones upstream AROUND it (never overwrites it,
# restores it from git if absent). Local patches are marked "LOCAL PATCH".
# Local patch log:
#   1. fill_store_delay_slot (ctor arg, or env MASPSX_FILL_STORE_DELAY_SLOT=1 —
#      the env var keeps the untracked maspsx.py driver unpatched): schedule an
#      absolute `sw $r,SYM` store macro into the delay slot of a following bare
#      `j $31`. cc1 cannot schedule the macro (its `lui $at` + `sw` expansion
#      is opaque to cc1's delay-slot filler), so it emits `sw` / `j $31` with an
#      empty slot; ASPSX expands the macro and moves the trailing store into
#      the slot:  lui $at,%hi(SYM) / j $31 / sw $r,%lo(SYM)($at).
#      ROM evidence (Parasite Eve disc1): 14-member sw delay-slot family
#      (func_8007FBC0 et al) fills; sb/sh macro stores and multi-store
#      epilogues stay pre-jr with a nop slot. Default OFF: opt-in per leaf.
#   2. three_word_symbol_store (ctor arg, or env
#      MASPSX_THREE_WORD_SYMBOL_STORE=1): for standalone indexed symbolic
#      loads AND stores (lw/lh/lb/... and sw/sh/sb/...) under ASPSX
#      2.21/addiu_at, select the ASPSX 2.30 three-word lui/addu/op-%lo form
#      instead of lui/addiu/addu/op-0. Compound semicolon lines retain the
#      legacy expansion. Default OFF: opt in per leaf.
#   3. fill_epilogue_delay_slot (ctor arg, or env
#      MASPSX_FILL_EPILOGUE_DELAY_SLOT=1): schedule the frame deallocation
#      (`addu $sp,$sp,N` / `addiu $sp,$sp,N`) into the delay slot of a
#      following bare `j $31`. cc1 only fills the return slot itself when no
#      callee-saved restore immediately precedes the jump; with restores it
#      emits the stack adjust before the `j` and GNU as leaves a nop. ASPSX's
#      reorder-mode scheduler moves it into the slot. ROM evidence (Parasite
#      Eve disc1): func_800811E4 (ra/s0/s1 restores) fills at 0x8008124c/50;
#      the 58 G0-matched leaves that already fill do so in cc1 itself (no
#      restores), so this stays opt-in and flag-off is byte-identical.
#      Default OFF: opt in per leaf.
#   4. symbol_load_dest_temp (ctor arg, or env
#      MASPSX_SYMBOL_LOAD_DEST_TEMP=1): for compound indexed symbolic
#      loads of the form `op $d,SYM($b)` under ASPSX 2.21/addiu_at, emit the
#      naive GNU-as expansion using the DESTINATION register as the address
#      temp (lui $d,%hi(SYM) / addu $d,$d,$b / op $d,%lo(SYM)($d)) instead of
#      maspsx's lui $at / addiu $at,%lo / addu $at,$at,$b / op $d,0($at).
#      The same gate also covers an indexed symbolic STORE that immediately
#      precedes a `j $31`: ROM emits the 3-word $at materialization before the
#      jump and schedules the store into the delay slot
#      (lui $at,%hi(SYM) / addu $at,$at,$b / j $31 / op $src,%lo(SYM)($at)).
#      ROM evidence (Parasite Eve disc1): func_80076B44
#      (`lui $v0,%hi(D_800A3348)` / `addu $v0,$v0,$a0` / `lbu $v0,%lo(...)($v0)`)
#      and func_80076B20 (same shape with an `sb` in the jr delay slot).
#      When the destination IS the index register (`lhu $2,SYM($2)`) the
#      dest-temp form is impossible (the `lui` would clobber the index), so
#      the load falls back to the $at form like GNU as: func_80074A44 has
#      both `lhu $v1,D_800957CC($v0)` (dest-temp) and
#      `lhu $v0,D_800957D8($v0)` ($at) in one body.
#      Default OFF: opt in per leaf; flag-off is byte-identical.
#   5. symbol_at_temp (ctor arg, or env MASPSX_SYMBOL_AT_TEMP=1): for compound
#      indexed symbolic loads/stores of the form `op $d,SYM($b)` under ASPSX
#      2.21/addiu_at, emit the 3-word $at form WITH the %lo displacement kept
#      (lui $at,%hi(SYM) / addu $at,$at,$b / op $d,%lo(SYM)($at)) instead of
#      maspsx's legacy 4-word lui/addiu/addu/op-0($at). Unlike patch 4 this
#      uses the assembler temporary rather than the destination register, so
#      it also covers sign-extended loads and narrower stores. Env may also
#      be a comma list of symbol names (e.g. MASPSX_SYMBOL_AT_TEMP=jtbl_80011388)
#      to apply the 3-word $at form to those operands only; other indexed
#      symbolic ops keep the legacy 4-word expansion. ROM evidence
#      (Parasite Eve disc1): func_80042770/func_80042964
#      (lui $at,%hi(D_800A0ED4) / addu $at,$at,$v0 / lbu $v0,%lo(...)($at));
#      4658 non-$at indexed accesses exist in the split asm, most of which are
#      this shape with a wider register pair than patch 4 can reach.
#      Default OFF: opt in per leaf; flag-off is byte-identical.
#   6. fill_branch_delay_slot (ctor arg, or env
#      MASPSX_FILL_BRANCH_DELAY_SLOT=1): fill the delay slot of a REORDER-mode
#      conditional branch with the instruction that follows it, instead of the
#      unconditional `nop`.
#      WHY cc1 leaves the slot empty: gcc's reorg.c refuses any delay-slot
#      candidate that SETS a resource the branch itself NEEDS
#      (`insn_sets_resource_p (trial, &needed, 1)` in fill_slots_from_thread),
#      so a compare that overwrites the register the branch just tested can
#      never be scheduled by cc1 even though it is architecturally legal (the
#      branch latches its operands before the slot executes). Retail's build
#      does perform that fill.
#      WHY THIS IS NARROW: this fill moves an instruction from the
#      fall-through-only position into a slot that executes on BOTH edges, so
#      it is only legal when the instruction's destination register is DEAD at
#      the branch target. ROM evidence: of the 357 reorder-mode conditional
#      branches in the repo's LINK_EXACT leaves, 70 are followed by an
#      instruction that redefines a tested register and retail keeps a `nop`
#      in every one of them (e.g. func_80077DC4 `bgez $4,$L2` / `subu $4,$0,$4`
#      -- an abs() whose $4 is LIVE at the target; filling it would
#      miscompile). The shape alone therefore does NOT license the fill; only
#      liveness does. This patch performs a real, conservative reachability /
#      liveness walk of the assembly from the branch target and refuses the
#      fill on ANY construct it cannot fully account for (calls, indirect
#      jumps, inline asm, reorder-mode branches inside the walked region,
#      unknown mnemonics, function end, a step budget). The candidate itself
#      must be a single-word, non-memory, non-trapping, non-hi/lo ALU
#      instruction from a closed allow-list with an immediate that cannot
#      provoke an `$at` expansion, and must be the immediately following line
#      (no label, directive or comment in between).
#      TWO GATES, both mandatory: a SAFETY gate (the liveness walk above) and
#      a FIDELITY gate (the candidate must be a compare -- slt/slti/sltu/sltiu
#      -- whose result the immediately following conditional branch consumes).
#      The fidelity gate exists because liveness alone licenses 2 fills retail
#      does not take (func_80012E7C `sll $2,$3,2`, func_8006A2E8 `move $2,$5`
#      -- both legal, both a `nop` in ROM).  With both gates on, the knob
#      changes exactly ONE of the repo's 815 matched leaves: func_80043474.
#      The non-compare rows of branch_fill_candidates are therefore inert
#      today; they are the structural single-word analysis that a future
#      widening of the fidelity gate would need, and must not be treated as
#      ROM-evidenced on their own.
#      ROM evidence (Parasite Eve disc1): func_80043474 (band classifier) --
#      `bnez $v0,.L800434B4` / `slti $v0,$a0,0x2D3` at 0x8004349C, where $v0 is
#      killed by the epilogue `addu $v0,$v1,$zero` on the taken path.
#      Default OFF: opt in per leaf; flag-off is byte-identical.
#   7. sized-`.extern` gp classification for the load-delay nop (ALWAYS ON --
#      a correctness fix, not a knob). `preprocess_lines` used to skip
#      `.extern SYM, size` lines, so `_uses_gp` could never recognise a
#      cc1 `-G<n>` external as gp-relative. For `load $r` / `op $r,SYM` where
#      GNU as collapses the macro to ONE gp-relative instruction, the
#      mandatory MIPS-I load-delay nop was then emitted only by the
#      version-gated `nop_at_expansion` clause (ASPSX < 2.30), i.e. for the
#      wrong reason, and silently dropped at >= 2.30. Sized externs that fit
#      `-G<n>` are now recorded in `extern_entries` and consulted by
#      `_uses_gp` only; operand rewriting is unchanged (GNU as already places
#      sized externs gp-relative). Requires the driver to forward cc1's
#      `-G<n>` to maspsx (it did not before; `sdata_limit` was always 0).
#      ROM evidence (Parasite Eve disc1): func_800339A0 0x800339FC
#      `lhu $v0,0x0($v1)` / `nop` / `sh $v0,0x114($gp)`, func_80046DFC
#      0x80046E7C `lw $v0,0x19C($gp)` / `nop` / `sw $v0,0x248($gp)`,
#      func_80012700 0x80012708 `lw $v0,0x24($a2)` / `nop` /
#      `sw $v0,0x8C($gp)` -- the only three era leaves that matched at
#      2.21 but not 2.30 before this fix; all three match at both after it.
#   8. fill_call_delay_slot (ctor arg, or env MASPSX_FILL_CALL_DELAY_SLOT=1):
#      the CALL/JUMP sibling of patch 1. Schedule the second word of an
#      absolute store macro (`sw/sh/sb $r,SYM`) or of an address macro
#      (`la $r,SYM`) into the delay slot of a following plain
#      `jal <symbol>` / `j <label>`:
#        lui $at,%hi(SYM) / jal T / sw $r,%lo(SYM)($at)
#        lui $r,%hi(SYM)  / jal T / addiu $r,$r,%lo(SYM)
#      Why this cannot be done by cc1 (and so has to live here): the `$at`
#      form is only ever written by the assembler, so retail's store was a
#      macro and the fill was ASPSX's reorder pass. Verified: cc1 2.8.1
#      DOES fill jal slots, but only in its default -msplit-addresses mode,
#      where it materializes %hi into a real GPR (`lui $2,%hi(SYM)` /
#      `sw $r,%lo(SYM)($2)`) — never `$at`; under -mno-split-addresses it
#      emits the macro and leaves the jal slot empty, exactly like 2.7.2.
#      No --aspsx-version value changes this either: maspsx forces
#      `.set noreorder` per function and inserts its own branch/jump nop,
#      and config_for_aspsx_version has no scheduling knobs at all.
#      ROM evidence (Parasite Eve disc1, whole-image scan): 90 sites /
#      45 functions — 36 absolute-store splits (16 jal+sw, 19 j+sw,
#      1 jal+sh), 53 `la` splits (49 jal, 4 j) and 1 indexed-store split
#      (not covered by this patch).
#      HAZARDS (each one is a guard below; when a hazard cannot be decided
#      from maspsx's line-oriented view the knob refuses the site):
#        H1 the only instruction that MOVES is the macro's second word, from
#           just before the jump to just after it. `jal` writes $31, so a
#           moved word that READS $31 would read the new return address:
#           refuse when the stored value register is $31/$ra.
#        H2 the `la` arm's moved word WRITES its register. Refuse when that
#           register is $31/$ra (it would destroy the return address the
#           `jal` just stored).
#        H3 register-indirect jumps (`jalr`/`jr`, which cc1 spells `jal $2` /
#           `j $2`) read their target register AT jump time, i.e. before the
#           delay slot executes. They are IN scope — ROM: func_80074BB8's
#           `lui $a0,%hi(D_80011814)` / `jalr $v0` / `addiu $a0,$a0,%lo(...)`
#           — but only when the moved word cannot disturb that register:
#           the `la` arm refuses target == its own destination (the jump
#           would read a half-formed address), and the store arm refuses
#           target == `$at` (clobbered by the surviving `lui`). Register
#           names are compared canonically so `$4`/`$a0` cannot fail open.
#           `j $31` (the return) is always refused: that slot belongs to
#           patches 1 and 3 and their ROM evidence, and stealing it would
#           change their behaviour when several knobs are on.
#        H4 `$at` is written by the `lui` that stays BEFORE the jump and read
#           by the store in the slot. Nothing can clobber it in between: the
#           only instruction in between is the jump, and j/jal write $31
#           only. `$at` is never live across the call because the store
#           retires before control transfers. Still refused if the stored
#           value register is itself `$at`.
#        H5 a label or a `.set` directive between the macro and the jump
#           means the jump is a branch target, or that cc1 already scheduled
#           the slot inside its own `.set noreorder` block. Refused (the
#           lookahead deliberately does not skip labels or `.set`).
#        H6 conditional branches are NOT touched: only `j`/`jal`. A branch's
#           slot is a separate ROM family with its own scheduling rules.
#        H7 compound (`;`) macro lines on either side are refused: their
#           expansion order is not a single instruction pair.
#        H8 gp-relative destinations are a single instruction with no second
#           word to move, so the gp arm is tested first; the `la` arm also
#           refuses outright when gp_allow_la is in effect (ASPSX >= 2.80).
#        H9 numeric operands (`sw $2,56200($4)`, `la $2,-4`) are not
#           symbol macros; refused.
#      Default OFF: opt in per leaf; flag-off is byte-identical.
#   9. explicit `%lo` store passthrough (ALWAYS ON). gcc-2.8.1-psx cc1
#      (per-leaf ERA_CC1_VER=2.8.1) emits absolute stores as an explicit
#      `lui $b,%hi(SYM)` / `op $r,%lo(SYM)($b)` pair; 2.7.2 never does. The
#      `%lo` store is already one instruction with a LO16 reloc, so the store
#      arm now passes it through exactly like the load arm always did instead
#      of re-expanding it into the 4-word $at macro. 2.7.2 output is
#      byte-identical (it never produces the form). Durable test
#      tests/test_lo_store_passthrough.py.
#  22. reorder_fill_calls (ctor arg, or env MASPSX_REORDER_FILL_CALLS=1): for
#      leaves compiled with cc1 -fno-delayed-branch (every slot left empty,
#      which retail shows for branch/j slots), model the two LOCAL fills
#      ASPSX's reorder pass still makes, as a pre-pass that rewrites them
#      into cc1-style `.set noreorder` blocks:
#        (A) X / jal T  ->  jal T / X            (call slot)
#        (B) X / lw $31,N($sp) / addu $sp,$sp,M / j $31
#            ->  lw $31,N($sp) / X / j $31 / addu $sp,$sp,M
#            (X fills the $31 load delay, the pop fills the return slot).
#      ROM evidence: func_80084B78 -- 0x80084BF4 `jal func_80083E50` /
#      `li $a1,1`, the second call with `move $a1,$zero` in its slot, while
#      every branch/j slot and the jalr/jal slots preceded by a branch or a
#      label stay nop; epilogue 0x80084C3C `lw $ra` / `move $v0,$zero` /
#      `jr $ra` / `addiu $sp,$sp,24`. Closes that leaf (21 words -> 0).
#      HAZARDS: HF1 X is a single-word ALU op from a closed list (move, li
#      with a one-word immediate, addu/addiu/subu/and/andi/or/ori/xor/xori/
#      nor/slt*/shifts), never a load/store/branch/macro/symbol operand;
#      HF2 X never names $zero as destination, $at, $sp, $fp or $ra (a jal
#      writes $ra before its slot runs); HF3 a jalr target register X
#      writes is refused (the jump reads it first); X must IMMEDIATELY
#      precede the jal / epilogue with no label or directive between (a
#      label means the call is a branch target), and only in reorder mode.
#      SWEEP (2026-09-24): 4255 era leaves, flag-off byte-identical, knob-on
#      changes 0. Default OFF; per-leaf.
#      RE-SWEEP 2026-09-24 (baseline = this file without the patch-22
#      pre-pass call): 4279 era leaves, flag-off 4279/4279 identical;
#      knob-on changes only its user func_80084B78.
#  21. load_delay_nop_before_loop_label (ctor arg, or env
#      MASPSX_LOAD_DELAY_NOP_BEFORE_LOOP_LABEL=1): the loop-head sibling of
#      patch 14. When a load is immediately followed by a label that is a
#      pure LOOP HEAD, emit the required load-delay nop BEFORE the label
#      (`lw` / nop / $L: / use) instead of after it, so the back-edge skips
#      it. ROM evidence: func_8007D1D4 0x8007D37C `lw $v1,D_8009B3FC` / nop /
#      0x8007D384 loop head; back-edge 0x8007D3A4 `bnez $v0,0x8007D384`
#      (1440fff7; stock placement gives fff6). Closes that leaf.
#      PER-LEAF, BAND ONLY: forcing the knob on over the corpus (4206 era
#      leaves) changes exactly 12 matched leaves, all below 0x80071A84, with
#      the identical shape (symbolic `lw`, then a back-edge-only label) where
#      retail keeps label-then-nop: func_80018300/80018364/800183E8/80018460,
#      func_8002F0B0, func_80055724, func_8005B500, func_8005CAEC,
#      func_8005D020, func_80062CE4, func_8006F224, func_8006F39C. Like patch
#      14 (all positives in 0x80071A84-0x8008D610) the placement is a
#      band-vs-older-region difference, not a local rule.
#      HAZARDS: HL21-1 the label must be the very next non-comment line
#      after the load; HL21-2 every branch/jump to it must come AFTER it
#      (back-edges only; the fall-through from the load is the one forward
#      entry, and it still executes the nop); HL21-3 no cc1 noreorder
#      back-edge may carry a load (or compound macro) in its slot. Default
#      OFF; flag-off byte-identical (4206/4206).
#      RE-SWEEP 2026-09-24 (after patches 22 + the fold-composition strip,
#      baseline = this file without the patch-21 clause): 4269 era leaves,
#      flag-off 4269/4269 identical; knob-on changes 17 = func_8007D1D4 (the
#      user) + 16 matched leaves all below 0x80071A84 (the 12 above plus the
#      newer func_800275CC, func_80027D14, func_80028E94, func_8005C688).
#  20. rodata_fold (ctor arg rodata_fold_symbols, or env
#      MASPSX_RODATA_FOLD=s0,s1,...): the i-th cc1 `$LC<n>` literal defined
#      in .rdata is retail's shared pool object s<i>; every instruction
#      operand naming `$LC<n>` (`la $r,$LCn`, `op $r,$LCn[+off]`) is
#      retargeted to it, like MASPSX_DISPATCH_FOLD for switch tables. The
#      build (disc1_build.strip_rodata_fold) then strips the object's
#      duplicate .rodata only after proving it byte-identical to the pool
#      blocks and that no relocation still points at it.
#      ROM evidence: func_800323C8 copies three initialized 18-byte local
#      arrays from the pool (0x800323D0 `lui $a2,0x8001` / `addiu
#      $a2,0xDFC` = D_80010DFC; D_80010E10; D_80010E24) with lwl/lwr.
#      HAZARDS: HR1 the list must name exactly as many symbols as cc1
#      emitted $LC labels (else ValueError -- a wrong count would silently
#      mis-map); HR2 whole-token `$LC<n>` instruction operands only, never
#      the .rdata definitions; order/bytes are re-proven by the build strip.
#      Default OFF (empty list); per-leaf.
#  19. fill_branch_split_li (ctor arg, or env MASPSX_FILL_BRANCH_SPLIT_LI=1):
#      PROVISIONAL (lead ruling 2026-09-24: reverted unless a leaf closes
#      with it by the end of the lane). cc1 splits a 32-bit constant into
#      `li $r,HI<<16` / `ori $r,$r,LO`; when that pair immediately follows
#      a reorder-mode conditional branch, retail (ASPSX) has the `lui` in
#      the branch's delay slot. The fill is legal only when $r is dead at
#      the branch target, so it reuses patch 6's liveness walk in an
#      EXTENDED mode (patch 6's own behaviour is unchanged): HS5 a
#      reorder-mode branch inside the walk pushes its target and continues
#      (a later fill can only move the next fall-through instruction into
#      its slot, which the fall-through walk already checks); HS6 a call
#      preserves a callee-saved register ($16-$23, $30), except a jalr
#      through it; HS7 mult/multu read two GPRs, mfhi/mflo write one.
#      Other gates: HS1 the li must be a pure high word (low 16 bits zero);
#      HS2 the next line must be the completing `ori $r,$r,imm`; HS3 no
#      ABI-fixed/$at destination; HS4 dead-at-target proof.
#      ROM evidence: func_80038D74 0x80038E28 `bnez $v0` / `lui $s0,0x8001`
#      (slot) / `ori $s0,$s0,0x3`; whole-EXE scan: 56 branches with a split
#      constant's lui in the slot vs 5 with a nop slot before lui/ori.
#      func_80038D74 goes 214 -> 6 words (the rest is a cc1 sched1 residual).
#      Default OFF; per-leaf.
#  DISPATCH_FOLD `la` form (extends the switch-dispatch retarget): cc1 may
#      hoist a switch table's address out of a loop as `la $r,$L<n>` and
#      index it with addu/`lw 0($r)`; such an `la` is now retargeted too,
#      only for `$L<n>` labels defined in .rdata (HD1) and plain two-operand
#      lines (HD2). ROM evidence: func_80051CC4 0x80051D2C/30
#      `lui $t6,0x8001` / `addiu $t6,0x11F8` = jtbl_800111F8 (2 words ->
#      LINK_EXACT). Inert unless MASPSX_DISPATCH_FOLD is set.
#  18. fill_call_volatile_store (ctor arg, or env
#      MASPSX_FILL_CALL_VOLATILE_STORE=1): the `jal` sibling of patch 11.
#      The single-word register-based store (sb/sh/sw imm($r)) that
#      immediately precedes a reorder-mode direct `jal <symbol>` moves into
#      the call's delay slot:  sw $3,0($2) / jal F / nop  ->  jal F / sw $3,0($2)
#      cc1 leaves that slot empty only for a `volatile` store (gcc's reorg
#      never slots a volatile insn; a plain store it fills itself), so in
#      practice this reaches volatile hardware-register writes.
#      ROM evidence: func_80076354 0x800763C4 `jal func_800773D0` /
#      `sw $v1,0($v0)` (the DMA CHCR = 0x11000002 kick); closes that leaf
#      with patch 3 on plain 2.7.2 -O2 -G0.
#      PER-LEAF ONLY: forcing it on over the corpus (1835 era leaves) changes
#      exactly 2 matched leaves, both volatile stores retail keeps before the
#      `jal` with a nop slot: func_80082E00 (`sb $2,0($3)` / jal
#      func_80083578) and ovl_0457 func_8012562C (`sw $2,0($16)` / jal
#      func_80080C48). No operand/position rule separates them from the
#      positive, so the choice is the leaf's.
#      HAZARDS: HC1 `jal` writes $31 before the slot runs -- refuse a moved
#      store that reads $31 (value or base) or $at; HC2 direct `jal <symbol>`
#      only (no jalr, no `j`: patch 11 owns `j <label>`); HC3 numeric 16-bit
#      offset (one instruction; symbolic stores are macros, patch 8's arm);
#      HC4 compound `;` lines refused; HC5 the call must be the next
#      non-comment line in reorder mode (a label or `.set` in between, or a
#      cc1 noreorder block, refuses). The store writes no GPR and a direct
#      `jal` reads none, so slot execution is equivalent to the pre-call
#      position. Default OFF; flag-off byte-identical (1835/1835).
#  17. la_absolute_small_data (ctor arg, or env
#      MASPSX_LA_ABSOLUTE_SMALL_DATA=1): a plain `la $r,SYM[+off]` whose
#      symbol GNU as would place in small data (a local .sdata/.sbss entry or
#      a sized `.extern` that fits -G<n>) is emitted as the explicit absolute
#      pair `lui $r,%hi(SYM)` / `addiu $r,$r,%lo(SYM)`. Stock maspsx already
#      models ASPSX < 2.80 as never forming a gp-relative `la`
#      (gp_allow_la=False) but then hands the line to GNU as, which DOES make
#      it `addiu $r,$gp,%gp_rel(SYM)`. Loads/stores of the same symbol are
#      untouched and stay gp-relative -- the split retail shows. Before this
#      patch the only workaround was MASPSX_FORCE_ABSOLUTE_SYMBOLS, which
#      also turns the symbol's stores absolute.
#      ROM evidence: func_80044274 -- `sw $v0,0x220($gp)` (D_8009CF90) and
#      `lui $a0,0x800A` / `addiu $a0,$a0,-0x3070` (&D_8009CF90) at
#      0x80044360, `lui $a1` / `addiu $a1,-0x306C` (&D_8009CF94); closed with
#      the plain C spelling (was an alias store through D_8009CF8C).
#      HAZARDS: HA1 only when gp_allow_la is off (ASPSX >= 2.80 forms a gp
#      la); HA2 compound `;` lines refused; HA3 numeric operands refused;
#      HA4 only symbols GNU as would place gp-relative (anything else already
#      assembles absolute) -- indexed `la SYM($b)` takes the indexed path
#      and is untouched. The pair is exactly GNU as's own absolute `la`
#      expansion, so no scheduling or register changes.
#      SWEEP (2026-09-23, tools/analysis/maspsx_knob_sweep.py): 1765 era
#      leaves, flag-off 1765/1765 byte-identical, knob-on changes 0 (every
#      matched -G8 leaf that takes a small-data address forces the symbol
#      absolute today).
#      DEFAULT ON (2026-09-23; env MASPSX_LA_ABSOLUTE_SMALL_DATA=0 opts out,
#      ctor arg True/False overrides the env): retail never forms a
#      gp-relative address. Raw scan of `addi/addiu $r,$gp,imm`: the EXE has
#      3557 gp-relative loads/stores and 4 such words -- 0x800725B4 is the
#      crt0 `$gp` setup (`lui gp,0x800A` / `addiu gp,gp,-0x3290`), 0x8009BC48
#      / 0x8009BD68 / 0x8009BD98 lie past .text (data); in all 30 extracted
#      PE.IMG overlay/room blobs every hit disassembles amid data words. So
#      this is a correctness fix of maspsx's own gp_allow_la=False model
#      (same class as patches 7 and 9), not a per-leaf choice. Flip sweep:
#      default-on vs the pre-flip file with the knob unset
#      (maspsx_knob_sweep.py --default-check): 2621/2621 era leaves
#      byte-identical; the upstream test_gp_rel `..._gp_allow_la_false`
#      expectation is amended (LOCAL PATCH, tracked) to the absolute pair.
#  16. div_no_reuse_nop (ctor arg, or env MASPSX_DIV_NO_REUSE_NOP=1): stock
#      maspsx treats the `mflo`/`mfhi` that ends a div/rem/divu/remu macro
#      expansion like a load and emits a "Reuse of $r" nop when the next
#      instruction reads the result. MIPS I has no such hazard for mflo/mfhi
#      (only the separate mult/div-after-mfxx hazard, which is kept).
#      ROM evidence (whole-image scan of div-macro tails, i.e. an mfxx within
#      3 words after a `break`): 2.8.1 band 0x80071A84-0x8008D610 -- 4 direct
#      reads, 0 with a nop (func_80077E64 0x80077F20, func_8007DB24
#      0x8007DB54, 0x8007A060, 0x8007A0E8); older region -- 14 reads, ALL
#      behind a nop (e.g. 0x80025964, 0x8001B880). Per-leaf, band only.
#      Only the --expand-div (trap-checked) expansion is affected: the
#      unexpanded `div $zero` + mfxx path keeps its reuse nop (forcing the
#      knob on there changed ~30 matched band leaves in the first sweep).
#      HAZARDS: only the reuse nop after the macro's own mfxx is dropped; the
#      _handle_mflo_mfhi mult/div hazard nops are untouched; mult's explicit
#      cc1 `mflo` lines are not affected. Default OFF; flag-off identical.
#      SWEEP (re-run 2026-09-23 after the --expand-div narrowing,
#      tools/analysis/maspsx_knob_sweep.py --knob MASPSX_DIV_NO_REUSE_NOP,
#      baseline = this file with the two patch-16 `elif` arms removed):
#      1748 era leaves (EXE + 7 overlay plans), flag-off 1748/1748
#      byte-identical; knob-on changes 4 = the one user func_80077E64 plus
#      three era_o2_g0_expand_div leaves OUTSIDE the band that retail keeps
#      the nop for (func_80065260, func_8008F898, func_8008F9CC) -- confirms
#      per-leaf, band-only.
#  15. unfill_epilogue_delay_slot (ctor arg, or env
#      MASPSX_UNFILL_EPILOGUE_DELAY_SLOT=1): the inverse of patch 3.
#      gcc-2.8.1-psx's reorg fills the return slot with the stack pop
#      (`lw $31` / nop / `j $31` / `addu $sp,$sp,N`); retail's compiler left
#      it for the assembler, which emitted `lw $31` / `addiu $sp` / `jr $ra` /
#      nop. The patch rewrites cc1's six-line noreorder epilogue block into
#      reorder-mode `addu $sp,$sp,N` / `j $31`; maspsx then needs no
#      load-delay nop (the pop does not read $31) and adds the slot nop.
#      ROM evidence (2.8.1 band 0x80071A84-0x8008D610, every `jr $ra` whose
#      slot or predecessor is the stack pop): ra-only frames 8 unfilled / 0
#      filled (e.g. func_8007D614 0x8007D6C8, func_80077404 0x80077540,
#      func_8007A238, func_8007BDD4, func_8007CE78, func_80085F08);
#      all other frame sizes mixed (67 filled / 24 unfilled), so per-leaf.
#      HAZARDS: HU1 only the exact cc1 block with a single
#      `addu/addiu $sp,$sp,<imm>` slot is rewritten; any other slot
#      instruction, a label inside, or a missing `.set` line refuses. The
#      pop and the return are both unconditional and adjacent, so moving
#      the pop ahead of the jump is semantics-preserving. Default OFF;
#      flag-off byte-identical.
#  14. load_delay_nop_before_label (ctor arg, or env
#      MASPSX_LOAD_DELAY_NOP_BEFORE_LABEL=1): stock maspsx emits a required
#      MIPS-I load-delay nop AFTER an intervening label (`lw` / `$L8:` /
#      nop / use). When that label is immediately followed by a cc1
#      directive that opens a block -- `.set noreorder` (a cc1-scheduled
#      branch) or `#.set volatile` (a volatile access) -- retail has the nop
#      BEFORE the label (`lw` / nop / `$L8:` / use), which moves the label one
#      word and changes every branch offset to it.
#      ROM evidence: func_80081414 (`lb $2` / nop / $L8: noreorder `beq`;
#      carried an `asm volatile("nop")` workaround before this patch),
#      func_80083578 and func_8007BF44 (`lw $3,SYM` / nop / $L2: volatile
#      `lhu`/`lbu` poll loop). Regression set that keeps label-then-nop
#      (label followed by a plain reorder-mode instruction): func_800292EC
#      $L5, func_8006F224 $L14/$L7, func_8006F39C $L24/$L31.
#      PER-LEAF ONLY: forcing the knob on over the whole matched corpus
#      changes exactly 3 leaves, all `.set noreorder`-after-label shapes in
#      the older compiler region (< 0x80071A84) that retail keeps
#      label-then-nop: func_800360B4 $L2, func_8005968C $L16, func_8005BF44
#      $L21. All three positives are in the 0x80071A84-0x8008D610 band.
#      Why this is sound: both placements satisfy the fall-through load
#      delay, and HN3 proves no branch predecessor arrives with a pending
#      load (reorder-mode branches get maspsx's own nop slot).
#      HAZARDS: HN1 the label must be the very next non-comment line after the
#      load (anything else in between keeps the stock path); HN2 only
#      `.set noreorder` / `#.set volatile` directly after the label; HN3 no
#      branch/jump to the label may carry a load (or a compound macro) in a
#      cc1-filled delay slot -- that path would lose its load delay.
#      Default OFF; flag-off byte-identical.
#  12. return_via_epilogue_jump (ctor arg, or env
#      MASPSX_RETURN_VIA_EPILOGUE_JUMP=1): gcc-2.8.1-psx's reorg turns a jump
#      to a frameless function's return into a second, mid-function return
#      insn (`.set noreorder` / `j $31` / <stolen slot insn>). Retail's
#      compiler kept the jump: `j <label on the final jr $ra>` with the same
#      stolen insn in the slot. The patch rewrites every such earlier
#      `j $31` to `j $L<fresh>` and puts that label on the final `j $31`.
#      Semantics are unchanged: in a frameless function the final `j $31`
#      is a bare return, so jumping to it returns.
#      ROM evidence: func_80072334 (memmove) 0x80072368
#      `j .L80072398` / `addu $v0,$a3,$zero`, .L80072398 = the final `jr $ra`.
#      HAZARDS: HR1 frameless only (`.frame $sp,0,$31` and a zero `.mask`):
#      with a frame an earlier return is a whole duplicated epilogue and the
#      final `j $31` would skip its restores; refused. HR2 only a `j $31`
#      that opens a cc1 noreorder block (its slot is already scheduled);
#      the final return is left in place. Default OFF; flag-off identical.
#  13. load_delay_call_slot_store (ctor arg, or env
#      MASPSX_LOAD_DELAY_CALL_SLOT_STORE=1): emit the MIPS-I load-delay `nop`
#      between a load and a following cc1-noreorder `jal <symbol>` whose
#      delay-slot store (sb/sh/sw) reads the loaded register, as value or
#      base. Architecturally the `jal` already covers the load delay, but
#      in retail the store was scheduled before the call behind a
#      load-delay nop and the assembler's reorder pass later moved it into
#      the slot, leaving the nop behind.
#      ROM evidence (disc1 survey of load / jal / slot-reads-load-dest): in
#      the 0x80071A84-0x8008D610 compiler region, both store slots keep the
#      nop (func_800742B8 0x80074304 `lw $v0` / nop / jal / `sw $v0,0($v1)`,
#      func_8007A2A4 0x8007A304 `lw $v0` / nop / jal / `sb $zero,0($v0)`)
#      and the single ALU slot does not (func_80089328 0x800895F8
#      `lh $a0` / jal / `addu $a1,$a0,$zero`). In the older region 20
#      store slots have NO nop (cc1 filled them), so this is per-leaf only.
#      HAZARDS: HL1 the jal must be opened by cc1's `.set noreorder` (else
#      maspsx adds its own slot nop and the store is not in the slot);
#      HL2 no label/other instruction between the load and the jal;
#      HL3 slot must be a single store reading the loaded register; jalr and
#      branches are refused. Default OFF; flag-off byte-identical.
#  35. drop_target_dup_li (ctor arg, or env MASPSX_DROP_TARGET_DUP_LI=1):
#      reorg's "steal from the target thread" that gcc-2.8.1-psx does not do.
#      cc1 fills a noreorder conditional branch's slot with `li $r,K` (taken
#      from the fall-through) and leaves the identical `li $r,K` as the first
#      instruction of the branch target. Retail's compiler instead moved the
#      target's copy into the slot, so the target starts one word later. The
#      patch deletes the target's `li` when it is provably redundant:
#        H1 the label is referenced exactly once in the unit, by a
#           conditional branch inside a cc1 `.set noreorder` block whose
#           delay slot is textually the same `li $r,K`;
#        H2 nothing falls through into the label: the preceding instruction
#           is the delay slot of a noreorder `j`/`jr` block, or a reorder-mode
#           `j`/`jr`, with only blank, comment or `.set` lines between;
#        H3 $r is not $0/$at, and K is a plain literal.
#      On the only path into the label, $r was already set to K by the slot,
#      so deleting the target's copy changes no value.
#      ROM evidence (Parasite Eve disc1): func_80084C4C switch dispatch
#      0x80084E1C `beq $v1,$v0,.L80084E5C` / `addiu $v0,$zero,0xFF`, target
#      .L80084E5C = `j .L80084E9C` / `sb $v0,0x46($s0)` (no reload of 0xFF).
#      Default OFF; flag-off byte-identical.
#  34. fill_jump_symbol_store (ctor arg, or env
#      MASPSX_FILL_JUMP_SYMBOL_STORE=1): the plain-`j <label>` sibling of
#      patch 4's store arm. An indexed symbolic store `op $src,SYM($b)`
#      (sb/sh/sw) that immediately precedes a reorder-mode `j <label>` is
#      emitted as ASPSX's 3-word $at form with the store in the jump's slot:
#        lui $at,%hi(SYM) / addu $at,$at,$b / j L / op $src,%lo(SYM)($at)
#      instead of the store before the jump plus a `nop` slot.
#      WHY cc1 cannot: the store is one RTL insn that GNU as expands to three
#      words; reorg never puts a multi-word macro in a slot, and patch 4
#      covers `j $31` only. The `lui`/`addu` stay before the jump, so the
#      slot word is the same store executed at the same point.
#      ROM evidence (Parasite Eve disc1): func_80076C34 0x80076E04
#      `lui $at,0x800C` / `addu $at,$at,$a0` / `j .L80076E38` /
#      `sw $v0,%lo(D_800BD034)($at)` (the `.a1 = &buf` GPU-queue store).
#      HAZARDS: HS1 only sb/sh/sw with a symbolic (non-numeric) operand and a
#      base register; HS2 neither the value nor the base register is $at;
#      HS3 the next input line must be a reorder-mode plain `j <label>`
#      (patch 11's lookahead: no label, `.set`, jr/jalr, jal or `j $31` in
#      between); HS4 compound `;` macro lines refused. Patch 4's `j $31` arm
#      and patch 5 keep priority. Default OFF; flag-off byte-identical.
#  11. fill_jump_stack_store (ctor arg, or env
#      MASPSX_FILL_JUMP_STACK_STORE=1): ASPSX's reorder pass fills the delay
#      slot of a plain `j <label>` with the single-word stack store that
#      immediately precedes it:
#        sw $0,0($sp) / j $L22 / nop   ->   j $L22 / sw $0,0($sp)
#      WHY cc1 leaves the slot empty: the store is to a `volatile` local
#      (cc1 brackets it with `#.set volatile`), and gcc's reorg.c never puts
#      a volatile insn into a delay slot. ASPSX does not know about C
#      volatility and schedules it. A NON-volatile stack store before a `j`
#      is normally moved by cc1 itself, so in practice the knob reaches the
#      volatile case.
#      ROM evidence (Parasite Eve disc1, whole-image scan of `j <label>`
#      slots): 4 `sw $zero,N($sp)` fills -- func_8007DCAC 0x8007DCB8 and
#      func_800862F4 0x80086410 (both the libspu `volatile` x13 wait-loop
#      entry: `v = K; i = 0; goto test`), func_800735C4 0x80073638 and
#      0x8007387C; plus 43 register-valued sb/sh/sw $sp fills whose origin
#      (cc1 or ASPSX) the ROM alone cannot tell apart.
#      HAZARDS (each a guard; anything undecidable is refused):
#        HJ1 only `j <label>`: `jal` writes $31 and `j $31`/`j $r` belong to
#            other patches (1/3/8); refused.
#        HJ2 only a `$sp`-based store (the volatile-local shape). The moved
#            store writes no GPR and `j <label>` reads none, so the slot is
#            semantically equivalent to the pre-jump position.
#        HJ3 numeric offset within 16 bits (a single instruction; larger
#            offsets expand to an $at macro and are refused).
#        HJ4 compound `;` macro lines refused.
#        HJ5 the jump must be the very next input line: a label (jump is a
#            branch target) or `.set` (cc1 noreorder block) in between is
#            refused, and nothing happens inside a noreorder region.
#      Default OFF: opt in per leaf; flag-off is byte-identical.
#  10. fill_branch_macro_split (ctor arg, or env
#      MASPSX_FILL_BRANCH_MACRO_SPLIT=1): the CONDITIONAL-BRANCH sibling of
#      patch 8. The second word of an absolute store macro or of a `la`
#      macro that immediately precedes a reorder-mode conditional branch
#      goes into the branch's delay slot:
#        lui $at,%hi(SYM) / beq $a,$b,L / sw $r,%lo(SYM)($at)
#        lui $r,%hi(SYM)  / beqz $v,L  / addiu $r,$r,%lo(SYM)
#      This is a DIFFERENT mechanism from patch 6 and must not be confused
#      with it: patch 6 moves a fall-through-only instruction (the one AFTER
#      the branch) into a slot that executes on both edges, so it needs a
#      liveness proof and a fidelity gate. Here the moved word was ALREADY
#      before the branch, i.e. already on both edges; moving it into the
#      slot leaves the register/memory state at the branch target and at the
#      fall-through byte-identical. No liveness is involved and patch 6's
#      gates do not apply (they key on the line after the branch; this arm
#      keys on the macro line before it, and consuming the branch line means
#      patch 6 never sees that branch at all).
#      HAZARDS (each a guard; the knob refuses anything it cannot decide):
#        HB1 the branch READS its operand registers at branch time, BEFORE
#            the slot runs. The `la` arm's moved word writes its register, so
#            a `la` whose destination is one of the branch's operands would
#            let the branch test a half-formed address (only the `lui`
#            applied): refused, canonical register compare. The store arm's
#            moved word writes no GPR, so a store OF a branch operand is
#            harmless (5 such sites in ROM). `$at` is never a cc1 branch
#            operand; refused anyway.
#        HB2 `$at` lifetime: written by the `lui` that stays before the
#            branch, read by the store in the slot, dead at both the target
#            and the fall-through. It is the assembler temporary, so no
#            surviving code on either edge can read it before redefining it.
#        HB3 `bltzal`/`bgezal` write $31 like `jal`; cc1 never emits them,
#            and they are refused (a moved `sw $31` / `la $31` would see the
#            new link value).
#        HB4 label / `.set` between the macro and the branch (branch target
#            or a cc1 noreorder block that already owns the slot): refused.
#            A branch inside an active `.set noreorder` region (is_reorder
#            False) has no maspsx slot to fill: refused.
#        HB5 compound `;` lines, numeric operands, gp-relative destinations,
#            indexed `la`: refused exactly as in patch 8 (H7/H8/H9).
#        HB6 only the six cc1 branch spellings (beq/bne/blez/bgtz/bltz/bgez)
#            with a label target are accepted; anything else is refused.
#      ROM evidence (Parasite Eve disc1, whole-image scan): 19 sites / 14
#      functions -- 16 store splits (12 beq+sw, 2 bne+sw, 1 blez+sw,
#      1 bgtz+sw) and 3 `la` splits (2 beq, 1 bltz); 0 sites where a `la`
#      destination is a branch operand. Closed here: func_800739C4,
#      func_80082E00, func_8007C214, func_800809E0.
#      Default OFF: opt in per leaf; flag-off is byte-identical.
import struct
import os
import re

from typing import List

branch_mnemonics = {
    "beq",
    "bgez",
    "bgtz",
    "blez",
    "bltz",
    "bne",
}
jump_mnemonics = {
    "j",
    "jal",
}
load_mnemonics = {
    "lb",
    "lbu",
    "lh",
    "lhu",
    "lw",
    "lwl",
    "lwr",
}
store_mnemonics = {
    "sb",
    "sh",
    "sw",
    "swl",
    "swr",
    "swc2",
}

single_reg_loads = {
    "mult",
    "multu",
    "div",
    "divu",
    "rem",
    "remu",
    "move",
    "negu",
    "nor",
}
double_reg_loads = {
    "and",
    "andi",
    "or",
    "ori",
    "xor",
    "xori",
    "addu",
    "subu",
    "sll",
    "slr",
    "srl",
    "sra",
    "slt",
    "slti",
    "sltu",
}


# --- LOCAL PATCH 6 (fill_branch_delay_slot) tables -------------------------
#
# Conditional branches, split by how many register operands precede the label.
# Anything not listed here is never treated as a conditional branch by the
# patch (and is refused when met inside the liveness walk).
cond_branch_two_reg = {"beq", "bne"}
cond_branch_one_reg = {"bgez", "bgtz", "blez", "bltz"}
cond_branch_mnemonics = cond_branch_two_reg | cond_branch_one_reg

# Closed allow-list of fill candidates: every entry is a single machine word
# after GNU as / maspsx processing, touches no memory, cannot trap, writes no
# hi/lo and writes exactly its first operand.  (`move` is rewritten to `addu`
# by expand_move; `li` is range-checked so as emits one word.)
#   name -> (operand count, immediate kind)
# immediate kind: None  = all operands must be registers
#                 "s16" = last operand may be a signed 16-bit literal
#                 "u16" = last operand may be an unsigned 16-bit literal
#                 "sh"  = last operand must be a 0..31 shift count
#                 "li"  = a single literal operand (range checked separately)
branch_fill_candidates = {
    "addu": (3, "s16"),
    "addiu": (3, "s16"),
    "subu": (3, "s16"),
    "and": (3, "u16"),
    "andi": (3, "u16"),
    "or": (3, "u16"),
    "ori": (3, "u16"),
    "xor": (3, "u16"),
    "xori": (3, "u16"),
    "nor": (3, None),
    "slt": (3, "s16"),
    "slti": (3, "s16"),
    "sltu": (3, "u15"),
    "sltiu": (3, "u15"),
    "sll": (3, "sh"),
    "srl": (3, "sh"),
    "sra": (3, "sh"),
    "sllv": (3, None),
    "srlv": (3, None),
    "srav": (3, None),
    "move": (2, None),
    "li": (2, "li"),
}

# Registers this patch refuses to treat as a fill destination: the ABI-fixed
# ones (whose liveness the walk cannot reason about) plus the assembler temp.
branch_fill_forbidden_dest = {
    "$0", "$zero", "$1", "$at",
    "$26", "$27", "$k0", "$k1",
    "$28", "$gp", "$29", "$sp", "$30", "$fp", "$s8", "$31", "$ra",
}

# Liveness classifier for the reachability walk.  An instruction whose
# mnemonic is in neither table makes the walk refuse the fill.
#   writes its FIRST operand, every other register operand is a read
live_write_first = {
    "addu", "addiu", "add", "addi", "subu", "sub", "and", "andi", "or", "ori",
    "xor", "xori", "nor", "sll", "srl", "sra", "sllv", "srlv", "srav",
    "slt", "slti", "sltu", "sltiu", "lui", "li", "la", "move", "negu", "neg",
    "not", "mflo", "mfhi", "lb", "lbu", "lh", "lhu", "lw",
}
#   writes no general register; every register operand is a read
live_write_none = {
    "sb", "sh", "sw", "nop", "break", "mtlo", "mthi",
}

reg_operand_re = re.compile(r"\$[A-Za-z0-9]+")

# Canonical GPR numbering, so `$v0` and `$2` compare equal.
_gpr_names = {
    "$zero": 0, "$at": 1, "$v0": 2, "$v1": 3,
    "$a0": 4, "$a1": 5, "$a2": 6, "$a3": 7,
    "$t0": 8, "$t1": 9, "$t2": 10, "$t3": 11,
    "$t4": 12, "$t5": 13, "$t6": 14, "$t7": 15,
    "$s0": 16, "$s1": 17, "$s2": 18, "$s3": 19,
    "$s4": 20, "$s5": 21, "$s6": 22, "$s7": 23,
    "$t8": 24, "$t9": 25, "$k0": 26, "$k1": 27,
    "$gp": 28, "$sp": 29, "$fp": 30, "$s8": 30, "$ra": 31,
}


def gpr_number(name: str):
    """Canonical GPR number for a register operand, or None if it is not a
    plain general-purpose register (float/COP registers included)."""
    num = _gpr_names.get(name)
    if num is not None:
        return num
    if re.match(r"^\$\d{1,2}$", name):
        num = int(name[1:])
        if 0 <= num <= 31:
            return num
    return None


def strip_comments(line: str) -> str:
    if line.count("#") > 0:
        line = line.split("#")[0]
    return line.strip()


def line_loads_from_reg(line: str, r_source: str) -> bool:
    """
    NOTE: Returns True even if line might use $at expansion
    """
    line = strip_comments(line)

    # escape dollar
    r_source = r_source.replace("$", r"\$")

    if match := re.match(r"^([A-z][A-z0-9]*)\s+(.*)$", line):
        op, rest = match.group(1, 2)
    else:
        return False

    if op in load_mnemonics:
        # lwl	$9,7($2)
        if re.match(rf"^.*\(\s*{r_source}\s*\)$", rest):
            return True

    elif op in store_mnemonics:
        if re.match(rf"^.*\(\s*{r_source}\s*\)$", rest):
            return True
        # "line_loads_from_reg" is a bit of a lie
        if re.match(rf"^{r_source},.*$", rest):
            return True

    elif op == "lwc2":
        # lwc2 $5, 4( $4
        if re.match(rf"^.*\(\s*{r_source}\s*\)$", rest):
            return True

    elif op == "jal":
        if re.match(rf"^.*,\s*{r_source}$", rest):
            return True

    elif op == "j":
        if re.match(rf"^{r_source}$", rest):
            return True

    elif op in ("ctc2", "mtc0", "mtc2"):
        if re.match(rf"^{r_source},.*$", rest):
            return True

    elif op in ("mtlo", "mthi"):
        if re.match(rf"^{r_source}$", rest):
            return True

    elif op in branch_mnemonics:
        if re.match(rf"^{r_source},.*$", rest):
            return True
        if re.match(rf"^.*,\s*{r_source},.*$", rest):
            return True

    elif op in single_reg_loads:
        if re.match(rf"^.*,\s*{r_source}$", rest):
            return True
        if op.startswith("mult"):
            if re.match(rf"^{r_source},.*$", rest):
                return True
        if op.startswith("div") or op.startswith("rem"):
            # e.g. div	$3,$3,$7
            if re.match(rf"^.*,{r_source}.*$", rest):
                return True

    elif op in double_reg_loads:
        if re.match(rf"^.*,\s*{r_source},.*$", rest):
            return True
        if re.match(rf"^.*,.*,\s*{r_source}$", rest):
            return True

    return False


def is_number(value: str) -> bool:
    if re.match(r"^-?\d+$", value) or re.match(r"^-?0x[A-Fa-f0-9]+$", value):
        return True
    return False


def uses_at(line: str) -> bool:
    line = strip_comments(line)

    # sw	$2,%lo(s_attr)($3)
    if match := re.match(r"^s[wbh]\s+(\$[a-z0-9]+),\s*%lo\(([^(]+)\)\(([^)]+)\)", line):
        return False

    # sw	$2,D_801813A4
    # sw	$3,g_CurrentRoom+40
    # sw	$2,D_us_8017863C.4
    if match := re.match(r"^s[wbh]\s+(\$[a-z0-9]+),\s*(-?[A-z0-9_.+]+)$", line):
        operand = match.group(2)
        if not is_number(operand):
            return True

    # sb	$2,g_InputSaveName($3)
    # sw	$2,-26($16)
    elif match := re.match(r"^s[wbh]\s+(\$[a-z0-9]+),\s*([^(]+)\(([^)]+)\)", line):
        operand = match.group(2)

    # lw	$2,-1000($16)
    elif match := re.match(r"l[a-z]+\s+(\$[a-z0-9]+),\s*([^(]+)\(([^)]+)\)", line):
        operand = match.group(2)

    else:
        return False

    if is_number(operand):
        num = int(operand, 0)
        if -32769 < num < 32768:
            return False

    return True


# LOCAL PATCH helper (patch 8): canonicalise a MIPS register spelling so that
# `$4` and `$a0` (or `$1` and `$at`) compare equal. cc1 only ever emits the
# numeric form, but the hazard guards must not fail open on an alias.
_MIPS_REG_NAMES = (
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
)
_MIPS_REG_CANON = {}
for _i, _n in enumerate(_MIPS_REG_NAMES):
    _MIPS_REG_CANON[f"${_n}"] = _i
    _MIPS_REG_CANON[f"${_i}"] = _i
_MIPS_REG_CANON["$s8"] = 30  # gas alias for $fp


def canon_reg(token):
    """Return the register number for `token`, or the token itself if unknown."""
    if token is None:
        return None
    return _MIPS_REG_CANON.get(token.strip(), token)


def parse_load_or_store(rest: str):
    if match := re.match(r"(\$[a-z0-9]+),\s*%lo\(([^(]+)\)\(([^(]+)\)", rest):
        r_dest, operand, r_source = match.group(1, 2, 3)
        needs_expanding = False
    elif match := re.match(r"(\$[a-z0-9]+),\s*([^(]+)\(([^)]+)\)", rest):
        r_dest, operand, r_source = match.group(1, 2, 3)
        needs_expanding = True
    elif match := re.match(r"(\$[a-z0-9]+),\s*([^(]+)", rest):
        r_dest, operand = match.group(1, 2)
        r_source = None
        needs_expanding = True
    else:
        raise Exception(f"Unable to parse load/store instruction: {rest}")

    if re.match(r"^-?\d+$", operand) or re.match(r"^-?0x[A-Fa-f0-9]+$", operand):
        is_addend = False
    else:
        is_addend = True

    return (r_source, r_dest, operand, is_addend, needs_expanding)


def div_needs_expanding(line: str) -> bool:
    inst, *rest = line.split()
    if not (inst.startswith("div") or inst.startswith("rem")):
        return False

    r_dest, *_ = rest[0].split(",")
    return r_dest not in ("$zero", "$0")


def expand_load_immediate(line: str) -> List[str]:
    res = []

    match = re.match(r"li\s+(\$[0-9A-z]+),\s?(-?[x0-9a-fA-F]+)", line)
    assert match is not None, "li regex failed"

    r_dest = match.group(1)
    operand = int(match.group(2), 0)

    if 0 < operand < 0x10000:
        res.append(f"ori\t{r_dest},$zero,{operand}")
    elif operand >= 0x10000:
        res.append(f"lui\t{r_dest},({operand} >> 16) & 0xFFFF")
        if operand & 0xFFFF:
            res.append(f"ori\t{r_dest},{r_dest},{operand} & 0xFFFF")
    elif 0 > operand > -0x8000:
        res.append(f"addiu\t{r_dest},$zero,{operand}")
    elif operand == -0x8000:
        res.append(f"addiu\t{r_dest},$zero,{operand} & 0xFFFF")
    elif operand < -0x8000:
        res.append(f"lui\t{r_dest},({operand} >> 16) & 0xFFFF")
        if operand & 0xFFFF:
            res.append(f"ori\t{r_dest},{r_dest},{operand} & 0xFFFF")
    else:
        # TODO: raise an exception here instead?
        # ori is actually addiu on ASPSX 2.56+
        res.append(f"ori\t{r_dest},0")

    return res


def expand_move(line: str):
    line = strip_comments(line)
    op, *rest = line.split()
    if op == "move":
        args = " ".join(rest)
        r_dest, r_source = args.split(",")
        return f"addu\t{r_dest},{r_source},$zero"
    return line


def is_label(line: str):
    return re.match(r"\$L(b|e)?\d+:$", line)


def is_instruction(line: str, ignore_nop=False, ignore_set=False, ignore_label=False):
    if len(line) == 0:
        return False

    if ignore_nop and line == "#nop":
        return False
    if ignore_set and line in (
        ".set\treorder",
        ".set\tnoreorder",
        ".set\tvolatile",
        ".set\tnovolatile",
    ):
        return False
    if ignore_label and is_label(line):
        return False

    if line.startswith(".stab"):
        return False
    if line.startswith(".def") or line.startswith(".bend") or line.startswith(".begin"):
        return False
    if line.startswith(".loc"):
        return False

    if line.startswith("L") and line[1] not in "0123456789" and line.endswith(":"):
        return False
    if line in (".set\tmacro", ".set\tnomacro"):
        return False
    if line in ("#.set\tvolatile", "#.set\tnovolatile"):
        return False
    if line in ("#APP", "#NO_APP"):
        return False

    return True


def get_next_register(reg: str):
    lut = {
        # li $fx
        "$f0": "$f1",
        "$f2": "$f3",
        "$f4": "$f5",
        "$f6": "$f7",
        "$f12": "$f13",
        "$f14": "$f15",
        # names
        "$v0": "$v1",
        "$a0": "$a1",
        "$a2": "$a3",
        "$t0": "$t1",
        "$t2": "$t3",
        "$s0": "$s1",
        "$s2": "$s3",
        # nums
        "$2": "$3",  # $v0
        "$4": "$5",  # $a0
        "$6": "$7",  # $a2
        "$8": "$9",  # $t0
        "$10": "$11",  # t2
        "$18": "$19",  # s2
    }
    next_reg = lut.get(reg)
    assert next_reg is not None, f"Unknown mapping for {reg}"
    return next_reg


def expand_macro(line: str):
    res = []
    for l in strip_comments(line).split(";"):
        l = l.strip()
        if len(l) == 0:
            continue
        op, *rest = l.split()
        res.append(
            (op, " ".join(rest)),
        )
    return res


def load_immediate_single(line: str):
    res = []
    r1, value = line[5:].split(",")
    (num,) = struct.unpack(">i", struct.pack(">f", float(value)))
    upper = (num & 0xFFFF_0000) >> 16
    lower = (num & 0x0000_FFFF) >> 0

    res.append(f"lui\t{r1},0x{upper:X}")
    # we don't always need the lower part
    if lower:
        res.append(f"ori\t{r1},0x{lower:X}")
    return res


def load_immediate_double(line: str):
    res = []
    r1, value = line[5:].split(",")
    r2 = get_next_register(r1)
    (num,) = struct.unpack(">q", struct.pack(">d", float(value)))

    r1_upper = (num & 0x0000_0000_FFFF_0000) >> 16
    r1_lower = (num & 0x0000_0000_0000_FFFF) >> 0
    r2_upper = (num & 0xFFFF_0000_0000_0000) >> 48
    r2_lower = (num & 0x0000_FFFF_0000_0000) >> 32

    if r1_upper or r1_lower:
        res.append(f"lui\t{r1},0x{r1_upper:X}")
        if r1_lower:
            res.append(f"ori\t{r1},0x{r1_lower:X}")
    else:
        res.append(f"li\t{r1},0x0")

    res.append(f"lui\t{r2},0x{r2_upper:X}")
    if r2_lower:
        res.append(f"ori\t{r2},0x{r2_lower:X}")

    return res


class MaspsxProcessor:
    is_reorder = True
    skip_instructions = 0
    file_num = 1
    line_index = 0

    def __init__(
        self,
        lines: List[str],
        sdata_limit=0,
        expand_div=False,
        expand_li=False,
        nop_at_expansion=False,
        nop_mflo_mfhi=True,
        sltu_at=False,
        addiu_at=False,
        div_uses_tge=False,
        gp_allow_offset=False,
        gp_allow_la=False,
        use_comm_section=False,
        use_comm_for_lcomm=False,
        fill_store_delay_slot=False,
        fill_epilogue_delay_slot=False,
        fill_call_delay_slot=False,
        fill_branch_delay_slot=False,
        fill_branch_split_li=False,
        rodata_fold_symbols=None,
        fill_branch_macro_split=False,
        three_word_symbol_store=False,
        symbol_load_dest_temp=False,
        symbol_at_temp=False,
        dispatch_fold_symbol=None,
        fill_jump_stack_store=False,
        fill_jump_reg_store=False,
        fill_jump_symbol_store=False,
        drop_target_dup_li=False,
        return_via_epilogue_jump=False,
        load_delay_call_slot_store=False,
        load_delay_nop_before_label=False,
        unfill_epilogue_delay_slot=False,
        div_no_reuse_nop=False,
        la_absolute_small_data=None,
        fill_call_volatile_store=False,
        load_delay_nop_before_loop_label=False,
        reorder_fill_calls=False,
        spill_before_symbol_load=False,
        sink_return_zero_into_flag_delay=False,
        sink_zero_store_into_beqz=False,
        unaligned_word_copy_return_temp=False,
        drop_unused_var8=False,
        swap_s2_s1_save_pairs=False,
        nop_before_label_skip_empty_app=False,
        load_before_stack_half=False,
        ori_small_li=False,
        bnez_postdec_from_minus_one=False,
        beqz_sym_store_delay=False,
        frame48_s5_after_s2=False,
        sink_reg_sw_into_bnez=False,
        addu_neg1_beq_delay=False,
        div_delay_reuse_v0=False,
        drop_jump_to_epilogue=False,
        sltu_a0_lt_a2_dest_v1=False,
        dup_half_shift=False,
        mult_mflo_to_multu=False,
        mflo_before_mfhi=False,
        narrow_shifted_word_load=False,
        hoist_block_li=None,
        narrow_shifted_reg_load=False,
        hoist_block_li_tail1=None,
    ):
        self.lines = [x.strip() for x in lines]

        self.sdata_limit = sdata_limit

        self.expand_div = expand_div
        self.expand_li = expand_li

        self.nop_at_expansion = nop_at_expansion
        self.nop_mflo_mfhi = nop_mflo_mfhi

        self.sltu_at = sltu_at
        self.addiu_at = addiu_at
        self.div_uses_tge = div_uses_tge

        self.gp_allow_offset = gp_allow_offset
        self.gp_allow_la = gp_allow_la

        self.use_comm_section = use_comm_section
        self.use_comm_for_lcomm = use_comm_for_lcomm

        # LOCAL PATCH: opt-in via ctor arg or env var (the env var keeps the
        # untracked maspsx.py driver unpatched).
        self.fill_store_delay_slot = (
            fill_store_delay_slot
            or os.environ.get("MASPSX_FILL_STORE_DELAY_SLOT") == "1"
        )
        # LOCAL PATCH: opt-in epilogue stack-adjust into the return delay slot.
        self.fill_epilogue_delay_slot = (
            fill_epilogue_delay_slot
            or os.environ.get("MASPSX_FILL_EPILOGUE_DELAY_SLOT") == "1"
        )
        # LOCAL PATCH (patch 8): opt-in store/`la` macro second word into the
        # delay slot of a plain `jal <symbol>` / `j <label>`.
        self.fill_call_delay_slot = (
            fill_call_delay_slot
            or os.environ.get("MASPSX_FILL_CALL_DELAY_SLOT") == "1"
        )
        # LOCAL PATCH 6: opt-in fill of a reorder-mode conditional branch delay
        # slot from the following instruction (see the patch log).
        # LOCAL PATCH 19: split-constant lui into a preceding branch slot.
        self.fill_branch_split_li = (
            fill_branch_split_li
            or os.environ.get("MASPSX_FILL_BRANCH_SPLIT_LI") == "1"
        )
        self.fill_branch_delay_slot = (
            fill_branch_delay_slot
            or os.environ.get("MASPSX_FILL_BRANCH_DELAY_SLOT") == "1"
        )
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None
        # LOCAL PATCH (patch 10): opt-in store/`la` macro second word into the
        # delay slot of a following conditional branch (see the patch log).
        self.fill_branch_macro_split = (
            fill_branch_macro_split
            or os.environ.get("MASPSX_FILL_BRANCH_MACRO_SPLIT") == "1"
        )
        # LOCAL PATCH: the env gate keeps the untracked maspsx.py driver
        # untouched and permits per-leaf selection.
        self.three_word_symbol_store = (
            three_word_symbol_store
            or os.environ.get("MASPSX_THREE_WORD_SYMBOL_STORE") == "1"
        )
        # LOCAL PATCH: naive dest-register address temp for compound indexed
        # symbolic loads/stores (see patch log entry 4).
        self.symbol_load_dest_temp = (
            symbol_load_dest_temp
            or os.environ.get("MASPSX_SYMBOL_LOAD_DEST_TEMP") == "1"
        )
        # LOCAL PATCH: 3-word $at address temp that keeps the %lo displacement
        # for compound indexed symbolic loads/stores (see patch log entry 5).
        # MASPSX_SYMBOL_AT_TEMP=1 (or ctor True) applies to every indexed
        # symbolic operand; a comma list restricts the rewrite to those names.
        raw_at_temp = os.environ.get("MASPSX_SYMBOL_AT_TEMP")
        if symbol_at_temp or raw_at_temp == "1":
            self.symbol_at_temp = True
            self.symbol_at_temp_only = None
        elif raw_at_temp and raw_at_temp not in ("0", "false"):
            self.symbol_at_temp = True
            self.symbol_at_temp_only = set(
                s for s in raw_at_temp.split(",") if s
            )
        else:
            self.symbol_at_temp = False
            self.symbol_at_temp_only = None
        # LOCAL PATCH (switch dispatch retarget): substitute the shared
        # rodata pool table symbol for cc1-local $L<n> switch-table
        # labels in compound loads/stores. See docs/design/
        # switch_table_ownership.md.
        self.dispatch_fold_symbol = (
            dispatch_fold_symbol
            or os.environ.get("MASPSX_DISPATCH_FOLD") or None
        )
        # Multi-table form: MASPSX_DISPATCH_FOLD=<sym0>,<sym1>,... maps the
        # i-th cc1 switch table ($L<n> label defined in .rdata, in .rdata
        # emission order) to the i-th pool symbol. A single symbol keeps the
        # original "every $L<digits> operand" substitution byte-identically.
        self.dispatch_fold_symbols = (
            [s.strip() for s in self.dispatch_fold_symbol.split(",") if s.strip()]
            if self.dispatch_fold_symbol
            else []
        )
        self._dispatch_fold_map = None
        # LOCAL PATCH 20: `$LC<n>` literals -> retail pool symbols.
        fold = rodata_fold_symbols or os.environ.get("MASPSX_RODATA_FOLD") or ""
        self.rodata_fold_symbols = [
            x.strip() for x in fold.split(",") if x.strip()
        ]
        # LOCAL PATCH 11: opt-in ASPSX reorder fill of a plain `j <label>`
        # slot with the immediately preceding single-word $sp store.
        self.fill_jump_stack_store = (
            fill_jump_stack_store
            or os.environ.get("MASPSX_FILL_JUMP_STACK_STORE") == "1"
        )
        # LOCAL PATCH K6: MASPSX_FILL_JUMP_REG_STORE. Patch 11 for any
        # register base (not only $sp): cc1's multi-word block move
        # (`movstrsi`, e.g. strcpy(dst, "..") -> lh/lb/sh/sb) is ONE insn,
        # so reorg never splits its last store into the following `j`
        # slot; ASPSX does (func_80081A7C 0x80081BE8 `j` / `sb $v1,34($s5)`).
        self.fill_jump_reg_store = (
            fill_jump_reg_store
            or os.environ.get("MASPSX_FILL_JUMP_REG_STORE") == "1"
        )
        # LOCAL PATCH 34: indexed symbolic store into a plain `j <label>` slot.
        self.fill_jump_symbol_store = (
            fill_jump_symbol_store
            or os.environ.get("MASPSX_FILL_JUMP_SYMBOL_STORE") == "1"
        )
        # LOCAL PATCH 35: drop a branch target's `li` duplicated in the slot.
        self.drop_target_dup_li = (
            drop_target_dup_li
            or os.environ.get("MASPSX_DROP_TARGET_DUP_LI") == "1"
        )
        # LOCAL PATCH 12: retarget a frameless function's mid-body return
        # (`j $31` in a cc1 noreorder block) to its final `j $31`.
        self.return_via_epilogue_jump = (
            return_via_epilogue_jump
            or os.environ.get("MASPSX_RETURN_VIA_EPILOGUE_JUMP") == "1"
        )
        # LOCAL PATCH 13: load-delay nop before a noreorder `jal` whose
        # delay-slot store reads the just-loaded register.
        self.load_delay_call_slot_store = (
            load_delay_call_slot_store
            or os.environ.get("MASPSX_LOAD_DELAY_CALL_SLOT_STORE") == "1"
        )
        # LOCAL PATCH 14: load-delay nop BEFORE a label that opens a cc1
        # directive block (see the patch log).
        self.load_delay_nop_before_label = (
            load_delay_nop_before_label
            or os.environ.get("MASPSX_LOAD_DELAY_NOP_BEFORE_LABEL") == "1"
        )
        # LOCAL PATCH 15: undo cc1's own `j $31` / `addu $sp` epilogue fill.
        self.unfill_epilogue_delay_slot = (
            unfill_epilogue_delay_slot
            or os.environ.get("MASPSX_UNFILL_EPILOGUE_DELAY_SLOT") == "1"
        )
        # LOCAL PATCH 16: no load-delay-style reuse nop after the mflo/mfhi
        # that ends a div/rem macro expansion.
        self.div_no_reuse_nop = (
            div_no_reuse_nop
            or os.environ.get("MASPSX_DIV_NO_REUSE_NOP") == "1"
        )
        # LOCAL PATCH 17: `la` of a small-data symbol expands absolute.
        # DEFAULT ON since 2026-09-23 (retail never forms a gp `la`);
        # MASPSX_LA_ABSOLUTE_SMALL_DATA=0 opts out. The ctor argument
        # defaults to None = follow the environment.
        env_la = os.environ.get("MASPSX_LA_ABSOLUTE_SMALL_DATA")
        self.la_absolute_small_data = (
            la_absolute_small_data
            if la_absolute_small_data is not None
            else env_la != "0"
        )
        # LOCAL PATCH 22: -fno-delayed-branch call-slot / epilogue fills.
        self.reorder_fill_calls = (
            reorder_fill_calls
            or os.environ.get("MASPSX_REORDER_FILL_CALLS") == "1"
        )
        # LOCAL PATCH: move one prologue `sw $sN,imm($sp)` ahead of a symbolic
        # `lw $r,SYM` that cc1 scheduled in front of it. Opt-in. The two
        # instructions are independent ($r is not $sN); retail keeps the
        # callee-save first so the load-delay slot can be the following `sw $ra`.
        self.spill_before_symbol_load = (
            spill_before_symbol_load
            or os.environ.get("MASPSX_SPILL_BEFORE_SYMBOL_LOAD") == "1"
        )
        # LOCAL PATCH 23: cc1 filled a `beq $2,$0,$Lep` delay with the
        # fall-through's symbolic load, and maspsx then yanks that macro out
        # and leaves a nop. Retail keeps `move $2,$0` in that delay (the
        # early-out return 0; $2 is the flag just tested) and the load only
        # on the fall-through. The same `move $2,$0` currently sits in front
        # of the noreorder `j $31` / `addu $sp` epilogue. Opt-in: one such
        # beq, delay is one ordinary instruction, epilogue zero is the
        # instruction immediately before that noreorder block. The delay
        # instruction moves to the fall-through (taken path no longer
        # executes it) and the epilogue zero is deleted so patch 15 can
        # unfill `j $31` / `addu $sp`. Default OFF.
        self.sink_return_zero_into_flag_delay = (
            sink_return_zero_into_flag_delay
            or os.environ.get("MASPSX_SINK_RETURN_ZERO_INTO_FLAG_DELAY") == "1"
        )
        # LOCAL PATCH: a `sw $0,SYM` immediately before a reorder-mode
        # `beq $r,$0,$L` is the address-temp half of a store that retail
        # keeps in that branch's delay (`lui $at` / `beqz` / `sw %lo($at)`).
        # cc1 emits the store first and leaves the slot empty. Opt-in.
        # The store writes neither the branch register nor memory the branch
        # reads, so moving it into the slot preserves both paths.
        self.sink_zero_store_into_beqz = (
            sink_zero_store_into_beqz
            or os.environ.get("MASPSX_SINK_ZERO_STORE_INTO_BEQZ") == "1"
        )
        # LOCAL PATCH 24: cc1 2.8.1's 4-byte movstrsi scratch is $t0.
        # PSY-Q uses $v1 when the source base is $v0 and $v0 otherwise.
        # Opt-in. Renames only a consecutive lwl/lwr/swl/swr $8 quartet
        # (lwl +3 / lwr +0, swl/swr to $sp three bytes apart) so the later
        # load-delay nop still sees the register the store reads.
        self.unaligned_word_copy_return_temp = (
            unaligned_word_copy_return_temp
            or os.environ.get("MASPSX_UNALIGNED_WORD_COPY_RETURN_TEMP") == "1"
        )
        # LOCAL PATCH 25: cc1 reserves an 8-byte stack temporary for
        # `ptr + 1 - i + i` and then deletes every use, leaving vars=8.
        # Opt-in. Shrinks a frame of exactly 32 whose only $sp memory
        # operands are the callee saves at 24 and 28, down to 24 with
        # those saves at 16 and 20. Refuses any other $sp use.
        self.drop_unused_var8 = (
            drop_unused_var8
            or os.environ.get("MASPSX_DROP_UNUSED_VAR8") == "1"
        )
        # LOCAL PATCH 26: cc1 saves $s1=$a0 before $s2=$a1. Retail saves
        # the $s2 pair first. Opt-in, and only that exact quartet.
        self.swap_s2_s1_save_pairs = (
            swap_s2_s1_save_pairs
            or os.environ.get("MASPSX_SWAP_S2_S1_SAVE_PAIRS") == "1"
        )
        # LOCAL PATCH 36 (K-ULW): cc1 follows `ulw $r` with a `#nop` load-delay
        # comment that maspsx drops; ASPSX kept a real nop after lwl/lwr.
        # Opt-in: an `ulw` line immediately followed by `#nop` gets a real nop.
        self.ulw_load_delay_nop = os.environ.get("MASPSX_ULW_LOAD_DELAY_NOP") == "1"
        # LOCAL PATCH 37 (K-SHIFT): cc1 2.8.1 reload_cse substitutes a hard
        # reg known to hold a constant into a shift count (`sll $d,$s,$t`,
        # assembled as sllv). PSY-Q cc1 never does. Opt-in: fold the count
        # back to the `li $t,K` immediate (0..31) seen earlier in the block.
        self.shift_count_const = os.environ.get("MASPSX_SHIFT_COUNT_CONST") == "1"
        # LOCAL PATCH 40 (K-RCSE): the same reload_cse artifact on a copy or
        # an add -- `move $d,$s` / `addu $d,$s,$t` where a source register
        # holds a known `li` constant -> `li $d,K` / `addu $d,$s,K`.
        self.move_const_li = os.environ.get("MASPSX_RELOAD_CSE_CONST") == "1"
        # LOCAL PATCH 41 (K-STEAL): the retail reorg steals the delay insn of
        # a `j` sequence at a reorder-mode conditional branch's target and
        # retargets the branch to that `j`'s label (gcc
        # steal_delay_list_from_target). cc1 2.8.1 cannot, because its target
        # is a RETURN (`j $31`). Runs after patch 12 turned those into `j $L`.
        # Opt-in; the stolen insn's destination must be dead on fall-through.
        self.branch_steal_jump_slot = (
            os.environ.get("MASPSX_BRANCH_STEAL_JUMP_SLOT") == "1"
        )
        # LOCAL PATCH 42 (K-TESTED-LI): the retail reorg fills a reorder-mode
        # conditional branch's slot with the next `li` even though it writes
        # a register the branch tests (cc1 reorg refuses: the trial sets a
        # resource the branch needs). Opt-in, and only for a VOID function:
        # a return counts as killing every caller-saved register.
        # LOCAL PATCH 44 (K-SINK-VRELOAD): a volatile stack local stands in
        # for a pseudo that retail spilled; retail's reload of it is placed by
        # sched2 one insn before its use. Opt-in: sink a `#.set volatile`
        # `lw $r,N($sp)` down to just before the insn preceding its first use.
        self.sink_volatile_stack_load = (
            os.environ.get("MASPSX_SINK_VOLATILE_STACK_LOAD") == "1"
        )
        # LOCAL PATCH 45 (K-STEAL-TGT): the retail reorg steals a branch
        # target's first insn into the slot even when it writes the tested
        # register, and retargets past it. cc1 refuses (the trial sets a
        # resource the branch needs). Opt-in; the destination must be dead
        # on the fall-through path.
        # "1": only an insn writing a tested register (cc1 never steals
        # those); "any": also untested destinations cc1 left unstolen.
        self.branch_steal_target_insn = os.environ.get(
            "MASPSX_BRANCH_STEAL_TARGET_INSN", ""
        ) in ("1", "any")
        self.branch_steal_target_any = (
            os.environ.get("MASPSX_BRANCH_STEAL_TARGET_INSN") == "any"
        )
        # LOCAL PATCH 46 (K-JMOVE): ASPSX fills a reorder-mode `j $L` slot
        # with the immediately preceding one-word register move. Opt-in.
        self.fill_jump_preceding_move = (
            os.environ.get("MASPSX_FILL_JUMP_PRECEDING_MOVE") == "1"
        )
        # LOCAL PATCH 47: patch 14 for any following label (not only one
        # that opens a cc1 directive block). Opt-in.
        self.load_delay_nop_before_any_label = (
            os.environ.get("MASPSX_LOAD_DELAY_NOP_BEFORE_ANY_LABEL") == "1"
        )
        self.fill_branch_tested_li_void = (
            os.environ.get("MASPSX_FILL_BRANCH_TESTED_LI_VOID") == "1"
        )
        # LOCAL PATCH 38 (K-SAVE-REV): the reverse of patch 26 -- cc1 saves
        # the $s2=$a1 pair before $s1=$a0; retail saves $s1 first.
        self.swap_s1_s2_save_pairs_rev = (
            os.environ.get("MASPSX_SWAP_S1_S2_SAVE_PAIRS_REV") == "1"
        )
        # LOCAL PATCH 39 (K-LDLI): cc1 puts a `li` between a symbolic pointer
        # load and the dependent `lw 0($r)`; retail loads both pointers first
        # (a preceding independent move fills the first load delay).
        self.symbol_pointer_load_li_after = (
            os.environ.get("MASPSX_SYMBOL_POINTER_LOAD_LI_AFTER") == "1"
        )
        # LOCAL PATCH 27: an empty asm barrier emits #APP/#NO_APP between a
        # label and the `.set noreorder` that patch 14 looks for, so the
        # load-delay nop stays after the label. Opt-in companion to patch 14.
        self.nop_before_label_skip_empty_app = (
            nop_before_label_skip_empty_app
            or os.environ.get("MASPSX_NOP_BEFORE_LABEL_SKIP_EMPTY_APP") == "1"
        )
        # LOCAL PATCH 28: a volatile `sh` to 0($sp) sits before the `lhu`
        # that retail schedules first. Opt-in. Swaps that pair when the
        # halfword load is not through $sp and the registers differ.
        # Comment lines between them (the `#.set volatile` pair) stay put.
        self.load_before_stack_half = (
            load_before_stack_half
            or os.environ.get("MASPSX_LOAD_BEFORE_STACK_HALF") == "1"
        )
        # LOCAL PATCH 29: cc1 emits `li $r,1` and gas turns that into
        # `addiu $r,$zero,1`. This leaf's constants are PSY-Q `ori $r,$zero,imm`
        # (0 < imm < 0x8000). Larger and negative `li` stay for gas.
        self.ori_small_li = (
            ori_small_li
            or os.environ.get("MASPSX_ORI_SMALL_LI") == "1"
        )
        # LOCAL PATCH 30: `n-- != 0` becomes `li $z,-1` / `addu $n,$n,-1` /
        # `bne $n,$z` with the loop store in the delay. Retail tests the
        # pre-decrement value (`bne $n,$zero`) and decrements in that slot,
        # with the store ahead of the branch. Opt-in, one occurrence, and
        # only when $z is otherwise dead.
        self.bnez_postdec_from_minus_one = (
            bnez_postdec_from_minus_one
            or os.environ.get("MASPSX_BNEZ_POSTDEC_FROM_MINUS_ONE") == "1"
        )
        # LOCAL PATCH 31: cc1 stores the incremented counter before `beqz`
        # and zeroes `$v0` in that delay. Retail puts the symbolic `sw` in
        # the delay and zeroes `$v0` on the taken path only, so the other
        # path `j`s over that zero with `li $v0,-1` in its delay.
        self.beqz_sym_store_delay = (
            beqz_sym_store_delay
            or os.environ.get("MASPSX_BEQZ_SYM_STORE_DELAY") == "1"
        )
        # LOCAL PATCH 32: cc1 saves $s5 before the $s2=$a0 pair and sizes the
        # frame at 48 (16 bytes of outgoing args + seven saves, aligned).
        # Retail saves $s2, copies $a0, then saves $s5, and the frame is 56
        # because of 8 bytes of unused locals. Opt-in. Fires only on that
        # exact quartet plus one `addu $sp,$sp,48`, and only when every other
        # $sp operand is an lw/sw.
        self.frame48_s5_after_s2 = (
            frame48_s5_after_s2
            or os.environ.get("MASPSX_FRAME48_S5_AFTER_S2") == "1"
        )
        # LOCAL PATCH 33: cc1 stores a value and then `bne`s on that same
        # register with the else-update as the following line. In reorder
        # mode the delay is a nop and the store stays ahead of the branch.
        # Retail puts that store in the delay so the taken path keeps the
        # pre-update value. Opt-in. Only `sw $r, DEST` immediately before
        # `bne $r, $0/$zero, $L`, and only when DEST does not use $r.
        self.sink_reg_sw_into_bnez = (
            sink_reg_sw_into_bnez
            or os.environ.get("MASPSX_SINK_REG_SW_INTO_BNEZ") == "1"
        )
        # LOCAL PATCH: cc1 compares against `li $r,-1` and reuses that
        # register for the decrement (`addu $r,$s,$r`) in the beq delay.
        # Retail writes `addiu $r,$s,-1` in the slot. Opt-in.
        self.addu_neg1_beq_delay = (
            addu_neg1_beq_delay
            or os.environ.get("MASPSX_ADDU_NEG1_BEQ_DELAY") == "1"
        )
        # LOCAL PATCH: cc1 parks `b + 1` in $a3 because $v0 still holds the
        # slt the preceding beq already consumed. Retail reuses $v0 for that
        # divisor (`addu $v0,$a1,1` in the delay) and the two expanded `rem`
        # guards plus the join compares follow. Opt-in. The first `mfhi $v1`
        # / `sll $a0,$v1,16` stay; only the second remainder and the compares
        # that spilled into $a0/$a1 move back to $v0/$v1.
        self.div_delay_reuse_v0 = (
            div_delay_reuse_v0
            or os.environ.get("MASPSX_DIV_DELAY_REUSE_V0") == "1"
        )
        # LOCAL PATCH: cc1 emits `j $Lepi` / `addu $v0,$zero,$zero` to skip an
        # empty asm barrier, and $Lepi is the very next `j $31`. The jump's
        # delay is the zeroing, so deleting the jump leaves that zeroing as
        # the fallthrough into the epilogue. Opt-in.
        self.drop_jump_to_epilogue = (
            drop_jump_to_epilogue
            or os.environ.get("MASPSX_DROP_JUMP_TO_EPILOGUE") == "1"
        )
        # LOCAL PATCH: after `bne $v0,$0` / `li $v0,1`, cc1 reuses $v0 for
        # `sltu $v0,$a0,$a2`. Retail keeps that compare in $v1 so the 1 in
        # the delay is not the same register as the next test. Opt-in. Only
        # that 5-instruction window.
        self.sltu_a0_lt_a2_dest_v1 = (
            sltu_a0_lt_a2_dest_v1
            or os.environ.get("MASPSX_SLTU_A0_LT_A2_DEST_V1") == "1"
        )
        # LOCAL PATCH: a signed-char zero test comes out as `sll $v0,$v0,24` /
        # `beq $v0` and `-1 << 16` is folded to `li $v0,-65536`. Retail tests
        # the copy in $v1, plants -1 in that delay, and shifts on both arms.
        # Opt-in. Only that window.
        self.dup_half_shift = (
            dup_half_shift
            or os.environ.get("MASPSX_DUP_HALF_SHIFT") == "1"
        )
        # LOCAL PATCH: cc1 emits `mult` when only `mflo` is consumed, because
        # the low 32 bits match `multu`. Retail still writes `multu`. Opt-in.
        # A following `mfhi` is left alone.
        self.mult_mflo_to_multu = (
            mult_mflo_to_multu
            or os.environ.get("MASPSX_MULT_MFLO_TO_MULTU") == "1"
        )
        # LOCAL PATCH: a signed 64-bit square is `mfhi` then `mflo`, then
        # two `addu $dst,$tmp,$zero` copies. Retail writes `mflo` then
        # `mfhi` into those destinations. Two hazard nops stay only when
        # another `mult` follows. Opt-in.
        self.mflo_before_mfhi = (
            mflo_before_mfhi
            or os.environ.get("MASPSX_MFLO_BEFORE_MFHI") == "1"
        )
        # LOCAL PATCH (K3): `lw $d,SYM($b)` ... `sra $d,$d,N` (16 <= N <= 31)
        # only uses the word's upper half. Retail narrows the load to
        # `lh $d,SYM+2($b)` and shifts by N-16 (the Psy-Q sin table read
        # `D_800966EC[i] >> 21` -> `lh D_800966EE` / `sra 5`). No vendored
        # cc1 emits it. Opt-in; only symbolic word loads.
        self.narrow_shifted_word_load = (
            narrow_shifted_word_load
            or os.environ.get("MASPSX_NARROW_SHIFTED_WORD_LOAD") == "1"
        )
        # LOCAL PATCH K3b: MASPSX_NARROW_SHIFTED_REG_LOAD. The same
        # narrowing for a register-base numeric-offset load:
        # `lw $d,N($b)` ... `sra $d,$d,M` (16 <= M <= 31) ->
        # `lh $d,N+2($b)` ... `sra $d,$d,M-16` (room func_8019A290's
        # `*(int *)p >> 21`). Separate flag so K3 users stay identical.
        self.narrow_shifted_reg_load = (
            narrow_shifted_reg_load
            or os.environ.get("MASPSX_NARROW_SHIFTED_REG_LOAD") == "1"
        )
        # LOCAL PATCH K4: MASPSX_HOIST_BLOCK_LI. In a straight-line run of
        # store groups that each reload the same absolute word
        # (`lw $x,SYM`), cc1 emits every group constant as `li $r,K` in
        # the reload's delay slot. Retail (HUD colour blocks of
        # func_800299CC / func_8002BC90 / func_8001D340) materialises the
        # first n-2 constants before reload 1 and the last two directly
        # after reloads 1 and 2. "1" = any symbol; otherwise a comma list
        # of the reloaded symbols the pass may touch. Opt-in.
        if hoist_block_li is None:
            hoist_block_li = os.environ.get("MASPSX_HOIST_BLOCK_LI", "")
        hoist_block_li = (hoist_block_li or "").strip()
        if hoist_block_li in ("", "0"):
            self.hoist_block_li = None
        elif hoist_block_li == "1":
            self.hoist_block_li = True
        else:
            self.hoist_block_li = {
                s.strip() for s in hoist_block_li.split(",") if s.strip()
            }
        # LOCAL PATCH K4 tail-1: MASPSX_HOIST_BLOCK_LI_TAIL1=<sym,...>. A K4
        # region any of whose lines names a listed symbol (func_8002BC90 /
        # func_8001D340's stride-72 D_800B0130 gauge blocks) instead moves
        # constants 0..n-2 before reload 1 and constant n-1 directly after
        # reload 1; reload 2 keeps its delay-slot nop. Only read when K4 is
        # on. Retail func_8002BC90 0x8002C724: five `li` / `lw D_8009CDDC` /
        # `li $4,1` ... `lw D_8009CDDC` / `nop`.
        if hoist_block_li_tail1 is None:
            hoist_block_li_tail1 = os.environ.get("MASPSX_HOIST_BLOCK_LI_TAIL1", "")
        self.hoist_block_li_tail1 = {
            s.strip() for s in (hoist_block_li_tail1 or "").split(",") if s.strip()
        }
        # LOCAL PATCH 21: load-delay nop before a loop-head label.
        self.load_delay_nop_before_loop_label = (
            load_delay_nop_before_loop_label
            or os.environ.get("MASPSX_LOAD_DELAY_NOP_BEFORE_LOOP_LABEL") == "1"
        )
        # LOCAL PATCH 18: register-based store into a following `jal` slot.
        self.fill_call_volatile_store = (
            fill_call_volatile_store
            or os.environ.get("MASPSX_FILL_CALL_VOLATILE_STORE") == "1"
        )

        self.bss_entries: dict[str, int] = {}
        self.sbss_entries: dict[str, int] = {}
        self.sdata_entries: dict[str, int] = {}
        # LOCAL PATCH 7: sized `.extern SYM, size` declarations that fit the
        # small-data limit.  GNU as places these gp-relative exactly like
        # `.comm` symbols, so a load/store macro on them collapses to ONE
        # instruction and a preceding load needs its MIPS-I load-delay nop
        # (see _uses_gp / _handle_nop_before_next_instruction).
        self.extern_entries: dict[str, int] = {}

        self.comm_symbols: set[str] = set()

    def preprocess_lines(self) -> None:
        in_sdata = False
        uses_size = False

        for line in self.lines:
            if line == "":
                continue

            if line.startswith(".align"):
                # TODO: worry about alignment later
                continue

            if line.startswith(".globl"):
                continue

            if line.startswith(".text"):
                in_sdata = False
                continue
            if line.startswith(".data"):
                in_sdata = False
                continue
            if line.startswith(".rdata"):
                in_sdata = False
                continue

            if line.startswith(".section") and line.endswith(".text"):
                in_sdata = False
                continue

            if line.startswith("#"):
                continue

            if line.startswith(".sdata"):
                in_sdata = True
                continue

            if line.startswith(".file"):
                in_sdata = False
                continue

            if line.startswith(".extern"):
                # e.g.	.extern	D_8009CE84, 2   (cc1 -G<n> emits these for
                # every external whose size fits the small-data limit)
                in_sdata = False
                if match := re.match(
                    r"\.extern\s+([^,\s]+)\s*,\s*(\d+)", line
                ):
                    symbol, size = match.group(1), int(match.group(2))
                    if size <= self.sdata_limit:
                        self.extern_entries[symbol] = size
                continue

            if line.startswith(".comm") or line.startswith(".lcomm"):
                # e.g.	.comm	MENU_RadarScale_800AB480,4
                in_sdata = False
                _, var = line.split()
                symbol, size_str, *_ = var.split(",")
                size = int(size_str)
                if size <= self.sdata_limit:
                    self.sbss_entries[symbol] = size
                else:
                    self.bss_entries[symbol] = size

                if line.startswith(".comm"):
                    self.comm_symbols.add(symbol)
                continue

            if in_sdata:
                # NOTE: newer compilers emit .size for sdata, old ones do not...
                if match := re.match(r"\.size\s+([^,]+),([0-9]+)", line):
                    current_symbol = match.group(1)
                    size = int(match.group(2))
                    self.sdata_entries[current_symbol] = size
                    uses_size = True
                    continue

                if not uses_size:
                    if line.endswith(":"):
                        current_symbol = line.replace(":", "")
                        self.sdata_entries[current_symbol] = 0
                    else:
                        if line.startswith(".type"):
                            continue

                        if line.startswith(".space"):
                            _, size_str = line.split()
                            size = int(size_str)
                        elif line.startswith(".word"):
                            size = 4
                        elif line.startswith(".half") or line.startswith(".short"):
                            size = 2
                        elif line.startswith(".byte"):
                            size = 1
                        elif line.startswith(".ascii"):
                            # e.g. .ascii	"Map poly groups\000"
                            # NOTE: len('.ascii\t""') == 9
                            size = len(line) - 9
                        else:
                            raise Exception(
                                f"Unable to parse .sdata instruction: {line}"
                            )
                        self.sdata_entries[current_symbol] += size

    def _dispatch_rdata_tables(self):
        """cc1-local `$L<digits>` labels defined inside `.rdata`, in order.

        cc1 emits each switch's jump table right after its tablejump, bracketed
        by `.rdata` ... `.text`; the definition order here is therefore the
        order of the tables in the object's .rodata section.
        """
        tables = []
        in_rdata = False
        for line in self.lines:
            directive = line.split("#", 1)[0].strip()
            head = directive.split()[0] if directive.split() else ""
            if head in (".rdata",):
                in_rdata = True
                continue
            if head in (".text", ".data", ".sdata", ".bss", ".sbss") or (
                head == ".section"
            ):
                in_rdata = directive.split()[1:2] == [".rodata"] if head == ".section" else False
                continue
            if in_rdata:
                match = re.match(r"^(\$L\d+):", directive)
                if match:
                    tables.append(match.group(1))
        return tables

    def _dispatch_fold_target(self, operand):
        # LOCAL PATCH (switch dispatch retarget, multi-table).
        if len(self.dispatch_fold_symbols) <= 1:
            return self.dispatch_fold_symbol
        if self._dispatch_fold_map is None:
            tables = self._dispatch_rdata_tables()
            if len(tables) != len(self.dispatch_fold_symbols):
                raise ValueError(
                    f"MASPSX_DISPATCH_FOLD names {len(self.dispatch_fold_symbols)} "
                    f"tables but cc1 emitted {len(tables)} .rdata switch tables "
                    f"({', '.join(tables) or 'none'})"
                )
            self._dispatch_fold_map = dict(zip(tables, self.dispatch_fold_symbols))
        if operand not in self._dispatch_fold_map:
            raise ValueError(
                f"MASPSX_DISPATCH_FOLD: {operand} is not a .rdata switch table"
            )
        return self._dispatch_fold_map[operand]

    def _rodata_lc_labels(self) -> List[str]:
        # cc1 `$LC<n>` literal labels defined inside `.rdata`, in emission
        # order (= their order in the object's .rodata).
        labels = []
        in_rdata = False
        for line in self.lines:
            directive = line.split("#", 1)[0].strip()
            head = directive.split()[0] if directive.split() else ""
            if head == ".rdata":
                in_rdata = True
                continue
            if head in (".text", ".data", ".sdata", ".bss", ".sbss", ".section"):
                in_rdata = head == ".section" and directive.split()[1:2] == [".rodata"]
                continue
            if in_rdata:
                m = re.match(r"^(\$LC\d+):", directive)
                if m:
                    labels.append(m.group(1))
        return labels

    def _fold_rodata_literals(self) -> None:
        # LOCAL PATCH 20 (rodata_fold, env MASPSX_RODATA_FOLD=s0,s1,...): the
        # i-th cc1 `$LC<n>` literal defined in .rdata is retail's shared pool
        # object s<i> (e.g. an initialized local array's image in the EXE
        # .rodata pool). Every operand that names `$LC<n>` -- `la $r,$LCn`,
        # a plain load `op $r,$LCn` -- and `$LCn+off` is retargeted to the
        # pool symbol, exactly like MASPSX_DISPATCH_FOLD does for switch
        # tables. The build then strips the object's duplicate .rodata only
        # after proving it byte-identical to the pool blocks
        # (disc1_build.strip_rodata_fold). HR1 the list must name exactly as
        # many symbols as cc1 emitted $LC labels (else ValueError: a wrong
        # count would silently mis-map); HR2 only whole-token `$LC<n>`
        # operands of instruction lines are rewritten, never the `.rdata`
        # definitions themselves.
        labels = self._rodata_lc_labels()
        if len(labels) != len(self.rodata_fold_symbols):
            raise ValueError(
                f"MASPSX_RODATA_FOLD names {len(self.rodata_fold_symbols)} "
                f"literals but cc1 emitted {len(labels)} .rdata $LC labels "
                f"({', '.join(labels) or 'none'})"
            )
        mapping = dict(zip(labels, self.rodata_fold_symbols))
        token = re.compile(r"(?<![\w$])(\$LC\d+)(?![\w])")
        out = []
        for line in self.lines:
            stripped = line.strip()
            if (
                stripped
                and not stripped.startswith((".", "#"))
                and not stripped.endswith(":")
                and token.search(stripped)
            ):
                new = token.sub(
                    lambda m: mapping.get(m.group(1), m.group(1)), line
                )
                if new != line:
                    out.append(f"# RODATA_FOLD: {stripped}")
                line = new
            out.append(line)
        self.lines = out

    def _fold_dispatch_table_la(self) -> None:
        # LOCAL PATCH (switch dispatch retarget, `la` form): cc1 may hoist the
        # switch table's address out of a loop as a plain `la $r,$L<n>` and
        # then index it with `addu`/`lw 0($r)`, so no indexed `$L<n>($b)`
        # operand ever reaches the load arm. Rewrite such an `la` to the pool
        # symbol exactly like the indexed form. HD1 only labels DEFINED in
        # .rdata (a real cc1 switch table) are rewritten, in single- and
        # multi-table mode alike; HD2 only a plain two-operand `la` line (no
        # `;` compound, no offset, no index).
        tables = set(self._dispatch_rdata_tables())
        if not tables:
            return
        out = []
        for line in self.lines:
            m = re.match(r"^la\t(\$\w+),(\$L\d+)$", line)
            if m and m.group(2) in tables:
                target = self._dispatch_fold_target(m.group(2))
                out.append(f"# DISPATCH_FOLD LA: {m.group(2)} -> {target}")
                line = f"la\t{m.group(1)},{target}"
            out.append(line)
        self.lines = out

    def _load_before_stack_half(self):
        """Move `lhu` ahead of a preceding `sh` to `$sp`."""
        sh_re = re.compile(r"^sh\t(\$\w+),(-?\d+)\(\$sp\)$")
        lhu_re = re.compile(r"^lhu\t(\$\w+),(-?\d+)\((\$\w+)\)$")
        lines = self.lines
        out = []
        i = 0
        while i < len(lines):
            if sh_re.match(lines[i]):
                j = i + 1
                while j < len(lines) and (
                    lines[j] == "" or lines[j].startswith("#")
                ):
                    j += 1
                m2 = lhu_re.match(lines[j]) if j < len(lines) else None
                sh = sh_re.match(lines[i])
                if (
                    m2
                    and m2.group(3) != "$sp"
                    and sh.group(1) != m2.group(1)
                ):
                    out.append(lines[j])
                    out.append(lines[i])
                    out.extend(lines[i + 1:j])
                    i = j + 1
                    continue
            out.append(lines[i])
            i += 1
        self.lines = out

    def _bnez_postdec_from_minus_one(self):
        """Rewrite one `bne $n,$z` against a hoisted -1 into a pre-test."""
        lines = self.lines
        li_re = re.compile(r"^li\t(\$\w+),(-1|0xffffffff)$")
        dec_re = re.compile(r"^addu\t(\$\w+),\1,-1$")
        bne_re = re.compile(r"^bne\t(\$\w+),(\$\w+),(\S+)$")
        sw_re = re.compile(r"^sw\t\$\w+,-?\d+\(\$\w+\)$")

        def bare(line: str) -> str:
            return strip_comments(line).strip()

        li_hits = []
        for i, line in enumerate(lines):
            m = li_re.match(bare(line))
            if m:
                li_hits.append((i, m.group(1)))
        if len(li_hits) != 1:
            return
        li_at, reg = li_hits[0]
        found = None
        for i in range(len(lines) - 6):
            dec = dec_re.match(bare(lines[i]))
            if not dec:
                continue
            cnt = dec.group(1)
            if bare(lines[i + 1]) != ".set\tnoreorder":
                continue
            if bare(lines[i + 2]) != ".set\tnomacro":
                continue
            bne = bne_re.match(bare(lines[i + 3]))
            if not bne or bne.group(1) != cnt or bne.group(2) != reg:
                continue
            if not sw_re.match(bare(lines[i + 4])):
                continue
            if bare(lines[i + 5]) != ".set\tmacro":
                continue
            if bare(lines[i + 6]) != ".set\treorder":
                continue
            found = (i, cnt, bne.group(3), bare(lines[i + 4]))
            break
        if found is None:
            return
        for j, line in enumerate(lines):
            if j == li_at or j == found[0] + 3:
                continue
            if reg in set(re.findall(r"\$\w+", bare(line))):
                return
        i, cnt, label, sw = found
        out = []
        for j, line in enumerate(lines):
            if j == li_at:
                continue
            if j == i:
                out.append(sw)
                continue
            if j == i + 3:
                out.append(f"bne\t{cnt},$zero,{label}")
                continue
            if j == i + 4:
                out.append(f"addu\t{cnt},{cnt},-1")
                continue
            out.append(line)
        self.lines = out

    def _ulw_load_delay_nop(self):
        """Keep cc1's `#nop` after `ulw $r,...` as a real load-delay nop."""
        out = []
        lines = self.lines
        for i, line in enumerate(lines):
            out.append(line)
            if (
                re.match(r"^ulw\t\$\w+,", line)
                and i + 1 < len(lines)
                and lines[i + 1] == "#nop"
            ):
                out += [".set\tnoreorder", "nop", ".set\treorder"]
        self.lines = out

    def _shift_count_const(self):
        """Undo cc1 2.8.1 post-reload reload_cse constant->register
        substitutions (PSY-Q cc1 has no such pass).

        Tracks registers whose latest write is `li $r,K` within one extended
        block, the way reload_cse does: a label resets everything; a `j`/`jr`
        resets after its delay slot (noreorder) or at once (reorder, where the
        next line is dead or a label); `jal`/`jalr` likewise, after the slot
        in noreorder mode and before the next line in reorder mode.
        Conditional branches do not reset (reload_cse does not either).

        MASPSX_SHIFT_COUNT_CONST: `sll|srl|sra $d,$s,$t` with $t == K
        (0..31) -> `sll|srl|sra $d,$s,K`.
        MASPSX_RELOAD_CSE_CONST: `move $d,$s` with $s == K (16-bit) ->
        `li $d,K` (`move $d,$0` for 0); `addu $d,$s,$t` with exactly one of
        $s/$t == K (16-bit signed) -> `addu $d,<other>,K`.
        """
        shift_re = re.compile(r"^(sll|srl|sra)\t(\$\w+),(\$\w+),(\$\w+)$")
        move_re = re.compile(r"^move\t(\$\w+),(\$\w+)$")
        addu_re = re.compile(r"^addu\t(\$\w+),(\$\w+),(\$\w+)$")
        li_re = re.compile(r"^li\t(\$\w+),(-?\d+)$")
        dest_re = re.compile(r"^(\w+)\t(\$\w+)")
        do_shift = getattr(self, "shift_count_const", False)
        do_move = getattr(self, "move_const_li", False)
        known = {}
        noreorder = False
        pending_reset = False
        out = []
        for line in self.lines:
            bare = line.split("#", 1)[0].strip()
            if bare == ".set\tnoreorder":
                noreorder = True
            elif bare == ".set\treorder":
                noreorder = False
            if re.match(r"^[\$.\w]+:$", bare):
                known = {}
                pending_reset = False
            is_insn = bool(bare) and not bare.startswith(".") and not bare.endswith(":")
            if is_insn:
                m = shift_re.match(bare) if do_shift else None
                if m and m.group(4) in known and 0 <= known[m.group(4)] <= 31:
                    line = f"{m.group(1)}\t{m.group(2)},{m.group(3)},{known[m.group(4)]}"
                    bare = line
                m = move_re.match(bare) if do_move else None
                if (
                    m and m.group(2) in known and m.group(1) != m.group(2)
                    and -0x8000 <= known[m.group(2)] <= 0x7FFF
                ):
                    if known[m.group(2)] == 0:
                        line = f"move\t{m.group(1)},$0"
                    else:
                        line = f"li\t{m.group(1)},{known[m.group(2)]}"
                    bare = line
                m = addu_re.match(bare) if do_move else None
                if m:
                    a, b = m.group(2), m.group(3)
                    if (a in known) != (b in known):
                        other, k = (b, known[a]) if a in known else (a, known[b])
                        if -0x8000 <= k <= 0x7FFF and other not in ("$0", "$zero"):
                            line = f"addu\t{m.group(1)},{other},{k}"
                            bare = line
                m = li_re.match(bare)
                if m:
                    known[m.group(1)] = int(m.group(2))
                elif bare.startswith("move\t") and bare.endswith(",$0"):
                    known[bare[5:].split(",")[0]] = 0
                else:
                    m = dest_re.match(bare)
                    if m and m.group(2) in known and not m.group(1).startswith(
                        ("sw", "sb", "sh", "b", "j")
                    ):
                        known.pop(m.group(2), None)
                if pending_reset:
                    known = {}
                    pending_reset = False
                if re.match(r"^(jal|j|jalr|jr)\b", bare):
                    if noreorder:
                        pending_reset = True
                    else:
                        known = {}
            out.append(line)
        self.lines = out

    def _swap_s1_s2_save_pairs_rev(self):
        """Swap a prologue `sw/move $s2,$a1` pair with the following `$s1,$a0` pair."""
        lines = self.lines
        move_s1 = ("move\t$17,$4", "addu\t$17,$4,$0", "addu\t$17,$4,$zero")
        move_s2 = ("move\t$18,$5", "addu\t$18,$5,$0", "addu\t$18,$5,$zero")
        for i in range(len(lines) - 3):
            if (
                lines[i] == "sw\t$18,24($sp)"
                and lines[i + 1] in move_s2
                and lines[i + 2] == "sw\t$17,20($sp)"
                and lines[i + 3] in move_s1
            ):
                a, b, c, d = lines[i:i + 4]
                lines[i:i + 4] = [c, d, a, b]
                self.lines = lines
                return

    def _symbol_pointer_load_li_after(self):
        """`lw $r,SYM` / `li $k,K` / `lw $d,0($r)` -> load, load, li; a
        preceding independent `move` moves into the first load delay."""
        out = list(self.lines)
        sym_re = re.compile(r"^lw\t(\$\w+),[A-Za-z_]\w*$")
        li_re = re.compile(r"^li\t(\$\w+),")
        ld_re = re.compile(r"^lw\t(\$\w+),0\((\$\w+)\)$")
        mv_re = re.compile(r"^move\t(\$\w+),(\$\w+)$")
        i = 0
        while i < len(out) - 2:
            b = sym_re.match(out[i])
            c = li_re.match(out[i + 1])
            d = ld_re.match(out[i + 2])
            if (
                b and c and d
                and d.group(2) == b.group(1)
                and c.group(1) not in (b.group(1), d.group(1))
            ):
                out[i + 1], out[i + 2] = out[i + 2], out[i + 1]
                pm = mv_re.match(out[i - 1]) if i > 0 else None
                if pm and b.group(1) not in (pm.group(1), pm.group(2)):
                    out[i - 1], out[i] = out[i], out[i - 1]
                i += 3
                continue
            i += 1
        self.lines = out

    def _branch_steal_jump_slot(self):
        """Reorder-mode `bXX ...,$L` where `$L:` opens a noreorder
        `j $M` / I block -> noreorder `bXX ...,$M` / I (I stays at $L too)."""
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None
        lines = self.lines
        labels = self._branch_fill_labels()
        reorder = self._branch_fill_reorder_flags()
        edits = {}
        for i, line in enumerate(lines):
            if not reorder[i]:
                continue
            bare = strip_comments(line)
            if not bare:
                continue
            op = bare.split(None, 1)[0]
            if op not in cond_branch_mnemonics:
                continue
            operands = self._branch_fill_operands(bare)
            want = 3 if op in cond_branch_two_reg else 2
            if len(operands) != want or operands[-1] not in labels:
                continue
            k = labels[operands[-1]] + 1
            body = []
            while k < len(lines) and len(body) < 6:
                t = strip_comments(lines[k])
                if t:
                    body.append(t)
                k += 1
            if (
                len(body) < 6
                or body[0] != ".set\tnoreorder"
                or body[1] != ".set\tnomacro"
                or body[4] != ".set\tmacro"
                or body[5] != ".set\treorder"
            ):
                continue
            jm = re.match(r"^j\t(\$L\d+)$", body[2])
            if not jm or jm.group(1) not in labels:
                continue
            cand = body[3]
            cop = cand.split(None, 1)[0]
            cargs = self._branch_fill_operands(cand)
            if cop == "move" and len(cargs) == 2:
                pass
            elif cop == "li" and len(cargs) == 2 and is_number(cargs[1]) and (
                -0x8000 <= int(cargs[1], 0) <= 0xFFFF
            ):
                pass
            else:
                continue
            dest = gpr_number(cargs[0])
            if dest is None or dest in (0, 1) or dest >= 26:
                continue
            if not self._branch_fill_reg_dead_at(None, dest, start_index=i + 1):
                continue
            new_ops = ",".join(operands[:-1] + [jm.group(1)])
            edits[i] = [
                ".set\tnoreorder",
                ".set\tnomacro",
                f"{op}\t{new_ops}",
                cand,
                ".set\tmacro",
                ".set\treorder",
            ]
        if edits:
            out = []
            for i, line in enumerate(lines):
                out.extend(edits.get(i, [line]))
            self.lines = out
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None

    def _fill_branch_tested_li_void(self):
        """Reorder-mode `bXX ...,$L` + `li $r,K` ($r tested by the branch,
        dead at $L with a void return) -> noreorder `bXX` / `li` pair."""
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None
        lines = self.lines
        labels = self._branch_fill_labels()
        reorder = self._branch_fill_reorder_flags()
        edits = {}
        drop = set()
        for i, line in enumerate(lines):
            if not reorder[i] or i in drop:
                continue
            bare = strip_comments(line)
            if not bare:
                continue
            op = bare.split(None, 1)[0]
            if op not in cond_branch_mnemonics:
                continue
            operands = self._branch_fill_operands(bare)
            want = 3 if op in cond_branch_two_reg else 2
            if len(operands) != want or operands[-1] not in labels:
                continue
            j = i + 1
            while j < len(lines) and lines[j] == "":
                j += 1
            if j >= len(lines) or not reorder[j]:
                continue
            cand = strip_comments(lines[j])
            m = re.match(r"^li\t(\$\w+),(-?\w+)$", cand)
            if not m or not is_number(m.group(2)):
                continue
            if not -0x8000 <= int(m.group(2), 0) <= 0xFFFF:
                continue
            dest = gpr_number(m.group(1))
            tested = [gpr_number(t) for t in operands[:-1]]
            if dest is None or dest in (0, 1) or dest not in tested:
                continue
            if not self._branch_fill_reg_dead_at(
                operands[-1], dest, void_return=True
            ):
                continue
            edits[i] = [
                ".set\tnoreorder",
                ".set\tnomacro",
                bare,
                cand,
                ".set\tmacro",
                ".set\treorder",
            ]
            drop.add(j)
        if edits:
            out = []
            for i, line in enumerate(lines):
                if i in drop:
                    continue
                out.extend(edits.get(i, [line]))
            self.lines = out
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None

    def _sink_volatile_stack_load(self):
        """`#.set volatile` / `lw $r,N($sp)` / `#.set novolatile` followed by
        straight-line insns that neither touch $r nor store to N($sp) ->
        move the triple to just before the insn that precedes $r's first use.
        """
        load_re = re.compile(r"^lw\t(\$\d+),(-?\d+)\(\$sp\)$")
        regs_re = re.compile(r"\$\w+")
        lines = self.lines
        i = 0
        while i < len(lines) - 2:
            m = load_re.match(strip_comments(lines[i + 1]))
            if not (
                lines[i] == "#.set\tvolatile"
                and m
                and lines[i + 2] == "#.set\tnovolatile"
            ):
                i += 1
                continue
            r, off = m.group(1), m.group(2)
            insns = []  # indices of the following straight-line insns
            use = None
            k = i + 3
            ok = True
            while k < len(lines):
                t = strip_comments(lines[k])
                if not t:
                    k += 1
                    continue
                if t.startswith(".") or t.endswith(":") or t.startswith("#"):
                    ok = False
                    break
                op = t.split(None, 1)[0]
                ops = self._branch_fill_operands(t)
                toks = regs_re.findall(" ".join(ops))
                if op in cond_branch_mnemonics or op in ("j", "jr", "jal", "jalr"):
                    ok = False
                    break
                if r in toks[1:] or (toks and toks[0] == r and op.startswith("s")):
                    use = k
                    break
                if toks and toks[0] == r:
                    ok = False  # $r overwritten before use
                    break
                if op in ("sw", "sh", "sb", "swl", "swr"):
                    base = ops[-1] if ops else ""
                    if not base.endswith("($sp)") or base == f"{off}($sp)":
                        ok = False
                        break
                insns.append(k)
                k += 1
            if not ok or use is None or len(insns) < 2:
                i += 1
                continue
            dest = insns[-1]  # insert before the insn that precedes the use
            triple = lines[i:i + 3]
            lines = lines[:i] + lines[i + 3:dest] + triple + lines[dest:]
            i = dest
        self.lines = lines

    def _branch_steal_target_insn(self):
        """Reorder-mode `bXX r..,$L` (next line is not fillable: another
        branch) whose `$L:` starts with a one-word ALU insn writing a tested
        register -> noreorder `bXX r..,$Lnew` / insn, `$Lnew:` after it."""
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None
        lines = self.lines
        labels = self._branch_fill_labels()
        reorder = self._branch_fill_reorder_flags()
        counter = getattr(self, "_steal_target_counter", 0)
        edits = {}
        new_labels = {}
        for i, line in enumerate(lines):
            if not reorder[i]:
                continue
            bare = strip_comments(line)
            if not bare:
                continue
            op = bare.split(None, 1)[0]
            if op not in cond_branch_mnemonics:
                continue
            operands = self._branch_fill_operands(bare)
            want = 3 if op in cond_branch_two_reg else 2
            if len(operands) != want or operands[-1] not in labels:
                continue
            j = i + 1
            while j < len(lines) and (
                lines[j] == "" or lines[j].startswith(".set\t")
            ):
                j += 1
            nxt = strip_comments(lines[j]) if j < len(lines) else ""
            if not nxt or nxt.split(None, 1)[0] not in cond_branch_mnemonics:
                continue
            t = labels[operands[-1]] + 1
            while t < len(lines) and lines[t] == "":
                t += 1
            if t >= len(lines) or not reorder[t]:
                continue
            cand = strip_comments(lines[t])
            cop = cand.split(None, 1)[0] if cand else ""
            cargs = self._branch_fill_operands(cand)
            lui_li = (
                cop == "li" and len(cargs) == 2 and is_number(cargs[1])
                and (int(cargs[1], 0) & 0xFFFF) == 0
                and -0x80000000 <= int(cargs[1], 0) <= 0xFFFFFFFF
            )
            spec = (2, None) if lui_li else branch_fill_candidates.get(cop)
            if spec is None or ";" in cand:
                continue
            if len(cargs) != spec[0]:
                continue
            if spec[1] in ("s16", "u16", "sh") and gpr_number(cargs[-1]) is None:
                if not is_number(cargs[-1]):
                    continue
                v = int(cargs[-1], 0)
                if spec[1] == "sh" and not 0 <= v <= 31:
                    continue
                if spec[1] != "sh" and not -0x8000 <= v <= 0xFFFF:
                    continue
            dest = gpr_number(cargs[0])
            tested = [gpr_number(x) for x in operands[:-1]]
            if dest is None or dest in (0, 1) or dest >= 26:
                continue
            if dest not in tested and not self.branch_steal_target_any:
                continue
            if any(gpr_number(x) == dest for x in cargs[1:]):
                continue
            if not self._branch_fill_reg_dead_at(
                None, dest, extended=True, start_index=i + 1
            ):
                continue
            # the target label right after the next branch, referenced only
            # by this branch: the fall-through already ran the slot, so the
            # insn MOVES (retail's reorg deletes the now-redundant copy)
            lab_idx = labels[operands[-1]]
            k = j + 1
            while k < len(lines) and lines[k] == "":
                k += 1
            refs = sum(
                1 for x in lines
                if re.search(r"[,\s]" + re.escape(operands[-1]) + r"$", strip_comments(x))
            )
            if k == lab_idx and refs == 1:
                new_ops = ",".join(operands)
                edits[i] = [
                    ".set\tnoreorder",
                    ".set\tnomacro",
                    f"{op}\t{new_ops}",
                    cand,
                    ".set\tmacro",
                    ".set\treorder",
                ]
                new_labels[t] = None
                continue
            counter += 1
            lab = f"$L{910000 + counter}"
            new_ops = ",".join(operands[:-1] + [lab])
            edits[i] = [
                ".set\tnoreorder",
                ".set\tnomacro",
                f"{op}\t{new_ops}",
                cand,
                ".set\tmacro",
                ".set\treorder",
            ]
            new_labels[t] = f"{lab}:"
        self._steal_target_counter = counter
        if edits:
            out = []
            for i, line in enumerate(lines):
                if i in new_labels and new_labels[i] is None:
                    continue  # moved into the branch slot
                out.extend(edits.get(i, [line]))
                if i in new_labels:
                    out.append(new_labels[i])
            self.lines = out
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None

    def _fill_jump_preceding_move(self):
        """Reorder-mode `move $d,$s` immediately followed by `j $L` ->
        noreorder `j $L` / `move $d,$s`."""
        self._branch_fill_reorder = None
        reorder = self._branch_fill_reorder_flags()
        lines = self.lines
        out = []
        i = 0
        while i < len(lines):
            a = strip_comments(lines[i])
            b = strip_comments(lines[i + 1]) if i + 1 < len(lines) else ""
            if (
                reorder[i]
                and re.match(r"^move\t\$\w+,\$\w+$", a)
                and re.match(r"^j\t\$L\d+$", b)
            ):
                out += [".set\tnoreorder", ".set\tnomacro", b, a,
                        ".set\tmacro", ".set\treorder"]
                i += 2
                continue
            out.append(lines[i])
            i += 1
        self.lines = out
        self._branch_fill_reorder = None
        self._branch_fill_label_map = None

    def _swap_s2_s1_save_pairs(self):
        """Swap a prologue `sw/move $s1,$a0` pair with the following `$s2,$a1` pair."""
        lines = self.lines
        move_s1 = ("move\t$17,$4", "addu\t$17,$4,$0", "addu\t$17,$4,$zero")
        move_s2 = ("move\t$18,$5", "addu\t$18,$5,$0", "addu\t$18,$5,$zero")
        for i in range(len(lines) - 3):
            if (
                lines[i] == "sw\t$17,20($sp)"
                and lines[i + 1] in move_s1
                and lines[i + 2] == "sw\t$18,24($sp)"
                and lines[i + 3] in move_s2
            ):
                a, b, c, d = lines[i:i + 4]
                lines[i:i + 4] = [c, d, a, b]
                self.lines = lines
                return

    def _drop_unused_var8(self):
        """Shrink a 32-byte frame whose 8-byte local area is never used."""
        mem_re = re.compile(
            r"^(?:lw|sw|lh|sh|lb|sb|lhu|lbu|lwl|lwr|swl|swr)\t\$\w+,(-?\d+)\(\$sp\)$"
        )
        alloc = []
        free = []
        mem = []
        for i, line in enumerate(self.lines):
            if "$sp" not in line and not line.startswith(".frame"):
                continue
            if line.startswith(".frame") or line.startswith(".mask"):
                continue
            m = re.match(r"^(subu|addu|addiu)\t\$sp,\$sp,(-?\d+)$", line)
            if m:
                imm = int(m.group(2))
                if m.group(1) == "subu" and imm == 32:
                    alloc.append(i)
                elif m.group(1) == "addiu" and imm == -32:
                    alloc.append(i)
                elif m.group(1) in ("addu", "addiu") and imm == 32:
                    free.append(i)
                else:
                    return
                continue
            m = mem_re.match(line)
            if m:
                mem.append((i, int(m.group(1))))
                continue
            return
        if len(alloc) != 1 or len(free) != 1 or not mem:
            return
        if any(off < 24 or off > 28 for _, off in mem):
            return
        lines = list(self.lines)
        lines[alloc[0]] = lines[alloc[0]].replace(",32", ",24").replace(",-32", ",-24")
        lines[free[0]] = lines[free[0]].replace(",32", ",24")
        for i, off in mem:
            lines[i] = lines[i].replace(f",{off}($sp)", f",{off - 8}($sp)", 1)
        self.lines = lines

    def _rename_unaligned_word_copy_temp(self):
        """Rename a $t0 4-byte unaligned copy to $v1 (base $v0) or $v0."""
        op_re = re.compile(r"^(lwl|lwr|swl|swr)\t\$8,(-?\d+)\((\$\w+)\)$")
        lines = self.lines
        out = []
        i = 0
        n = len(lines)
        while i < n:
            matched = False
            m0 = op_re.match(lines[i]) if i + 3 < n else None
            if m0 and m0.group(1) == "lwl" and m0.group(2) == "3":
                base = m0.group(3)
                m1 = op_re.match(lines[i + 1])
                m2 = op_re.match(lines[i + 2])
                m3 = op_re.match(lines[i + 3])
                quartet = (
                    m1
                    and m1.group(1) == "lwr"
                    and m1.group(2) == "0"
                    and m1.group(3) == base
                    and m2
                    and m2.group(1) == "swl"
                    and m2.group(3) == "$sp"
                    and m3
                    and m3.group(1) == "swr"
                    and m3.group(3) == "$sp"
                    and int(m3.group(2)) == int(m2.group(2)) - 3
                )
                dest = "$3" if base == "$2" else "$2"
                if quartet and dest != base:
                    for k in range(4):
                        out.append(lines[i + k].replace("$8,", f"{dest},", 1))
                    i += 4
                    matched = True
            if not matched:
                out.append(lines[i])
                i += 1
        self.lines = out

    def _sink_reg_sw_into_bnez(self, res):
        """Move `sw $r,SYM` into the empty delay of the following
        `bne $r,$0,$L`. A symbol store is split so `%hi` stays ahead of
        the branch and `%lo($at)` occupies the slot. The stock nop that
        filled that slot is dropped. Flag-off never calls this.
        """
        def real(line):
            s = line.strip()
            return bool(s) and not s.startswith("#") and not s.startswith(".") and not s.endswith(":")

        out = []
        i = 0
        while i < len(res):
            sw = res[i].split()
            if len(sw) == 2 and sw[0] == "sw" and "," in sw[1]:
                reg, dest = sw[1].split(",", 1)
                j = i + 1
                while j < len(res) and not real(res[j]):
                    j += 1
                k = j + 1
                while k < len(res) and not real(res[k]):
                    k += 1
                bne = res[j].split() if j < len(res) else []
                nop = res[k].split() if k < len(res) else []
                label = None
                if len(bne) == 2 and bne[0] == "bne":
                    ops = bne[1].split(",")
                    if (len(ops) == 3 and ops[0] == reg
                            and ops[1] in ("$0", "$zero")):
                        label = ops[2]
                if (label is not None and reg != "$0" and reg not in dest
                        and nop and nop[0] == "nop"):
                    block = [".set\tnoat", f"lui\t$at,%hi({dest})"] if "(" not in dest else []
                    slot = (f"sw\t{reg},%lo({dest})($at)" if "(" not in dest
                            else res[i].strip())
                    block += [
                        ".set\tnoreorder",
                        ".set\tnomacro",
                        f"bne\t{reg},$0,{label}",
                        slot,
                        ".set\tmacro",
                        ".set\treorder",
                    ]
                    if "(" not in dest:
                        block.append(".set\tat")
                    # Keep any comment/blank lines that sat between sw and nop,
                    # except the nop itself.
                    out.extend(block)
                    i = k + 1
                    continue
            out.append(res[i])
            i += 1
        return out

    def _frame48_s5_after_s2(self, res):
        """See the constructor note. Returns res unchanged unless the
        prologue is exactly `subu $sp,48` / `sw $21` / `sw $18` /
        `addu $18,$4,$zero` and the epilogue is one `addu $sp,$sp,48`.
        """
        real = []
        for i, line in enumerate(res):
            s = line.strip()
            if not s or s.startswith("#") or s.startswith(".") or s.endswith(":"):
                continue
            real.append(i)
        if len(real) < 4:
            return res
        def parts(i):
            return res[i].split()

        if parts(real[0]) != ["subu", "$sp,$sp,48"]:
            return res
        sw_s5 = parts(real[1])
        sw_s2 = parts(real[2])
        move = parts(real[3])
        if (len(sw_s5) != 2 or sw_s5[0] != "sw" or not sw_s5[1].startswith("$21,")
                or not sw_s5[1].endswith("($sp)")):
            return res
        if (len(sw_s2) != 2 or sw_s2[0] != "sw" or not sw_s2[1].startswith("$18,")
                or not sw_s2[1].endswith("($sp)")):
            return res
        if move != ["addu", "$18,$4,$zero"]:
            return res
        epilogues = [i for i in real if parts(i) == ["addu", "$sp,$sp,48"]]
        if len(epilogues) != 1:
            return res
        for i in real:
            for tok in parts(i):
                if "$sp" in tok and i not in (real[0], epilogues[0]):
                    if not (tok.endswith("($sp)") and parts(i)[0] in ("sw", "lw")):
                        return res
        # Move the $s5 save to just after the $s2 = $a0 copy.
        s5_line = res[real[1]]
        del res[real[1]]
        # real[3] shifted left by one.
        insert_at = real[3]
        res.insert(insert_at, s5_line)

        def bump_mem(line):
            def repl(m):
                return f"{int(m.group(1)) + 8}($sp)"
            return re.sub(r"(-?\d+)\(\$sp\)", repl, line)

        out = []
        for line in res:
            toks = line.split()
            if toks == ["subu", "$sp,$sp,48"] or toks == ["addu", "$sp,$sp,48"]:
                line = line.replace("$sp,$sp,48", "$sp,$sp,56")
            out.append(bump_mem(line))
        return out

    def _drop_target_dup_li_pass(self):
        # LOCAL PATCH 35 (drop_target_dup_li); see the header for H1-H3.
        lines = self.lines
        li_re = re.compile(r"^li\t(\$\w+),(-?(?:0x[0-9A-Fa-f]+|\d+))$")

        def instr(i):
            return strip_comments(lines[i]).strip()

        def is_noise(line):
            return line == "" or line.startswith("#") or line.startswith(".set\t")

        reorder = []
        state = True
        for line in lines:
            reorder.append(state)
            if line.startswith(".set\t"):
                if line.endswith("\tnoreorder"):
                    state = False
                elif line.endswith("\treorder"):
                    state = True
        for idx, line in enumerate(lines):
            if not (line.startswith("$L") and line.endswith(":")):
                continue
            label = line[:-1]
            # the target's first instruction
            j = idx + 1
            while j < len(lines) and is_noise(lines[j]):
                j += 1
            if j >= len(lines) or ";" in lines[j]:
                continue
            m = li_re.match(instr(j))
            if not m or canon_reg(m.group(1)) in (0, 1):  # H3
                continue
            want = instr(j)
            # H1: exactly one reference, a noreorder conditional branch
            refs = [
                k for k, x in enumerate(lines)
                if k != idx and re.search(r"(^|[\s,])" + re.escape(label) + r"$", strip_comments(x).strip())
            ]
            if len(refs) != 1:
                continue
            k = refs[0]
            bop = instr(k).split(None, 1)[0] if instr(k) else ""
            if bop not in cond_branch_mnemonics or reorder[k]:
                continue
            s_ = k + 1
            while s_ < len(lines) and lines[s_] in ("", ".set\tmacro", ".set\tnomacro"):
                s_ += 1
            if s_ >= len(lines) or instr(s_) != want:
                continue
            # H2: no fall-through into the label
            q = idx - 1
            while q >= 0 and is_noise(lines[q]):
                q -= 1
            if q < 0:
                continue
            prev = instr(q).split(None, 1)
            ok = False
            if prev and prev[0] in ("j", "jr") and reorder[q]:
                ok = True
            else:
                q2 = q - 1
                while q2 >= 0 and lines[q2] in ("", ".set\tmacro", ".set\tnomacro"):
                    q2 -= 1
                if q2 >= 0 and not reorder[q2]:
                    p2 = instr(q2).split(None, 1)
                    if p2 and p2[0] in ("j", "jr"):
                        ok = True
            if not ok:
                continue
            lines[j] = "# DROP_TARGET_DUP_LI " + lines[j]

    def _cdk_split_dispatch(self) -> None:
        # LOCAL PATCH (K-CDKJT, MASPSX_CDK_SPLIT_DISPATCH=1): gcc-2.7.2-cdk
        # always splits addresses, so its switch dispatch is
        #     beq  $c,$0,L ; lui $t,%hi($Ln) (slot) ; addiu $t,$t,%lo($Ln)
        #     sll  $x,$x,2 ; addu $x,$x,$t ; lw $t,0($x) ; [#nop] ; j $t
        # Stock 2.7.2 (and retail ASPSX) index the table symbol directly:
        #     beq  $c,$0,L ; sll $t,$x,2 (slot) ; lw $t,$Ln($t) ; j $t
        # Rewrite the first shape into the second so the ordinary indexed-
        # symbol load path (and MASPSX_DISPATCH_FOLD) applies. Guards: the
        # exact 5-insn shape with one $Ln label in both halves, $t != $x,
        # and the next instruction after the lw is `j $t`, so the only
        # changed register value ($x ends as the scaled index instead of
        # the entry address) is never read by the dispatch itself.
        L = self.lines
        out = []
        i = 0
        n = len(L)
        while i < n:
            m_lui = re.match(r"^lui\t(\$\w+),%hi\((\$L\d+)\)(\s*#.*)?$", L[i]) if i + 1 < n else None
            if m_lui and out and re.match(r"^b\w*\t", out[-1]):
                t, lab = m_lui.group(1), m_lui.group(2)
                j = i + 1
                sets = []
                while j < n and L[j] in (".set\tmacro", ".set\treorder", ""):
                    sets.append(L[j])
                    j += 1
                m_add = j < n and re.match(
                    rf"^addiu\t{re.escape(t)},{re.escape(t)},%lo\({re.escape(lab)}\)(\s*#.*)?$", L[j])
                m_sll = j + 1 < n and re.match(r"^sll\t(\$\w+),(\$\w+),2$", L[j + 1])
                ok = bool(m_add and m_sll) and m_sll.group(1) == m_sll.group(2) and m_sll.group(1) != t
                if ok:
                    x = m_sll.group(1)
                    ok = (j + 3 < n and L[j + 2] == f"addu\t{x},{x},{t}"
                          and L[j + 3] == f"lw\t{t},0({x})")
                if ok:
                    k = j + 4
                    while k < n and L[k] in ("", "#nop"):
                        k += 1
                    ok = k < n and L[k] == f"j\t{t}"
                if ok and ".set\tmacro" in sets and ".set\treorder" in sets:
                    out.append(f"sll\t{t},{x},2")
                    out.extend(sets)
                    out.append(f"# CDK_SPLIT_DISPATCH: {lab}")
                    out.append(f"lw\t{t},{lab}({t})")
                    i = j + 4
                    continue
            out.append(L[i])
            i += 1
        self.lines = out

    def process_lines(self):
        if os.environ.get("MASPSX_CDK_SPLIT_DISPATCH") == "1":
            self._cdk_split_dispatch()
        if getattr(self, "ulw_load_delay_nop", False):
            self._ulw_load_delay_nop()
        if getattr(self, "shift_count_const", False) or getattr(
            self, "move_const_li", False
        ):
            self._shift_count_const()
        if getattr(self, "swap_s1_s2_save_pairs_rev", False):
            self._swap_s1_s2_save_pairs_rev()
        if getattr(self, "symbol_pointer_load_li_after", False):
            self._symbol_pointer_load_li_after()
        if getattr(self, "drop_target_dup_li", False):
            self._drop_target_dup_li_pass()
        self.is_reorder = True
        self._dispatch_fold_map = None
        self.skip_instructions = 0
        self.file_num = 1

        # LOCAL PATCH 6 caches are derived from self.lines; drop them here so a
        # reused processor re-derives them.
        self._branch_fill_label_map = None
        self._branch_fill_reorder = None

        self.bss_entries = {}
        self.sbss_entries = {}
        self.sdata_entries = {}
        self.extern_entries = {}

        self.preprocess_lines()
        if self.narrow_shifted_word_load or self.narrow_shifted_reg_load:
            self.lines = self._narrow_shifted_word_load(self.lines)
        if self.hoist_block_li:
            self.lines = self._hoist_block_li(self.lines)
        if self.unaligned_word_copy_return_temp:
            self._rename_unaligned_word_copy_temp()
        if self.drop_unused_var8:
            self._drop_unused_var8()
        if self.swap_s2_s1_save_pairs:
            self._swap_s2_s1_save_pairs()
        if self.load_before_stack_half:
            self._load_before_stack_half()
        if self.bnez_postdec_from_minus_one:
            self._bnez_postdec_from_minus_one()
        if self.dispatch_fold_symbol:
            self._fold_dispatch_table_la()
        if self.rodata_fold_symbols:
            self._fold_rodata_literals()
        if self.return_via_epilogue_jump:
            self._retarget_mid_function_returns()
        if getattr(self, "sink_volatile_stack_load", False):
            self._sink_volatile_stack_load()
        if getattr(self, "branch_steal_jump_slot", False):
            self._branch_steal_jump_slot()
        if getattr(self, "branch_steal_target_insn", False):
            self._branch_steal_target_insn()
        if getattr(self, "fill_jump_preceding_move", False):
            self._fill_jump_preceding_move()
        if getattr(self, "fill_branch_tested_li_void", False):
            self._fill_branch_tested_li_void()
        if self.sink_return_zero_into_flag_delay:
            self._sink_return_zero_into_flag_delay()
        if self.unfill_epilogue_delay_slot:
            self._unfill_epilogue_delay_slots()
        if self.reorder_fill_calls:
            self._reorder_fill_calls_and_epilogue()
        if self.spill_before_symbol_load:
            self._spill_before_symbol_load()

        res = []
        in_include_asm_hack = False
        for i, line in enumerate(self.lines):
            self.line_index = i

            if ".ent\t__maspsx_include_asm_hack" in line:
                in_include_asm_hack = True

            if in_include_asm_hack:
                if "# maspsx-keep" in line:
                    res += [line]
                else:
                    res += [f"# {line} # DEBUG: skipped due to include asm hack"]
                if ".end\t__maspsx_include_asm_hack" in line:
                    in_include_asm_hack = False
                continue

            if is_instruction(line) and self.skip_instructions > 0:
                self.skip_instructions -= 1
                res += [f"# {line}  # DEBUG: skipped"]
            else:
                res += self.process_line(line)

        for section, entries in [
            ("sbss", self.sbss_entries),
            ("bss", self.bss_entries),
        ]:
            for i, (symbol, size) in enumerate(entries.items()):
                if i == 0:
                    res.append(f".section .{section}")

                if self.use_comm_section and (
                    symbol in self.comm_symbols or self.use_comm_for_lcomm
                ):
                    # implicit alignment for COMMON
                    res.append(f"\t.comm {symbol},{size}")
                    continue

                if section == "sbss":
                    if size >= 8:
                        res.append("\t.align 3")
                    elif size >= 4:
                        res.append("\t.align 2")
                    elif size >= 2:
                        res.append("\t.align 1")

                # only mark bss symbols as global
                if section == "bss":
                    res.append(
                        f"\t.globl {symbol}",
                    )
                res.extend(
                    [
                        f"{symbol}:",
                        f"\t.space {size}",
                    ]
                )

        if self.sink_reg_sw_into_bnez:
            res = self._sink_reg_sw_into_bnez(res)
        if self.frame48_s5_after_s2:
            res = self._frame48_s5_after_s2(res)
        if self.addu_neg1_beq_delay:
            res = self._addu_neg1_beq_delay(res)
        if self.div_delay_reuse_v0:
            res = self._div_delay_reuse_v0(res)
        if self.drop_jump_to_epilogue:
            res = self._drop_jump_to_epilogue(res)
        if self.sltu_a0_lt_a2_dest_v1:
            res = self._sltu_a0_lt_a2_dest_v1(res)
        if self.dup_half_shift:
            res = self._dup_half_shift(res)
        if self.mult_mflo_to_multu:
            res = self._mult_mflo_to_multu(res)
        if self.mflo_before_mfhi:
            res = self._mflo_before_mfhi(res)
        if os.environ.get("MASPSX_INDEX_PAIR_JAL_SCHEDULE") == "1":
            res = self._index_pair_jal_schedule(res)
        return res

    def _index_pair_jal_schedule(self, res):
        """Reorder one index-pair argument block ahead of a jal.

        cc1 emits the byte-0 shift first, a load-delay nop after the
        second index byte, the stack argument in the jal delay, and a
        fall-through `-1` in `$v1`. Retail shifts byte 1 first, puts
        the byte-0 add in that nop's place, stores the stack argument
        before the two result loads, takes the format address in the
        jal delay, and jumps over a `$v0 = 0` to the status test.
        """
        def bare(line):
            return line.split("#", 1)[0].strip()

        real = []
        for idx, line in enumerate(res):
            s = bare(line)
            if s and not s.startswith(".") and not s.endswith(":"):
                real.append((idx, s))
        labels = {}
        for idx, line in enumerate(res):
            s = bare(line)
            if s.endswith(":"):
                labels[s[:-1]] = idx

        for n in range(len(real) - 16):
            seq = [real[n + k][1] for k in range(17)]
            if not (
                re.match(r"^sll\s*\$4,\$4,2$", seq[0])
                and re.match(r"^sll\s*\$2,\$2,2$", seq[1])
                and re.match(r"^addu\s*\$2,\$2,\$16$", seq[2])
                and re.match(r"^lw\s*\$3,0\(\$2\)$", seq[3])
                and re.match(r"^lbu\s*\$2,", seq[4])
                and seq[5] == "nop"
                and re.match(r"^sll\s*\$2,\$2,2$", seq[6])
                and re.match(r"^addu\s*\$2,\$2,\$19$", seq[7])
                and re.match(r"^addu\s*\$4,\$4,\$16$", seq[8])
                and re.match(r"^lw\s*\$6,0\(\$2\)$", seq[9])
                and re.match(r"^lw\s*\$7,0\(\$4\)$", seq[10])
                and re.match(r"^la\s*\$4,(\S+)$", seq[11])
                and re.match(r"^jal\s*(\S+)$", seq[12])
                and re.match(r"^sw\s*\$3,(-?\d+)\(\$sp\)$", seq[13])
                and re.match(r"^jal\s*(\S+)$", seq[14])
                and seq[15] == "nop"
                and re.match(r"^li\s*\$3,(-1|0xffffffff)$", seq[16])
            ):
                continue
            sym = re.match(r"^la\s*\$4,(\S+)$", seq[11]).group(1)
            call = re.match(r"^jal\s*(\S+)$", seq[12]).group(1)
            stack = re.match(r"^sw\s*\$3,(-?\d+)\(\$sp\)$", seq[13]).group(1)
            tail = re.match(r"^jal\s*(\S+)$", seq[14]).group(1)
            # The status test is the next real op and must be `bne $3`.
            if n + 17 >= len(real):
                continue
            bne = re.match(r"^bne\s*\$3,\$0,(\S+)$", real[n + 17][1])
            if not bne:
                continue
            dest = bne.group(1)
            tail_li = None
            if n + 18 < len(real) and re.match(
                r"^li\s*\$2,(-1|0xffffffff)$", real[n + 18][1]
            ):
                tail_li = real[n + 18][0]
            join = dest + "_join"
            first = real[n][0]
            last = tail_li if tail_li is not None else real[n + 17][0]
            label_line = None
            label_text = None
            for j in range(real[n + 16][0] + 1, real[n + 17][0]):
                if bare(res[j]).endswith(":"):
                    label_line = j
                    label_text = bare(res[j])
                    break
            if label_line is None:
                continue
            repl_lines = [
                "sll\t$2,$2,2",
                "addu\t$2,$2,$16",
                "sll\t$4,$4,2",
                "lw\t$3,0($2)",
                seq[4],
                "addu\t$4,$4,$16",
                "sll\t$2,$2,2",
                "addu\t$2,$2,$19",
                f"sw\t$3,{stack}($sp)",
                "lw\t$6,0($2)",
                "lw\t$7,0($4)",
                f"lui\t$4,%hi({sym})",
                f"jal\t{call}",
                f"addiu\t$4,$4,%lo({sym})",
                f"jal\t{tail}",
                "nop",
                f"j\t{join}",
                "li\t$2,-1",
                label_text,
                "addu\t$2,$zero,$zero",
                f"{join}:",
                f"bne\t$2,$zero,{dest}",
                "li\t$2,-1",
            ]
            out = []
            inserted = False
            for i, line in enumerate(res):
                if first <= i <= last:
                    if not inserted:
                        out.extend(repl_lines)
                        inserted = True
                    continue
                out.append(line)
            return out
        return res

    def _addu_neg1_beq_delay(self, res):
        """`li $r,-1` / `beq $s,$r,$L` / `addu $r,$s,$r` -> delay `addiu $r,$s,-1`."""
        def bare(line):
            return line.split("#", 1)[0].strip()

        def next_real(start):
            j = start
            while j < len(res):
                s = bare(res[j])
                if s and not s.startswith(".") and not s.endswith(":"):
                    return j
                j += 1
            return None

        i = 0
        while i < len(res):
            li = re.match(r"^li\s+(\$\w+),(-1|0xffffffff)$", bare(res[i]))
            if li:
                reg = li.group(1)
                j = next_real(i + 1)
                k = next_real(j + 1) if j is not None else None
                if j is not None and k is not None:
                    beq = re.match(r"^beq\s+(\$\w+),(\$\w+),(\S+)$", bare(res[j]))
                    addu = re.match(
                        r"^addu\s+(\$\w+),(\$\w+),(\$\w+)$", bare(res[k]))
                    if (beq and addu and beq.group(2) == reg
                            and addu.group(1) == reg
                            and {addu.group(2), addu.group(3)} == {beq.group(1), reg}
                            and beq.group(1) != reg):
                        res[k] = f"addiu\t{reg},{beq.group(1)},-1"
                        i = k + 1
                        continue
            i += 1
        return res

    def _div_delay_reuse_v0(self, res):
        """Rename the $a3 divisor window described on div_delay_reuse_v0."""
        def bare(line):
            return line.split("#", 1)[0].strip()

        real = []
        for idx, line in enumerate(res):
            s = bare(line)
            if not s or s.startswith(".") or s.endswith(":") or s == "nop":
                continue
            real.append((idx, s))

        # Registers are the retail names ($2=$v0, $3=$v1, $4=$a0, $5=$a1,
        # $6=$a2, $7=$a3). Labels are captured. Nops are skipped so a
        # load-delay debug nop between mfhi and the consumer does not hide
        # the window; those nops are left in place.
        pats = [
            r"^beq\s+\$2,\$0,(\S+)$",
            r"^addu\s+\$7,\$5,1$",
            r"^div\s+\$zero,\$3,\$7$",
            r"^bnez\s+\$7,(\S+)$",
            r"^break\s+0x7$",
            r"^addiu\s+\$at,\$zero,-1$",
            r"^bne\s+\$7,\$at,(\S+)$",
            r"^lui\s+\$at,0x8000$",
            r"^bne\s+\$3,\$at,(\S+)$",
            r"^break\s+0x6$",
            r"^mfhi\s+\$3$",
            r"^sll\s+\$4,\$3,16$",
            r"^j\s+(\S+)$",
            r"^slt\s+\$4,\$6,\$4$",
            r"^bgez\s+\$4,(\S+)$",
            r"^sll\s+\$5,\$5,16$",
            r"^div\s+\$zero,\$3,\$7$",
            r"^bnez\s+\$7,(\S+)$",
            r"^break\s+0x7$",
            r"^addiu\s+\$at,\$zero,-1$",
            r"^bne\s+\$7,\$at,(\S+)$",
            r"^lui\s+\$at,0x8000$",
            r"^bne\s+\$3,\$at,(\S+)$",
            r"^break\s+0x6$",
            r"^mfhi\s+\$2$",
            r"^addu\s+\$3,\$7,\$2$",
            r"^sll\s+\$4,\$3,16$",
            r"^slt\s+\$5,\$5,\$6$",
            r"^bne\s+\$5,\$0,(\S+)$",
            r"^slt\s+\$4,\$4,\$6$",
            r"^beq\s+\$4,\$0,(\S+)$",
        ]
        compiled = [re.compile(p) for p in pats]
        n = len(pats)
        i = 0
        while i + n <= len(real):
            groups = []
            ok = True
            for k, cre in enumerate(compiled):
                m = cre.match(real[i + k][1])
                if not m:
                    ok = False
                    break
                groups.append(m)
            # The negative-path compares and the join all target one label.
            if ok and not (groups[14].group(1) == groups[28].group(1) == groups[30].group(1)):
                ok = False
            if not ok:
                i += 1
                continue
            repl = {
                1: "addu\t$2,$5,1",
                2: "div\t$zero,$3,$2",
                3: f"bnez\t$2,{groups[3].group(1)}",
                6: f"bne\t$2,$at,{groups[6].group(1)}",
                13: "slt\t$2,$6,$4",
                16: "div\t$zero,$3,$2",
                17: f"bnez\t$2,{groups[17].group(1)}",
                20: f"bne\t$2,$at,{groups[20].group(1)}",
                24: "mfhi\t$3",
                25: "addu\t$2,$2,$3",
                26: "sll\t$4,$2,16",
                27: "slt\t$2,$5,$6",
                28: f"bne\t$2,$0,{groups[28].group(1)}",
                29: "slt\t$2,$4,$6",
                30: f"beq\t$2,$0,{groups[30].group(1)}",
            }
            for k, text in repl.items():
                res[real[i + k][0]] = text
            i += n
        return res

    def _drop_jump_to_epilogue(self, res):
        """Delete `j $L` when $L is the following `j $31` and the delay is `addu $v0,$0,$0`."""
        def bare(line):
            return line.split("#", 1)[0].strip()

        real = []
        for idx, line in enumerate(res):
            s = bare(line)
            if s and not s.startswith(".") and not s.endswith(":"):
                real.append((idx, s))
        drop = []
        for n, (idx, s) in enumerate(real):
            m = re.match(r"^j\s+(\$\w+)$", s)
            if not m or n + 2 >= len(real):
                continue
            label = m.group(1)
            delay = real[n + 1][1]
            epi = real[n + 2][1]
            if delay not in ("addu\t$2,$0,$zero", "addu\t$2,$zero,$zero", "addu $2,$0,$zero"):
                # bare() does not collapse the tab, so compare the stripped form.
                if not re.match(r"^addu\s+\$2,\$0,\$zero$", delay) and not re.match(
                    r"^addu\s+\$2,\$zero,\$zero$", delay
                ):
                    continue
            if not re.match(r"^j\s+\$31$", epi) and epi not in ("jr\t$31", "jr\t$ra"):
                if not re.match(r"^jr\s+\$(31|ra)$", epi):
                    continue
            # Only labels (including the target) may sit between the delay and the epilogue.
            saw = False
            blocked = False
            for line in res[real[n + 1][0] + 1:real[n + 2][0]]:
                t = bare(line)
                if not t:
                    continue
                if t.endswith(":") and t[:-1] == label:
                    saw = True
                    continue
                if t.endswith(":"):
                    continue
                blocked = True
                break
            if saw and not blocked:
                drop.append(idx)
        for idx in reversed(drop):
            del res[idx]
        return res

    def _sltu_a0_lt_a2_dest_v1(self, res):
        """Rename one `sltu $v0,$a0,$a2` / `bne $v0` pair to $v1."""
        def bare(line):
            return line.split("#", 1)[0].strip()

        real = []
        for idx, line in enumerate(res):
            s = bare(line)
            if s and not s.startswith(".") and not s.endswith(":"):
                real.append((idx, s))
        for n in range(len(real) - 4):
            if not re.match(r"^bne\s+\$2,\$0,\S+$", real[n][1]):
                continue
            if not re.match(r"^li\s+\$2,(1|0x1|0x00000001)$", real[n + 1][1]):
                continue
            if not re.match(r"^sltu\s+\$2,\$4,\$6$", real[n + 2][1]):
                continue
            bne = re.match(r"^bne\s+\$2,\$0,(\S+)$", real[n + 3][1])
            if not bne:
                continue
            if not re.match(r"^li\s+\$2,(-1|0xffffffff|-0x1)$", real[n + 4][1]):
                continue
            res[real[n + 2][0]] = "sltu\t$3,$4,$6"
            res[real[n + 3][0]] = f"bne\t$3,$0,{bne.group(1)}"
        return res

    def _dup_half_shift(self, res):
        """Rewrite the signed-char `-1 << 16` window described above."""
        def bare(line):
            return line.split("#", 1)[0].strip()

        i = 0
        while i < len(res):
            if not re.match(r"^sll\s+\$2,\$2,24$", bare(res[i])):
                i += 1
                continue
            idxs = [i]
            j = i + 1
            while len(idxs) < 6 and j < len(res):
                s = bare(res[j])
                if s and not s.startswith(".") and not s.endswith(":"):
                    idxs.append(j)
                j += 1
            if len(idxs) < 6:
                break
            beq = re.match(r"^beq\s+\$2,\$0,(\$\w+)$", bare(res[idxs[1]]))
            li_ok = re.match(r"^li\s+\$2,-65536$", bare(res[idxs[2]]))
            lhu = re.match(r"^lhu\s+\$2,\S+$", bare(res[idxs[3]]))
            nop_ok = bare(res[idxs[4]]) == "nop"
            sll_ok = re.match(r"^sll\s+\$2,\$2,16$", bare(res[idxs[5]]))
            if not (beq and li_ok and lhu and nop_ok and sll_ok):
                i += 1
                continue
            label = beq.group(1)
            saw = False
            k = idxs[5] + 1
            while k < len(res):
                s = bare(res[k])
                if not s:
                    k += 1
                    continue
                if s.endswith(":"):
                    if s[:-1] == label:
                        saw = True
                    k += 1
                    continue
                break
            if not saw:
                i += 1
                continue
            taken = label + "b"
            lhu_line = res[idxs[3]]
            res[idxs[0]] = f"beq\t$3,$0,{taken}"
            res[idxs[1]] = "addiu\t$2,$zero,-1"
            res[idxs[2]] = lhu_line
            res[idxs[3]] = f"j\t{label}"
            res[idxs[4]] = "sll\t$2,$2,16"
            res.insert(idxs[5], f"{taken}:")
            i = idxs[5] + 2
        return res

    def _mflo_before_mfhi(self, res):
        """`mfhi $H / mflo $L / addu $LO,$L,$zero / addu $HI,$H,$zero`.

        Becomes `mflo $LO / mfhi $HI`. Two nops are inserted only when the
        next real instruction is `mult` (the hi/lo hazard before another
        multiply). A following `addu` keeps no nops.

        A shift copy of the same pair,
        `srl $LO,$L,N / sll $HI,$H,M`, is folded into those half reads.
        """
        def bare(line):
            return line.split("#", 1)[0].strip()

        real = []
        for idx, line in enumerate(res):
            s = bare(line)
            if s and not s.startswith(".") and not s.endswith(":") and s != "nop":
                real.append((idx, s))
        repl = {}
        drop = set()
        consumed = set()
        for n in range(len(real) - 4):
            if n in consumed:
                continue
            i_mult, s_mult = real[n]
            if not re.match(r"^mult\s+\$\w+,\$\w+$", s_mult):
                continue
            mh = re.match(r"^mfhi\s+(\$\w+)$", real[n + 1][1])
            ml = re.match(r"^mflo\s+(\$\w+)$", real[n + 2][1])
            if not mh or not ml:
                continue
            h, l = mh.group(1), ml.group(1)
            mv_lo = re.match(
                r"^addu\s+(\$\w+)," + re.escape(l) + r",\$zero$", real[n + 3][1]
            )
            mv_hi = re.match(
                r"^addu\s+(\$\w+)," + re.escape(h) + r",\$zero$", real[n + 4][1]
            )
            if not mv_lo or not mv_hi:
                continue
            nxt = real[n + 5][1] if n + 5 < len(real) else ""
            repl[real[n + 1][0]] = f"mflo\t{mv_lo.group(1)}"
            repl[real[n + 2][0]] = f"mfhi\t{mv_hi.group(1)}"
            if nxt.startswith("mult"):
                repl[real[n + 3][0]] = "nop"
                repl[real[n + 4][0]] = "nop"
            else:
                drop.add(real[n + 3][0])
                drop.add(real[n + 4][0])
            consumed.update(range(n, n + 5))
        # `mfhi $H / mflo $L / srl $LO,$L,N / sll $HI,$H,M` folds the copy
        # into the half reads: `mflo $LO / mfhi $HI / srl $LO,$LO,N / sll $HI,$HI,M`.
        for n in range(len(real) - 4):
            if n in consumed or (n + 1) in consumed:
                continue
            i_mult, s_mult = real[n]
            if not re.match(r"^mult\s+\$\w+,\$\w+$", s_mult):
                continue
            mh = re.match(r"^mfhi\s+(\$\w+)$", real[n + 1][1])
            ml = re.match(r"^mflo\s+(\$\w+)$", real[n + 2][1])
            if not mh or not ml:
                continue
            h, l = mh.group(1), ml.group(1)
            srl_m = re.match(
                r"^srl\s+(\$\w+)," + re.escape(l) + r",((?:0x)?[0-9A-Fa-f]+)$",
                real[n + 3][1],
            )
            sll_m = re.match(
                r"^sll\s+(\$\w+)," + re.escape(h) + r",((?:0x)?[0-9A-Fa-f]+)$",
                real[n + 4][1],
            )
            if not srl_m or not sll_m:
                continue
            lo_dest, hi_dest = srl_m.group(1), sll_m.group(1)
            if lo_dest == l or hi_dest == h or lo_dest == hi_dest:
                continue
            repl[real[n + 1][0]] = f"mflo\t{lo_dest}"
            repl[real[n + 2][0]] = f"mfhi\t{hi_dest}"
            repl[real[n + 3][0]] = f"srl\t{lo_dest},{lo_dest},{srl_m.group(2)}"
            repl[real[n + 4][0]] = f"sll\t{hi_dest},{hi_dest},{sll_m.group(2)}"
            consumed.update(range(n, n + 5))
        out = []
        for i, line in enumerate(res):
            if i in drop:
                continue
            out.append(repl.get(i, line))
        return out

    def _hoist_block_li(self, res):
        """K4: re-place the constants of a repeated-reload store run.

        A region is a maximal run with no label, directive, branch, jump
        or call. Its group loads are the lines textually equal to the
        first absolute `lw $x,SYM` in it that is directly followed by an
        `li $r,imm` (comment-only lines are skipped). Its constants are
        the `li` lines directly following a group load. With at least two
        constants and two group loads, constants 0..n-3 move to just
        before load 1, constant n-2 to just after load 1 and constant n-1
        to just after load 2. Every move must go strictly earlier and
        cross no line (other than another moved constant) naming the
        constant's register; otherwise the region is left unchanged.
        """
        def bare(line):
            return line.split("#", 1)[0].strip()

        ctl_re = re.compile(r"^(b\w*|j|jal|jr|jalr|syscall|break)\b")
        li_re = re.compile(r"^li\s+(\$\w+),-?(0x[0-9a-fA-F]+|\d+)$")
        lw_re = re.compile(r"^lw\s+(\$\w+),([A-Za-z_.][\w.$]*)$")
        allowed = self.hoist_block_li

        def boundary(s):
            return s.startswith(".") or s.endswith(":") or bool(ctl_re.match(s))

        out = list(res)
        regions = []
        i = 0
        while i < len(out):
            s = bare(out[i])
            if s and boundary(s):
                i += 1
                continue
            j = i
            while j < len(out):
                t = bare(out[j])
                if t and boundary(t):
                    break
                j += 1
            regions.append((i, j))
            i = j

        for start, end in reversed(regions):
            real = [k for k in range(start, end) if bare(out[k])]
            nxt = {real[a]: real[a + 1] for a in range(len(real) - 1)}
            load_text = None
            for k in real:
                m = lw_re.match(bare(out[k]))
                if not m:
                    continue
                if allowed is not True and m.group(2) not in allowed:
                    continue
                f = nxt.get(k)
                if f is not None and li_re.match(bare(out[f])):
                    load_text = bare(out[k])
                    break
            if load_text is None:
                continue
            loads = [k for k in real if bare(out[k]) == load_text]
            consts = []
            for k in loads:
                f = nxt.get(k)
                if f is None:
                    continue
                m = li_re.match(bare(out[f]))
                if m:
                    consts.append((f, m.group(1)))
            n = len(consts)
            if n < 2 or len(loads) < 2:
                continue
            # insert position: before line index `tgt`
            tail1 = self.hoist_block_li_tail1
            if tail1 and any(
                re.search(r"(?<![\w.$])" + re.escape(t) + r"(?![\w.$])", bare(out[k]))
                for k in real
                for t in tail1
            ):
                targets = [loads[0]] * (n - 1) + [loads[0] + 1]
            else:
                targets = [loads[0]] * (n - 2) + [loads[0] + 1, loads[1] + 1]
            moved = {pos for pos, _ in consts}
            ok = True
            for (pos, reg), tgt in zip(consts, targets):
                if tgt > pos:
                    ok = False
                    break
                reg_re = re.compile(re.escape(reg) + r"(?!\w)")
                for k in range(tgt, pos):
                    s = bare(out[k])
                    if k not in moved and s and reg_re.search(s):
                        ok = False
                        break
                if not ok:
                    break
            if not ok or all(t == p for (p, _), t in zip(consts, targets)):
                continue
            inserts = {}
            for (pos, _), tgt in zip(consts, targets):
                inserts.setdefault(tgt, []).append(out[pos])
            new = []
            for k in range(start, end):
                new.extend(inserts.get(k, []))
                if k not in moved:
                    new.append(out[k])
            out[start:end] = new
        return out

    def _narrow_shifted_word_load(self, res):
        """`lw $d,SYM[+off][($b)]` ... `sra $d,$d,N` -> `lh $d,SYM+off+2` / `sra N-16`.

        The window between the load and the shift must be straight-line
        (no label, branch, jump or directive) and must neither read nor
        write $d. N == 16 drops the shift.
        """
        def bare(line):
            return line.split("#", 1)[0].strip()

        load_re = re.compile(
            r"^lw\s+(\$\w+),([A-Za-z_.$][\w.$]*)([+-]\d+)?(\(\$\w+\))?$"
        )
        # K3b: register base with a decimal offset, `lw $d,N($b)`.
        reg_load_re = re.compile(r"^lw\s+(\$\w+),(-?\d+)(\(\$\w+\))$")
        ctl_re = re.compile(r"^(b\w*|j|jal|jr|jalr|syscall|break)\b")
        out = list(res)
        drop = set()
        for i, line in enumerate(out):
            m = load_re.match(bare(line)) if self.narrow_shifted_word_load else None
            if m:
                d, sym, off, base = m.group(1), m.group(2), m.group(3), m.group(4) or ""
                if sym.startswith("$"):
                    continue
            else:
                rm = (reg_load_re.match(bare(line))
                      if self.narrow_shifted_reg_load else None)
                if not rm:
                    continue
                d, sym, off, base = rm.group(1), None, rm.group(2), rm.group(3)
            reg_re = re.compile(re.escape(d) + r"(?!\w)")
            k = i + 1
            hit = None
            while k < len(out):
                s = bare(out[k])
                if not s:
                    k += 1
                    continue
                if s.startswith(".") or s.endswith(":") or ctl_re.match(s):
                    break
                sm = re.match(r"^sra\s+(\$\w+),(\$\w+),(\d+)$", s)
                if sm and sm.group(1) == d and sm.group(2) == d:
                    hit = (k, int(sm.group(3)))
                    break
                if reg_re.search(s):
                    break
                k += 1
            if hit is None or not 16 <= hit[1] <= 31:
                continue
            k, n = hit
            disp = (int(off) if off else 0) + 2
            if sym is None:
                out[i] = f"lh\t{d},{disp}{base}"
            else:
                sign = "+" if disp >= 0 else "-"
                out[i] = f"lh\t{d},{sym}{sign}{abs(disp)}{base}"
            if n == 16:
                drop.add(k)
            else:
                out[k] = f"sra\t{d},{d},{n - 16}"
        return [x for j, x in enumerate(out) if j not in drop]

    def _mult_mflo_to_multu(self, res):
        """`mult $a,$b` / `mflo` -> `multu`. Does not touch a following `mfhi`."""
        def bare(line):
            return line.split("#", 1)[0].strip()

        real = []
        for idx, line in enumerate(res):
            s = bare(line)
            if s and not s.startswith(".") and not s.endswith(":") and s != "nop":
                real.append((idx, s))
        for n, (idx, s) in enumerate(real[:-1]):
            m = re.match(r"^mult\s+(\$\w+),(\$\w+)$", s)
            if not m:
                continue
            if re.match(r"^mflo\s+\$\w+$", real[n + 1][1]):
                res[idx] = f"multu\t{m.group(1)},{m.group(2)}"
        return res

    def get_next_instruction(
        self, skip=0, ignore_nop=False, ignore_set=False, ignore_label=False
    ):
        i = self.line_index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if is_instruction(
                line,
                ignore_nop=ignore_nop,
                ignore_set=ignore_set,
                ignore_label=ignore_label,
            ):
                if skip == 0:
                    return line
                skip -= 1
            i += 1

        return ""  # warn user?

    def _symbol_at_temp_applies(self, operand: str) -> bool:
        # LOCAL PATCH helper for patch 5: True when the 3-word $at+%lo form
        # should rewrite this indexed symbolic operand. A None allow-list
        # means every symbol (MASPSX_SYMBOL_AT_TEMP=1 / ctor True).
        if not self.symbol_at_temp:
            return False
        if self.symbol_at_temp_only is None:
            return True
        return operand in self.symbol_at_temp_only

    def _line_after_beq_is_label(self) -> bool:
        # The instruction after the following `beq` is a label, so the
        # delay slot is empty and a preceding store can fill it.
        i = self.line_index + 1
        saw_beq = False
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line.startswith("#"):
                i += 1
                continue
            if not saw_beq:
                if line.startswith("beq\t"):
                    saw_beq = True
                    i += 1
                    continue
                return False
            return line.endswith(":")
        return False

    def _next_line_is_beqz(self):
        # Next non-blank, non-comment line is `beq $r,$0,$Label` (or
        # `$zero`). Returns that line, or None. Does not skip `.set` or
        # labels: those mean cc1 already owns the slot.
        i = self.line_index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line.startswith("#"):
                i += 1
                continue
            if re.match(r"^beq\t\$[0-9a-z]+,\$(?:0|zero),\$L\d+$", line):
                return line
            return None
        return None

    def _next_line_is_return_jump(self) -> bool:
        # LOCAL PATCH helper: True when the next non-blank, non-comment input
        # line is exactly `j $31`. Deliberately does NOT skip `.set` or label
        # lines: a label between the store and the jump means the store is
        # conditional, and a cc1 `.set noreorder` block means cc1 already
        # scheduled the jump itself — filling either slot would be wrong.
        i = self.line_index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line.startswith("#"):
                i += 1
                continue
            op, *rest = line.split()
            return op == "j" and rest == ["$31"]
        return False

    # --- LOCAL PATCH 6: fill_branch_delay_slot ---------------------------
    #
    # Everything below refuses the fill (returns None / False) whenever it
    # cannot fully account for the code it is looking at.  Refusing restores
    # the stock `nop`, so every bail-out is the pre-patch behaviour.

    def _branch_fill_labels(self) -> dict:
        if self._branch_fill_label_map is None:
            self._branch_fill_label_map = {
                line[:-1]: i
                for i, line in enumerate(self.lines)
                if is_label(line)
            }
        return self._branch_fill_label_map

    def _branch_fill_reorder_flags(self) -> List[bool]:
        """Per-input-line reorder state, mirroring process_line's tracking.

        Needed because a branch inside a cc1 `.set noreorder` block owns the
        next line as its delay slot, while a reorder-mode branch does not.
        """
        if self._branch_fill_reorder is None:
            flags = []
            state = True
            for line in self.lines:
                flags.append(state)
                if line.startswith(".set\t"):
                    if line.endswith("\tnoreorder"):
                        state = False
                    elif line.endswith("\treorder"):
                        state = True
            self._branch_fill_reorder = flags
        return self._branch_fill_reorder

    @staticmethod
    def _branch_fill_operands(line: str) -> List[str]:
        line = strip_comments(line)
        parts = line.split(None, 1)
        if len(parts) < 2:
            return []
        return [p.strip() for p in parts[1].split(",")]

    def _branch_fill_next_index(self, index: int):
        """Index of the next line that is a real instruction, or None.

        Blank lines, comments and `.set macro/nomacro/volatile/novolatile`
        noise are stepped over; a label or any other directive returns None so
        the caller refuses (we must never reason across a basic-block edge we
        did not model).
        """
        i = index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "":
                i += 1
                continue
            if line.startswith("#"):
                if line in ("#APP", "#NO_APP"):
                    return None
                i += 1
                continue
            if line in (
                ".set\tmacro",
                ".set\tnomacro",
                ".set\tvolatile",
                ".set\tnovolatile",
            ):
                i += 1
                continue
            if line.startswith(".") or is_label(line) or line.endswith(":"):
                return None
            return i
        return None

    def _branch_fill_delay_slot_index(self, index: int):
        """Index of the delay-slot instruction of the branch/jump at `index`.

        Strict on purpose: only blank lines and the `.set macro/nomacro` pair
        cc1 wraps a scheduled branch in may intervene.  A comment (cc1's own
        `#nop` placeholder included), a label or any other directive means we
        are not looking at a plain `branch` / `slot` pair, so refuse.
        """
        i = index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line in (".set\tmacro", ".set\tnomacro"):
                i += 1
                continue
            if line.startswith("#") or line.startswith(".") or line.endswith(":"):
                return None
            return i
        return None

    def _branch_fill_consumer_index(self, index: int):
        """Index of the instruction that follows `index`, skipping the cc1
        `.set` noise that wraps a scheduled branch.  A label or any other
        directive returns None (the value could then be consumed elsewhere)."""
        i = index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "":
                i += 1
                continue
            if line.startswith("#"):
                if line in ("#APP", "#NO_APP"):
                    return None
                i += 1
                continue
            if line.startswith(".set\t"):
                i += 1
                continue
            if line.startswith(".") or is_label(line) or line.endswith(":"):
                return None
            return i
        return None

    def _branch_fill_effect(self, line: str, reg: int, extended: bool = False):
        """Effect of a single instruction on `reg`: "read", "write", "none" or
        "bail" (mnemonic or operand shape not fully understood)."""
        line = strip_comments(line)
        if ";" in line:
            return "bail"
        if line == "" or line == "nop":
            return "none"
        parts = line.split(None, 1)
        op = parts[0]
        if extended and op in ("mult", "multu", "mfhi", "mflo"):
            # LOCAL PATCH 19 walk (HS7): cc1's 2-operand mult/multu reads two
            # GPRs and writes only hi/lo; mfhi/mflo write one GPR.
            ops = self._branch_fill_operands(line)
            nums = [gpr_number(t) for t in ops]
            if any(n is None for n in nums):
                return "bail"
            if op in ("mult", "multu"):
                if len(nums) != 2:
                    return "bail"
                return "read" if reg in nums else "none"
            if len(nums) != 1:
                return "bail"
            return "write" if nums[0] == reg else "none"
        if op in live_write_none:
            writes = None
        elif op in live_write_first:
            writes = "first"
        else:
            return "bail"
        operands = self._branch_fill_operands(line)
        regs = []
        for tok in reg_operand_re.findall(" ".join(operands)):
            num = gpr_number(tok)
            if num is None:
                return "bail"
            regs.append(num)
        if writes == "first":
            first = operands[0] if operands else ""
            dest = gpr_number(first)
            if dest is None:
                return "bail"
            # every register after the destination is a pure read
            if reg in regs[1:]:
                return "read"
            return "write" if dest == reg else "none"
        return "read" if reg in regs else "none"

    def _branch_fill_reg_dead_at(
        self, label: str, reg: int, extended: bool = False, start_index=None,
        void_return: bool = False,
    ) -> bool:
        """True when `reg` is provably dead at `label`.

        A conservative forward reachability walk: every path leaving `label`
        must overwrite `reg` before any instruction reads it, and every
        construct the walk cannot model (a call, an indirect or computed jump,
        inline asm, a reorder-mode conditional branch, an unknown mnemonic,
        the end of the function, a blown step budget) makes it answer False.
        """
        labels = self._branch_fill_labels()
        if start_index is None:
            if label not in labels:
                return False
            start_index = labels[label]
        reorder = self._branch_fill_reorder_flags()
        caller_saved = 2 <= reg <= 15 or reg in (24, 25)

        stack = [start_index]
        seen = set()
        steps = 0
        budget = 4096

        while stack:
            i = stack.pop()
            killed = False
            while True:
                steps += 1
                if steps > budget or i is None or i >= len(self.lines):
                    return False
                if i in seen:
                    # this program point has already been proven safe on an
                    # earlier path (an unsafe one would have returned False)
                    killed = True
                    break
                line = self.lines[i]
                if line == "":
                    i += 1
                    continue
                if line.startswith("#"):
                    if line in ("#APP", "#NO_APP"):
                        return False
                    i += 1
                    continue
                if is_label(line):
                    i += 1
                    continue
                if line.startswith("."):
                    if line.startswith(".set\t") or line.startswith(".loc\t"):
                        i += 1
                        continue
                    # .end / .ent / section or data directives: stop guessing
                    return False
                if line.endswith(":"):
                    # a non-cc1 label (function entry, asm label)
                    return False

                seen.add(i)
                op = strip_comments(line).split(None, 1)[0]

                if op in cond_branch_mnemonics:
                    if reorder[i] and extended:
                        # LOCAL PATCH 19 walk (HS5): a reorder-mode branch
                        # reads its operands, then either edge continues. Its
                        # slot is maspsx's nop or a fill that MOVES the next
                        # fall-through instruction there; that instruction is
                        # walked on the fall-through, so any read of `reg` it
                        # makes is still seen, and a write it makes on the
                        # taken edge only kills earlier. Sound to skip.
                        operands = self._branch_fill_operands(line)
                        if not operands or operands[-1] not in labels:
                            return False
                        if any(
                            gpr_number(t) == reg
                            for t in reg_operand_re.findall(
                                " ".join(operands[:-1])
                            )
                        ):
                            return False
                        stack.append(labels[operands[-1]])
                        i += 1
                        continue
                    if reorder[i]:
                        # no textual delay slot, and with this very patch on
                        # the slot's contents are not knowable here
                        return False
                    operands = self._branch_fill_operands(line)
                    if not operands:
                        return False
                    target = operands[-1]
                    slot = self._branch_fill_delay_slot_index(i)
                    if slot is None or target not in labels:
                        return False
                    effect = self._branch_fill_effect(self.lines[slot], reg, extended)
                    if effect in ("bail", "read"):
                        return False
                    if effect == "write":
                        killed = True
                        break
                    stack.append(labels[target])
                    i = slot + 1
                    continue

                if op in ("j", "jr"):
                    operands = self._branch_fill_operands(line)
                    if len(operands) != 1:
                        return False
                    target = operands[0]
                    if reorder[i]:
                        if target in ("$31", "$ra"):
                            if void_return and caller_saved:
                                killed = True
                                break
                            # reg is live out of the function at a return we
                            # cannot prove kills it
                            return False
                        if target not in labels:
                            return False
                        i = labels[target]
                        continue
                    slot = self._branch_fill_delay_slot_index(i)
                    if slot is None:
                        return False
                    effect = self._branch_fill_effect(self.lines[slot], reg, extended)
                    if effect in ("bail", "read"):
                        return False
                    if effect == "write":
                        killed = True
                        break
                    if target in ("$31", "$ra") and void_return and caller_saved:
                        killed = True
                        break
                    if target in ("$31", "$ra") or target not in labels:
                        return False
                    i = labels[target]
                    continue

                if op in ("jal", "jalr"):
                    # LOCAL PATCH 19 walk (HS6): a call preserves a
                    # callee-saved register ($16-$23, $30) and does not
                    # consume its value; argument setup is separate code the
                    # walk sees. A register-indirect call through `reg` reads
                    # it. Caller-saved registers stay refused.
                    if not extended or not (16 <= reg <= 23 or reg == 30):
                        return False
                    operands = self._branch_fill_operands(line)
                    if any(gpr_number(t) == reg for t in operands):
                        return False
                    if not reorder[i]:
                        slot = self._branch_fill_delay_slot_index(i)
                        if slot is None:
                            return False
                        effect = self._branch_fill_effect(self.lines[slot], reg, extended)
                        if effect in ("bail", "read"):
                            return False
                        if effect == "write":
                            killed = True
                            break
                        i = slot + 1
                        continue
                    i += 1
                    continue

                effect = self._branch_fill_effect(line, reg, extended)
                if effect in ("bail", "read"):
                    return False
                if effect == "write":
                    killed = True
                    break
                i += 1

            if not killed:
                return False
        return True

    def _branch_fill_split_li(self, index: int, cand: str, target: str):
        # LOCAL PATCH 19 (fill_branch_split_li). `cand` (line `index`, the
        # line right after the branch) is `li $r,C` with C's low half zero --
        # the high word cc1 splits off a 32-bit constant -- and the next
        # instruction is `ori $r,$r,imm` completing it. Retail (ASPSX) puts
        # that lui in the branch's delay slot. HS1 C must need exactly one
        # `lui` (low 16 bits zero, not a 16-bit literal); HS2 the next
        # non-blank line must be the completing `ori $r,$r,imm` (no label in
        # between: the pair must be one straight-line materialization);
        # HS3 $r must not be ABI-fixed/$at; HS4 SAFETY: $r provably dead at
        # the branch target (patch 6's liveness walk), since the slot runs on
        # the taken edge too. Returns the explicit `lui` line or None.
        args = self._branch_fill_operands(cand)
        if len(args) != 2 or not is_number(args[1]):
            return None
        dest = args[0]
        if dest in branch_fill_forbidden_dest:
            return None
        dest_num = gpr_number(dest)
        if dest_num is None or dest_num in (0, 1) or dest_num >= 26:
            return None
        value = int(args[1], 0) & 0xFFFFFFFF
        if value & 0xFFFF or value <= 0xFFFF:  # HS1
            return None
        nxt = self._branch_fill_next_index(index)
        if nxt is None or any(
            self.lines[j] != "" for j in range(index + 1, nxt)
        ):
            return None
        ori = strip_comments(self.lines[nxt])
        if ";" in ori or ori.split(None, 1)[0] != "ori":  # HS2
            return None
        oargs = self._branch_fill_operands(ori)
        if (
            len(oargs) != 3
            or gpr_number(oargs[0]) != dest_num
            or gpr_number(oargs[1]) != dest_num
            or not is_number(oargs[2])
        ):
            return None
        if not self._branch_fill_reg_dead_at(target, dest_num, extended=True):  # HS4
            return None
        return f"lui\t{dest},0x{value >> 16:x}"

    def _branch_fill_candidate(self, branch_line: str):
        """The line to emit in this reorder-mode conditional branch's delay
        slot, or None to keep the stock `nop`."""
        if not (self.fill_branch_delay_slot or self.fill_branch_split_li):
            return None
        branch_line = strip_comments(branch_line)
        op = branch_line.split(None, 1)[0]
        if op not in cond_branch_mnemonics:
            return None
        operands = self._branch_fill_operands(branch_line)
        want = 3 if op in cond_branch_two_reg else 2
        if len(operands) != want:
            return None
        target = operands[-1]
        if target not in self._branch_fill_labels():
            return None

        index = self._branch_fill_next_index(self.line_index)
        if index is None:
            return None
        # the candidate must be the IMMEDIATELY following line; only blank
        # lines may separate it, so `skip_instructions` lands on it exactly
        if any(self.lines[j] != "" for j in range(self.line_index + 1, index)):
            return None

        cand = self.lines[index]
        if ";" in cand:
            return None
        cand_stripped = strip_comments(cand)
        cop = cand_stripped.split(None, 1)[0]
        if self.fill_branch_split_li and cop == "li":
            split = self._branch_fill_split_li(index, cand_stripped, target)
            if split is not None:
                return split
        if not self.fill_branch_delay_slot:
            return None
        spec = branch_fill_candidates.get(cop)
        if spec is None:
            return None
        nargs, imm_kind = spec
        cargs = self._branch_fill_operands(cand_stripped)
        if len(cargs) != nargs:
            return None
        dest = cargs[0]
        if dest in branch_fill_forbidden_dest:
            return None
        dest_num = gpr_number(dest)
        if dest_num is None or dest_num in (0, 1) or dest_num >= 26:
            return None
        # operands other than an allowed trailing literal must be plain GPRs
        for arg in cargs[1:-1] if nargs == 3 else []:
            if gpr_number(arg) is None:
                return None
        if nargs == 2 and imm_kind is None and gpr_number(cargs[1]) is None:
            return None

        last = cargs[-1]
        if imm_kind == "li" and gpr_number(last) is not None:
            return None
        if gpr_number(last) is None:
            # a literal: it must be in range for a one-word encoding, so GNU
            # as / maspsx can never reach for `$at`
            if not is_number(last):
                return None
            value = int(last, 0)
            if imm_kind == "s16":
                if not -0x8000 <= value <= 0x7FFF:
                    return None
                if cop == "subu" and not -0x7FFF <= value <= 0x8000:
                    return None
            elif imm_kind == "u16":
                if not 0 <= value <= 0xFFFF:
                    return None
            elif imm_kind == "u15":
                if not 0 <= value <= 0x7FFF:
                    return None
            elif imm_kind == "sh":
                if not 0 <= value <= 31:
                    return None
            elif imm_kind == "li":
                # `li` is one word only for a 16-bit-representable value; the
                # expand_li path must also produce exactly one instruction
                if not -0x8000 <= value <= 0xFFFF:
                    return None
                if self.expand_li and len(expand_load_immediate(cand_stripped)) != 1:
                    return None
            else:
                return None
        elif imm_kind == "sh":
            # sllv/srlv/srav are spelled out separately; a register shift
            # count under sll/srl/sra would be a different instruction
            return None

        # FIDELITY GATE (as opposed to the safety gate below).  Liveness alone
        # licenses more fills than retail actually performs: of the 70
        # same-shape sites in the repo's LINK_EXACT leaves, retail keeps the
        # `nop` at all 70 even though 2 of them (func_80012E7C `sll $2,$3,2`,
        # func_8006A2E8 `move $2,$5`) are provably dead-at-target and would be
        # legal to fill.  The ROM-evidenced family is specifically a COMPARE
        # that the immediately following conditional branch consumes -- the
        # compare-chain shape of func_80043474 -- so require exactly that.
        if cop not in ("slt", "slti", "sltu", "sltiu"):
            return None
        consumer = self._branch_fill_consumer_index(index)
        if consumer is None:
            return None
        consumer_line = strip_comments(self.lines[consumer])
        cons_op = consumer_line.split(None, 1)[0]
        if cons_op not in cond_branch_mnemonics:
            return None
        cons_regs = [
            gpr_number(tok)
            for tok in reg_operand_re.findall(consumer_line)
        ]
        if dest_num not in cons_regs:
            return None

        # SAFETY GATE: the fill executes on the taken edge too, so the
        # destination must be provably dead there.
        if not self._branch_fill_reg_dead_at(target, dest_num):
            return None

        # emit exactly what the normal path would have emitted for this line
        if cop == "move":
            return expand_move(cand_stripped)
        if cop == "li" and self.expand_li:
            return expand_load_immediate(cand_stripped)[0]
        return cand

    def _next_line_is_plain_call(self):
        # LOCAL PATCH helper (patch 8): inspect the next non-blank,
        # non-comment input line and return `(line, target_register)` when it
        # is a plain jump/call whose delay slot may be filled with a macro's
        # second word, else None. `target_register` is None for a direct
        # `jal <symbol>` / `j <label>` and e.g. "$2" for the register-indirect
        # forms, which cc1 spells `jal $2` (jalr) and `j $2` (jr).
        #
        # Like _next_line_is_return_jump this deliberately does NOT skip
        # `.set` or label lines (hazard H5): a label means the jump is a
        # branch target and the macro is not guaranteed to have run, and a
        # cc1 `.set noreorder` block means cc1 already owns that slot.
        #
        # `j $31` (the function return) is always refused: that slot belongs
        # to patches 1 and 3, whose ROM evidence covers it, and stealing it
        # here would change their behaviour when both knobs are on.
        i = self.line_index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line.startswith("#"):
                i += 1
                continue
            if ";" in line:  # H7: compound macro line
                return None
            parts = line.split()
            if len(parts) != 2:
                return None
            op, target = parts
            if op not in ("j", "jal"):  # H6: not a conditional branch
                return None
            if not target.startswith("$"):
                return (line, None)
            if canon_reg(target) == 31:  # patch 1 / patch 3 own this slot
                return None
            # H3: a register-indirect jump reads its target register at jump
            # time, i.e. BEFORE the delay slot runs. The caller must check
            # that the moved word does not write that register.
            return (line, target)
        return None

    def _next_line_is_label_jump(self):
        # LOCAL PATCH helper (patch 11): the next non-blank, non-comment
        # input line when it is a reorder-mode plain `j <label>` (HJ1), else
        # None. Labels and `.set` lines are NOT skipped (HJ5), exactly like
        # patch 8's lookahead.
        if not self.is_reorder:  # HJ5: cc1's noreorder block owns the slot
            return None
        call = self._next_line_is_plain_call()
        if call is None:
            return None
        # cc1 spells labels `$L<n>`, which the shared lookahead reports as a
        # `$`-target; only a real register (jr/jalr) is refused here.
        if call[1] is not None and isinstance(canon_reg(call[1]), int):
            return None  # HJ1: no jr/jalr (and `j $31` was refused upstream)
        if call[0].split()[0] != "j":  # HJ1: `jal` writes $31; not this knob
            return None
        return call[0]

    def _next_line_is_cond_branch(self):
        # LOCAL PATCH helper (patch 10): inspect the next non-blank,
        # non-comment input line and return `(line, read_registers)` when it
        # is a plain reorder-mode conditional branch whose delay slot may be
        # filled with a macro's second word, else None. `read_registers` is
        # the set of canonical register numbers the branch tests (HB1).
        #
        # Same lookahead discipline as _next_line_is_plain_call: labels and
        # `.set` lines are NOT skipped (HB4), compound lines refused (HB5).
        if not self.is_reorder:  # HB4: no maspsx slot inside noreorder
            return None
        i = self.line_index + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line.startswith("#"):
                i += 1
                continue
            if ";" in line:  # HB5
                return None
            parts = line.split()
            if len(parts) != 2:
                return None
            op, operands = parts
            if op not in branch_mnemonics:  # HB6 (also excludes bltzal/bgezal)
                return None
            fields = operands.split(",")
            if len(fields) < 2 or len(fields) > 3:
                return None
            target = fields[-1]
            regs = fields[:-1]
            # cc1 labels are spelled `$L<n>`; a register target (`beq ..,$2`)
            # is not a branch we understand.
            if isinstance(canon_reg(target), int):
                return None
            if not all(r.startswith("$") for r in regs):
                return None
            reads = set()
            for r in regs:
                c = canon_reg(r)
                if not isinstance(c, int):
                    return None  # unknown register spelling: refuse
                reads.add(c)
            return (line, reads)
        return None

    def _uses_gp(self, line: str) -> bool:
        if self.sdata_limit == 0:
            return False

        line = strip_comments(line)
        if uses_at(line):
            op, *rest = line.split("\t")
            if op in load_mnemonics or op in store_mnemonics:
                (
                    _,
                    _,
                    operand,
                    _,
                    _,
                ) = parse_load_or_store(" ".join(rest))

                if operand.count("+") == 1:
                    symbol, _ = operand.split("+")
                    gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols
                else:
                    symbol = operand
                    gp_allowed = True

                if gp_allowed and (
                    symbol in self.sbss_entries
                    or symbol in self.sdata_entries
                    # LOCAL PATCH 7: sized externs resolve gp-relative too.
                    # Only the hazard decision consults this; the rewrite to
                    # an explicit %gp_rel operand stays GNU as's job (it
                    # already honours the sized `.extern`), so flag-off text
                    # is unchanged apart from the mandatory nop.
                    or symbol in self.extern_entries
                ):
                    return True

        return False

    def _handle_nop_before_next_instruction(
        self, next_instruction: str, r_dest: str
    ) -> List[str]:
        res: List[str] = []

        if line_loads_from_reg(next_instruction, r_dest):
            nop_required = False

            if not uses_at(next_instruction):
                reason = f"'{next_instruction}' does not use $at"
                nop_required = True
            if self._uses_gp(next_instruction):
                reason = f"'{next_instruction}' uses $gp"
                nop_required = True
            if uses_at(next_instruction) and self.nop_at_expansion:
                reason = (
                    f"'{next_instruction}' inject nop beween {r_dest} and $at expansion"
                )
                nop_required = True

            if nop_required:
                label = self.get_next_instruction(
                    skip=0, ignore_nop=True, ignore_set=True
                )
                if is_label(label) and (
                    getattr(self, "load_delay_nop_before_any_label", False)
                    or (
                        self.load_delay_nop_before_label
                        and self._label_opens_directive_block(label)
                    )
                    or (
                        self.load_delay_nop_before_loop_label
                        and self._label_is_loop_head(label)
                    )
                ):
                    # LOCAL PATCH 14: leave the label in place; the nop goes
                    # before it.
                    res.append(
                        f"nop # DEBUG: Reuse of '{r_dest}'. {reason} "
                        f"(LOAD_DELAY_NOP_BEFORE_LABEL: {label})"
                    )
                    return res
                if is_label(label):
                    res.append(label)
                    self.skip_instructions = 1
                res.append(f"nop # DEBUG: Reuse of '{r_dest}'. {reason}")
        elif self.load_delay_call_slot_store and self._call_slot_store_reads(
            next_instruction, r_dest
        ):
            res.append(
                f"nop # DEBUG: LOAD_DELAY_CALL_SLOT_STORE: the `{next_instruction}` "
                f"delay-slot store reads {r_dest}"
            )
        else:
            res.append(
                f"#nop # DEBUG: '{next_instruction}' does not load from {r_dest}"
            )

        return res

    def _label_is_loop_head(self, label: str) -> bool:
        # LOCAL PATCH helper (patch 21). True when `label` is the very next
        # non-comment line after the current load (HL21-1, as patch 14's HN1)
        # and it is a pure loop head: at least one branch/jump targets it,
        # EVERY branch/jump that targets it sits AFTER it in the function
        # (backward edges only -- the only forward entry is the fall-through
        # from the load, HL21-2), and no such branch in a cc1 `.set
        # noreorder` block carries a load (or a compound macro) in its delay
        # slot (HL21-3, as patch 14's HN3): with the nop moved before the
        # label, the back-edges reach the first use without a pending load.
        i = self.line_index + 1
        while i < len(self.lines) and (
            self.lines[i] == "" or self.lines[i].startswith("#")
        ):
            i += 1
        if i >= len(self.lines) or self.lines[i] != label:  # HL21-1
            return False
        label_index = i
        target = label[:-1]
        edges = 0
        noreorder = False
        for k, line in enumerate(self.lines):
            if line == ".set\tnoreorder":
                noreorder = True
            elif line == ".set\treorder":
                noreorder = False
            parts = line.split(None, 1)
            if len(parts) != 2:
                continue
            op, operands = parts
            if op not in branch_mnemonics and op not in ("j", "jal"):
                continue
            if operands.split(",")[-1].strip() != target:
                continue
            if k < label_index:  # HL21-2: a forward entry
                return False
            edges += 1
            if not noreorder:
                continue
            m = k + 1
            while m < len(self.lines) and (
                self.lines[m] == "" or self.lines[m].startswith("#")
            ):
                m += 1
            if m >= len(self.lines):
                return False
            slot_op = self.lines[m].split(None, 1)[0]
            if slot_op in load_mnemonics or ";" in self.lines[m]:  # HL21-3
                return False
        return edges > 0

    def _label_opens_directive_block(self, label: str) -> bool:
        # LOCAL PATCH helper (patch 14). Walking the raw input from the
        # current load: skip blank and comment lines up to `label` (HN1: it
        # must be the very next non-comment line -- no other label or
        # instruction in between), then True when the first non-blank line
        # after the label is a cc1 directive that opens a block: `.set
        # noreorder` (a cc1-scheduled branch) or `#.set volatile` (a volatile
        # access). HN2: any other line (a plain instruction, another label,
        # `.set reorder`, `#APP`) keeps the stock label-then-nop placement.
        i = self.line_index + 1
        while i < len(self.lines) and (
            self.lines[i] == "" or self.lines[i].startswith("#")
        ):
            i += 1
        if i >= len(self.lines) or self.lines[i] != label:  # HN1
            return False
        i += 1
        while i < len(self.lines) and self.lines[i] == "":
            i += 1
        if i >= len(self.lines):
            return False
        if (
            self.nop_before_label_skip_empty_app
            and i + 1 < len(self.lines)
            and self.lines[i] == "#APP"
            and self.lines[i + 1] == "#NO_APP"
        ):
            i += 2
            while i < len(self.lines) and self.lines[i] == "":
                i += 1
        if i >= len(self.lines):
            return False
        if self.lines[i] not in (".set\tnoreorder", "#.set\tvolatile"):  # HN2
            return False
        # HN3: every branch/jump to this label must arrive without a pending
        # load. A reorder-mode branch gets maspsx's own nop slot; a branch in
        # a cc1 `.set noreorder` block whose delay-slot instruction is a load
        # would reach the label's first use with no delay once the nop moves
        # before the label -- refuse.
        target = label[:-1]
        noreorder = False
        for k, line in enumerate(self.lines):
            if line == ".set\tnoreorder":
                noreorder = True
            elif line == ".set\treorder":
                noreorder = False
            parts = line.split(None, 1)
            if len(parts) != 2:
                continue
            op, operands = parts
            if op not in branch_mnemonics and op not in ("j", "jal"):
                continue
            if operands.split(",")[-1].strip() != target:
                continue
            if not noreorder:
                continue
            m = k + 1
            while m < len(self.lines) and (
                self.lines[m] == "" or self.lines[m].startswith("#")
            ):
                m += 1
            if m >= len(self.lines):
                return False
            slot_op = self.lines[m].split(None, 1)[0]
            if slot_op in load_mnemonics or ";" in self.lines[m]:
                return False
        return True

    def _call_slot_store_reads(self, next_instruction: str, r_dest: str) -> bool:
        # LOCAL PATCH helper (patch 13). True when, walking the raw input from
        # the current load, the next instruction is a direct `jal <symbol>`
        # opened by cc1's own `.set noreorder` (so the following instruction
        # really is its delay slot: HL1), no label intervenes (HL2), and that
        # slot instruction is a single sb/sh/sw that reads `r_dest` as its
        # value or base register (HL3).
        parts = next_instruction.split()
        if len(parts) != 2 or parts[0] != "jal" or parts[1].startswith("$"):
            return False
        i = self.line_index + 1
        noreorder = False
        while i < len(self.lines):
            line = self.lines[i]
            if line == "" or line.startswith("#"):
                i += 1
                continue
            if line == ".set\tnoreorder":
                noreorder = True
            elif line.startswith(".set"):
                pass
            elif line == next_instruction:
                break
            else:
                return False  # HL2: a label or another instruction in between
            i += 1
        else:
            return False
        if not noreorder:  # HL1
            return False
        i += 1
        while i < len(self.lines) and (
            self.lines[i] == "" or self.lines[i].startswith("#")
        ):
            i += 1
        if i >= len(self.lines) or ";" in self.lines[i]:
            return False
        slot = self.lines[i].split(None, 1)
        if len(slot) != 2 or slot[0] not in ("sb", "sh", "sw"):  # HL3
            return False
        want = canon_reg(r_dest)
        if not isinstance(want, int):
            return False
        return any(
            canon_reg(r) == want for r in re.findall(r"\$\w+", slot[1])
        )

    @staticmethod
    def _reorder_fill_candidate(line: str):
        # LOCAL PATCH 22 helper: `line` is a single-word ALU instruction that
        # may be moved (HF1). Returns (dest_gpr, {read gprs}) or None.
        if ";" in line or line.startswith("#") or line.endswith(":"):
            return None
        parts = strip_comments(line).strip().split(None, 1)
        if len(parts) != 2:
            return None
        op, operands = parts
        ops = [x.strip() for x in operands.split(",")]
        three = {"addu", "addiu", "subu", "and", "andi", "or", "ori", "xor",
                 "xori", "nor", "slt", "slti", "sltu", "sltiu", "sll", "srl",
                 "sra", "sllv", "srlv", "srav"}
        if op == "move" and len(ops) == 2:
            regs = ops
        elif op == "li" and len(ops) == 2:
            if not is_number(ops[1]) or not -0x8000 <= int(ops[1], 0) <= 0xFFFF:
                return None  # one word only (no lui/ori pair)
            regs = [ops[0]]
        elif op in three and len(ops) == 3:
            if is_number(ops[2]):
                v = int(ops[2], 0)
                if not -0x8000 <= v <= 0xFFFF:
                    return None
                regs = ops[:2]
            else:
                regs = ops
        else:
            return None
        nums = [gpr_number(r) for r in regs]
        if any(n is None for n in nums):
            return None
        # HF2: never $zero-dest, $at, $sp, $fp or $ra
        if nums[0] in (0, 1) or any(n in (1, 29, 30, 31) for n in nums):
            return None
        return nums[0], set(nums[1:])

    def _spill_before_symbol_load(self) -> None:
        # Swap `lw $r,SYM` with the immediately following `sw $sN,imm($sp)`
        # when $r is not $sN. cc1 sometimes hoists that symbolic load above
        # one callee-save; retail keeps the save first.
        out = []
        i = 0
        lines = self.lines
        while i < len(lines):
            a = lines[i].split()
            b = lines[i + 1].split() if i + 1 < len(lines) else []
            if (
                len(a) == 2
                and a[0] == "lw"
                and "(" not in a[1]
                and "," in a[1]
                and len(b) == 2
                and b[0] == "sw"
                and b[1].endswith("($sp)")
                and "," in b[1]
                and a[1].split(",")[0] != b[1].split(",")[0]
            ):
                out.append(lines[i + 1])
                out.append(lines[i])
                i += 2
                continue
            out.append(lines[i])
            i += 1
        self.lines = out

    def _reorder_fill_calls_and_epilogue(self) -> None:
        # LOCAL PATCH 22 (reorder_fill_calls, env MASPSX_REORDER_FILL_CALLS=1).
        # For cc1 output built with -fno-delayed-branch (every slot empty),
        # model the two local fills ASPSX's reorder pass makes, rewriting them
        # into cc1-style `.set noreorder` blocks that maspsx passes through:
        #  (A) X / jal T            ->  jal T / X          (call slot)
        #  (B) X / lw $31,N($sp) / addu $sp,$sp,M / j $31
        #                           ->  lw $31,N($sp) / X / j $31 / addu $sp,..
        # (X fills the $31 load delay; the pop fills the return slot.)
        # Only in reorder mode, X must immediately precede (no label, no
        # directive in between) and be a single-word ALU op (HF1/HF2).
        lines = self.lines
        out = []
        noreorder = False
        i = 0

        def nxt(k):
            k += 1
            while k < len(lines) and (lines[k] == "" or lines[k].startswith("#")):
                k += 1
            return k

        while i < len(lines):
            line = lines[i]
            if line == ".set\tnoreorder":
                noreorder = True
            elif line == ".set\treorder":
                noreorder = False
            cand = None if noreorder else self._reorder_fill_candidate(line)
            if cand is not None:
                dest, reads = cand
                j = nxt(i)
                nl = lines[j] if j < len(lines) else ""
                parts = nl.split()
                # (A) call slot: direct `jal sym` or `jal $r` (jalr)
                if len(parts) == 2 and parts[0] == "jal":
                    target = parts[1]
                    tnum = gpr_number(target) if target.startswith("$") else None
                    # HF3: jalr reads its target at jump time -- X must not
                    # write it; `jal $31,$r` spellings are refused.
                    if "," not in target and (tnum is None or tnum != dest):
                        out += [f"# REORDER_FILL_CALLS: {line} -> {nl} slot",
                                ".set\tnoreorder", ".set\tnomacro", nl, line,
                                ".set\tmacro", ".set\treorder"]
                        i = j + 1
                        continue
                # (B) epilogue
                if re.match(r"^lw\t\$(31|ra),-?\d+\(\$(sp|29)\)$", nl):
                    k = nxt(j)
                    m = nxt(k)
                    if (
                        k < len(lines) and m < len(lines)
                        and re.match(r"^addi?u\t\$(sp|29),\$(sp|29),-?\d+$", lines[k])
                        and lines[m] == "j\t$31"
                    ):
                        out += [f"# REORDER_FILL_CALLS: epilogue {line}",
                                nl, line,
                                ".set\tnoreorder", ".set\tnomacro", "j\t$31",
                                lines[k], ".set\tmacro", ".set\treorder"]
                        i = m + 1
                        continue
            out.append(line)
            i += 1
        self.lines = out

    def _sink_return_zero_into_flag_delay(self) -> None:
        # LOCAL PATCH 23. See the ctor comment. Refuses unless the shape is
        # unique: one `beq $2,$0,$L`, one epilogue `move $2,$0` directly
        # before the noreorder return block, and a single ordinary delay
        # instruction. Idempotent: the epilogue zero is gone after one run.
        lines = self.lines
        zero = "move\t$2,$0"
        sunk = "addu\t$2,$0,$zero"

        def next_real(idx):
            while idx < len(lines) and (
                lines[idx] == "" or lines[idx].startswith("#")
            ):
                idx += 1
            return idx

        epi = None
        for i, line in enumerate(lines):
            if line != zero:
                continue
            j = next_real(i + 1)
            if j >= len(lines) or lines[j] != ".set\tnoreorder":
                continue
            seq = []
            k = j
            while k < len(lines) and len(seq) < 4:
                if lines[k] != "" and not lines[k].startswith("#"):
                    seq.append(lines[k])
                k += 1
            if (
                len(seq) == 4
                and seq[0] == ".set\tnoreorder"
                and seq[1] == ".set\tnomacro"
                and seq[2] == "j\t$31"
                and re.match(r"^addi?u\t\$sp,\$sp,-?\d+$", seq[3])
            ):
                epi = i
                break
        if epi is None:
            return

        lab = None
        for i in range(epi - 1, -1, -1):
            if lines[i].endswith(":") and not lines[i].startswith("."):
                lab = lines[i][:-1]
                break
        if not lab or not re.match(r"\$L\d+$", lab):
            return
        beqs = [
            i
            for i, line in enumerate(lines)
            if line == f"beq\t$2,$0,{lab}"
        ]
        if len(beqs) != 1:
            return
        bi = beqs[0]
        di = next_real(bi + 1)
        if di >= len(lines) or di == epi:
            return
        delay = lines[di]
        if (
            delay == zero
            or delay.endswith(":")
            or delay.startswith(".")
            or re.match(r"^(b|j|jal)", delay)
        ):
            return

        out = []
        for i, line in enumerate(lines):
            if i == bi:
                # Reorder mode would ignore a filled slot and emit its own
                # nop. A noreorder pair makes the `addu` the real delay.
                out.append(".set\tnoreorder")
                out.append(line)
                continue
            if i == di:
                out.append(sunk)
                out.append(".set\treorder")
                out.append(
                    f"# SINK_RETURN_ZERO: {delay} moved to the fall-through"
                )
                out.append(delay)
                continue
            if i == epi:
                out.append(
                    "# SINK_RETURN_ZERO: move $2,$0 sunk into beq $2,$0 delay"
                )
                continue
            out.append(line)
        self.lines = out

    def _unfill_epilogue_delay_slots(self) -> None:
        # LOCAL PATCH 15. Rewrite every cc1-scheduled epilogue block
        #   .set noreorder / .set nomacro / j $31 / addu $sp,$sp,N /
        #   .set macro / .set reorder
        # into reorder-mode `addu $sp,$sp,N` / `j $31`, so the stack pop runs
        # before the return and maspsx emits its own `nop` slot. HU1: the
        # block must be exactly those six lines (blank/comment lines between
        # them are allowed), the slot a single `addu`/`addiu $sp,$sp,<imm>`.
        # Idempotent: the rewritten form no longer matches.
        lines = self.lines
        out = []
        i = 0
        while i < len(lines):
            if lines[i] == ".set\tnoreorder":
                seq = []
                j = i
                while j < len(lines) and len(seq) < 6:
                    if lines[j] != "" and not lines[j].startswith("#"):
                        seq.append((j, lines[j]))
                    j += 1
                texts = [t for _, t in seq]
                if (
                    len(texts) == 6
                    and texts[1] == ".set\tnomacro"
                    and texts[2] == "j\t$31"
                    and re.match(
                        r"^addi?u\t\$(sp|29),\$(sp|29),-?\d+$", texts[3]
                    )
                    and texts[4] == ".set\tmacro"
                    and texts[5] == ".set\treorder"
                ):
                    out.append(
                        f"# UNFILL_EPILOGUE_DELAY_SLOT: {texts[3]} moved before j $31"
                    )
                    out.append(texts[3])
                    out.append("j\t$31")
                    i = seq[-1][0] + 1
                    continue
            out.append(lines[i])
            i += 1
        self.lines = out

    def _retarget_mid_function_returns(self) -> None:
        # LOCAL PATCH 12. Per `.ent`/`.end` function: when the function is
        # frameless (`.frame $sp,0,$31` and a zero `.mask`: HR1) and cc1 emitted
        # more than one `j $31`, every earlier `j $31` that sits in a cc1
        # `.set noreorder` block (HR2) becomes `j $L<fresh>` and the label is
        # placed on the function's final `j $31`. Idempotent: a second run
        # finds a single `j $31` and changes nothing.
        counter = getattr(self, "_retarget_label_counter", 0)
        lines = self.lines
        out = []
        i = 0
        while i < len(lines):
            line = lines[i]
            if not line.startswith(".ent\t"):
                out.append(line)
                i += 1
                continue
            name = line.split("\t", 1)[1]
            end = i + 1
            while end < len(lines) and lines[end] != f".end\t{name}":
                end += 1
            body = lines[i:end]
            frameless = any(
                re.match(r"^\.frame\t\$sp,0,\$31", x) for x in body
            ) and any(x.startswith(".mask\t0x00000000") for x in body)
            returns = [k for k, x in enumerate(body) if x == "j\t$31"]
            if frameless and len(returns) > 1:
                final = returns[-1]
                moved = []
                for k in returns[:-1]:
                    back = k - 1
                    while back >= 0 and (
                        body[back] == "" or body[back].startswith("#")
                        or body[back] == ".set\tnomacro"
                    ):
                        back -= 1
                    if back >= 0 and body[back] == ".set\tnoreorder":  # HR2
                        moved.append(k)
                if moved:
                    label = f"$L{900000 + counter}"
                    counter += 1
                    for k in moved:
                        body[k] = f"j\t{label}"
                    body = body[:final] + [f"{label}:"] + body[final:]
            out.extend(body)
            i = end
        self._retarget_label_counter = counter
        self.lines = out

    def _handle_mflo_mfhi(self, r_source=None) -> List[str]:
        # we cannot use a div/mult within 2 instructions of mflo/mfhi
        res: List[str] = []

        if not self.nop_mflo_mfhi:
            return res

        next_instruction = self.get_next_instruction(
            skip=0, ignore_nop=True, ignore_set=True, ignore_label=True
        )
        next_next_instruction = self.get_next_instruction(
            skip=1, ignore_nop=True, ignore_set=True, ignore_label=True
        )

        if any(
            next_instruction.startswith(x)
            for x in ["mult\t", "multu\t", "div\t", "divu\t", "rem\t", "remu\t"]
        ):
            # #nop
            # #nop
            # mult...
            skip = 0
            while True:
                inst = self.get_next_instruction(skip=skip)
                skip += 1
                if inst == next_instruction:
                    res.append("nop")
                    res.append("nop")
                    if div_needs_expanding(inst):
                        res.append("# DEBUG: div needs expanding")
                        skip -= 1
                    else:
                        res.append(expand_move(inst))
                    break
                if not inst.startswith("#"):
                    res.append(expand_move(inst))
            self.skip_instructions = skip

        elif any(
            next_next_instruction.startswith(x)
            for x in ["mult\t", "multu\t", "div\t", "divu\t", "rem\t", "remu\t"]
        ):

            # #nop
            # #nop
            # bne or addu or lh ...
            # mult ...
            skip = 0
            no_reorder = False
            while True:
                inst = self.get_next_instruction(skip=skip)

                if inst.startswith(".set") and inst.endswith("noreorder"):
                    no_reorder = True
                    skip += 1
                    continue

                skip += 1
                if inst == next_instruction:
                    op, *_ = inst.strip().split()
                    if op in load_mnemonics:
                        # allow for $at handling later in the script
                        skip = 0
                        break

                    if op in ("mflo", "mfhi"):
                        # allow for mflo/mfhi handling later on
                        skip = 0
                        break

                    if op == "li":
                        expanded = expand_load_immediate(inst)

                        if self.expand_li:
                            res += expanded
                        else:
                            res.append(inst)

                        if len(expanded) == 2:
                            res.append(
                                "#nop  # DEBUG: mflo/mfhi with mult/div/rem and li expands to 2 ops"
                            )
                        else:
                            res.append(
                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and li expands to 1 op"
                            )

                    else:

                        if no_reorder:
                            res.append(
                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction (noreorder)"
                            )
                            res.append(".set\tnoreorder")
                            res.append(expand_move(inst))
                        else:
                            if r_source and line_loads_from_reg(inst, r_source):
                                # NOTE: only relevant when div has been expanded (i.e. -0 flag)
                                res.extend(
                                    [
                                        f"nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction which loads from {r_source}",
                                        expand_move(inst),
                                    ]
                                )
                            else:
                                if op in branch_mnemonics:
                                    res.extend(
                                        [
                                            inst,
                                            "nop # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction (branch)",
                                        ]
                                    )
                                else:
                                    maybe_label = self.get_next_instruction(skip=skip)
                                    if is_label(maybe_label):
                                        res.extend(
                                            [
                                                expand_move(inst),
                                                maybe_label,
                                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction (label)",
                                            ]
                                        )
                                        skip += 1
                                    else:
                                        res.extend(
                                            [
                                                expand_move(inst),
                                                "nop  # DEBUG: mflo/mfhi with mult/div/rem and 1 instruction",
                                            ]
                                        )

                elif inst == next_next_instruction:
                    # reached mult/div/rem
                    if div_needs_expanding(inst):
                        res.append("# DEBUG: div needs expanding")
                        skip -= 1
                    else:
                        res.append(inst)
                    break
                elif not inst.startswith("#"):
                    res.append(expand_move(inst))
            self.skip_instructions = skip

        else:
            # do nothing
            pass

        return res

    def process_line(self, line: str):
        res = []

        if len(line) == 0:
            return [line]

        if line.startswith("#"):
            return []

        if line.startswith("."):
            if (
                line.startswith(".def\t")
                or line.startswith(".begin\t")
                or line.startswith(".bend\t")
            ):
                # skip these coff directives - gnu as does not like them
                pass

            elif line.startswith(".set\t"):
                if line.endswith("\tnoreorder"):
                    self.is_reorder = False
                elif line.endswith("\treorder"):
                    self.is_reorder = True

            elif line.startswith(".file\t"):
                # fix same-numbered files
                _, file_num, filename = line.split(maxsplit=2)
                res.append(f".file\t{self.file_num} {filename}")
                self.file_num += 1

            elif line.startswith(".ent\t"):
                # enforce noreorder for each function
                res.append(line)
                res.append(".set\tnoreorder")

            elif line.startswith(".comm") or line.startswith(".lcomm"):
                # already handled via preprocess_lines
                pass

            elif line.startswith(".data"):
                res.append(".section .data")
            elif line.startswith(".sdata"):
                res.append(".section .sdata")
            elif line.startswith(".rdata"):
                res.append(".section .rodata")

            else:
                res.append(line)

            return res

        if line.startswith("$L"):
            return [line]

        actual_r_dest = None
        is_macro = ";" in line
        if is_macro:
            expanded = expand_macro(line)
            if len(expanded) > 0:
                actual_op, *actual_rest = expanded[-1]
                if actual_op in load_mnemonics:
                    _, actual_r_dest, _, _, _ = parse_load_or_store(
                        " ".join(actual_rest)
                    )

        op, *rest = line.split()

        # LOCAL PATCH: fill the return delay slot with the epilogue frame
        # deallocation. Opt-in; see the patch log at the top of this file.
        if (
            self.fill_epilogue_delay_slot
            and op in ("addu", "addiu")
            and len(rest) == 1
            and rest[0].startswith("$sp,$sp,")
            and self._next_line_is_return_jump()
        ):
            frame = rest[0].split(",")[2]
            res.extend(
                [
                    "# FILL_EPILOGUE_DELAY_SLOT START",
                    ".set\tnoreorder",
                    "j\t$31",
                    f"addiu\t$sp,$sp,{frame}",
                    ".set\treorder",
                    "# FILL_EPILOGUE_DELAY_SLOT END",
                ]
            )
            self.skip_instructions = 1
            return res

        # LOCAL PATCH (patch 8, `la` arm): put the address macro's second word
        # into the delay slot of the following plain jal/j:
        #   lui $r,%hi(SYM) / jal T / addiu $r,$r,%lo(SYM)
        # This is a dedicated block rather than a widening of the store
        # branch's `la` condition so that nothing else in that chain becomes
        # newly reachable for `la` when the knob is on.
        if (
            (self.fill_call_delay_slot or self.fill_branch_macro_split)
            and op == "la"
            and not is_macro  # H7
            and len(rest) == 1
            # H8: with gp_allow_la (ASPSX >= 2.80) a `la` of a small-data
            # symbol is a single gp-relative instruction with no second word
            # to move. That eligibility is not decidable here for every
            # operand spelling, so refuse the whole mode.
            and not self.gp_allow_la
        ):
            la_parts = rest[0].split(",")
            la_operand = la_parts[1] if len(la_parts) == 2 else ""
            call = (
                self._next_line_is_plain_call()
                if self.fill_call_delay_slot else None
            )
            # LOCAL PATCH (patch 10): the same split before a conditional
            # branch. `branch[1]` is the set of registers the branch tests.
            branch = (
                self._next_line_is_cond_branch()
                if call is None and self.fill_branch_macro_split else None
            )
            la_ok = (
                la_operand
                and "(" not in la_operand  # not an indexed form
                # H2: the moved word WRITES this register; $31 would destroy
                # the return address `jal` has just stored.
                and canon_reg(la_parts[0]) not in (31, 1)
                # H9: a numeric operand is not a symbol macro.
                and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", la_operand)
            )
            if (
                la_ok
                and call is not None
                # H3: a register-indirect jump reads its target register
                # BEFORE the slot runs, so the moved word must not be what
                # completes that address.
                and canon_reg(call[1]) != canon_reg(la_parts[0])
            ):
                r_dest = la_parts[0]
                res.extend(
                    [
                        "# FILL_CALL_DELAY_SLOT LA START",
                        f"lui\t{r_dest},%hi({la_operand})",
                        call[0],
                        f"addiu\t{r_dest},{r_dest},%lo({la_operand})",
                        "# FILL_CALL_DELAY_SLOT LA END",
                    ]
                )
                self.skip_instructions = 1
                return res
            if (
                la_ok
                and branch is not None
                # HB1: the branch reads its operands BEFORE the slot runs, so
                # the moved word must not be what completes one of them.
                and canon_reg(la_parts[0]) not in branch[1]
            ):
                r_dest = la_parts[0]
                res.extend(
                    [
                        "# FILL_BRANCH_MACRO_SPLIT LA START",
                        f"lui\t{r_dest},%hi({la_operand})",
                        branch[0],
                        f"addiu\t{r_dest},{r_dest},%lo({la_operand})",
                        "# FILL_BRANCH_MACRO_SPLIT LA END",
                    ]
                )
                self.skip_instructions = 1
                return res

        if op in load_mnemonics:
            r_source, r_dest, operand, is_addend, needs_expanding = parse_load_or_store(
                " ".join(rest)
            )

            next_instruction = self.get_next_instruction(
                skip=0, ignore_nop=True, ignore_set=True, ignore_label=True
            )
            # Naively handle scenario where *next* line is a macro
            if ";" in next_instruction:
                next_instruction = next_instruction.split(";")[0]

            if not needs_expanding:
                # newer GCCs can emit %hi() and %lo() separately...
                res.append(f"{line} # DEBUG: leaving for assembler to expand")
                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

            elif is_addend and r_source is None:
                # e.g. lb	$s0,D_800E52E0
                if operand.count("+") == 1:
                    symbol, offset = operand.split("+")
                    gp_rel = f"%gp_rel({symbol}+{offset})($gp)"
                    gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols
                else:
                    symbol = operand
                    gp_rel = f"%gp_rel({symbol})($gp)"
                    gp_allowed = True

                if gp_allowed and (
                    symbol in self.sdata_entries or symbol in self.sbss_entries
                ):
                    res.append(f"{op}\t{r_dest},{gp_rel}")
                else:
                    res.append(line)

                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

            elif is_addend and r_source:
                # e.g. lw	$2,test_sym($4)
                # LOCAL PATCH (switch dispatch retarget): when
                # MASPSX_DISPATCH_FOLD=<sym> is set alongside the
                # three-word gate and the operand is a compiler-local
                # label ($L<digits> — cc1's own switch table), substitute
                # <sym> (the shared rodata pool table) for the local
                # label; named symbols are never substituted and gate-off
                # keeps every prior behavior.
                if (
                    self.three_word_symbol_store
                    and self.dispatch_fold_symbol
                    and re.match(r"^\$L\d+$", operand)
                ):
                    operand = self._dispatch_fold_target(operand)
                dest_temp_used = (
                    self.symbol_load_dest_temp
                    and not is_macro
                    and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)
                    # The destination cannot serve as the address temp when
                    # it IS the index register (`lhu $2,SYM($2)`): the
                    # `lui $2` would clobber the index before the `addu`
                    # read it.  GNU as (and retail: func_80074A44 has both
                    # `lhu $v1,D_800957CC($v0)` dest-temp and
                    # `lhu $v0,D_800957D8($v0)` via $at) fall back to the
                    # $at form in exactly that case.
                    and canon_reg(r_dest) != canon_reg(r_source)
                )
                # LOCAL PATCH (patch 5): 3-word $at address temp, %lo kept.
                at_temp_used = (
                    self._symbol_at_temp_applies(operand)
                    and not is_macro
                    and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)
                )
                if at_temp_used:
                    res.extend(
                        [
                            "# SYMBOL_AT_TEMP START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# SYMBOL_AT_TEMP END",
                        ]
                    )
                # LOCAL PATCH (patch 4): naive dest-register address temp.
                if dest_temp_used and not at_temp_used:
                    res.extend(
                        [
                            "# DEST_TEMP START",
                            ".set\tnoat",
                            f"lui\t{r_dest},%hi({operand})",
                            f"addu\t{r_dest},{r_dest},{r_source}",
                            f"{op}\t{r_dest},%lo({operand})({r_dest})",
                            ".set\tat",
                            "# DEST_TEMP END",
                        ]
                    )
                if dest_temp_used or at_temp_used:
                    pass
                elif self.addiu_at and not self.three_word_symbol_store:
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addiu\t$at,$at,%lo({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},0x0($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )
                elif self.addiu_at and self.three_word_symbol_store:
                    # Compound macro lines keep the legacy lui/addiu/addu
                    # expansion; standalone lines use the 3-word $at form.
                    if is_macro:
                        res.extend(
                            [
                                "# EXPAND_AT START",
                                ".set\tnoat",
                                f"lui\t$at,%hi({operand})",
                                f"addiu\t$at,$at,%lo({operand})",
                                f"addu\t$at,$at,{r_source}",
                                f"{op}\t{r_dest},0x0($at)",
                                ".set\tat",
                                "# EXPAND_AT END",
                            ]
                        )
                    else:
                        res.extend(
                            [
                                "# EXPAND_AT START",
                                ".set\tnoat",
                                f"lui\t$at,%hi({operand})",
                                f"addu\t$at,$at,{r_source}",
                                f"{op}\t{r_dest},%lo({operand})($at)",
                                ".set\tat",
                                "# EXPAND_AT END",
                            ]
                        )
                else:
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )

                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

            else:
                if (
                    os.environ.get("MASPSX_NUMERIC_LOAD_ASPSX") == "1"
                    and r_source
                    and (int(operand, 0) > 32767 or int(operand, 0) < -32768)
                ):
                    # LOCAL PATCH (K5, MASPSX_NUMERIC_LOAD_ASPSX=1): an indexed
                    # load from a numeric address outside int16 (cc1 2.8.1
                    # -mno-split-addresses, `lw $d,0x1F801088($b)` for a
                    # hardware-register array). ASPSX uses the destination
                    # register as the address temp when it differs from the
                    # index (`lui $d / addu $d,$d,$b / lw $d,%lo($d)`), and
                    # `$at` in ASPSX operand order otherwise. Retail
                    # func_8007CEAC 0x8007CEE4 / 0x8007CF08 / 0x8007CF4C.
                    if canon_reg(r_dest) != canon_reg(r_source):
                        tmp = r_dest
                    else:
                        tmp = "$at"
                    res.extend(
                        [
                            "# NUMERIC_LOAD_ASPSX START",
                            ".set\tnoat",
                            f"lui\t{tmp},%hi({operand})",
                            f"addu\t{tmp},{tmp},{r_source}",
                            f"{op}\t{r_dest},%lo({operand})({tmp})",
                            ".set\tat",
                            "# NUMERIC_LOAD_ASPSX END",
                        ]
                    )
                elif r_source and (int(operand, 0) > 32767 or int(operand, 0) < -32768):
                    # e.g. lhu	$2,49344($2)
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,{r_source},$at",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )
                else:
                    # e.g. lhu	$2,528482304
                    res.append(line)

                # Naively handle scenario where *current* line is a macro
                if actual_r_dest is not None:
                    r_dest = actual_r_dest

                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

        elif op in store_mnemonics or (op == "la" and self.sdata_limit > 0):
            r_source, r_dest, operand, is_addend, needs_expanding = parse_load_or_store(
                " ".join(rest)
            )

            if not needs_expanding:
                # LOCAL PATCH: an explicit `op $r,%lo(SYM)($base)` store (the
                # form gcc-2.8.1-psx cc1 emits after its own `lui $base,%hi`;
                # 2.7.2 never emits it) is already one instruction with a
                # LO16 reloc — pass it through exactly like the load arm above
                # instead of re-expanding it into the 4-word $at macro.
                res.append(f"{line} # DEBUG: leaving for assembler to expand")

            elif is_addend and r_source is None:
                # e.g. sw	$v0,D_800E52E0
                if operand.count("+") == 1:
                    symbol, offset = operand.split("+")
                    gp_rel = f"%gp_rel({symbol}+{offset})($gp)"
                    gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols
                else:
                    symbol = operand
                    gp_rel = f"%gp_rel({symbol})($gp)"
                    gp_allowed = True

                if op == "la" and not self.gp_allow_la:
                    gp_allowed = False

                if (
                    # LOCAL PATCH 17 (la_absolute_small_data): ASPSX < 2.80
                    # never forms a gp-relative `la`; GNU as would, for any
                    # symbol it places in small data. Emit the absolute pair
                    # explicitly so only the loads/stores stay gp-relative.
                    self.la_absolute_small_data
                    and op == "la"
                    and not self.gp_allow_la  # HA1
                    and not is_macro  # HA2
                    and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", symbol)  # HA3
                    and (
                        symbol in self.sdata_entries
                        or symbol in self.sbss_entries
                        or symbol in self.extern_entries
                    )  # HA4
                ):
                    res.extend(
                        [
                            "# LA_ABSOLUTE_SMALL_DATA START",
                            f"lui\t{r_dest},%hi({operand})",
                            f"addiu\t{r_dest},{r_dest},%lo({operand})",
                            "# LA_ABSOLUTE_SMALL_DATA END",
                        ]
                    )
                elif gp_allowed and (
                    symbol in self.sdata_entries or symbol in self.sbss_entries
                ):
                    res.append(f"{op}\t{r_dest},{gp_rel}")
                elif (
                    self.beqz_sym_store_delay
                    and op == "sw"
                    and r_source is None
                    and (beq := self._next_line_is_beqz()) is not None
                    and (self._line_after_beq_is_label()
                         or os.environ.get("MASPSX_BEQZ_SYM_STORE_DELAY_ANY") == "1")
                ):
                    res.extend(
                        [
                            "# BEQZ_SYM_STORE_DELAY START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            beq,
                            f"sw\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# BEQZ_SYM_STORE_DELAY END",
                        ]
                    )
                    self.skip_instructions = 1
                elif (
                    self.sink_zero_store_into_beqz
                    and op == "sw"
                    and r_dest in ("$0", "$zero")
                    and (beq := self._next_line_is_beqz()) is not None
                ):
                    res.extend(
                        [
                            "# SINK_ZERO_STORE_INTO_BEQZ START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            beq,
                            f"sw\t$0,%lo({operand})($at)",
                            ".set\tat",
                            "# SINK_ZERO_STORE_INTO_BEQZ END",
                        ]
                    )
                    self.skip_instructions = 1
                elif (
                    self.fill_store_delay_slot
                    and op == "sw"
                    and self._next_line_is_return_jump()
                ):
                    # LOCAL PATCH: fill the return delay slot with the store.
                    # cc1 cannot schedule the absolute-store macro (its
                    # `lui $at` + `sw` expansion is opaque to cc1's delay-slot
                    # filler) around `j $31`, so it emits `sw $r,SYM` / `j $31`
                    # with an empty slot. ASPSX's scheduler expands the macro
                    # and moves the trailing store into the slot:
                    #   lui $at,%hi(SYM) / j $31 / sw $r,%lo(SYM)($at)
                    # ROM-proven for the 14-member sw delay-slot family.
                    # sw only: sb/sh macro stores stay pre-jr (nop slot) in
                    # ROM. Consuming the `j $31` line via skip_instructions
                    # also suppresses the nop maspsx would append for it.
                    res.extend(
                        [
                            "# FILL_STORE_DELAY_SLOT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            "j\t$31",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# FILL_STORE_DELAY_SLOT END",
                        ]
                    )
                    self.skip_instructions = 1
                elif (
                    # LOCAL PATCH (patch 8, absolute-store arm): the macro's
                    # second word goes into the delay slot of the following
                    # plain jal/j, exactly as ASPSX's reorder pass schedules
                    # it:  lui $at,%hi(SYM) / jal T / sw $r,%lo(SYM)($at)
                    self.fill_call_delay_slot
                    and op in ("sb", "sh", "sw")
                    and not is_macro  # H7
                    # H1: `jal` writes $31, so a value register of $31 would
                    # store the NEW return address once moved past the call.
                    # H4: $at is the address temp the `lui` sets up.
                    and canon_reg(r_dest) not in (31, 1)
                    and (call := self._next_line_is_plain_call()) is not None
                    # H3: the moved store writes no register, so a
                    # register-indirect target is safe unless it is the `$at`
                    # the surviving `lui` overwrites before the jump.
                    and canon_reg(call[1]) != 1
                ):
                    res.extend(
                        [
                            "# FILL_CALL_DELAY_SLOT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            call[0],
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# FILL_CALL_DELAY_SLOT END",
                        ]
                    )
                    self.skip_instructions = 1
                elif (
                    # LOCAL PATCH (patch 10, absolute-store arm): the macro's
                    # second word goes into the delay slot of the following
                    # reorder-mode conditional branch:
                    #   lui $at,%hi(SYM) / beq $a,$b,L / sw $r,%lo(SYM)($at)
                    # The store was already before the branch (on both edges)
                    # and writes no GPR, so moving it into the slot changes
                    # nothing the branch reads and nothing either edge sees.
                    self.fill_branch_macro_split
                    and op in ("sb", "sh", "sw")
                    and not is_macro  # HB5
                    # HB3 ($31 spelled by a link branch) / HB2 ($at value reg)
                    and canon_reg(r_dest) not in (31, 1)
                    and (branch := self._next_line_is_cond_branch()) is not None
                    # HB1: cc1 never tests $at, but the surviving `lui $at`
                    # would clobber it before the branch reads it.
                    and 1 not in branch[1]
                ):
                    res.extend(
                        [
                            "# FILL_BRANCH_MACRO_SPLIT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            branch[0],
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# FILL_BRANCH_MACRO_SPLIT END",
                        ]
                    )
                    self.skip_instructions = 1
                else:
                    res.append(line)
            elif is_addend and r_source:
                # e.g. sw	$a0,ctlbuf($v0)
                # LOCAL PATCH (patch 4 store arm): a bare indexed symbolic
                # store that immediately precedes a `j $31` in ROM emits the
                # 3-word $at materialization BEFORE the jump and schedules the
                # store itself into the delay slot:
                #   lui $at,%hi(SYM) / addu $at,$at,$b / j $31 /
                #   op $src,%lo(SYM)($at)
                # ROM: func_80076B20 tail (sb $a0,%lo(D_800A3348)($at) in the
                # jr delay slot). Everything else keeps the legacy path.
                store_at_temp_used = (
                    self._symbol_at_temp_applies(operand)
                    and not is_macro
                    and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)
                )
                if store_at_temp_used:
                    res.extend(
                        [
                            "# SYMBOL_AT_TEMP STORE START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# SYMBOL_AT_TEMP STORE END",
                        ]
                    )
                elif (
                    self.symbol_load_dest_temp
                    and not is_macro
                    and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)
                    and self._next_line_is_return_jump()
                ):
                    res.extend(
                        [
                            "# DEST_TEMP STORE_FILL START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,$at,{r_source}",
                            "j\t$31",
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# DEST_TEMP STORE_FILL END",
                        ]
                    )
                    self.skip_instructions = 1
                elif (
                    # LOCAL PATCH 34 (fill_jump_symbol_store)
                    self.fill_jump_symbol_store
                    and op in ("sb", "sh", "sw")  # HS1
                    and not is_macro  # HS4
                    and not re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)  # HS1
                    and canon_reg(r_dest) != 1  # HS2
                    and canon_reg(r_source) != 1  # HS2
                    and (jump := self._next_line_is_label_jump()) is not None  # HS3
                ):
                    res.extend(
                        [
                            "# FILL_JUMP_SYMBOL_STORE START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addu\t$at,$at,{r_source}",
                            jump,
                            f"{op}\t{r_dest},%lo({operand})($at)",
                            ".set\tat",
                            "# FILL_JUMP_SYMBOL_STORE END",
                        ]
                    )
                    self.skip_instructions = 1
                elif (
                    self.addiu_at
                    and op != "la"
                    and (not self.three_word_symbol_store or is_macro)
                ):
                    res.extend(
                        [
                            "# EXPAND_AT START",
                            ".set\tnoat",
                            f"lui\t$at,%hi({operand})",
                            f"addiu\t$at,$at,%lo({operand})",
                            f"addu\t$at,$at,{r_source}",
                            f"{op}\t{r_dest},0x0($at)",
                            ".set\tat",
                            "# EXPAND_AT END",
                        ]
                    )
                else:
                    res.append(line)
            elif (
                # LOCAL PATCH 11 (fill_jump_stack_store): ASPSX's reorder
                # pass moves a single-word stack store into the empty delay
                # slot of the plain `j <label>` that follows it:
                #   sw $0,0($sp) / j L / nop  ->  j L / sw $0,0($sp)
                (self.fill_jump_stack_store or self.fill_jump_reg_store)
                and op in ("sb", "sh", "sw")
                and not is_macro  # HJ4
                and r_source
                and (
                    canon_reg(r_source) == 29  # HJ2: $sp base only
                    if not self.fill_jump_reg_store
                    # K6: any base except $at; value not $at either
                    else canon_reg(r_source) != 1 and canon_reg(r_dest) != 1
                )
                and re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)
                and -32768 <= int(operand, 0) <= 32767  # HJ3: one word
                and (jump := self._next_line_is_label_jump()) is not None
            ):
                res.extend(
                    [
                        "# FILL_JUMP_STACK_STORE START",
                        jump,
                        line,
                        "# FILL_JUMP_STACK_STORE END",
                    ]
                )
                self.skip_instructions = 1
            elif (
                # LOCAL PATCH 18 (fill_call_volatile_store): the `jal`
                # sibling of patch 11. ASPSX's reorder pass moves the
                # single-word register-based store that immediately precedes
                # a reorder-mode direct `jal <symbol>` into its empty slot:
                #   sw $3,0($2) / jal F / nop  ->  jal F / sw $3,0($2)
                # cc1 leaves that slot empty only when the store is volatile
                # (reorg never slots a volatile insn).
                self.fill_call_volatile_store
                and op in ("sb", "sh", "sw")
                and not is_macro  # HC4
                and r_source
                and isinstance(canon_reg(r_source), int)
                # HC1: `jal` writes $31 before the slot runs; a moved store
                # that reads $31 (value or base) would see the new link.
                and canon_reg(r_source) not in (31, 1)
                and canon_reg(r_dest) not in (31, 1)
                and re.match(r"^-?(0x[0-9A-Fa-f]+|\d+)$", operand)
                and -32768 <= int(operand, 0) <= 32767  # HC3: one word
                and self.is_reorder  # HC5: no cc1 noreorder block
                and (call := self._next_line_is_plain_call()) is not None
                and call[1] is None  # HC2: direct `jal <symbol>` only
                and call[0].split()[0] == "jal"
            ):
                res.extend(
                    [
                        "# FILL_CALL_VOLATILE_STORE START",
                        call[0],
                        line,
                        "# FILL_CALL_VOLATILE_STORE END",
                    ]
                )
                self.skip_instructions = 1
            elif r_source and (int(operand, 0) > 32767 or int(operand, 0) < -32768):
                # e.g. sw	$2,56200($4)
                res.extend(
                    [
                        "# EXPAND_AT START",
                        ".set\tnoat",
                        f"lui\t$at,%hi({operand})",
                        f"addu\t$at,{r_source},$at",
                        f"{op}\t{r_dest},%lo({operand})($at)",
                        ".set\tat",
                        "# EXPAND_AT END",
                    ]
                )
            else:
                res.append(line)

        elif op in branch_mnemonics or op in jump_mnemonics:
            res.append(line)
            if self.is_reorder:
                # LOCAL PATCH 6: opt-in fill from the following instruction
                # when that instruction's destination is provably dead at the
                # branch target; otherwise the stock nop.
                fill = self._branch_fill_candidate(line)
                if fill is not None:
                    res.extend(
                        [
                            "# FILL_BRANCH_DELAY_SLOT START",
                            fill,
                            "# FILL_BRANCH_DELAY_SLOT END",
                        ]
                    )
                    self.skip_instructions = 1
                else:
                    res.append("nop  # DEBUG: branch/jump")

        elif op == "move":
            # expand move $2,$16 to addu $2,$16,$zero
            res.append(expand_move(line))

        elif op in ("addu", "subu", "sra", "srl", "srr", "sll", "or"):
            # no extra processing required
            res.append(line)
            # TODO: check if this line is a macro and insert a nop if required...

        elif op == "li":
            # TODO: handle non-soft floats?
            small = None
            if self.ori_small_li:
                match = re.match(
                    r"li\s+(\$[0-9A-z]+),\s?(-?[x0-9a-fA-F]+)", line
                )
                if match:
                    imm = int(match.group(2), 0)
                    if 0 < imm < 0x8000:
                        small = f"ori\t{match.group(1)},$zero,{imm}"
            if small is not None:
                res.append(small)
            elif self.expand_li:
                res += expand_load_immediate(line)
            else:
                res.append(line)

        elif op == "li.s":
            res += load_immediate_single(line)

        elif op == "li.d":
            res += load_immediate_double(line)

        elif op in ("mflo", "mfhi"):
            res.append(line)
            res += self._handle_mflo_mfhi()

        elif op == "break":
            # turn 'break 7' into 'break 0x0,0x7'
            num = int(rest[0], 0)
            line = f"break\t0x{num >> 10:X},0x{num & 0x3FF:X}"
            res.append(line)

        elif op in ("div", "rem"):
            r_dest, r_source, r_operand = rest[0].split(",")
            if r_dest in ("$zero", "$0"):
                # e.g. div $zero, $v0, $a0
                return [line]

            move_from = "mfhi" if op == "rem" else "mflo"
            if self.expand_div:
                res.extend(
                    [
                        "# EXPAND_DIV START",
                        ".set\tnoat",
                        f"div\t$zero,{r_source},{r_operand}",
                        f"bnez\t{r_operand},.L_NOT_DIV_BY_ZERO_{self.line_index}",
                        "nop",
                        "break\t0x7",
                        f".L_NOT_DIV_BY_ZERO_{self.line_index}:",
                        "addiu\t$at,$zero,-1",
                        f"bne\t{r_operand},$at,.L_DIV_BY_POSITIVE_SIGN_{self.line_index}",
                        "lui\t$at,0x8000",
                        f"bne\t{r_source},$at,.L_DIV_BY_POSITIVE_SIGN_{self.line_index}",
                        "nop",
                        "tge\t$zero,$zero,93" if self.div_uses_tge else "break\t0x6",
                        f".L_DIV_BY_POSITIVE_SIGN_{self.line_index}:",
                        f"{move_from}\t{r_dest}",
                        ".set\tat",
                        "# EXPAND_DIV END",
                    ]
                )
            else:
                res.extend(
                    [
                        "# EXPAND_ZERO_DIV START",
                        f"div\t$zero,{r_source},{r_operand}",
                        f"{move_from}\t{r_dest}",
                        "# EXPAND_ZERO_DIV END",
                    ]
                )

            extra_nops = self._handle_mflo_mfhi(r_source=r_dest)
            if len(extra_nops) > 0:
                res += extra_nops
            elif self.div_no_reuse_nop and self.expand_div:
                # LOCAL PATCH 16: mflo/mfhi results are available to the next
                # instruction; no load-delay-style reuse nop.
                res.append(
                    f"#nop # DEBUG: DIV_NO_REUSE_NOP: no reuse nop after {r_dest}"
                )
            else:
                next_instruction = self.get_next_instruction(
                    skip=0, ignore_set=True, ignore_label=True
                )
                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

        elif op in ("divu", "remu"):
            r_dest, r_source, r_operand = rest[0].split(",")
            if r_dest in ("$zero", "$0"):
                # e.g. divu $zero, $v1, $a2
                return [line]

            move_from = "mfhi" if op == "remu" else "mflo"
            if self.expand_div:
                res.extend(
                    [
                        "# EXPAND_DIVU START",
                        ".set\tnoat",
                        f"divu\t$zero,{r_source},{r_operand}",
                        f"bnez\t{r_operand},.L_NOT_DIV_BY_ZERO_{self.line_index}",
                        "nop",
                        "break\t0x7",
                        f".L_NOT_DIV_BY_ZERO_{self.line_index}:",
                        f"{move_from}\t{r_dest}",
                        ".set\tat",
                        "# EXPAND_DIVU END",
                    ]
                )
            else:
                res.extend(
                    [
                        "# EXPAND_ZERO_DIVU START",
                        f"divu\t$zero,{r_source},{r_operand}",
                        f"{move_from}\t{r_dest}",
                        "# EXPAND_ZERO_DIVU END",
                    ]
                )

            extra_nops = self._handle_mflo_mfhi(r_source=r_dest)
            if len(extra_nops) > 0:
                res += extra_nops
            elif self.div_no_reuse_nop and self.expand_div:
                # LOCAL PATCH 16: mflo/mfhi results are available to the next
                # instruction; no load-delay-style reuse nop.
                res.append(
                    f"#nop # DEBUG: DIV_NO_REUSE_NOP: no reuse nop after {r_dest}"
                )
            else:
                next_instruction = self.get_next_instruction(
                    skip=0, ignore_set=True, ignore_label=True
                )
                extra_nops = self._handle_nop_before_next_instruction(
                    next_instruction, r_dest
                )
                res.extend(extra_nops)

        elif op == "sltu":
            r_dest, r_source, r_operand = rest[0].split(",")
            if re.match(r"^-?\d+$", r_operand) or re.match(
                r"^-?0x[A-Fa-f0-9]+$", r_operand
            ):
                value = int(r_operand)
                if self.sltu_at and value < 0:
                    res.append(f"li\t$at,{r_operand}")
                    res.append(f"{op}\t{r_dest},{r_source},$at")
                else:
                    # TODO: do we want to expand sltu into sltiu?
                    res.append(line)
            else:
                res.append(line)

        else:
            res.append(line)

        return res
