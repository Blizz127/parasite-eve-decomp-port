/*
 * func_80035C84 — per-actor pose snapshot, view-code publish, motion integrate.
 *
 * VRAM 0x80035C84 / file 0x26484 / size 0x180 (96 words), inside 25838.s.
 *
 * Snapshots the pose (+0x28/+0x2C/+0x30 and the +0x38 halfwords into
 * +0x40.. and +0x50..), republishes the actor via func_800361F4, then (when
 * D_8009D2E8 bit 0 is clear) derives the view code from (*D_8009D254)->0x4C
 * and calls func_8003999C with the D_800943C0 pad table. Finally it
 * integrates the +0x88/+0x8C/+0x90 delta when +0x98 bit 1 is set, the
 * +0x78/+0x7C/+0x80 delta, and the +0x58/+0x5C/+0x60 delta into the pose,
 * returning the updated +0x2C value.
 *
 * Build: era -O2 -G8 + aspsx 2.30. Levers:
 *  - D_8009D2E8 is the gp+0x578 scalar; D_8009D254 is the gp+0x4E4
 *    pointer-to-pointer ((*D_8009D254)->0x4C, so `int **`).
 *  - D_800943C0 stays ABSOLUTE via an incomplete array (it is larger than
 *    the -G8 small-data window).
 *  - The view-code mask must be staged in a local `v` before the unconditional
 *    `sp10 = a->f0E`, which reproduces retail's load order (pointer chain
 *    first, then the byte).
 *  - The final +0x2C update result must be kept in a local (`r = a->f2C +
 *    a->f5C; a->f2C = r; ...; return r;`) — `return a->f2C;` makes cc1 emit a
 *    fresh load instead of reusing the stored value.
 */
typedef struct A A;
struct A {
    char pad00[0x0E];
    unsigned char f0E;
    char pad0F[0x19];
    int f28, f2C, f30;
    char pad34[4];
    unsigned short f38, f3A, f3C;
    char pad3E[2];
    int f40, f44, f48;
    char pad4C[4];
    unsigned short f50, f52, f54;
    char pad56[2];
    int f58, f5C, f60;
    char pad64[4];
    int f68, f6C, f70;
    char pad74[4];
    int f78, f7C, f80;
    char pad84[4];
    int f88, f8C, f90;
    char pad94[4];
    unsigned int f98;
};

extern int D_8009D2E8;
extern int **D_8009D254;
extern int D_800943C0[];
extern void func_800361F4(A *a);
extern void func_8003999C(A *actor, int *table, int *codep);

int func_80035C84(A *a) {
    int sp10;
    int r;

    a->f40 = a->f28;
    a->f44 = a->f2C;
    a->f48 = a->f30;
    a->f50 = a->f38;
    a->f52 = a->f3A;
    a->f54 = a->f3C;
    func_800361F4(a);
    if ((D_8009D2E8 & 1) == 0) {
        int v = *(int *)((char *)*D_8009D254 + 0x4C);
        sp10 = a->f0E;
        if ((v & 0xC0) == 0x80) {
            sp10 = 0x11;
        }
        func_8003999C(a, D_800943C0, &sp10);
    }
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
    r = a->f2C + a->f5C;
    a->f2C = r;
    a->f30 += a->f60;
    return r;
}
