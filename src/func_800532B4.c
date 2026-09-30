extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);

unsigned char *func_800532B4(int a0)
{
    unsigned char *res;

    if ((unsigned int)(a0 - 0x100) < 0x80) {
        res = (unsigned char *)((a0 << 5) + (int)D_800BEEAC);
        goto done;
    }
    if ((unsigned int)(a0 - 1) < 0xFF) {
        res = func_8005DB44(a0 - 1);
        goto done;
    }
    if ((unsigned int)(a0 - 0x200) < 9) {
        res = (unsigned char *)((a0 << 5) + (int)D_8009DE64);
        goto done;
    }
    res = 0;
done:
    return res;
}
