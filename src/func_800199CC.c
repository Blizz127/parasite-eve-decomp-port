typedef struct S { unsigned char pad[0x24E]; short f24E; unsigned short f250; } S;
extern S *D_8009D2F0;

int func_800199CC(int **a0) {
    int *p;
    S *s;
    int v;

    p = *a0;
    s = D_8009D2F0;
    v = *p;
    s->f250 |= 0x10;
    s->f24E = (short)v;
    return 1;
}
