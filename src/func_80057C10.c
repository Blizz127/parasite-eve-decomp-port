/* VRAM 0x80057C10 / file 0x48410 / size 0x44. */
extern int func_80062A34(int, int);
extern int func_80063428(int);
extern void func_80057D30(int);
void func_80057C10(void) {
    int v = func_80062A34(2, 1);
    if (v != 0) {
        v = func_80063428(v);
        if (v >= 0) {
            func_80057D30(v);
        }
    }
}
