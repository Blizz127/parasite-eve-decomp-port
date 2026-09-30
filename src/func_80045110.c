/*
 * func_80045110 — window teardown with optional extra list (retail 0x80045110).
 *
 * VRAM 0x80045110 / file 0x35910 / size 0xC0 (48 words). Sits between
 * func_80044E98 (0x35698) and func_800451D0 (0x359D0) inside the 35698.s run.
 *
 * When arg1 is non-zero: release the gp-relative handles at D_8009CE54 + 0xBC
 * (0x1A0($gp)) and +0xB0 (0x194($gp)) via func_80052E30/func_80057D30, run
 * func_80055760, forward arg0->unk4->unk4 to func_80062F1C, then attach two
 * windows returned by func_80062A34(2, 1) and — when the mode word
 * D_8009CE54 + 0xB8 (0x19C($gp)) equals 2 — func_80062A34(2, 0x33).
 *
 * Build: era -O2 -G8 (scalar declarations keep the three gp slots
 * small-data-relative).
 */
extern int D_8009CE54;
extern void func_80052E30(int);
extern void func_80057D30(int);
extern void func_80055760(void);
extern void func_80062F1C(int);
extern int func_80062A34(int, int);
extern int func_800647D0(int, int);
extern int func_80052F70(void);
extern int func_80058C4C(int);

void func_80045110(char *arg0, int arg1) {
    int t;

    if (arg1 != 0) {
        func_80052E30(*(int *)((char *)&D_8009CE54 + 0xBC));
        func_80057D30(*(int *)((char *)&D_8009CE54 + 0xB0));
        func_80055760();
        func_80062F1C(*(int *)(*(int *)(arg0 + 4) + 4));
        t = func_80062A34(2, 1);
        if (t != 0) {
            func_800647D0(t, func_80052F70());
        }
        if (*(int *)((char *)&D_8009CE54 + 0xB8) == 2) {
            t = func_80062A34(2, 0x33);
            if (t != 0) {
                func_800647D0(t, func_80058C4C(0x3803FE));
            }
        }
    }
}
