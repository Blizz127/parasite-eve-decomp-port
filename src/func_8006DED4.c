/*
 * func_8006DED4 — three-point spline/colour builder that packs six shorts into
 * a stack record and forwards to func_8006DF50 via func_8006DFA8.
 *
 * VRAM 0x8006DED4 / file 0x5E6D4 / size 0x7C (31 words).
 *
 * Retail (asm/disc1/5E39C.s):
 *   short buf[?];  buf[0..2] = { a3, a4, a5 };          (sp+0x18,0x1A,0x1C)
 *   func_8006DFA8(sp+0x18, sp+0x20, sp+0x24);
 *   struct r = { *(int *)(sp+0x20), (short)*(int *)(sp+0x24) };
 *   return func_8006DF50(a0, a1, a2, r.a, r.b);
 *
 * Build: era -O2 -G0.
 * ROM: asm/disc1/5E39C.s @ file 0x5E6D4, 31 words (0x7C bytes).
 */

extern void func_8006DFA8(short *a0, int *a1, int *a2);
extern int func_8006DF50(int a0, int a1, int a2, int a3, int a4);

int func_8006DED4(int a0, int a1, int a2, int a3, unsigned short a4, unsigned short a5) {
    short buf[4];
    int v0;
    int v1;

    buf[0] = (short)a3;
    buf[1] = (short)a4;
    buf[2] = (short)a5;
    func_8006DFA8(buf, &v0, &v1);
    return func_8006DF50(a0, a1, a2, v0, v1);
}
