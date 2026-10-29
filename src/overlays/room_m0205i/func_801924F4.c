/* room_m0205i (PE.IMG room m0205i chunk 2, VRAM 0x8018EFE8)
 * func_801924F4 — blob offset 0x350C, 0x3F8 bytes. Profile era_o2_g0 (default).
 * Room effect callback (cmd 0/1/2 shape; cmd 2 draws two func_800CEE20 sprites
 * scaled by func_80077DC4). Twin: room_m0231i func_80192548 (masked-body port).
 * Levers (the lane ovl5 park, 11 words, closed 2026-09-30):
 *   - the 4-byte colour struct as unsigned char[4]-shaped S2 (block move with an
 *     addiu base); sv declared before t; one q = D_800F32D0 local;
 *   - the cmd-2 scale `(func_80077DC4(h6) / 4 + 0x1000) * s1 / 4096` goes into
 *     its own variable s3 (the divide temp lands in $a2 and the sra fills the
 *     bne delay slot);
 *   - the first texture lookup reads D_800F336C directly, the second through k.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char a, b, c, d; } S2;
extern S2 D_8018F1CC;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_8009D254;
extern int D_800E27EC;
extern short D_800942EC;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800CF844();
extern int func_800C6B90();
extern unsigned short func_80077AA4();
extern void func_800CEE20();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

int func_801924F4(int a0, unsigned char *o, int *a2)
{
    SVECTOR sv;
    S2 t;
    int c;
    int s1;
    int s3;
    int s2;
    int k;
    int v;
    int *p;
    unsigned char *q;

    t = D_8018F1CC;
    switch (a0) {
    case 1:
        SH(o, 0x14) += a2[0];
        SH(o, 0x16) += a2[1];
        SH(o, 0x12) += a2[2];
        c = func_80077CF4(SH(o, 0x12)) * SH(o, 0x16) / 4096;
        sv.vx = 0;
        sv.vy = SH(o, 0x10);
        sv.vz = 0;
        func_800CF844(o, o + 8, SH(o, 0x14), &sv, c, 0);
        if (func_800C6B90(o + 8, 100) && D_800E2368[0xD] != 0
            && (**(int **)P(q = D_800F32D0, 8) & 0x3F000000) == 0x1000000) {
            W(*(void **)D_8009D254, 0x4C) |= 0x4000;
            p = *(int **)P(q, 8);
            *p = (*p & 0xC0FFFFFF) | 0x21000000;
            p = *(int **)P(q, 8);
            *p |= 0x80000000;
        }
        if (D_800E27EC >= 0x40) {
            return 1;
        }
        break;
    case 2:
        s2 = func_80077DC4(D_800E27EC << 4) / 32;
        s2 = SH(o, 0xE) * s2 / 128;
        s1 = (D_800E27EC << 8) + 0x1000;
        s3 = (func_80077DC4(SH(o, 6)) / 4 + 0x1000) * s1 / 4096;
        v = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            v += 4;
        }
        func_800CEE20(o + 8, 0, s3, s3, D_800F336A * ((D_800E27EC / 4) & 1) + 0xFD,
                      func_80077AA4(0x80, v), 1, s2, &t);
        sv.vx = 0x400;
        sv.vy = 0;
        sv.vz = 0;
        sv.pad = 1;
        SH(o, 0xA) = D_800942EC;
        k = D_800F336C;
        v = D_800E1204[k];
        if (k == 4 && D_800F3428 != 0) {
            v += 4;
        }
        func_800CEE20(o + 8, &sv, s3, s3, D_800F336A * ((D_800E27EC / 4) & 1) + 0xFD,
                      func_80077AA4(0x80, v), 2, s2 / 4, 0);
        break;
    }
    return 0;
}
