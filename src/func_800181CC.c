/* Actor-list matcher: with a null key it forwards the context pairs to
 * func_8002FF78; otherwise it walks the D_8009D20C list for the first entry
 * whose type/owner keys match, then hands it to func_80030220.
 * VRAM 0x800181CC / file 0x89CC / size 0xD4.  era -O2 -G0. */
typedef struct Node181CC Node181CC;
struct Node181CC {
    char pad00[0x04];
    Node181CC *next;          /* +0x04 */
    char pad08[0x04];
    unsigned char f0C;        /* +0x0C */
    unsigned char f0D;        /* +0x0D */
    char pad0E[0x8A];
    unsigned int f98;         /* +0x98 */
};

typedef struct {
    int *f0;                  /* +0x00 */
    int *f4;                  /* +0x04 */
    unsigned char *f8;        /* +0x08 */
    int *fC;                  /* +0x0C */
} Ctx181CC;

extern Node181CC *D_8009D20C;
extern void func_8002FF78(int a0, int a1);
extern void func_80030220(Node181CC *a0, int a1, int a2);

int func_800181CC(Ctx181CC *arg0) {
    int v = *arg0->f0;
    int key;
    Node181CC *p;

    if (v == 0) {
        func_8002FF78(*(unsigned char *)arg0->f8, *arg0->fC);
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
                func_80030220(p, *(unsigned char *)arg0->f8, *arg0->fC);
            }
        }
    }
    return 1;
}
