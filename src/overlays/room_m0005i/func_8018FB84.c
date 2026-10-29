/* room_m0005i (PE.IMG room m0005i chunk 2, VRAM 0x8018EFE8)
 * func_8018FB84 — blob offset 0xB9C, 0x240 bytes. Profile era_o2_g0 (-O2 -G0, default).
 * Particle callback: cmd 1 integrates position by velocity and ends once D_800E27EC >= 6;
 * cmd 2 loads the D_800BCFA4[0] rotation/translation and draws the D_80190BA8 sprite at the
 * D_80190BA0 offset with a D_800E27EC-driven blink/size and the D_800E1204 texture.
 * Matched in the default room-function switch shape (lane r005, no extra lever).
 * inline GTE/asm: Psy-Q inline_c.h-shaped COP2 macros (gte_SetRotMatrix/gte_SetTransMatrix) —
 * count separately from plain C. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, flag, vx, vy, vz; } Particle;
typedef struct { char c[4]; } Q4;

extern MATRIX *D_800BCFA4[];
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern unsigned short D_800F336C;
extern int D_800F3428;
extern Q4 D_8018F00C;
extern char D_80190BA8[];
extern unsigned short D_80190BA0, D_80190BA2, D_80190BA4;
extern int func_80077AA4();
extern int func_80077DC4();
extern void func_800D2370();

int func_8018FB84(int cmd, Particle *p)
{
    SVECTOR a;
    Q4 q = D_8018F00C;
    int s0, s1, s2, r, t;

    switch (cmd) {
    case 1:
        p->x += p->vx;
        p->y += p->vy;
        p->z += p->vz;
        if (D_800E27EC >= 6)
            return 1;
        break;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        a.vx = p->x;
        a.vy = p->y;
        a.vz = p->z;
        a.vx += D_80190BA0;
        a.vy += D_80190BA2;
        a.vz += D_80190BA4;
        s2 = (D_800E27EC & 1) << 7;
        s1 = ((D_800E27EC << 3) & 0x10) + 0x20;
        r = func_80077DC4((D_800E27EC << 10) / 6);
        t = D_800E1204[D_800F336C];
        s0 = r / 24;
        if (D_800F336C == 4 && D_800F3428 != 0)
            t += 4;
        func_800D2370(D_80190BA8, &a, 0x44C, 0x6E, s2, s1, 0x7F, 0xF,
                      func_80077AA4(0, t) & 0xFFFF, &q, &q, (short)s0, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
