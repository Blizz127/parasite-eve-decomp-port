extern unsigned char *D_8009D0C8;
extern unsigned char *D_8009D0C0;
extern int D_8009D0C4;
extern int D_8009D218;
extern unsigned char D_800C20A4[];
extern unsigned char D_800C0DE0[];

void func_8005BCBC(unsigned char *a0)
{
    unsigned char *p;

    D_8009D0C8 = a0;
    if (a0 != 0) {
        p = D_800C20A4;
        if (a0[6] == 9) {
            p += 0x10;
        }
    } else {
        p = D_800C0DE0;
        if (D_8009D218 != 0) {
            p += 0x10;
        }
    }
    D_8009D0C0 = p;
    D_8009D0C4 = 8;
}
