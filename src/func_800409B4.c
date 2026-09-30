/* VRAM 0x800409B4 / file 0x311B4 / size 0x1CC (115 words).
 *
 * Card subsystem boot init: guards on D_800A1850, opens the eight card
 * events (func_800726E4) and stores their handles at D_800BCDA8[0..7],
 * runs the card bring-up calls, enables each handle (func_80072704), then
 * clears the two card-record selector bytes D_800A0ED4[0] / [0x418].
 *
 * era_o2_g0_three_word: the trailing clear loop's `D_800A0ED4[v0]` store
 * needs MASPSX_THREE_WORD_SYMBOL_STORE=1 to emit retail's
 * `lui $at,%hi; addu $at,$at,$v0; sb $zero,%lo($at)`.
 *
 * The clear-loop index must be a second variable: reusing the enable-loop
 * variable keeps it in callee-saved $s1 across the calls, but retail uses
 * the call-clobbered $v0. */
extern unsigned char D_800A0ED4[];
extern int D_800A1850;
extern int D_800BCDA8[];
extern void func_80042BD8(void);
extern void func_80042BEC(void);
extern void func_80042C00(void);
extern void func_80042C14(void);
extern void func_80042C28(void);
extern void func_80042C3C(void);
extern void func_80042C50(void);
extern void func_80042C64(void);
extern void func_80072714(void);
extern int func_800726E4(unsigned int cls, unsigned int spec, int mode, void (*handler)(void));
extern void func_8007DDD4(int value);
extern void func_8007DE40(void);
extern void func_800726D4(void);
extern void func_8007DD64(int value);
extern void func_80072704(int handle);
extern void func_80072724(void);

void func_800409B4(void) {
    int i;
    int j;

    if (D_800A1850 == 0) {
        D_800A1850 = 1;
        func_80072714();
        D_800BCDA8[0] = func_800726E4(0xF4000001, 0x4, 0x1000, func_80042BD8);
        D_800BCDA8[1] = func_800726E4(0xF4000001, 0x8000, 0x1000, func_80042BEC);
        D_800BCDA8[2] = func_800726E4(0xF4000001, 0x100, 0x1000, func_80042C00);
        D_800BCDA8[3] = func_800726E4(0xF4000001, 0x2000, 0x1000, func_80042C14);
        D_800BCDA8[4] = func_800726E4(0xF0000011, 0x4, 0x1000, func_80042C28);
        D_800BCDA8[5] = func_800726E4(0xF0000011, 0x8000, 0x1000, func_80042C3C);
        D_800BCDA8[6] = func_800726E4(0xF0000011, 0x100, 0x1000, func_80042C50);
        D_800BCDA8[7] = func_800726E4(0xF0000011, 0x2000, 0x1000, func_80042C64);
        func_8007DDD4(0);
        func_8007DE40();
        func_800726D4();
        func_8007DD64(0);
        for (i = 0; i < 8; i++) {
            func_80072704(D_800BCDA8[i]);
        }
        func_80072724();
    }
    for (j = 0x418; j >= 0; j -= 0x418) {
        D_800A0ED4[j] = 0;
    }
}
