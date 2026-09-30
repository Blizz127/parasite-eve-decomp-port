/*
 * func_80035E04 — VRAM 0x80035E04 / file 0x26604 / size 0x150 (84 words).
 * Per-actor update step. If D_8009D1A0 bit 0x100 is set it just republishes
 * the actor via func_800361F4. Otherwise it snapshots the pose (+0x28.. into
 * +0x40.. and the +0x38 halfwords into +0x50..), republishes, optionally
 * integrates the +0x88/+0x8C/+0x90 delta when +0x98 bit 1 is set, then
 * integrates the +0x78/+0x7C/+0x80 and +0x58/+0x5C/+0x60 deltas into the
 * pose. D_8009D1A0 is loaded absolutely, so it stays an incomplete array.
 * Build: era -O2 -G8.
 */

typedef struct A A;
struct A {
    char pad00[0x28];             /* 0x00..0x27 */
    int f28, f2C, f30;            /* 0x28,0x2C,0x30 */
    char pad34[4];                /* 0x34..0x37 */
    unsigned short f38, f3A, f3C; /* 0x38,0x3A,0x3C */
    char pad3E[2];                /* 0x3E..0x3F */
    int f40, f44, f48;            /* 0x40,0x44,0x48 */
    char pad4C[4];                /* 0x4C..0x4F */
    unsigned short f50, f52, f54; /* 0x50,0x52,0x54 */
    char pad56[2];                /* 0x56..0x57 */
    int f58, f5C, f60;            /* 0x58,0x5C,0x60 */
    char pad64[4];                /* 0x64..0x67 */
    int f68, f6C, f70;            /* 0x68,0x6C,0x70 */
    char pad74[4];                /* 0x74..0x77 */
    int f78, f7C, f80;            /* 0x78,0x7C,0x80 */
    char pad84[4];                /* 0x84..0x87 */
    int f88, f8C, f90;            /* 0x88,0x8C,0x90 */
    char pad94[4];                /* 0x94..0x97 */
    unsigned int f98;             /* 0x98 */
};

extern unsigned int D_8009D1A0[];
extern void func_800361F4(A *a);

void func_80035E04(A *a) {
    if ((D_8009D1A0[0] & 0x100u) != 0) {
        func_800361F4(a);
        return;
    }
    a->f40 = a->f28;
    a->f44 = a->f2C;
    a->f48 = a->f30;
    a->f50 = a->f38;
    a->f52 = a->f3A;
    a->f54 = a->f3C;
    func_800361F4(a);
    if ((a->f98 & 2u) != 0) {
        a->f68 += a->f88;
        a->f6C += a->f8C;
        a->f70 += a->f90;
    }
    a->f68 += a->f78;
    a->f6C += a->f7C;
    a->f70 += a->f80;
    a->f28 += a->f68;
    a->f2C += a->f6C;
    a->f30 += a->f70;
    a->f28 += a->f58;
    a->f2C += a->f5C;
    a->f30 += a->f60;
}
