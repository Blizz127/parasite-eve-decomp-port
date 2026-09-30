extern int D_8009CF0C;

extern void func_8005E8A4(int, int);
extern void func_80062A7C(int);
extern void func_8005E968(int);
extern void func_8005F5B8(int);

void func_80047714(void) {
    func_8005E8A4(0, 0xA);
    func_80062A7C(0x69);
    if (D_8009CF0C != 0) {
        func_8005E968(0x78703C);
        func_8005E8A4(0xA, 0x16);
        func_8005F5B8(0x1F);
        func_8005E8A4(0, 0x10);
        func_8005F5B8(0x21);
        func_8005E968(0x808080);
    }
}
