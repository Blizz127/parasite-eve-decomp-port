/*
 * func_800C2FF0 — sprite/CLUT rectangle corner emitter (retail 0x800C2FF0).
 *
 * VRAM 0x800C2FF0 / file 0xB37F0 / size 0xA8 (42 words), inside B3390.s.
 *
 * Given two byte coordinates, publishes the four 16-bit corners of a
 * rectangle scaled by 0x10 into the D_800F3310..D_800F332C halfword
 * table (a 4x3 array of {neg-x, neg-y, 0} / {x, neg-y, 0} /
 * {neg-x, y, 0} / {x, y, 0} pairs), stores the raw coordinates to
 * D_800F345C/D_800F345D, then overwrites those two bytes with the
 * coordinate minus one. Returns -y*0x10 in $v0.
 *
 * era -O2 -G0. The `(arg & 0xFF) * 0x10` scalings and their negations are
 * computed once into locals; the byte-minus-one values are also locals
 * (retail computes them first and stores them last, in the `jr` shadow
 * region).
 */
extern unsigned char D_800F345C;
extern unsigned char D_800F345D;
extern short D_800F3310;
extern short D_800F3312;
extern short D_800F3314;
extern short D_800F3318;
extern short D_800F331A;
extern short D_800F331C;
extern short D_800F3320;
extern short D_800F3322;
extern short D_800F3324;
extern short D_800F3328;
extern short D_800F332A;
extern short D_800F332C;

int func_800C2FF0(unsigned char arg0, unsigned char arg1) {
    int a0 = (arg0 & 0xFF) * 0x10;
    int a1 = (arg1 & 0xFF) * 0x10;
    int v1 = -a0;
    int v0 = -a1;
    unsigned char n0 = arg0 - 1;
    unsigned char n1 = arg1 - 1;
    D_800F345C = arg0;
    D_800F345D = arg1;
    D_800F3310 = v1;
    D_800F3312 = v0;
    D_800F3314 = 0;
    D_800F3318 = a0;
    D_800F331A = v0;
    D_800F331C = 0;
    D_800F3320 = v1;
    D_800F3322 = a1;
    D_800F3324 = 0;
    D_800F3328 = a0;
    D_800F332A = a1;
    D_800F332C = 0;
    D_800F345C = n0;
    D_800F345D = n1;
    return v0;
}
