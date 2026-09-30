/* VRAM 0x80040210 / file 0x30A10 / size 0x144. */
extern unsigned char D_8009EE8C[];
extern int D_800A1704;
extern int D_800A1708;
extern int D_800A170C;
extern int D_800A1710;
extern int D_800A1714;
extern int D_800A1718;
extern int D_800A171C;
extern int D_80092228;
extern int D_8009222C;
extern int D_800A7918;

extern void func_80071A24(unsigned char *address, int size);
extern int func_8005DE08(int);
extern int func_80043474(int);
extern int func_8005D940(void);
extern void func_8004006C(unsigned char *, int);

unsigned char *func_80040210(int a0, int secs) {
    func_80071A24(D_8009EE8C, 0x44);
    D_800A1708 = a0;
    D_800A170C = secs / 3600;
    D_800A1710 = secs / 60 - D_800A170C * 60;
    D_800A1714 = secs - (secs / 60) * 60;
    if (D_800A1704 != 0) {
        D_800A1718 = func_8005DE08(-1);
        func_8004006C(D_8009EE8C, D_8009222C);
    } else {
        D_800A1718 = func_80043474(D_800A7918);
        D_800A171C = func_8005DE08(func_8005D940());
        func_8004006C(D_8009EE8C, D_80092228);
    }
    return D_8009EE8C;
}
