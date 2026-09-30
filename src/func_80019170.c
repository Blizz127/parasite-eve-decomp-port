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
    char pad190[0x24];
    unsigned char f1B4[4];    /* +0x1B4 */
};

typedef struct {
    int *f0;
    int *f4;
    short *f8;
} Ctx;

extern Node *D_8009D20C;
extern Node *D_8009D254;
extern Node *D_8009D2F0;

extern void func_8003E0A4(void *a0, void *a1, int a2);

int func_80019170(Ctx *arg0) {
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
    return 1;
found:
    func_8003E0A4(D_8009D2F0->f1B4, p->f1B4, *arg0->f8);
    D_8009D2F0->f18C = p;
    return 1;
}
