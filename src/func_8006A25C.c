/*
 * func_8006A25C — game-over destination saver (retail 0x8006A25C).
 *
 * VRAM 0x8006A25C / file 0x5AA5C / size 0x8C (35 words), inside 56438.s.
 *
 * Runs the sound/CD teardown sequence (81268, 86F34(0), 86FF8, 87024,
 * 85744), and when func_80038D0C() returns a nonzero low byte also calls
 * func_80039970(). Then it snapshots the current destination word
 * D_8009D280 into D_800A77F4, overwrites D_8009D280 with the game-over
 * destination 0xA9400048, and sets bit 0x100 in the flag word
 * D_800B0CD8. Returns nothing ($v0 is left holding the OR result).
 *
 * era -O2 -G0. Shape levers: the `unsigned int *p = &D_800B0CD8` local
 * makes cc1 keep a shared base in $a0 for the read-modify-write (retail
 * `lw $v0,0($a0)` / `sw $v0,0($a0)`), and the tail order must be
 * `D_800A77F4 = t; D_8009D280 = 0xA9400048; *p |= 0x100;` with a void
 * return — the natural `t = ...; D_8009D280 = ...; *p |= ...;
 * D_800A77F4 = t;` order swaps the constant into $v1.
 */
extern unsigned int D_800B0CD8;
extern unsigned int D_8009D280;
extern unsigned int D_800A77F4;
extern void func_80081268(void);
extern void func_80086F34(int);
extern void func_80086FF8(void);
extern void func_80087024(void);
extern void func_80085744(void);
extern int func_80038D0C(void);
extern void func_80039970(void);

void func_8006A25C(void) {
    unsigned int *p;
    unsigned int t;
    func_80081268();
    func_80086F34(0);
    func_80086FF8();
    func_80087024();
    func_80085744();
    if (func_80038D0C() & 0xFF) {
        func_80039970();
    }
    p = &D_800B0CD8;
    t = D_8009D280;
    D_800A77F4 = t;
    D_8009D280 = 0xA9400048;
    *p = *p | 0x100;
}
