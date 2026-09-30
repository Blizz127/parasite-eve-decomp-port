typedef struct Ent {
    int f0;
    struct Ent *next;
    struct Ent *prev;
    unsigned char fC;
    unsigned char fD;
    unsigned char padE[0x28 - 0xE];
    int f28;
    int f2C;
    int f30;
    unsigned char pad34[0x98 - 0x34];
    int f98;
    unsigned char pad9C[0x27C - 0x9C];
    unsigned char f27C;
    unsigned char f27D;
} Ent;

typedef struct Vm {
    unsigned char pad0[0x10];
    int f10;
} Vm;

extern unsigned int D_800B0CD8[];
extern unsigned char *D_8009D254;
extern Ent *D_8009D2F0;
extern Ent *D_8009D20C;
extern short D_800BD028[2];
extern unsigned char D_800BD025[3];
extern unsigned char D_8009CDF8[3];
extern int D_8009CE00;
extern Vm *D_8009D300;

extern void func_800665A0(unsigned char *, int, int);
extern void func_800375D0(int);
extern void func_800E00CC(short *, int, int, int, int, int, int);
extern Ent *func_80035038(unsigned char *, Ent *, int);
extern void func_80033A40(void);
extern void func_800339A0(int);
extern int func_8006914C(int);

int func_80016910(int **a)
{
    short b1[3];
    short b2[3];
    unsigned char d[2];
    unsigned int *p;
    Ent *e;
    int r;

    switch (*a[0]) {
    case 0x7D0:
        break;
    case 0x835:
        p = D_800B0CD8;
        *p |= 0x800;
        break;
    case 0x898:
        func_800665A0(D_8009D254 + 0x28, *a[1], *a[2]);
        break;
    case 0x899:
        D_800BD028[0] = *a[1];
        D_800BD028[1] = *a[2];
        break;
    case 0x8FC:
        func_800375D0(*a[1] != 0);
        break;
    case 0x960:
        b1[0] = (short)(*a[1] >> 16);
        b1[1] = (short)(*a[2] >> 16);
        b1[2] = (short)(*a[3] >> 16);
        func_800E00CC(b1, 0, *(short *)a[4], *(unsigned char *)a[5], 0, 0, 0);
        break;
    case 0x961:
        D_8009CDF8[0] = *a[1];
        D_8009CDF8[1] = *a[2];
        D_8009CDF8[2] = *a[3];
        break;
    case 0x962:
        b2[0] = (short)(*a[1] >> 16);
        b2[1] = (short)(*a[2] >> 16);
        b2[2] = (short)(*a[3] >> 16);
        func_800E00CC(b2, 1, *(short *)a[4], *(unsigned char *)a[5], D_8009CDF8[0], D_8009CDF8[1], D_8009CDF8[2]);
        break;
    case 0x9C4:
        d[0] = *a[1];
        d[1] = *a[2];
        {
            Ent *n = func_80035038(d, D_8009D2F0, 0);
            n->f28 = *a[3];
            n->f2C = *a[4];
            n->f30 = *a[5];
        }
        break;
    case 0xA28:
        D_8009D2F0->f27C = *a[1];
        break;
    case 0xA29:
        D_8009D2F0->f27D = *a[1];
        break;
    case 0xA8C:
        D_800BD025[0] = *a[1];
        D_800BD025[1] = *a[2];
        D_800BD025[2] = *a[3];
        break;
    case 0xAF0:
        func_80033A40();
        break;
    case 0xAF1:
        func_800339A0(*(unsigned char *)a[1]);
        break;
    case 0xB54:
        {
            unsigned int *q = D_800B0CD8;
            *q |= 0x400000;
        }
        break;
    case 0xBB8:
        if (*a[1] != 0) {
            for (e = D_8009D20C; e != 0; e = e->next) {
                if (e->fC == *a[1] && e->fD == *a[2] && !(e->f98 & 0x10)) {
                    return 1;
                }
            }
        }
        break;
    case 0xC1C:
        r = func_8006914C(1);
        if (r == 1) {
            D_8009CE00 -= 0x28;
            D_8009D300->f10 = r;
            return 0;
        }
        break;
    case 0xC80:
        {
            unsigned int *q = D_800B0CD8;
            *q |= 0x8000000;
        }
        break;
    }
    return 1;
}
