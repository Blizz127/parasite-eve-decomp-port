/* VRAM 0x800306E0 / file 0x20EE0 / size 0x1B4. */
extern unsigned char *D_8009D278;
extern int func_80052514(void);
extern int func_80071A54(void);
extern void func_80025BD8(unsigned char **);

void func_800306E0(unsigned char **o) {
    unsigned char *e;
    int d;
    register int s asm("$16");

    e = *o;
    if (*(int *)(e + 0x10) <= 0) {
        return;
    }
    d = (signed char)(D_8009D278[4] - e[4]);
    if (d > 0) {
        s = func_80052514();
        if (func_80071A54() % 100 * 10 < s) {
            func_80025BD8(o);
        }
    } else if (d == 0) {
        s = func_80052514() * 3;
        if (func_80071A54() % 100 * 50 < s) {
            func_80025BD8(o);
        }
    } else if (d == -1) {
        s = func_80052514() * 3;
        if (func_80071A54() % 100 * 100 < s) {
            func_80025BD8(o);
        }
    } else if (d < -1) {
        s = func_80052514();
        if (func_80071A54() % 100 * 50 < s) {
            func_80025BD8(o);
        }
    }
}
