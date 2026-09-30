typedef struct {
    unsigned int tag;
    unsigned int c0;
    unsigned int xy0;
    unsigned int uv0;
    unsigned int c1;
    unsigned int xy1;
    unsigned int uv1;
    unsigned int c2;
    unsigned int xy2;
    unsigned int uv2;
    unsigned int c3;
    unsigned int xy3;
    unsigned int uv3;
} GT4;

typedef struct {
    unsigned int tag;
    unsigned int c0;
    unsigned int xy0;
    unsigned int uv0;
    unsigned int c1;
    unsigned int xy1;
    unsigned int uv1;
    unsigned int c2;
    unsigned int xy2;
    unsigned int uv2;
} GT3;

typedef struct {
    unsigned int tag;
    unsigned int c0;
    unsigned int xy0;
    unsigned int c1;
    unsigned int xy1;
    unsigned int c2;
    unsigned int xy2;
    unsigned int c3;
    unsigned int xy3;
} G4;

typedef struct {
    unsigned int tag;
    unsigned int c0;
    unsigned int xy0;
    unsigned int c1;
    unsigned int xy1;
    unsigned int c2;
    unsigned int xy2;
} G3;

typedef struct {
    short pad[2];
    unsigned short c[4];
} Face;

typedef struct {
    unsigned char pad0[8];
    unsigned short n4;
    unsigned short n3;
    unsigned short g4;
    unsigned short g3;
} Hdr;

typedef struct {
    Hdr *hdr;
    unsigned char pad4[0xC];
    Face *faces;
    unsigned char pad14[0x40];
    unsigned char *prims;
    unsigned char pad58[0x62];
    short fBA;
} Obj;

extern unsigned int D_800B1638[];
extern unsigned int D_800A6360[];

void func_8003BCE0(Obj *o, short flag, short buf)
{
    Face *f;
    unsigned char *p;
    int i;
    unsigned char code;
    unsigned int *lit;
    unsigned int *pal;
    void *q;

    if (o->hdr == 0 || o->fBA == 0) {
        return;
    }
    lit = D_800B1638;
    pal = D_800A6360;
    f = o->faces;
    p = o->prims;
    for (i = 0; i < o->hdr->n4;) {
        q = (GT4 *)p + buf;

        if ((((GT4 *)q)->tag & 0xFFFFFF) || flag) {
            code = ((unsigned char *)q)[7];
            ((GT4 *)q)->c0 = lit[f->c[0]];
            ((GT4 *)q)->c1 = lit[f->c[1]];
            ((GT4 *)q)->c2 = lit[f->c[2]];
            ((GT4 *)q)->c3 = lit[f->c[3]];
            ((unsigned char *)q)[7] = code;
        }
        p += sizeof(GT4) * 2;
        i++;
        f++;
    }
    for (i = 0; i < o->hdr->n3;) {
        q = (GT3 *)p + buf;

        if ((((GT3 *)q)->tag & 0xFFFFFF) || flag) {
            code = ((unsigned char *)q)[7];
            ((GT3 *)q)->c0 = lit[f->c[0]];
            ((GT3 *)q)->c1 = lit[f->c[1]];
            ((GT3 *)q)->c2 = lit[f->c[2]];
            ((unsigned char *)q)[7] = code;
        }
        p += sizeof(GT3) * 2;
        i++;
        f++;
    }
    for (i = 0; i < o->hdr->g4;) {
        q = (G4 *)p + buf;

        if ((((G4 *)q)->tag & 0xFFFFFF) || flag) {
            code = ((unsigned char *)q)[7];
            ((G4 *)q)->c0 = pal[f->c[0]];
            ((G4 *)q)->c1 = pal[f->c[1]];
            ((G4 *)q)->c2 = pal[f->c[2]];
            ((G4 *)q)->c3 = pal[f->c[3]];
            ((unsigned char *)q)[7] = code;
        }
        p += sizeof(G4) * 2;
        i++;
        f++;
    }
    for (i = 0; i < o->hdr->g3;) {
        q = (G3 *)p + buf;

        if ((((G3 *)q)->tag & 0xFFFFFF) || flag) {
            code = ((unsigned char *)q)[7];
            ((G3 *)q)->c0 = pal[f->c[0]];
            ((G3 *)q)->c1 = pal[f->c[1]];
            ((G3 *)q)->c2 = pal[f->c[2]];
            ((unsigned char *)q)[7] = code;
        }
        p += sizeof(G3) * 2;
        i++;
        f++;
    }
}
