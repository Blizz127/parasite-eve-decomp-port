extern int D_800A804C;
extern int D_800A802C;
extern unsigned char D_800A8028[];

int func_8005DD8C(int a0)
{
    unsigned char *base;
    unsigned char *rec;
    short *p;
    int idx;

    idx = D_800A8028[a0 + D_800A804C];
    base = D_800A8028;
    if (idx == 0) {
        return 0;
    }
    rec = base + D_800A802C;
    p = (short *)(rec + *(int *)(rec + 0x10));
    idx += 0x7F;
    if ((unsigned int)idx >= (unsigned int)*(unsigned short *)p) {
        return 0;
    }
    return (int)p + p[idx + 1];
}
