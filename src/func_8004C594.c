/* VRAM 0x8004C594 / file 0x3CD94 / size 0x48. */
extern int func_80062A34(int, int);
extern char *func_80062D2C(int, int, int, int);
extern void func_8004C608(void);
void func_8004C594(void) {
    char *p = (char *)func_80062A34(1, 0x13);
    if (p == 0) {
        p = func_80062D2C(0x13, 0, 0, 0);
        *(void **)(p + 0x30) = func_8004C608;
    }
}
