typedef struct { char b[45]; } S45;
typedef struct { char b[9]; } S9;

extern unsigned char D_800106A4[];
extern unsigned char D_800106D4[];
extern unsigned char *D_8009D278;
extern void func_8006C4C4();

void func_800209F0(void) {
    S45 v10;
    S9 v40;
    unsigned char *p;
    unsigned char *h;
    int k;
    int m;
    unsigned char *n;

    v10 = *(S45 *)D_800106A4;
    v40 = *(S9 *)D_800106D4;
    p = D_8009D278;
    h = *(unsigned char **)(p + 0x68);
    k = h[6];
    m = *(unsigned short *)(p + 0x22);
    {
        int t = m * (unsigned char)v40.b[k];
        p[0x12] = 4;
        t = t / 10;
        n = D_8009D278;
        *(short *)(p + 0x24) = t;
        n[0x13] = 5;
    }
    D_8009D278[0x14] = 6;
    D_8009D278[0x17] = 7;
    D_8009D278[0x15] = 8;
    D_8009D278[0x18] = 9;
    D_8009D278[0x16] = 0xA;
    D_8009D278[0x19] = 0xB;
    {
        unsigned char *q = (unsigned char *)&v10 + k * 5;
        (*(unsigned char **)(D_8009D278 + 0x68))[0x14] = q[0];
        (*(unsigned char **)(D_8009D278 + 0x68))[0x15] = q[1];
        (*(unsigned char **)(D_8009D278 + 0x68))[0x16] = q[2];
        (*(unsigned char **)(D_8009D278 + 0x68))[0x17] = q[3];
        *(int *)(*(unsigned char **)(D_8009D278 + 0x68) + 8) = q[4];
        func_8006C4C4(*(short *)(*(unsigned char **)(D_8009D278 + 0x68) + 6));
    }
}
