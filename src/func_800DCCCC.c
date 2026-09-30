extern int D_800E27EC;
extern char D_800E1F18;
extern char D_800E222C;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern int func_80077DC4(int a0);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D2370(char *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int *a10, int a11, int a12);

int func_800DCCCC(int a0, short *a1)
{
    int blk[4];
    int h;
    int s2v;
    int s1v;
    int w;
    int idx;
    int t;
    int q;
    register int a asm("$5");
    register int tt asm("$7");
    register int prod asm("$6");
    register int quot asm("$2");

    switch (a0) {
    case 1:
        a = *(short *)((char *)a1 + 0xC);
        tt = D_800E27EC;
        *(unsigned short *)((char *)a1 + 2) += *(unsigned short *)((char *)a1 + 0xE);
        *(unsigned short *)((char *)a1 + 4) += 8;
        prod = a * tt;
        if (prod < 0) {
            prod += 0x7F;
        }
        asm volatile("" : "=r"(prod) : "0"(prod));
        q = tt << 4;
        asm volatile("" : "=r"(q) : "0"(q));
        quot = prod >> 7;
        quot = a - quot;
        *(short *)a1 = quot;
        asm volatile("" : "=r"(quot) : "0"(quot));
        *(short *)((char *)a1 + 8) = 0x72;
        a1[5] = func_80077DC4(q) / 4 + 200;
        if (D_800E27EC < 0x40) {
            break;
        }
        return 1;
    case 2:
        func_800CF3AC(&D_800E1F18, blk, D_800E27EC);
        h = D_800E27EC / 2;
        s2v = (h & 3) << 6;
        s1v = ((h & 7) / 4 << 4) + 0xA0;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0xB0, w);
        func_800D2370(&D_800E222C, a1, a1[5], a1[4], s2v, s1v, 0x40, 0x10, t & 0xFFFF, 0, blk, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
