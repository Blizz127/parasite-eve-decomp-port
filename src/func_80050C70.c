/* VRAM 0x80050C70 / file 0x41470 / size 0x44. */
extern void func_80064C54(int);
extern int func_800527B4(void);
extern void func_80064C80(void);
void func_80050C70(int arg0) {
    func_80064C54(arg0 + 0x2E);
    if (func_800527B4() == arg0) {
        func_80064C80();
    }
}
