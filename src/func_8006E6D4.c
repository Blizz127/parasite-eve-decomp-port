/*
 * func_8006E6D4 — CD sector-read issue with mode-mismatch exit (69 words).
 *
 * VRAM 0x8006E6D4, file 0x5EED4, size 0x114. Era gcc-2.7.2-psx -O2 -G0,
 * pinless (no register asm, no barrier, no maspsx knob).
 *
 *   if (D_800B0CD8 & 0x01000000) return -1;        busy/locked
 *   if (func_8007F72C() != 1) return -1;
 *   if (func_8007F778() != 0) return -1;
 *   if (func_8007F7A8() != D_800B0DD4)
 *       func_800719E4(1);                          B(38h) = exit(1)
 *   D_800B0CD8 |= 0x01004000;
 *   lba += off;
 *   func_80080B44(lba, &loc);
 *   status = func_80080E34(&loc, sectors, dest, 0x80);
 *   if (status != 0) return status;                nonzero = issued
 *   D_800B0CD8 &= 0xFEFFBFFF;
 *   func_80071A74(D_8001136C, lba, sectors);
 *   return -1;
 *
 * Match note (the former PARK residual): cc1 re-emits `lui $v1,0x100`
 * after `jal func_800719E4` whenever the call is modelled as returning,
 * because the constant is then live across a call-clobbered register.
 * func_800719E4 is the BIOS trampoline `li $t2,0xB0; jr $t2; li $t1,0x38`
 * (see pc_port/platform/func_800719E4_port.c), i.e. B(38h) =
 * exit(exitcode): see PSXSPX "B-Functions", `B(38h) or A(06h)
 * exit(exitcode)`. Declaring it `__attribute__((noreturn))` is therefore
 * the true signature. The no-return edge makes the merge block single
 * predecessor, so reorg threads retail's `lui` into the `beq` delay slot
 * and deletes the rematerialize - byte-exact. The shared merge block is
 * still reached when the modes match, so the emitted stream is correct
 * even though the callee is fatal (all 12 retail callers discard $v0).
 *
 * Evidence: docs/evidence/func-8006E6D4/REPORT.md.
 * Verified with tools/analysis/era_leaf_match.sh src/func_8006E6D4.c \
 *   0x8006E6D4 0x114 -O2 -G0 -> all 69 words match modulo the 13
 *   link-time relocation words (R_MIPS_26 targets + HI16/LO16 symbol
 *   materializations) and the trimmed GNU as alignment pad.
 */
extern unsigned int D_800B0CD8;
extern unsigned short D_800B0DD4;
extern unsigned char D_8001136C[];

extern int func_8007F72C(void);
extern int func_8007F778(void);
extern int func_8007F7A8(void);
extern void func_800719E4(int) __attribute__((noreturn));
extern void func_80080B44(int lba, unsigned char *loc);
extern int func_80080E34(unsigned char *loc, int sectors, unsigned char *dest,
                         int flags);
extern void func_80071A74(unsigned char *p, int lba, int sectors);

int func_8006E6D4(int lba, int off, unsigned char *dest, int sectors)
{
    unsigned char loc[8];
    int status;
    unsigned int *flagsPtr = &D_800B0CD8;

    if (*flagsPtr & 0x01000000)
        return -1;
    if (func_8007F72C() != 1)
        return -1;
    if (func_8007F778() != 0)
        return -1;
    if (func_8007F7A8() != D_800B0DD4)
        func_800719E4(1);
    *flagsPtr |= 0x01004000;
    lba += off;
    func_80080B44(lba, loc);
    status = func_80080E34(loc, sectors, dest, 0x80);
    if (status != 0)
        return status;
    *flagsPtr &= 0xFEFFBFFF;
    func_80071A74(D_8001136C, lba, sectors);
    return -1;
}
