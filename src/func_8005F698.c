extern int *D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D0D8;
extern int D_8009CDB0;
extern unsigned char D_800A22B0[];
extern unsigned char D_800A2270[];
extern void func_800527C0(int);
extern int func_8005DC28(int);
extern void func_8005EED4(int);

void func_8005F698(unsigned char *p, int width)
{
    int *sp;
    int *sq;
    int x0;
    int y0;
    int w;
    int c;
    int v;
    int t;

    while (*p != 0xFF) {
        sp = D_8009D12C;
        if ((unsigned int)sp < (unsigned int)D_800A22B0) {
            x0 = D_8009D124;
            y0 = D_8009D128;
            D_8009D12C = sp + 2;
            sp[0] = x0;
            sp[1] = y0;
        } else {
            func_800527C0(2);
        }
        for (w = 0; w < width;) {
            if (*p == 0xFF) {
                break;
            }
            func_8005EED4(*p);
            c = *p++;
            v = c & 0xFF;
            if (D_8009D0D8 != 0) {
                v = v + (D_8009D0D8 << 8);
                D_8009D0D8 = 0;
            }
            if ((unsigned int)(c & 0xFF) >= 0xFA) {
                D_8009D0D8 = (c & 0xFF) - 0xFA;
                v = -1;
            }
            if (v >= 0) {
                t = 0;
                if (v < 0xA || v == 0xF) {
                    t = 1;
                }
                D_8009CDB0 = t + 1;
                if (v >= 0x100) {
                    v -= 0x13;
                }
                w += ((func_8005DC28(v) >> 4) & 0xF) + D_8009CDB0;
            }
        }
        sq = D_8009D12C;
        if ((unsigned int)D_800A2270 < (unsigned int)sq) {
            D_8009D12C = sq - 2;
            D_8009D124 = sq[-2];
            D_8009D128 = sq[-1];
        } else {
            func_800527C0(3);
        }
        {
            int *q = &D_8009D124;
            *q = D_8009D124;
            D_8009D128 += 0xE;
        }
    }
}
