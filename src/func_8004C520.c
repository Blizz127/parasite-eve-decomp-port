extern int D_8009CFF4;

extern void func_8005E8A4(int, int);
extern int func_8005DC4C(int);
extern int func_8005DC9C(int);
extern int func_8005DCEC(int);
extern void func_8005F594(int);
extern void func_8005F27C(int);

void func_8004C520(void) {
    func_8005E8A4(2, 4);
    func_8005F594(func_8005DC4C(0x26));
    func_8005E8A4(0, 0x10);
    func_8005F594(func_8005DC9C(D_8009CFF4 + 0xEB));
    func_8005E8A4(2, 0x14);
    func_8005F27C(func_8005DCEC(D_8009CFF4 + 0xEB));
}
