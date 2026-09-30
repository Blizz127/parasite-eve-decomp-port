#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef struct Node Node;
struct Node {
    char pad00[0x04];
    Node *next;               /* +0x04 */
    char pad08[0x04];
    unsigned char f0C;        /* +0x0C */
    unsigned char f0D;        /* +0x0D */
    char pad0E[0x1A];
    int f28;                  /* +0x28 */
    int f2C;                  /* +0x2C */
    int f30;                  /* +0x30 */
    char pad34[0x6];
    short f3A;                /* +0x3A */
    char pad3C[0x5C];
    unsigned int f98;         /* +0x98 */
};

typedef struct {
    int *f0;
    int *f4;
    int *f8;
} Ctx;

extern Node *D_8009D20C;
extern Node *D_8009D254;
extern Node *D_8009D2F0;

int func_800143B0(Ctx *arg0) {
    int v = *arg0->f0;
    int key;
    Node *p;

    if (v == 0) {
        Node *q = D_8009D254;
        if (q == 0) {
            goto fail;
        }
        p = q;
        goto found;
    }
    key = v;
    p = D_8009D20C;
    if (p == 0) {
        goto fail;
    }
loop:
    if ((p->f0C != key) || (p->f0D != *arg0->f4) || (p->f98 & 0x10)) {
        p = p->next;
        if (p != 0) {
            goto loop;
        }
    }
    if (p != 0) {
        goto found;
    }
fail:
    *arg0->f8 = -1;
    return 1;
found:
    *arg0->f8 = ABS(D_8009D2F0->f28 - p->f28);
    *arg0->f8 += ABS(D_8009D2F0->f2C - p->f2C);
    *arg0->f8 += ABS(D_8009D2F0->f30 - p->f30);
    return 1;
}
