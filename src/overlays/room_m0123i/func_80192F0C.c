/* room_m0123i (PE.IMG room m0123i chunk 2, VRAM 0x8018EFE8)
 * func_80192F0C — blob offset 0x3f24, 0x70c bytes. Profile era_o2_g0 (default).
 * Day 3 boss-fight effect driver (mode 0 = allocate the 16-particle pool with
 * callback func_80192BDC, 1 = grow + spawn at frame 1, done after frame 32,
 * 2 = draw: four func_800D3BC8 billboards, two func_800D004C fans, the
 * func_800D0728 ring during frames 16..32).
 * Hand-matched (no masked twins). Levers: the p00 store is spelled as a plain
 * u16 store through &D_800F3368 (retail keeps it ahead of the independent
 * index loads, cf. room_m0318i func_80196C48's (s16 *)&D_800F3368), the
 * 16..32 window temp is block-scoped, and the D_80195690 copy is an SVECTOR. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct {
    unsigned short p00, p02, palette, p06, tpage, p0A;
    short depth;
    unsigned short extent_x, extent_y;
} RenderEffectParameters;
extern RenderEffectParameters D_800F3368;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800F3428;
extern SVECTOR D_8018F1E0;
extern unsigned char D_801954BC[];
extern unsigned char D_801954E4[];
extern short D_80195684;
extern SVECTOR D_80195688;
extern short D_8019568E;
extern SVECTOR D_80195690;
extern int func_80192BDC();
extern unsigned int func_80071A54(void);
extern int func_80077AA4(int a0, int a1);
extern int func_80077DC4(int a0);
extern int func_800CE560(void *pool, int size, int count, int (*fn)());
extern short *func_800CE610(void *pool);
extern void func_800CE8F0(void *owner, int index, SVECTOR *in, SVECTOR *out);
extern void func_800CE9D4(void *owner, int index, SVECTOR *out);
extern void func_800CF3AC(void *track, CVECTOR *color, int time);
extern void func_800D004C(SVECTOR *pos, int w, int h, int seg, SVECTOR *rot, int sx, int sy,
                          CVECTOR *c0, CVECTOR *c1, int intensity, int mode);
extern void func_800D0728(SVECTOR *pos, int a1, int a2, int a3, SVECTOR *rot, int sx, int sy,
                          void *data, CVECTOR *color, int intensity, int mode);
extern void func_800D3BC8(SVECTOR *pos, int sx, int sy, int depth, int clut, int mode,
                          int intensity, int a7, int angle);
extern int func_800D3F64(int id, int group);
extern int func_800D3FD8(void);

int func_80192F0C(int mode, short *p)
{
    SVECTOR off;
    SVECTOR pos;
    SVECTOR rot;
    CVECTOR col;
    short *q;
    int i;
    int k;
    int s;

    off = D_8018F1E0;
    switch (mode) {
    case 0:
        {
            int *t = D_800F33E0;
            p[0] = 0;
            return func_800CE560((void *)t[2], 8, 16, func_80192BDC);
        }
    case 1:
        if (p[0] < 128) {
            p[0] += 4;
        }
        if (D_800E27EC == 1) {
            for (i = 0; i < 16; i++) {
                q = func_800CE610((void *)D_800F33E0[2]);
                if (q != 0) {
                    q[0] = 0;
                    q[2] = (func_80071A54() & 0x1FF) + 0x200;
                    q[1] = func_80071A54();
                }
            }
            func_800D3F64(0x580, func_800D3FD8());
            func_800D3F64(0x5BD, 0x80);
        }
        if (D_800E27EC > 32) {
            return 1;
        }
        break;
    case 2:
        *(unsigned short *)&D_800F3368 = 0x40;
        D_800F3368.p02 = 4;
        D_800F3368.extent_x = 0x40;
        D_800F3368.extent_y = 0x40;
        D_800F3368.tpage = D_800E2850[D_800E11FA];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 1;
        D_800F3368.p0A = 0;
        D_800F3368.depth = 50;
        func_800CE8F0((void *)D_800F32D0[2], 23, &off, &pos);
        rot.vx = 0;
        rot.vy = 0;
        rot.vz = -D_800E27EC * 64;
        rot.pad = 0;
        k = p[0];
        if (D_800E27EC != 0) {
            k = k * 2 / 3;
        }
        func_80077DC4(D_800E27EC * 1024 / 40);
        *(unsigned short *)&D_800F3368 = 0x40;
        D_800F3368.p02 = 4;
        D_800F3368.extent_x = 0x40;
        D_800F3368.extent_y = 0x40;
        func_800D3BC8(&pos, 0x1000, 0x1000, 80,
                      func_80077AA4(0, D_800E1204[D_800F3368.palette] +
                                    ((D_800F3368.palette == 4 && D_800F3428) ? 6 : 2)) & 0xFFFF,
                      1, k, 0, 0);
        *(unsigned short *)&D_800F3368 = 0x20;
        D_800F3368.p02 = 2;
        D_800F3368.extent_x = 0x20;
        D_800F3368.extent_y = 0x20;
        D_800F3368.tpage = D_800E2850[D_800E11EA];
        D_800F3368.palette = 3;
        D_800F3368.p06 = 0;
        func_800D3BC8(&pos, 0x1000, 0x1000, 77, func_80077AA4(80, D_800E120A) & 0xFFFF, 1, k, 0, 0xA00);
        *(unsigned short *)&D_800F3368 = 0x10;
        D_800F3368.p02 = 1;
        D_800F3368.extent_x = 0x10;
        D_800F3368.extent_y = 0x10;
        func_800D3BC8(&pos, 0x1000, 0x1000, 72,
                      func_80077AA4(48, D_800E1204[D_800F3368.palette] +
                                    ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)) & 0xFFFF,
                      1, k, 0, 0xC00);
        *(unsigned short *)&D_800F3368 = 0x40;
        D_800F3368.p02 = 4;
        D_800F3368.extent_x = 0x40;
        D_800F3368.extent_y = 0x40;
        func_800D3BC8(&pos, 0x1000, 0x1000, 73,
                      func_80077AA4(64, D_800E1204[D_800F3368.palette] +
                                    ((D_800F3368.palette == 4 && D_800F3428) ? 4 : 0)) & 0xFFFF,
                      1, k, 0, 0xE00);
        func_800CF3AC(D_801954BC, &col, D_800E27EC / 2);
        func_800D004C(&pos, 700, 700, 16, 0, 0x1000, 0x1000, &col, 0, 64, 1);
        func_800CF3AC(D_801954E4, &col, D_800E27EC * 48 / 40);
        func_800D004C(&pos, 160, 160, 8, 0, 0x1000, 0x1000, &col, 0, 128, 1);
        D_80195684 = p[0];
        D_80195690.vx = pos.vx;
        D_80195690.vy = pos.vy;
        D_80195690.vz = pos.vz;
        func_800CE9D4((void *)D_800F32D0[2], 0, &D_80195688);
        {
        int t;
        t = D_800E27EC - 16;
        if ((unsigned int)t < 17) {
            s = func_80077DC4(t * 64);
            func_800CF3AC(D_801954E4, &col, t * 3);
            D_8019568E = 1;
            func_800D0728(&pos, 700, 1000, 24, &D_80195688, s, s, 0, &col, 128, 1);
        }
        }
        *(unsigned short *)&D_800F3368 = 0x10;
        D_800F3368.p02 = 1;
        D_800F3368.extent_x = 0x10;
        D_800F3368.extent_y = 0x10;
        D_800F3368.tpage = D_800E2850[D_800E11E8];
        D_800F3368.palette = 2;
        D_800F3368.p06 = 0;
        D_800F3368.depth = 12;
        break;
    }
    return 0;
}
