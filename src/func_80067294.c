typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;

#define setaddr(p, a) (((P_TAG *)(p))->addr = (unsigned int)(a))
#define getaddr(p) (((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

typedef struct {
    unsigned int tag;
    unsigned int code;
    short x, y;
    unsigned int uv;
} SPRT16;

typedef struct {
    unsigned int f0;
    unsigned short w;
    unsigned short h;
    unsigned char pad8[4];
    unsigned short x;
    unsigned short y;
    unsigned char pad10[8];
    short ox;
    short oy;
    unsigned char pad1C[0xA];
    unsigned short n;
    int cells;
    unsigned char pad2C[4];
    SPRT16 *p16;
    unsigned int *p8;
} Ent;

typedef struct {
    unsigned char pad0[0x26];
    unsigned short z;
    unsigned char pad28[0x10];
    unsigned short x;
    unsigned short y;
} Cam;

extern int D_8009CDDC;
extern unsigned int *D_800B0E38[];
extern Cam *D_800B1624;

int func_80067294(Ent *e)
{
    unsigned int *ot;
    SPRT16 *p16;
    unsigned int *p8;
    unsigned int *c;
    unsigned int n;
    unsigned int i;
    short x, y;
    int sx, sy;
    int z;
    int z2;

    ot = D_800B0E38[D_8009CDDC];
    p16 = e->p16;
    n = e->n;
    if (D_8009CDDC != 0) {
        p16 += n;
    }
    p8 = e->p8;
    if (D_8009CDDC != 0) {
        p8 += n * 2;
    }
    x = sx = e->x + D_800B1624->x;
    y = sy = e->y + D_800B1624->y;
    z = D_800B1624->z + ((e->f0 >> 8) & 0xFFF);
    c = (unsigned int *)((unsigned char *)e + e->cells);
    if (*(unsigned char *)e & 4) {
        { int tx = (short)sx - 320;
        x = (tx + e->w) % e->w; }
        { int ty = (short)sy - 224;
        y = (ty + e->h) % e->h; }
        x -= e->w - 320;
        y -= e->h - 224;
        for (i = 0; i < n; i++) {
            short xx, yy, zz;
            xx = x + (c[i] >> 22);
            if (xx >= 320) {
                xx -= e->w;
            } else if (xx < -15) {
                xx += e->w;
            }
            if ((unsigned short)(xx + 15) < 0x14F) {
                yy = y + ((c[i] >> 12) & 0x3FF);
                if (yy >= 224) {
                    yy -= e->h;
                } else if (yy < -15) {
                    yy += e->h;
                }
                if ((unsigned short)(yy + 15) < 0xEF) {
                    zz = z + (*(unsigned short *)&c[i] & 0xFFF);
                    if ((unsigned short)(zz - 8) < 0xFF1) {
                        unsigned int *o = (unsigned int *)((int)zz * 4 + (int)ot);
                        p16[i].x = xx;
                        p16[i].y = yy;
                        addPrim(o, &p16[i]);
                        addPrim(o, &p8[i * 2]);
                    }
                }
            }
        }
    } else {
        for (i = 0; i < n; i++) {
            short xx, yy, zz;
            xx = x + (c[i] >> 22);
            if ((unsigned short)(xx + 15) < 0x14F) {
                yy = y + ((c[i] >> 12) & 0x3FF);
                if ((unsigned short)(yy + 15) < 0xEF) {
                    zz = z + (*(unsigned short *)&c[i] & 0xFFF);
                    if ((unsigned short)(zz - 8) < 0xFF1) {
                        unsigned int *o = (unsigned int *)((int)zz * 4 + (int)ot);
                        p16[i].x = xx;
                        p16[i].y = yy;
                        addPrim(o, &p16[i]);
                        addPrim(o, &p8[i * 2]);
                    }
                }
            }
        }
    }
    e->ox = x;
    e->oy = y;
    return 0;
}
