extern unsigned char *D_8009D2F0;
extern void *D_8009D254;
extern unsigned char D_800B0CEC[];
extern unsigned short D_800B0D88;
extern void func_8003C5D8(void *a0, int a1);

int func_800190BC(short **a0) {
    unsigned char *s;
    void *t;

    func_8003C5D8(D_8009D2F0 + 0x1B4, **a0);
    s = D_8009D2F0;
    t = D_8009D254;
    *(unsigned short *)(s + 0x250) |= 2;
    if (s == t) {
        func_8003C5D8(D_800B0CEC, **a0);
        D_800B0D88 |= 2;
    }
    return 1;
}
