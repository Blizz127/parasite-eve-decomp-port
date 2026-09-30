/*
 * func_80068CE0 — six-stage subsystem refresh (retail 0x80068CE0).
 *
 * VRAM 0x80068CE0 / file 0x594E0 / size 0x48 (18 words), in 5747C.s
 * (the run that resumes after func_80066BD8).
 *
 * Calls func_80066CE8, func_80065674, func_80067E1C, func_80067A78,
 * func_80067B74, func_80067D18 in order and returns 0. Frame -0x18 with
 * only $ra saved at 0x10; the `return 0` lands in the epilogue and the
 * frame restore sits in the `jr` delay slot.
 *
 * era -O2 -G0 (also byte-identical on -O1 -G0).
 */
extern void func_80066CE8(void);
extern void func_80065674(void);
extern void func_80067E1C(void);
extern void func_80067A78(void);
extern void func_80067B74(void);
extern void func_80067D18(void);

int func_80068CE0(void) {
    func_80066CE8();
    func_80065674();
    func_80067E1C();
    func_80067A78();
    func_80067B74();
    func_80067D18();
    return 0;
}
