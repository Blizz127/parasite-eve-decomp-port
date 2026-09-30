/* VRAM 0x80050CB4 / file 0x414B4 / size 0x44. */
extern void func_80064C54(int);
extern int func_80064A48(void);
extern void func_80064C80(void);
void func_80050CB4(int arg0) {
    func_80064C54(arg0 + 0x31);
    if (func_80064A48() == arg0) {
        func_80064C80();
    }
}
