typedef struct { int f30; } HND;
typedef struct {
    int p0[3];
    int fC;
    int p10;
    int f14;
    int p18[29];
    int *f8C;
} CTX;

extern CTX *D_800F32D0;
extern int *D_800E2368;

int func_800D4698(CTX *a0, int a1, int a2, int a3, int a4, int a5)
{
    int (*f)(int, int, int, int);
    int *p;

    p = &a0->fC;
    if (a1 == 0) {
        f = (int (*)(int, int, int, int))a0->f8C[0xC];
        D_800F32D0 = a0;
        D_800E2368 = p;
        if (f != 0) {
            a0->f14 = f(a2, a3, a4, a5);
        }
    }
    return 0;
}
