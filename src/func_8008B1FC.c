extern unsigned char *D_8009D2C8;
extern unsigned char D_800B8AC0[];
extern unsigned char D_800BA560[];
extern void func_8008AB9C(unsigned char *a0);

void func_8008B1FC(unsigned char *a0)
{
    register unsigned char *rec asm("$6");
    int id;
    unsigned char *s;
    unsigned char *p;
    unsigned char *arg;
    int v;

    rec = a0;
    id = *(int *)(rec + 0x10);
    if (id != 0) {
        s = D_8009D2C8;
        if (id != *(unsigned short *)(s + 0x54)) {
            goto other;
        }
    }
    p = D_8009D2C8;
    *(int *)(p + 0x48) = (*(int *)(rec + 4) & 0x7F) << 16;
    *(short *)(p + 0x50) = 0;
    func_8008AB9C(D_800B8AC0);
    return;
other:
    if (id == 0) {
        return;
    }
    if (id != *(unsigned short *)(s + 0xBC)) {
        return;
    }
    arg = D_800BA560;
    v = *(int *)(rec + 4);
    D_8009D2C8 = s + 0x68;
    *(short *)(s + 0xB8) = 0;
    *(int *)(s + 0xB0) = (v & 0x7F) << 16;
    func_8008AB9C(arg);
    D_8009D2C8 = D_8009D2C8 - 0x68;
}
