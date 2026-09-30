typedef struct Node Node;
struct Node {
    char pad00[0x04];
    Node *next;               /* +0x04 */
    char pad08[0x04];
    unsigned char f0C;        /* +0x0C */
    unsigned char f0D;        /* +0x0D */
    unsigned char f0E;        /* +0x0E */
    unsigned char f0F;        /* +0x0F */
    char pad10[0x6];
    short f16;                /* +0x16 */
    char pad18[0x80];
    unsigned int f98;         /* +0x98 */
};

typedef struct {
    int *f0;
    int *f4;
    int *f8;
    int *fC;
} Ctx;

extern Node *D_8009D20C;
extern Node *D_8009D254;
extern Node *D_8009D2F0;

int func_80014228(Ctx *arg0) {
    Node *p;

    if (*arg0->f4 == 0) {
        Node *q = D_8009D254;
        if (q == 0) {
            goto fail;
        }
        p = q;
        goto found;
    }
    p = D_8009D2F0;
    if (*arg0->f4 == p->f0C && *arg0->f8 == p->f0D) {
        goto found;
    }
    for (p = D_8009D20C; p != 0; p = p->next) {
        if (p->f0C == *arg0->f4 && p->f0D == *arg0->f8 && !(p->f98 & 0x10)) {
            break;
        }
    }
    if (p != 0) {
        goto found;
    }
fail:
    *arg0->fC = -1;
    return 1;
found:
    switch (*arg0->f0) {
    case 0:
        *arg0->fC = p->f0E;
        break;
    case 1:
        *arg0->fC = p->f98;
        break;
    case 2:
        *arg0->fC = p->f16;
        break;
    case 3:
        *arg0->fC = p->f0F;
        break;
    }
    return 1;
}
