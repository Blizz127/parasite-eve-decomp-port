/* VRAM 0x8003E474 / file 0x2EC74 / size 0x17C. */
typedef struct {
    unsigned char pad[8];
    unsigned short n4;
    unsigned short n3;
} MeshHdr;

typedef struct {
    MeshHdr *hdr;
    unsigned char pad4[0x50];
    unsigned char *prims;
} Mesh;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    unsigned char r1, g1, b1, pad1;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    unsigned char r2, g2, b2, pad2;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad3;
    unsigned char r3, g3, b3, pad4;
    short x3, y3;
    unsigned char u3, v3;
    unsigned short pad5;
} PolyGT4;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    unsigned char r1, g1, b1, pad1;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    unsigned char r2, g2, b2, pad2;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad3;
} PolyGT3;

void func_8003E474(Mesh *m, int du, int dv, signed char dc) {
    PolyGT4 *p4;
    PolyGT3 *p3;
    int i;
    int j;

    p4 = (PolyGT4 *)m->prims;
    for (i = 0; i < m->hdr->n4; i++) {
        for (j = 0; j < 2; j++, p4++) {
            p4->v0 += dv;
            p4->u0 += du;
            p4->v1 += dv;
            p4->u1 += du;
            p4->v2 += dv;
            p4->u2 += du;
            p4->v3 += dv;
            p4->clut += dc;
        }
    }
    p3 = (PolyGT3 *)p4;
    for (i = 0; i < m->hdr->n3; i++) {
        for (j = 0; j < 2; j++, p3++) {
            p3->v0 += dv;
            p3->u0 += du;
            p3->v1 += dv;
            p3->u1 += du;
            p3->v2 += dv;
            p3->u2 += du;
            p3->clut += dc;
        }
    }
}
