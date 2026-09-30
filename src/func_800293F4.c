/* HP clamp/copy + record flag storm.
 * VRAM 0x800293F4 / file 0x19BF4 / size 0x1F0 (124 words).
 * gcc-2.7.2-psx -O2 -G8 + maspsx 2.21 --dont-expand-li.
 *
 * gp-relative: D_8009D278 (record*), D_8009D1D0, D_8009D244.
 * lui: D_8009D234, D_8009D2E8, D_8009D298/9A/9B/9C.
 *
 * Separate rec temporaries so each D278 load can take a different
 * register (ROM: a1, v1, a1, a0, v1, v0). register asm pins r2/r3/flags
 * to a1/a0/v0. D_8009D2E8 is a 4-byte scalar whose ROM addressing is
 * 2-word lui/lw + lui/$at sw; era_compile strips its sdata `.extern`
 * (`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2E8`) so GNU as does not emit
 * gp-relative RMW.
 */
extern unsigned char *D_8009D278;
extern unsigned int D_8009D1D0;
extern unsigned char D_8009D244;

extern unsigned char D_8009D234[];
extern short D_8009D298[];
extern unsigned char D_8009D29A[];
extern unsigned char D_8009D29B[];
extern unsigned int D_8009D29C[];

extern unsigned int D_8009D2E8;

extern void func_80021D4C(void);
extern void func_800374E8(void);

void func_800293F4(int a0) {
    unsigned char *r0;
    unsigned char *r1;
    register unsigned char *r2 asm("$5");
    register unsigned char *r3 asm("$4");
    unsigned char *r4;
    unsigned char *r5;
    short max_hp;
    short cur_hp;
    unsigned short hp;
    register unsigned int flags asm("$2");

    r0 = D_8009D278;
    max_hp = *(short *)(r0 + 0x1C);
    cur_hp = *(short *)(r0 + 0x0C);
    if (max_hp < cur_hp)
        *(short *)(r0 + 0x0C) = max_hp;

    r1 = D_8009D278;
    r1[0x12] = 4;

    r2 = D_8009D278;
    hp = *(unsigned short *)(r2 + 0x0C);
    D_8009D1D0 = 0;
    *(short *)(r2 + 0x10) = 0;
    *(unsigned int *)(r2 + 0x34) = 0;
    *(short *)(r2 + 0x0E) = (short)hp;

    if ((a0 & 0xFF) == 1) {
        D_8009D234[0] = 0x5A;
        D_8009D244 = 1;
        flags = *(unsigned int *)(r2 + 0x4C);
        flags |= 0x800000u;
    } else {
        flags = *(unsigned int *)(r2 + 0x4C);
        D_8009D244 = 0;
        flags &= ~0x400000u;
        flags &= ~0x800000u;
    }
    *(unsigned int *)(r2 + 0x4C) = flags;
    /* Keep the join store before the mask hoist; gcc -O2 otherwise delays it. */
    asm volatile("" ::: "memory");

    r3 = D_8009D278;
    r3[0x56] = 0;
    flags = *(unsigned int *)(r3 + 0x4C);
    flags &= ~0x00000003u;
    flags &= ~0x0000000Cu;
    flags &= ~0x00000030u;
    flags &= ~0x000000C0u;
    flags &= ~0x00000100u;
    flags &= ~0x00000200u;
    flags &= ~0x00000400u;
    flags &= ~0x00000800u;
    flags &= ~0x00001000u;
    flags &= ~0x00002000u;
    flags &= ~0x00004000u;
    flags &= ~0x00008000u;
    flags &= ~0x00080000u;
    flags &= ~0x00010000u;
    flags &= ~0x00060000u;
    flags &= ~0x00100000u;
    flags &= ~0x00200000u;
    flags &= ~0x01000000u;
    flags &= ~0x0E000000u;
    flags &= ~0x10000000u;
    flags &= ~0x20000000u;
    r4 = D_8009D278;
    *(unsigned int *)(r3 + 0x4C) = flags;
    r4[0x5E] = 0;
    r5 = D_8009D278;
    r5[0x66] = 0;

    D_8009D2E8 &= ~0x10u;
    func_80021D4C();
    func_800374E8();
    D_8009D298[0] = 0;
    D_8009D29A[0] = 0;
    D_8009D29B[0] = 0;
    D_8009D29C[0] = 0;
}
