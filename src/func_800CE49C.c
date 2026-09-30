typedef struct { int p0[3]; int fC; int p10[31]; int f8C; } CTX;
typedef struct { int p0[13]; int f34; } NODE;

extern NODE *D_800E1044[];
extern char D_800C2244;
extern void func_80071A74(char *);

int func_800CE49C(CTX *a0, int a1)
{
    NODE *p;
    register int i asm("$2");

    i = a1 << 2;
    p = *(NODE **)((char *)D_800E1044 + i);
    if (p == 0) {
        func_80071A74(&D_800C2244);
        return -1;
    }
    a0->f8C = (int)p;
    a0->fC = p->f34;
    return 0;
}
