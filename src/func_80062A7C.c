extern int *D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D0D8;
extern int D_8009CDB0;
extern int D_8009D138;
extern unsigned char D_800A22B0[];
extern unsigned char D_800A2270[];
extern void func_800527C0(int);
extern int func_8005DC28(int);
extern void func_8005EED4(int);
extern unsigned char *func_8005DC4C(int);

void func_80062A7C(int a0)
{
    unsigned char *str;
    unsigned char *p;
    int *sp;
    int w;
    int ch;
    int v;
    int t;
    register int fe asm("$2");
    int off;
    register int x asm("$3");
    register int y asm("$5");
    int x0;
    int y0;
    int *sp2;
    int *sp3;
    int *sp4;

    str = func_8005DC4C(a0);
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
    p = str;
    __asm__ __volatile__("" : "=r"(p) : "0"(p));
    w = 0;
    if (*str != 0xFF) {
    lp:
        {
            __asm__ __volatile__("" : "=r"(p) : "0"(p));
            ch = *p;
            p++;
            v = ch & 0xFF;
            if (D_8009D0D8 != 0) {
                v = v + (D_8009D0D8 << 8);
                D_8009D0D8 = 0;
            }
            if ((unsigned int)(ch & 0xFF) >= 0xFA) {
                D_8009D0D8 = (ch & 0xFF) - 0xFA;
                v = -1;
            }
            ch = v;
            if (ch >= 0) {
                t = 0;
                if (ch < 0xA) {
                    goto s1;
                }
                if (ch != 0xF) {
                    goto s2;
                }
            s1:
                t = 1;
            s2:
                D_8009CDB0 = t + 1;
                if (ch >= 0x100) {
                    ch -= 0x13;
                }
                w += ((func_8005DC28(ch) >> 4) & 0xF) + D_8009CDB0;
            }
        }
        if (*p != 0xFF) {
            goto lp;
        }
        p = str;
    }
    x = w + 4;
    off = D_8009D138 - x;
    y = D_8009D128;
    x = D_8009D124;
    off = off >> 1;
    D_8009D128 = y;
    x = x + off;
    D_8009D124 = x;
    if (p != 0) {
        sp2 = D_8009D12C;
        if ((unsigned int)sp2 < (unsigned int)D_800A22B0) {
            sp2[0] = x;
            sp2[1] = y;
            D_8009D12C = sp2 + 2;
        } else {
            func_800527C0(2);
        }
        ch = *p;
        fe = 0xFF;
        __asm__ __volatile__("" : "=r"(ch) : "0"(ch));
        if ((ch & 0xFF) != fe) {
            do {
                func_8005EED4(ch);
                p++;
                ch = *p;
            } while (ch != 0xFF);
        }
        sp3 = D_8009D12C;
        if ((unsigned int)D_800A2270 < (unsigned int)sp3) {
            D_8009D12C = sp3 - 2;
            D_8009D124 = sp3[-2];
            D_8009D128 = sp3[-1];
        } else {
            func_800527C0(3);
        }
    }
    sp4 = D_8009D12C;
    if ((unsigned int)D_800A2270 < (unsigned int)sp4) {
        D_8009D12C = sp4 - 2;
        D_8009D124 = sp4[-2];
        D_8009D128 = sp4[-1];
    } else {
        func_800527C0(3);
    }
}
