typedef struct {
    unsigned char pad0[0x10];
    int f10;
} Info;

typedef struct Node Node;
struct Node {
    Info *f0;
    Node *next;
    unsigned char pad8[0x22];
    short f2A;
    unsigned char pad2C[0x6];
    short f32;
    unsigned char pad34[0x64];
    unsigned int f98;
    unsigned char pad9C[0x1CC];
    short f268;
    short pad26A;
    short f26C;
};

typedef struct {
    Node *f0;
    int f4;
    short f8;
    short padA;
} Entry;

extern Node *D_8009D20C;
extern Node *D_8009D254;
extern Entry D_8009E000[];
extern signed char D_8009CE44;

extern int func_80030534(Node *a0);
extern int func_80079FB4(int a0, int a1);
extern void func_800216E4(Entry *a, signed char lo, signed char hi);

void func_8002156C(void) {
    Node *p;

    D_8009CE44 = 0;
    for (p = D_8009D20C; p != 0; p = p->next) {
        {
            register Node *a asm("$5") = D_8009D254;

            if (p == a) {
                continue;
            }
        }
        {
            register Info *inf asm("$6") = p->f0;

            if (inf == 0) {
                continue;
            }
        }
        if ((p->f98 & 0x2040) == 0x40) {
            continue;
        }
        if (p->f98 & 0x4000) {
            continue;
        }
        if (p->f0->f10 <= 0) {
            continue;
        }
        D_8009E000[D_8009CE44].f0 = p;
        D_8009E000[D_8009CE44].f4 = func_80030534(p);
        D_8009E000[D_8009CE44].f8 = func_80079FB4(p->f268 - D_8009D254->f2A, p->f26C - D_8009D254->f32);
        D_8009CE44++;
    }
    if (D_8009CE44 >= 2) {
        func_800216E4(D_8009E000, 0, D_8009CE44 - 1);
    }
    {
        register int n asm("$2") = D_8009CE44;
        register int o asm("$3") = n * 12;

        *(Node **)((unsigned char *)D_8009E000 + o) = 0;
    }
}
