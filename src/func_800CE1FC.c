typedef struct {
    char p0[0x268];
    unsigned short f268;
    unsigned short f26A;
    unsigned short f26C;
} T;

extern T *D_800E2848;
extern unsigned short D_800E2808;
extern unsigned short D_800E280A;
extern unsigned short D_800E280C;

extern int *func_800C2B10(int a0);
extern T ***func_800C2B28(int a0);
extern void func_800CEDA8(int a0);

void func_800CE1FC(void)
{
    int i;
    int *p;
    T **base;
    T *t;
    register int v asm("$2");

    p = func_800C2B10(0xD);
    i = *p;
    base = *func_800C2B28(1);
    D_800E2848 = base[i];
    i++;
    if (base[i] == 0) {
        p = func_800C2B10(0xE);
        *p = 1;
    }
    p = func_800C2B10(0xD);
    *p = i;
    t = D_800E2848;
    v = t->f268;
    D_800E2808 = v;
    v = t->f26A;
    D_800E280A = v;
    v = t->f26C;
    D_800E280C = v;
    func_800CEDA8(0);
}
