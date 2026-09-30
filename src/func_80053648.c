extern unsigned char D_800C20A4[];
extern unsigned char *func_8005DC9C(int);
extern void func_800534E4(unsigned char *, unsigned char *);

void func_80053648(unsigned char *a0)
{
    unsigned char *tbl;

    if (a0[5] & 0x10) {
        tbl = D_800C20A4;
        if (a0[6] == 9) {
            tbl += 0x10;
        }
    } else {
        tbl = func_8005DC9C(a0[4] - 1);
    }
    func_800534E4(a0, tbl);
}
