extern unsigned char D_800B8AC0[];
extern unsigned char *D_8009D2C8;
extern int D_8009D2C0;
extern void func_8008AB9C(unsigned char *a0);
extern void func_8008ABF0(void);

void func_8008C55C(void) {
    D_8009D2C0 = 1;
    func_8008AB9C(D_800B8AC0);
    D_8009D2C8 += 0x68;
    func_8008AB9C(D_800B8AC0 + 0x1AA0);
    D_8009D2C8 -= 0x68;
    func_8008ABF0();
}
