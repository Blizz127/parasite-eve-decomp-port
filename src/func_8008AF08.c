extern unsigned char *D_8009D2C8;
extern unsigned char D_800B8968[];
extern unsigned char D_800B8AC0[];
extern unsigned char D_800B6B80[];
extern void func_8008D820(unsigned char *a0, unsigned char *a1, int a2);

void func_8008AF08(void)
{
    unsigned char *p;
    unsigned char *q;
    unsigned int i;
    unsigned short v;

    if (*(int *)(D_8009D2C8 + 4) == 0) {
        return;
    }
    p = D_800B8968;
    func_8008D820(D_8009D2C8, p, 0x68);
    func_8008D820(D_800B8AC0, D_800B6B80, 0x1AA0);
    if ((*(int *)p & 0x100) == 0) {
        return;
    }
    i = 0;
    q = D_800B6B80 + 0x5A;
    do {
        v = *(unsigned short *)q;
        if (v >= 0x50) {
            *(unsigned short *)q = v - 0x30;
        }
        i++;
        q += 0x11C;
    } while (i < 0x18);
}
