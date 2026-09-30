typedef struct { int f0; int f4; int f8; } N;

extern N *D_800F32D0;

int func_800D3FD8(void)
{
    N *p;
    int n;

    p = (N *)((N *)D_800F32D0->f8)->f0;
    if (p == 0) {
        n = 0x80;
    } else {
        n = p->f8;
        if (n >= 0x41) {
            n = 0x80;
        }
    }
    return n;
}
