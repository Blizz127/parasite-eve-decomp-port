/* room_m0273i — func_80194E6C, blob offset 0x5E84, 0x77C bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Written from the retail disassembly (asm/overlays/room_m0273i/5E84.s).
 * Slime (m0273i) drip/trail particle callback; a1 = particle record:
 *   +0..+4 position (x,y,z), +8/+A/+C velocity-angle words, +0xE landed flag,
 *   +0x10..+0x2F x trail, +0x30..+0x4F z trail, +0x50..+0x5F y trail
 *   (8 shorts each, ring index +0x60, fill count +0x62).
 * Mode 1 (update): while landed (+0xE) it counts the trail down (+0x62) and
 * returns 1 when it reaches zero.  Otherwise it spins the angle (+8/+A,
 * +0xC grows away from zero by 2), builds a RotMatrix from {+8, +A, 0},
 * rotates the D_8019ACA4 vector with it (translation = position, GTE mvmva)
 * into the position, lands on the floor line D_800942EC - 0x80, or within
 * 0x140 of the player (D_8009D254 +0x1FC/+0x204; then averages the position
 * with the player's +0x238 record into D_8019AEAC, sets D_8019AEB2, the
 * player flag 0x4000 and the hit latch bit), logs the landing point into the
 * D_8019AEB8/C8/D8 ring, and pushes two rotated trail points (D_8019ACAC /
 * D_8019ACB4) into the record's ring.
 * Mode 2 (draw): state 0 (not landed) draws the glow head at the particle
 * (sprite size from the D_800966EC curve of age D_800E27EC); then both trail
 * rows (2 points each) are drawn with fading brightness.
 * Layout: each ring slot owns two x shorts at +0x10+4*ring and two z shorts
 * at +0x30+4*ring (the pair of rotated trail points), one y short at
 * +0x50+2*ring.  Retail quirk kept: the draw descriptor y (sp+0x5A) is never
 * reset between ring entries, only advanced by 0x800 per trail point.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
/* ldv0 + rtv0-style mvmva(sf=1, mx=rot, v=V0, cv=TR, lm=0; word 0x4A480012) + stsv into dst */
#define gte_ldv0(r0) __asm__ volatile ( \
    "lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0) : "memory")
#define gte_mvmva() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%0)\n\tsh $13,2(%0)\n\tsh $14,4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern short D_800942EC;
extern unsigned char *D_8009D254;
extern int *D_800F32D0;
extern unsigned char D_8019ACA4[];
extern unsigned char D_8019ACAC[];
extern unsigned char D_8019ACB4[];
extern unsigned char D_8019ACBC[];
extern short D_8019AEEC;
extern short D_8019AEAC[];
extern short D_8019AEB2;
extern unsigned short D_8019AEE8;
extern unsigned short D_8019AEB8[];
extern unsigned short D_8019AEC8[];
extern unsigned short D_8019AED8[];
extern unsigned char D_800966EC[];
extern unsigned short D_800E11E8;
extern unsigned short D_800E11EA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3376;
extern short D_800F3378;

extern int func_8005186C();
extern int func_80077AA4();
extern void func_80079754();
extern void func_800CEE20();

int func_80194E6C(int mode, unsigned char *a1)
{
    MATRIX m;
    GteShortVector r;      /* sp+0x48: rotation angles / rotated trail point */
    GteShortVector t;      /* sp+0x50: trail point handed to the draw call   */
    GteShortVector d;      /* sp+0x58: draw descriptor                       */
    int i, s1, s3, s4, s6, s7, s5, s2, dx, dz, v, n, idx;
    int w;
    unsigned char *p;
    int *o;

    if (mode == 1) {
        if (SH(a1, 0xE) != 0) {
            unsigned short c = H(a1, 0x62) - 1;
            H(a1, 0x62) = c;
            return (int)((unsigned int)c << 16) < 1;
        }
        n = SH(a1, 0xC);
        if (n != 0) {
            int c;
            H(a1, 0xA) = H(a1, 0xA) + n;
            c = SH(a1, 0xC);
            if (c >= 0)
                SH(a1, 0xC) = c + 2;
            else
                SH(a1, 0xC) = c - 2;
        }
        r.x = H(a1, 8);
        r.z = 0;
        r.y = H(a1, 0xA);
        func_80079754(&r, &m);
        for (i = 0; i < 3; i++)
            m.t[i] = ((short *)a1)[i];
        gte_SetRotMatrix(&m);
        gte_SetTransMatrix(&m);
        gte_ldv0(D_8019ACA4);
        gte_mvmva();
        gte_stsv(a1);
        if (!(SH(a1, 2) < D_800942EC - 0x80)) {
            SH(a1, 2) = D_800942EC;
            SH(a1, 0xE) = 1;
        }
        if (!(SH(a1, 2) < D_8019AEEC)) {
            if (!(D_8009D254[0xE] < 4)) {
                dx = *(int *)(D_8009D254 + 0x1FC) - SH(a1, 0);
                dz = *(int *)(D_8009D254 + 0x204) - SH(a1, 4);
                if (func_8005186C(dx * dx + dz * dz) < 0x140) {
                    p = D_8009D254;
                    SH(a1, 0xE) = 1;
                    for (i = 0; i < 3; i++) {
                        int a = ((short *)a1)[i];
                        v = *(int *)(*(unsigned char **)(p + 0x238) + i * 4 + 0x14);
                        D_8019AEAC[i] = (v + a) / 2;
                    }
                    D_8019AEB2 = 1;
                    p = *(unsigned char **)D_8009D254;
                    *(int *)(p + 0x4C) |= 0x4000;
                    o = *(int **)D_800F32D0[2];
                    if (o != 0)
                        *o |= 0x80000000;
                }
            }
        }
        if (SH(a1, 0xE) != 0) {
            unsigned short k = D_8019AEE8;
            D_8019AEE8 = k + 1;
            D_8019AEB8[(short)k] = H(a1, 0);
            D_8019AEC8[(short)k] = H(a1, 2);
            D_8019AED8[(short)k] = H(a1, 4);
        }
        idx = (H(a1, 0x60) - 1) & 7;
        H(a1, 0x60) = idx;
        gte_ldv0(D_8019ACAC);
        gte_mvmva();
        gte_stsv(&r);
        H(a1, 0x50 + idx * 2) = r.y;
        H(a1, 0x10 + idx * 4) = r.x;
        H(a1, 0x30 + idx * 4) = r.z;
        gte_ldv0(D_8019ACB4);
        gte_mvmva();
        gte_stsv(&r);
        H(a1, 0x10 + idx * 4 + 2) = r.x;
        H(a1, 0x30 + idx * 4 + 2) = r.z;
        n = SH(a1, 0x62);
        if (n < 8)
            H(a1, 0x62) = n + 1;
        return 0;
    }
    if (mode != 2)
        return 0;
    if (SH(a1, 0xE) == 0) {
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        d.x = H(a1, 8) + 0x400;
        d.z = -0x400;
        d.w = 1;
        d.y = H(a1, 0xA);
        if (D_800E27EC < 0x11) {
            v = *(short *)(D_800966EC + ((D_800E27EC << 8) & 0x3F00));
            s4 = (v * 0x1800) / 4096 + 0x800;
        } else {
            s4 = 0x2000;
        }
        w = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0)
            w += 4;
        func_800CEE20(a1, &d, (short)s4, 0x1000, 0xC,
                      func_80077AA4(0x30, w) & 0xFFFF, 1, 0x80, 0);
        s3 = 0x80;
    } else {
        s3 = SH(a1, 0x62) << 4;
    }
    s4 = ((0x80 - (short)s3) << 6) + 0x1000;
    D_800F336A = 1;
    D_800F3368 = 0x10;
    D_800F3376 = 0x10;
    D_800F3378 = 0x10;
    D_800F336C = 2;
    D_800F336E = 0;
    d.x = 0;
    d.y = 0;
    d.z = D_800E27EC << 8;
    d.w = 0;
    D_800F3370 = D_800E2850[D_800E11E8];
    s6 = H(a1, 0x60);
    s7 = 0;
    if (SH(a1, 0x62) > 0) {
        do {
            s1 = 0;
            s5 = (short)s6 * 2;
            t.y = H(a1, 0x50 + s5);
            s2 = (short)s4;
            do {
                int off = (s5 + (short)s1) * 2; /* 4*ring + 2*k bytes */
                t.x = H(a1, 0x10 + off);
                t.z = H(a1, 0x30 + off);
                w = D_800E1204[D_800F336C];
                if (D_800F336C == 4 && D_800F3428 != 0)
                    w += 4;
                func_800CEE20(&t, &d, s2, s2, 0xDC,
                              func_80077AA4(0x30, w) & 0xFFFF, 1, (short)s3,
                              D_8019ACBC);
                s1 = s1 + 1;
                d.y = d.y + 0x800;
            } while ((short)s1 < 2);
            s3 -= 0x10;
            s4 += 0x400;
            s7 = s7 + 1;
            d.z = d.z + 0x100;
            s6 = (s6 + 1) & 7;
        } while ((short)s7 < SH(a1, 0x62));
    }
    return 0;
}
