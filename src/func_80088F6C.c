extern unsigned char D_800B8AC0[];
extern void func_800878F0(int a0, unsigned char *a1, int a2);

void func_80088F6C(unsigned char *a0, int a1)
{
    unsigned char *p;
    int k;
    short t2;

    func_800878F0(*(int *)(a0 + 0xF0), a0 + 0xF0, *(int *)(a0 + 0x38));
    k = a1;
    asm ("" : "=r" (k) : "0" (k));
    p = D_800B8AC0 + k * 0x11C;
    *(short *)(a0 + 0x118) = *(short *)(p + 0x118);
    t2 = *(short *)(p + 0x11A);
    *(int *)(a0 + 0xF4) |= 0x1FF93;
    *(short *)(a0 + 0x11A) = t2;
    func_800878F0(k, a0 + 0xF0, *(int *)(a0 + 0x38));
}
