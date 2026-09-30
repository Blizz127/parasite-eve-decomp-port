typedef struct {
    unsigned short m;
    short n;
    int v;
    int w;
} R;

typedef struct {
    char c0;
    char c1;
    char f2;
    char f3;
    char c4[0xC];
    int f10;
    char c14[4];
    char f18;
    char f19;
    short f1A;
    short f1C;
    short f1E[7];
    R arr[8];
} W;

int func_800D4620(W *w)
{
    int i;
    R *r;
    short *g;

    w->f2 = 1;
    w->f3 = 0;
    w->f1A = 0;
    w->f1C = 0;
    w->f10 = (int)((char *)w + 0x90);
    w->f18 = 0;
    w->f19 = 0;
    g = (short *)((char *)w + 0xC);
    for (i = 6; i >= 0; i--) {
        g[i + 9] = 0;
    }
    r = (R *)(g + 0x10);
    i = 0;
    while (i < 8) {
        i++;
        r->m = 0xFFFF;
        r->n = 0;
        r->v = 0;
        r++;
    }
    return 0;
}
