extern int func_80062A34(int, int);
extern int func_80063428(int);
extern void func_800556E8(int);

void func_8004304C(void) {
    int v;

    v = func_80062A34(2, 7);
    if (v == 0) {
        v = func_80062A34(2, 0xD);
        if (v == 0) {
            v = func_80062A34(2, 0x10);
        }
    }
    func_800556E8(func_80063428(v));
}
