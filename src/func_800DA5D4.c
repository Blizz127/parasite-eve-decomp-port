extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D8;
extern unsigned char *D_800F32D0;
extern unsigned short D_800E2214;
extern unsigned short D_800E2216;
extern unsigned short D_800E2218;
extern unsigned short D_800942EC;
extern int func_80071A54();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800C6D5C();
extern void func_800DA1FC();

int func_800DA5D4(int a0, int *a1)
{
    short *p;
    int t;
    unsigned char *q;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_800F32D8;
        *a1 = t;
        func_800C6D5C(q, 0, 0);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x18, func_800DA1FC);
    case 1:
        if (D_800E27EC < 0x28 && D_800E27EC % 6 == 0) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[5] = *a1;
                p[3] = 0;
                p[6] = 0;
                t = func_80071A54();
                *a1 -= 0x555 + (t & 0xFF);
            }
        }
        if (D_800E27EC < 0x46) {
            break;
        }
        return 1;
    case 2:
        D_800E2214 = *(unsigned short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x268);
        D_800E2216 = *(unsigned short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x26A);
        D_800E2218 = *(unsigned short *)(*(unsigned char **)(D_800F32D0 + 8) + 0x26C);
        D_800E2216 = D_800942EC;
        break;
    default:
        return 0;
    }
    return 0;
}
