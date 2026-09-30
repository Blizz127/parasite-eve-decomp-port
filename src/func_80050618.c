extern int D_8009CEF4;
extern int D_8009CF0C;
extern unsigned char D_800923B8[];

extern int func_80063428(int);
extern void func_8005EB58(int);
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);

void func_80050618(int a0) {
    if (a0 != func_80063428(D_8009CEF4)) {
        func_8005EB58(1);
    }
    func_8005E8A4(-2, -2);
    if (D_8009CF0C != 0) {
        unsigned char *p = &D_800923B8[D_8009CF0C * 8];
        func_8005EB64(p[a0]);
    }
}
