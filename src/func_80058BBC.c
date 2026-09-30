extern short *D_8009D07C;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);

unsigned char *func_80058BBC(int a0)
{
    int v;
    int w;
    unsigned char *res;

    v = D_8009D07C[a0];
    w = v;
    if ((unsigned int)(v - 0x100) < 0x80) {
        res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
        goto done;
    }
    if ((unsigned int)(v - 1) < 0xFF) {
        res = func_8005DB44(v - 1);
        goto done;
    }
    if ((unsigned int)(w - 0x200) < 9) {
        res = (unsigned char *)((w << 5) + (int)D_8009DE64);
        goto done;
    }
    res = 0;
done:
    return res;
}
