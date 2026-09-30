/*
 * func_80046ABC — allocate + wire two render objects (retail 0x80046ABC).
 *
 * VRAM 0x80046ABC / file 0x372BC / size 0x9C (39 words), in the 35698.s run
 * that resumes after func_80044E14.
 *
 * Allocates object A via func_80062D2C(8, arg0, 0, 0) and object B via
 * func_8006322C(8, A, A), installs the three callback pointers
 * (A+0x2C = func_80046B58, B+0x30 = func_8004FC80, B+0x8C = func_8004FC3C),
 * registers B with func_80062CB8, refreshes state via func_80055610, and
 * returns func_800647D0(B, func_80054288()).
 *
 * era -O2 -G0 (also byte-identical on -O2 -G8). The two object pointers
 * must be `char *`/local so the callbacks are stored through the same
 * `$s0`/`$s1` base registers retail uses and the pointer materialization
 * stays in the two `jal` delay slots.
 */
extern char *func_80062D2C(int, int, int, int);
extern char *func_8006322C(int, char *, char *);
extern void func_80062CB8(int);
extern void func_80055610(void);
extern int func_80054288(void);
extern int func_800647D0(char *, int);
extern void func_80046B58(void);
extern void func_8004FC80(void);
extern void func_8004FC3C(void);

int func_80046ABC(int arg0) {
    char *a;
    char *b;
    a = func_80062D2C(8, arg0, 0, 0);
    b = func_8006322C(8, a, a);
    *(void **)(a + 0x2C) = func_80046B58;
    *(void **)(b + 0x30) = func_8004FC80;
    *(void **)(b + 0x8C) = func_8004FC3C;
    func_80062CB8((int)b);
    func_80055610();
    return func_800647D0(b, func_80054288());
}
