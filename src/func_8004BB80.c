/*
 * func_8004BB80 — score/mode gate (retail 0x8004BB80).
 *
 * VRAM 0x8004BB80 / file 0x3C380 / size 0x100 (64 words). Carved out of the
 * [0x3C380, asm] run.
 *
 * Only runs when arg1 bit 0x10000 is set (otherwise returns 1).  The gp
 * score pair lives in the unnamed gap above D_8009CFB0 (gp+0x278/0x27C =
 * D_8009CFB0+0x38/+0x3C), so it is addressed off that scalar.  era -O2 -G8.
 */
extern int D_8009CFB0;
extern int D_800C0E00[];
extern int func_80062F1C();
extern int func_80062A34(int, int);
extern int func_80052764();
extern int func_8005DBF8();
extern int func_8005B8A8(int, int);
extern int func_8004BE4C();
extern int func_8005270C();
extern int func_80057ECC();
extern int func_80048654();
extern int func_800525EC();
extern void func_800512AC(int, int);

int func_8004BB80(int obj, int a1) {
    int lo;
    int hi;
    int s0;

    if (a1 & 0x10000) {
        lo = *(int *)((char *)&D_8009CFB0 + 0x38);
        hi = *(int *)((char *)&D_8009CFB0 + 0x3C);
        if (lo < hi) {
            *(int *)((char *)&D_8009CFB0 + 0x38) = hi;
        } else {
            func_80062F1C();
            func_80062F1C(func_80062A34(1, 0x16));
            func_80052764();
            s0 = func_8005B8A8(*(int *)((char *)&D_8009CFB0 + 0x38),
                               func_8005DBF8());
            if (s0 != func_8005B8A8(D_800C0E00[0], func_8005DBF8())) {
                func_8004BE4C();
                func_8005270C();
            } else if (func_80057ECC()) {
                func_80048654();
            } else {
                func_800512AC(0xA, 0);
            }
            D_800C0E00[0] = *(int *)((char *)&D_8009CFB0 + 0x38);
        }
        func_800525EC();
    }
    return 1;
}
