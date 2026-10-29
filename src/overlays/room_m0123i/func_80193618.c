/* room_m0123i (PE.IMG room m0123i chunk 2, VRAM 0x8018EFE8)
 * func_80193618 — blob offset 0x4630, 0xfd4 bytes. Profile era_o2_g0 (default).
 * Day 3 boss-fight effect (task callback, effect system func_800D413C):
 * mode 0 = arm (flag the boss model, load models 0xC5483704/0xC5883704 into
 * D_80195698/D_8019569C), 1 = per-frame (follow model joint 0x17, 3 states:
 * grow 8 frames, pulse 32, fade 16 then done; collision test can flag the
 * actor and switch the model's draw mode), 2 = draw (sprite + two fans +
 * three func_800D3BC8 billboards while p->s26 fades, then the two lit models
 * through RotMatrix/MulMatrix0/ScaleMatrix).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count
 * separately from plain C.
 * Hand-matched (no masked twins). Levers: D_800B0E64 and D_800E11EA are read
 * as array elements (retail CSEs their addresses); the palette/p06/tpage
 * triples are written tpage first (so &tpage is the CSE base); the camera
 * slot gets an empty volatile asm before its set (keeps sched1 from hoisting
 * it) plus the func_80192BDC register barrier. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { MATRIX *value; unsigned char reserved[8]; } RenderMatrixSlot;
typedef struct {
    unsigned short p00, p02, palette, p06, tpage, p0A;
    short depth;
    unsigned short extent_x, extent_y;
} RenderEffectParameters;
typedef struct {
    SVECTOR pos;      /* 0x00 */
    SVECTOR tgt;      /* 0x08 */
    SVECTOR dir;      /* 0x10 */
    int s18;          /* 0x18 */
    int s1C;          /* 0x1C */
    int s20;          /* 0x20 */
    short s24;        /* 0x24 */
    short s26;        /* 0x26 */
    short s28;        /* 0x28 */
    short state;      /* 0x2A */
    short timer;      /* 0x2C */
    short s2E;        /* 0x2E */
    short s30;        /* 0x30 */
} Obj;

extern RenderEffectParameters D_800F3368;
extern RenderMatrixSlot D_800BCFA4;
extern int *D_800F32D0;
extern unsigned char *D_800E2368;
extern int **D_8009D254;
extern void *D_800B0E64[];
extern int D_800E27EC;
extern unsigned short D_800E11EA[];
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800F3428;
extern SVECTOR D_8018F1E8;
extern unsigned char D_8019550C[];
extern void *D_80195698;
extern void *D_8019569C;
extern void *func_8006E498(void *, unsigned int);
extern void func_800C6D5C(void *, int, int);
extern int func_80077A64(int, int, int, int);
extern int func_80077AA4(int, int);
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern void func_80079754(SVECTOR *, MATRIX *);
extern void func_800785D4(MATRIX *, MATRIX *, MATRIX *);
extern void func_80078CC4(MATRIX *, VECTOR *);
extern void func_800C6EC0(unsigned short, unsigned short);
extern void func_800C6ED8(int);
extern void func_800C6EF8(void *);
extern void func_800C6F4C(void *);
extern void func_800C6FA0(void *, int);
extern void func_800C7098(void *, int, int, int);
extern void func_800C71E4(void *, MATRIX *);
extern void func_800CE8F0(void *owner, int index, SVECTOR *in, SVECTOR *out);
extern void func_800CE9D4(void *owner, int index, SVECTOR *out);
extern int func_800CEB8C(SVECTOR *, SVECTOR *, int);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);
extern void func_800CF3AC(void *track, CVECTOR *color, int time);
extern void func_800CFB7C(SVECTOR *, int, SVECTOR *);
extern void func_800D004C(void *, int, int, int, void *, int, int, void *, void *, int, int);
extern void func_800D3BC8(SVECTOR *pos, int sx, int sy, int depth, int clut, int mode,
                          int intensity, int a7, int angle);

int func_80193618(int mode, Obj *p)
{
    VECTOR sc;
    SVECTOR v;
    SVECTOR rot;
    SVECTOR tmpl;
    MATRIX m;
    MATRIX rm;
    CVECTOR col;
    unsigned short clut;
    int s;

    tmpl = D_8018F1E8;
    switch (mode) {
    case 0:
        p->s26 = 200;
        p->state = 0;
        p->timer = 0;
        p->s2E = 0;
        p->s1C = 0;
        p->s20 = 0x1000;
        if (D_800E2368[0xD] != 0) {
            int *o = (int *)D_800F32D0[2];
            if (o != 0) {
                int *q = (int *)o[0];
                if (q != 0) {
                    unsigned char *b = (unsigned char *)q[6];
                    if (*b == 1) {
                        *b = 2;
                    }
                }
            }
        }
        p->s28 = 1;
        D_80195698 = func_8006E498(D_800B0E64[0], 0xC5483704);
        D_8019569C = func_8006E498(D_800B0E64[0], 0xC5883704);
        func_800C6D5C(D_80195698, 0, 0);
        func_800C6D5C(D_8019569C, 0, 0xF0);
        return 0;
    case 1:
        func_800CE8F0((void *)D_800F32D0[2], 0x17, &tmpl, &p->pos);
        func_800CE9D4((void *)D_800F32D0[2], 0, &p->dir);
        p->s24 = 0x80;
        switch (p->state) {
        case 0:
            p->timer++;
            p->s18 = func_80077DC4(p->timer * 128) / 2 + 0x1000;
            if (p->s1C < 0x800) {
                p->s1C += 0x100;
            }
            p->s20 -= 0x80;
            p->s2E = func_80077DC4(p->timer * 128) + 0x1000;
            p->s30 = p->timer;
            if (p->timer >= 8) {
                p->state = 1;
                p->timer = 0;
            }
            break;
        case 1:
            p->s18 = 0x1000;
            p->timer++;
            p->s2E = (D_800E27EC & 1) * 512 + 0x1000;
            if (p->s30 < 0x20) {
                p->s30++;
            }
            if (p->timer >= 0x20) {
                p->state = 2;
                p->timer = 0;
            }
            break;
        case 2:
            p->s18 = 0x1000;
            p->s1C = 0x800;
            p->s2E = 0x1000;
            p->timer++;
            p->s30++;
            p->s24 = 0x80 - p->timer * 8;
            if (p->timer >= 0x10) {
                return 1;
            }
            break;
        }
        func_800CFB7C(&p->dir, 0x7D0, &v);
        p->tgt.vx = p->pos.vx;
        p->tgt.vy = p->pos.vy;
        p->tgt.vz = p->pos.vz;
        p->tgt.vx += v.vx;
        p->tgt.vy += v.vy;
        p->tgt.vz += v.vz;
        if (func_800CEB8C(&p->pos, &p->tgt, 0x140) == 0) {
            return 0;
        }
        if (p->s28 == 0) {
            return 0;
        }
        if (D_800E2368[0xD] != 0) {
            int *t = D_800F32D0;
            if ((**(int **)t[2] & 0x3F000000) == 0x01000000) {
                (*D_8009D254)[0x13] |= 0x4000;
                **(int **)t[2] = (**(int **)t[2] & 0xC0FFFFFF) | 0x11000000;
                **(int **)t[2] |= 0x80000000;
            }
        }
        p->s28 = 0;
        break;
    case 2:
        D_800F3368.depth = 50;
        D_800F3368.p00 = 0x40;
        D_800F3368.p02 = 4;
        D_800F3368.extent_x = 0x40;
        D_800F3368.extent_y = 0x40;
        D_800F3368.tpage = D_800E2850[D_800E11FA];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 1;
        D_800F3368.p0A = 0;
        if (p->s2E != 0) {
            rot.vx = 0;
            rot.vy = 0;
            rot.pad = 0;
            rot.vz = D_800E27EC << 6;
            s = p->s2E;
            func_800CF3AC(D_8019550C, &col, p->s30);
            func_800CEE20((short *)p, (short *)&rot, s * 3 / 2, s * 3 / 2, 0x50,
                          func_80077AA4(0, D_800E1204[D_800F3368.palette] +
                                        ((D_800F3368.palette == 4 && D_800F3428) ? 6 : 2)) & 0xFFFF,
                          1, 0xA0, (int *)&col);
            func_800D004C(p, 0x2BC, 0x2BC, 0xC, 0, s, s, &col, 0, 0x50, 1);
            func_800D004C(p, 0xF0, 0xF0, 8, 0, s, s, &col, 0, 0x80, 1);
            if (p->s26 > 0) {
                p->s26 -= 8;
                if (p->s26 > 0) {
                    v.vx = p->pos.vx;
                    v.vy = p->pos.vy;
                    v.vz = p->pos.vz;
                    D_800F3368.p00 = 0x20;
                    D_800F3368.p02 = 2;
                    D_800F3368.extent_x = 0x20;
                    D_800F3368.extent_y = 0x20;
                    D_800F3368.tpage = D_800E2850[D_800E11EA[0]];
                    D_800F3368.palette = 3;
                    D_800F3368.p06 = 0;
                    func_800D3BC8(&v, 0x1000, 0x1000, 0x4D, func_80077AA4(0x50, D_800E120A) & 0xFFFF,
                                  1, p->s26, 0, 0xA00);
                    D_800F3368.p00 = 0x10;
                    D_800F3368.p02 = 1;
                    D_800F3368.extent_x = 0x10;
                    D_800F3368.extent_y = 0x10;
                    func_800D3BC8(&v, 0x1000, 0x1000, 0x48,
                                  func_80077AA4(0x30, D_800E1204[D_800F3368.palette] +
                                                ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)) & 0xFFFF,
                                  1, p->s26, 0, 0xC00);
                    *(unsigned short *)&D_800F3368 = 0x40;
                    D_800F3368.p02 = 4;
                    D_800F3368.extent_x = 0x40;
                    D_800F3368.extent_y = 0x40;
                    func_800D3BC8(&v, 0x1000, 0x1000, 0x49,
                                  func_80077AA4(0x40, D_800E1204[D_800F3368.palette] +
                                                ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)) & 0xFFFF,
                                  1, p->s26, 0, 0xE00);
                }
            }
        }
        D_800F3368.tpage = D_800E2850[D_800E11EA[0]];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 0;
        if (D_800E27EC < 0x11) {
            rot.vx = 0x800;
            rot.vy = 0;
            rot.vz = D_800E27EC << 4;
            func_80079754(&rot, &rm);
            m = *(MATRIX *)((int *)D_800F32D0[2])[0x238 / 4];
            func_800785D4(&m, &rm, &m);
            sc.vx = sc.vy = func_80077CF4(D_800E27EC << 6) / 4 + 0x400;
            sc.vz = 0x1000;
            func_80078CC4(&m, &sc);
            m.t[0] = p->pos.vx;
            m.t[1] = p->pos.vy;
            m.t[2] = p->pos.vz;
            {
                MATRIX **sl;
                __asm__ volatile ("");
                sl = &D_800BCFA4.value;
                __asm__ volatile ("" : "=r"(sl) : "0"(sl));
                gte_SetRotMatrix(*sl);
                gte_SetTransMatrix(*sl);
            }
            D_800F3368.tpage = D_800E2850[D_800E11EA[0]];
            D_800F3368.palette = 3;
            D_800F3368.p06 = 0;
            func_800C6EC0(D_800E2850[D_800E11EA[0]] | func_80077A64(0, 1, 0, 0), func_80077AA4(0x60, D_800E1204[D_800F3368.palette] +
                                              ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)));
            func_800C6ED8(1);
            func_800C6EF8(D_8019569C);
            func_800C7098(D_8019569C, col.r, col.g, col.b);
            s = func_80077DC4(D_800E27EC << 6) / 64;
            func_800C6FA0(D_8019569C, s & 0xFFFF);
            func_800C71E4(D_8019569C, &m);
            func_800C6F4C(D_8019569C);
        }
        rot.vx = 0x800;
        rot.vy = 0;
        rot.vz = D_800E27EC << 7;
        func_80079754(&rot, &rm);
        m = *(MATRIX *)((int *)D_800F32D0[2])[0x238 / 4];
        func_800785D4(&m, &rm, &m);
        sc.vx = sc.vy = p->s18 * 2 / 5;
        sc.vz = 0x1800;
        func_80078CC4(&m, &sc);
        m.t[0] = p->pos.vx;
        m.t[1] = p->pos.vy;
        m.t[2] = p->pos.vz;
        {
            MATRIX **sl;
            __asm__ volatile ("");
            sl = &D_800BCFA4.value;
            __asm__ volatile ("" : "=r"(sl) : "0"(sl));
            gte_SetRotMatrix(*sl);
            gte_SetTransMatrix(*sl);
        }
        D_800F3368.tpage = D_800E2850[D_800E11EA[0]];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 0;
        func_800C6EC0(D_800E2850[D_800E11EA[0]] | func_80077A64(0, 3, 0, 0), func_80077AA4(0x60, D_800E1204[D_800F3368.palette] +
                                          ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)));
        func_800C6ED8(1);
        func_800C6EF8(D_80195698);
        func_800C7098(D_80195698, col.r, col.g, col.b);
        func_800C71E4(D_80195698, &m);
        func_800C6F4C(D_80195698);
        func_800C6EF8(D_80195698);
        sc.vz = 0x1000;
        sc.vx = sc.vy = p->s20;
        func_80078CC4(&m, &sc);
        D_800F3368.tpage = D_800E2850[D_800E11EA[0]];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 0;
        func_800C6EC0(D_800E2850[D_800E11EA[0]] | func_80077A64(0, 3, 0, 0), func_80077AA4(0x60, D_800E1204[D_800F3368.palette] +
                                          ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)));
        func_800C6ED8(1);
        func_800C7098(D_80195698, col.r, col.g, col.b);
        func_800C71E4(D_80195698, &m);
        func_800C6F4C(D_80195698);
        func_800C6EF8(D_80195698);
        sc.vz = 0x1000;
        sc.vx = sc.vy = p->s1C;
        func_80078CC4(&m, &sc);
        D_800F3368.tpage = D_800E2850[D_800E11EA[0]];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 0;
        func_800C6EC0(D_800E2850[D_800E11EA[0]] | func_80077A64(0, 1, 0, 0), func_80077AA4(0x60, D_800E1204[D_800F3368.palette] +
                                          ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)));
        func_800C6ED8(1);
        func_800C7098(D_80195698, col.r, col.g, col.b);
        func_800C71E4(D_80195698, &m);
        func_800C6F4C(D_80195698);
        break;
    }
    return 0;
}
