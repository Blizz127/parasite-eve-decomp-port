typedef struct {
    char p0[0x268];
    unsigned short f268;
    unsigned short f26A;
    unsigned short f26C;
} T;

extern T *D_800E2804;
extern unsigned short D_800E27F0;
extern unsigned short D_800E27F2;
extern unsigned short D_800E27F4;
extern unsigned char D_800B0CE8;
extern int D_800B0E14;

extern int *func_800C2B10(int a0);
extern T ***func_800C2B28(int a0);
extern void func_80086608(int a0, int a1, int a2, int a3);
extern void func_800CEDA8(int a0);

void func_800CD980(void)
{
    int i;
    int *p;
    T **base;
    T *t;
    register int v asm("$2");

    p = func_800C2B10(0xD);
    i = *p;
    base = *func_800C2B28(1);
    D_800E2804 = base[i];
    i++;
    if (base[i] == 0) {
        p = func_800C2B10(0xE);
        *p = 1;
    }
    p = func_800C2B10(0xD);
    *p = i;
    t = D_800E2804;
    v = t->f268;
    D_800E27F0 = v;
    v = t->f26A;
    D_800E27F2 = v;
    v = t->f26C;
    D_800E27F4 = v;
    if (D_800B0CE8 != 0) {
        func_80086608(D_800B0E14, 0, 0x80, 0x7F);
    }
    func_800CEDA8(0);
}
