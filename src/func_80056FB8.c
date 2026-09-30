extern unsigned char D_800A1F94[];
extern unsigned char D_800A1F98;
extern unsigned char D_800A1F99;
extern unsigned char D_800A1F9A;
extern unsigned char D_800A1FB4[];
extern unsigned char D_800A1FB8;
extern unsigned char D_800A1FB9;
extern unsigned char D_800A1FBA;
extern unsigned char D_800C20A4[];
extern unsigned char *func_8005DC9C(int);
extern void func_800534E4(unsigned char *, unsigned char *);
extern void func_8005E8A4(int, int);

void func_80056FB8(void)
{
    unsigned char *tbl;

    if (D_800A1F99 & 0x10) {
        tbl = D_800C20A4;
        if (D_800A1F9A == 9) {
            tbl += 0x10;
        }
    } else {
        tbl = func_8005DC9C(D_800A1F98 - 1);
    }
    func_800534E4(D_800A1F94, tbl);
    func_8005E8A4(0, 0x18);
    if (D_800A1FB9 & 0x10) {
        tbl = D_800C20A4;
        if (D_800A1FBA == 9) {
            tbl += 0x10;
        }
    } else {
        tbl = func_8005DC9C(D_800A1FB8 - 1);
    }
    func_800534E4(D_800A1FB4, tbl);
}
