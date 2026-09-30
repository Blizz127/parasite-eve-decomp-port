extern unsigned char D_800B2900[];
extern void func_8008F0D0(unsigned char *a, unsigned char *b, int c);
void func_8008F178(unsigned char *a0, int a1) {
    unsigned char *p = D_800B2900 + (a1 << 6);
    *(short *)(a0 + 0x5A) = (short)a1;
    func_8008F0D0(a0, p, *(int *)p);
}
