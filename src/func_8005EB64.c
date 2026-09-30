typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern int D_8009D10C;
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;
extern unsigned int *D_8009D11C;
extern int D_8009D124;
extern int D_8009D128;
extern void func_800527C0(int);
extern unsigned char *func_8005DADC(int);
extern void func_80077C84(unsigned int *, int, int, int);

void func_8005EB64(int id)
{
    unsigned int *p;
    unsigned int *t;
    unsigned char *g;
    int tp;

    p = 0;
    g = func_8005DADC(id);
    if (D_8009D100 + 5 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 5;
        p = D_8009D100 - 5;
    } else {
        func_800527C0(1);
    }
    if (p != 0) {
        if (D_8009D10C != 0) {
            p[1] = D_8009D114;
        } else {
            p[1] = D_8009D110;
        }
        ((unsigned char *)p)[3] = 4;
        ((unsigned char *)p)[7] = 0x64;
    }
    ((short *)p)[4] = D_8009D124;
    ((short *)p)[5] = D_8009D128;
    ((unsigned char *)p)[0xC] = g[0];
    ((unsigned char *)p)[0xD] = g[1];
    ((unsigned short *)p)[7] = *(unsigned short *)(g + 2);
    ((short *)p)[8] = g[4];
    ((short *)p)[9] = g[5];
    ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)p;
    tp = g[6];
    t = 0;
    if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 2;
        t = D_8009D100 - 2;
    } else {
        func_800527C0(1);
    }
    if (t != 0) {
        func_80077C84(t, 0, 0, ((tp & 3) << 7) | 7);
    }
    ((Tag *)t)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)t;
}
