/* func_800DD9E4 — EXE, VRAM 0x800DD9E4, file offset 0xCE1E4, 0x38C bytes.
 * Effect task callback (cmd 1 = tick/done, cmd 2 = draw): two-phase drift/flash, then a glow point (func_800D1DEC) and an effect sprite (func_800CEE20).
 * Profile era_o2_g0 (default).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-800DD9E4/REPORT.md */
/* Psy-Q inline_c.h-shaped GTE macros (inline COP2 asm; count separately from plain C). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ( \
    "lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)\n\tlwc2 $2,0(%1)\n\tlwc2 $3,4(%1)\n\t" \
    "lwc2 $4,0(%2)\n\tlwc2 $5,4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25,0(%0)\n\tswc2 $26,4(%0)\n\tswc2 $27,8(%0)" : : "r"(r0) : "memory")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14,0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $12,0(%0)\n\tswc2 $13,0(%1)\n\tswc2 $14,0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24,0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ( \
    "mfc2 $12,$19\n\tnop\n\tsra $12,$12,2\n\tsw $12,0(%0)" : : "r"(r0) : "$12", "memory")
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_SetGeomScreen(r0) __asm__ volatile ("ctc2 %0,$26" : : "r"(r0))

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, p6; short state, timer, phase; } EffObj;

extern MATRIX *D_800BCFA4[];
extern SVECTOR D_800E223C;
extern char D_800E207C[];
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern unsigned short D_800E1204[];
extern int func_80077DC4();
extern int func_80077CF4();
extern void func_800783E4();
extern void func_800CF3AC();
extern int func_80071A54();
extern void func_800D1DEC();
extern int func_80077AA4();
extern void func_800CEE20();

int func_800DD9E4(int cmd, EffObj *e)
{
    typedef struct { unsigned char r, g, b, cd; } CV;
    CV col;
    SVECTOR pos;
    int scale;
    int c;
    int tex;

    switch (cmd) {
    case 1:
        e->timer++;
        if (e->state >= 2) {
            return 1;
        }
        break;
    case 2:
        scale = 0x1000;
        switch (e->state) {
        case 0:
            c = 0x1000 - func_80077DC4((e->timer << 10) / 28);
            func_800783E4(e, &D_800E223C, 0x1000 - c, c, e);
            e->y += func_80077CF4(e->phase) / 128;
            e->phase += 0xA0;
            pos.vx = e->x;
            pos.vy = e->y;
            pos.vz = e->z;
            func_800CF3AC(D_800E207C, &col, (D_800E27EC << 6) / 28);
            scale = 0x2000 - (e->timer << 12) / 28;
            if (e->timer >= 0x1C) {
                e->state = 1;
                e->timer = 0;
                e->x += (func_80071A54() & 0xFF) - 0x80;
                e->y += (func_80071A54() & 0xFF) - 0x80;
                e->z += (func_80071A54() & 0xFF) - 0x80;
            }
            break;
        case 1:
            pos.vx = e->x;
            pos.vy = e->y;
            pos.vz = e->z;
            c = func_80077DC4((e->timer << 10) / 12) / 32;
            *(unsigned int *)&col = c | (c << 8);
            e->x += (func_80071A54() & 7) - 3;
            e->y += (func_80071A54() & 7) - 3;
            e->z += (func_80071A54() & 7) - 3;
            if (e->timer >= 8) {
                e->state = 2;
            }
            break;
        }
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        func_800D1DEC(&pos, &col, 0x80, 1);
        tex = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            tex += 4;
        }
        func_800CEE20(&pos, 0, scale, scale, 0x80, func_80077AA4(0x20, tex) & 0xFFFF, 3, 0x80, &col);
        break;
    default:
        return 0;
    }
    return 0;
}
