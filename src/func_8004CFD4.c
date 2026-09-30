extern unsigned char D_800A1A20[];

extern void func_8005E8A4(int, int);
extern void func_8005F594(unsigned char *);

void func_8004CFD4(void) {
    func_8005E8A4(0, 0xA);
    func_8005F594(D_800A1A20);
    func_8005E8A4(0, 0xE);
    func_8005F594(D_800A1A20 + 0x40);
}
