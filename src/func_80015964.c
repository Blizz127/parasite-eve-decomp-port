extern unsigned char func_80038D0C(void);
extern void func_80038D74(void);
extern unsigned char func_8003944C(int a0);
extern unsigned char func_800392EC(void);
extern void func_80039970(void);
extern int func_8006E3D4(void *a0);

extern unsigned char D_8009CD78[];
extern unsigned char D_8009CD80[];
extern unsigned char D_8009173C[][24];
extern unsigned char D_8009180C[][8];
extern int D_8009D280;
extern int D_8009D1C4;
extern unsigned int D_8009D1A0;
extern unsigned int D_800B0CD8[];

int func_80015964(unsigned char **a0) {
    unsigned char k;
    unsigned char row;

    if (!func_80038D0C()) {
        func_80038D74();
    }
    k = func_8003944C(**a0);
    if (k == 0xFF) {
        if (func_800392EC() < 2) {
            func_80039970();
            D_8009D280 = func_8006E3D4(D_8009CD78);
        } else {
            func_80039970();
            D_8009D280 = func_8006E3D4(D_8009CD80);
        }
        return 1;
    }
    row = (unsigned char)(func_800392EC() - 1);
    row /= 10;
    {
        unsigned char *p = D_8009173C[row];
        D_8009D280 = func_8006E3D4(D_8009180C[p[k]]);
    }
    if (D_8009D1C4 != D_8009D280) {
        return 1;
    }
    D_8009D1A0 |= 0x2000;
    D_800B0CD8[0] |= 0x800;
    return 1;
}
