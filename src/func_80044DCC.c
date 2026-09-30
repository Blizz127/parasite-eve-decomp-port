/* VRAM 0x80044DCC / file 0x355CC / size 0x48. */
extern int func_80062A34(int, int);
extern void func_80062F1C(int);
extern void func_80062CE4(void);
void func_80044DCC(int arg0) {
    func_80062F1C(func_80062A34(1, arg0 + 0x29));
    if (arg0 != 0) {
        func_80062CE4();
    }
}
