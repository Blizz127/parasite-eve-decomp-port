typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
    unsigned char u3, v3;
    unsigned short pad3;
} GT4;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad2;
} GT3;

typedef struct {
    unsigned char pad0[8];
    unsigned short n4;
    unsigned short n3;
} Hdr;

typedef struct {
    Hdr *hdr;
    unsigned char pad4[0x50];
    unsigned char *prims;
} Obj;

void func_8003D94C(Obj *a0, short x, short y, int unused, int c)
{
    short flag;
    int tp;
    int ts;
    int cc;
    int z;
    short d;
    short clut;
    short i;
    short k;
    unsigned char *ptr;
    GT4 *p4;
    GT3 *p3;

    cc = c;
L:
    z = 0;
    if (z) goto L;
    flag = y == 0x80;
    tp = ((x >> 6) - 8) * 2 + (y >> 7) + (y >> 8) * 30;
    asm("" : "=r"(ts) : "0"(tp));
    ptr = a0->prims;
    if (x != 0x3C0) {
        d = (unsigned int)tp >> 1;
        asm("" : "=r"(c) : "0"(c));
        clut = c * 64 - 0x7080;
    } else {
        if (((GT4 *)ptr)->tpage == 0x1F) {
            return;
        }
        d = (short)ts >> 1;
        clut = cc * 64 - 0x7080;
    }
    for (i = 0; i < a0->hdr->n4; i++) {
        for (k = 0; k < 2; k++, ptr += sizeof(GT4)) {
            p4 = (GT4 *)ptr;
            if (flag) {
                if (p4->v0 < 0x80) {
                    p4->v0 += 0x80;
                    p4->v1 += 0x80;
                    p4->v2 += 0x80;
                    p4->v3 += 0x80;
                    p4->tpage += d;
                } else {
                    p4->v0 += 0x80;
                    p4->v1 += 0x80;
                    p4->v2 += 0x80;
                    p4->v3 += 0x80;
                    p4->tpage += d + 1;
                }
            } else {
                p4->tpage += d;
            }
            p4->clut += clut;
        }
    }
    asm("" : "=r"(ptr) : "0"(ptr));
    asm("" : "=r"(ptr) : "0"(ptr));
    for (i = 0; i < a0->hdr->n3; i++) {
        for (k = 0; k < 2; k++, ptr += sizeof(GT3)) {
            p3 = (GT3 *)ptr;
            if (flag) {
                if (p3->v0 < 0x80) {
                    p3->v0 += 0x80;
                    p3->v1 += 0x80;
                    p3->v2 += 0x80;
                    p3->tpage += d;
                } else {
                    p3->v0 += 0x80;
                    p3->v1 += 0x80;
                    p3->v2 += 0x80;
                    p3->tpage += d + 1;
                }
            } else {
                p3->tpage += d;
            }
            p3->clut += clut;
        }
    }
}
