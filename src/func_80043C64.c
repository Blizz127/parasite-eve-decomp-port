/* VRAM 0x80043C64 / file 0x34464 / size 0x140. */
extern int D_8009CF40;
extern int D_800A18D8[];
extern int D_800A18FC[];

extern int *func_80051E48(void);
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern void func_8005B91C(int, int, int *, int);
extern void func_800605F8(int);
extern void func_8005F5B8(int);

void func_80043C64(void) {
    int *p;
    int i;
    int v;
    int d;
    int out;

    p = func_80051E48() + 1;
    func_8005E8A4(4, 5);
    for (i = 1; i < 7; i++) {
        v = D_800A18D8[i] + ((D_8009CF40 * D_800A18FC[i]) >> 7);
        d = *p;
        func_8005E8A4(2, 0);
        func_8005EB64(i + 0x8C);
        func_8005E8A4(0x4A, 0);
        func_8005B91C(i, v, &out, 0);
        p++;
        func_800605F8(out + 1);
        if (d != 0) {
            func_8005E8A4(6, 0);
            func_8005F5B8(d > 0 ? 0x6F : 0x70);
            func_800605F8(d >= 0 ? d : -d);
            func_8005E8A4(-0x18, 0);
        }
        func_8005E8A4(-0x5E, i == 4 ? 0x16 : 0xE);
    }
}
