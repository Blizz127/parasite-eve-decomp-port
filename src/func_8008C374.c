extern unsigned char D_800B8AC0[];
extern unsigned char *D_8009D2C8;
extern void func_8008A354(int a0, unsigned char *a1);

void func_8008C374(void) {
    func_8008A354(0, D_800B8AC0);
    D_8009D2C8 += 0x68;
    func_8008A354(0, D_800B8AC0 + 0x1AA0);
    D_8009D2C8 -= 0x68;
}
