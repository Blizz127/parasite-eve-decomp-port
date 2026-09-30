/* VRAM 0x8003CEF8 / file 0x2D6F8 / size 0x158. */
typedef struct {
    unsigned char pad[8];
    unsigned short n4;
    unsigned short n3;
} MeshHdr;

typedef struct {
    MeshHdr *hdr;
    unsigned char pad4[0x50];
    unsigned char *prims;
    unsigned char pad58[0x62];
    short fBA;
} Mesh;

extern int D_8009CDDC;
extern short D_800921D8[][4];

void func_8003CEF8(Mesh *m, short mode) {
    int k;
    int cur;
    unsigned char *p;
    int i;
    unsigned char *q;
    int d;

    k = D_8009CDDC;
    cur = 0;
    if (m->hdr == 0 || m->fBA == 0) {
        return;
    }
    p = m->prims;
    if (m->hdr->n4 != 0) {
        cur = (*(unsigned short *)(p + k * 52 + 0x1A) & 0x7F) >> 5;
    } else if (m->hdr->n3 != 0) {
        cur = (*(unsigned short *)(p + k * 40 + 0x1A) & 0x7F) >> 5;
    }
    if (mode == cur) {
        return;
    }
    d = D_800921D8[cur][mode];
    for (i = 0; i < m->hdr->n4; i++) {
        q = p + k * 52;
        *(unsigned short *)(q + 0x1A) += d;
        p += 0x68;
    }
    for (i = 0; i < m->hdr->n3; i++) {
        q = p + k * 40;
        *(unsigned short *)(q + 0x1A) += d;
        p += 0x50;
    }
}
