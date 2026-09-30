typedef struct Node Node;
struct Node {
    char pad00[0x04];
    Node *next;               /* +0x04 */
    char pad08[0x04];
    unsigned char f0C;        /* +0x0C */
    unsigned char f0D;        /* +0x0D */
    char pad0E[0x8A];
    unsigned int f98;         /* +0x98 */
    char pad9C[0xF0];
    Node *f18C;               /* +0x18C */
};

typedef struct {
    int *f0;
    int *f4;
} Ctx;

extern Node *D_8009D20C;
extern Node *D_8009D254;
extern Node *D_8009D2F0;

int func_80019F04(Ctx *arg0) {
    int v = *arg0->f0;
    int key;
    Node *p;
    Node *s;

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
    return 1;
found:
    s = D_8009D2F0;
    s->f18C = p;
    p->f98 |= 0x100000;
    s->f98 |= 0x600000;
    return 1;
}
