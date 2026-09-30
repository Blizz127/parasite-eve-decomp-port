/*
 * func_80044E98 — window-mode dispatch (retail 0x80044E98).
 *
 * VRAM 0x80044E98 / file 0x35698 / size 0xF4 (61 words), head of the 35698.s
 * run and the first leaf carved out of it.
 *
 * When arg1 keeps bit 16 the handler queries func_80063428(func_80062A20(
 * arg0, 0)) and switches on the result: case 0 runs the "close" path
 * (func_80062F1C, optional func_80062CE4 for arg0[9] == 0x2A, the gp-relative
 * callback at D_8009CFA0 + 8 with arg 1, then func_800525EC), case 1 falls
 * into the shared bit-0x40 path, default returns 1. The bit-0x40 path runs
 * the same body with callback arg 0 and func_80052634.
 *
 * Build: era -O2 -G8. The compare chain (`beqz` -> case 0 body, `beq $v1,$v0`
 * -> case 1) is a `switch` with case 0 / case 1 / default: an if/else chain
 * lays the case-0 body out first and shifts every later branch. All five
 * gp-relative slots land in the D_8009CE54/D_8009CF90/D_8009CF94/D_8009CFA0
 * small-data symbols, which are declared as scalars to keep the loads
 * gp-relative.
 */
extern int D_8009CFA0;
extern int func_80062A20(int *, int);
extern int func_80063428(int);
extern int func_80062F1C(int *);
extern int func_80062CE4(void);
extern int func_800525EC(void);
extern int func_80052634(void);
typedef void (*Fn)(int *, int);

int func_80044E98(int *arg0, int arg1) {
    Fn f;

    if (arg1 & 0x10000) {
        switch (func_80063428(func_80062A20(arg0, 0))) {
        case 0:
            func_80062F1C(arg0);
            if (arg0[9] == 0x2A) {
                func_80062CE4();
            }
            f = *(Fn *)((char *)&D_8009CFA0 + 8);
            if (f != 0) {
                f(arg0, 1);
            }
            func_800525EC();
            return 1;
        case 1:
            goto case1;
        default:
            return 1;
        }
    } else if (arg1 & 0x40) {
    case1:
        func_80062F1C(arg0);
        if (arg0[9] == 0x2A) {
            func_80062CE4();
        }
        f = *(Fn *)((char *)&D_8009CFA0 + 8);
        if (f != 0) {
            f(arg0, 0);
        }
        func_80052634();
    }
    return 1;
}
