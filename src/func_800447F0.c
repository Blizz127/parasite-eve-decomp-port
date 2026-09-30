/*
 * func_800447F0 — VRAM 0x800447F0 / file 0x34FF0 / size 0x134 (77 words).
 * Menu row/scroll update. Gets the func_80062A34(2,1) state and the
 * func_8005DA8C(1) cursor record, calls func_80063158(act, 0, ...) with the
 * (state+0x38 << 4) + 4 biased cursor, then depending on
 * (state+0x58 - state+0x38) - state+0x5C (0 or 1) re-anchors the cursor and
 * finally paints five func_8005E8A4/func_8005FA3C rows.
 * Build: era -O2 -G8.
 */

typedef struct S S;
struct S {
    char pad00[0x38];             /* 0x00..0x37 */
    int f38;                      /* +0x38 */
    char pad3C[0x1C];             /* 0x3C..0x57 */
    int f58;                      /* +0x58 */
    int f5C;                      /* +0x5C */
    int f60;                      /* +0x60 */
    char pad64[4];                /* 0x64..0x67 */
    int f68;                      /* +0x68 */
};

extern S *func_80062A34(int a, int b);
extern int *func_8005DA8C(int a);
extern void func_80063158(int *act, int a, int b);
extern void func_8005E8A4(int a, int b);
extern int func_8005DC4C(int a);
extern void func_8005F27C(int a);
extern int func_8005401C(void);
extern void func_8005FA3C(int a);
extern void func_8005EB64(int a);
extern int func_80052F70(void);

void func_800447F0(int *act) {
    S *s0;
    int *p;
    int v1;
    int a2;

    s0 = func_80062A34(2, 1);
    p = func_8005DA8C(1);
    v1 = s0->f38 << 4;
    v1 = v1 + 4;
    a2 = p[1] + v1 - act[7];
    func_80063158(act, 0, a2);
    if (s0->f68 != 0) {
        v1 = (s0->f58 - s0->f38) - s0->f5C;
        switch (v1) {
        case 0:
            func_80063158(act, 0, s0->f60 - 0x10);
            break;
        case 1:
            a2 = s0->f60;
            if (a2 < 0) {
                func_80063158(act, 0, a2);
            }
            break;
        }
    }
    func_8005E8A4(0xA, 2);
    func_8005F27C(func_8005DC4C(0x25));
    func_8005E8A4(0x46, 4);
    func_8005FA3C(func_8005401C());
    func_8005EB64(0x4C);
    func_8005E8A4(5, 0);
    func_8005FA3C(func_80052F70());
}
