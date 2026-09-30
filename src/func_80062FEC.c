extern int *D_8009D154;
extern int *D_8009D15C;
extern int D_8009D10C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D138;
extern void func_80062830(int *s);
extern void func_80061C34(int a0, int a1, int a2, int a3);

void func_80062FEC(void)
{
    register int *s asm("$16");
    register int m asm("$17");
    int f;
    int v;
    int a5;
    int t;
    int r;

    f = 0;
    m = 0;
    s = D_8009D154;
    if (s != 0) {
        do {
            if (s[8] == 1) {
                v = s[17];
                s[15] = 0;
                m = m | v;
            }
            s = (int *)s[0];
        } while (s != 0);
    }
    s = D_8009D15C;
    if (s != 0) {
        do {
            if (s[8] == 1) {
                s[15] = f;
                f = 1;
            }
            s = (int *)s[1];
        } while (s != 0);
    }
    s = D_8009D154;
    if (s != 0) {
        do {
            if (s[8] == 1 && s[18] == 0) {
                a5 = 0;
                if (s[16] == 0) {
                    a5 = s[15] & 1;
                }
                s[15] = a5;
                t = 0;
                if (m != 0) {
                    t = (s[17] == 0);
                }
                r = a5 | t;
                s[15] = r;
                D_8009D10C = r;
                D_8009D124 = 0;
                D_8009D128 = 0;
                D_8009D138 = s[13];
                func_80062830(s);
                D_8009D124 = s[6];
                D_8009D128 = s[7];
                D_8009D10C = s[15];
                func_80061C34(s[13], s[14], s[19], s[17]);
            }
            s = (int *)s[0];
        } while (s != 0);
    }
}
