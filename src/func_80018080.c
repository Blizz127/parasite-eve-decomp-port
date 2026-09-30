typedef struct Node Node;
struct Node {
    char pad00[0x04];
    Node *next;               /* +0x04 */
    char pad08[0x04];
    unsigned char f0C;        /* +0x0C */
    unsigned char f0D;        /* +0x0D */
    char pad0E[0x8A];
    unsigned int f98;         /* +0x98 */
};

typedef struct {
    int *f0;
    int *f4;
    unsigned char *f8;
    int *fC;
} Ctx;

extern Node *D_8009D20C;
extern int func_8002FE78(int a0);
extern int func_8003010C(Node *a0, int a1);

int func_80018080(Ctx *arg0) {
    int v = *arg0->f0;
    int key;
    Node *p;

    if (v == 0) {
        *arg0->fC = func_8002FE78(*arg0->f8);
    } else {
        key = v;
        p = D_8009D20C;
        if (p != 0) {
loop:
            if ((p->f0C != key) || (p->f0D != *arg0->f4) || (p->f98 & 0x10)) {
                p = p->next;
                if (p != 0) {
                    goto loop;
                }
            }
            if (p != 0) {
                *arg0->fC = func_8003010C(p, *arg0->f8);
            }
        }
    }
    return 1;
}
