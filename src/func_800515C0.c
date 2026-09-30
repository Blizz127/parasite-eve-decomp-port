extern unsigned char *D_8009D254;
extern unsigned short D_800C0E08;
void func_800515C0(unsigned short a0) {
    unsigned char *p = D_8009D254;
    if (p != 0) {
        unsigned char *q = *(unsigned char **)p;
        if (q != 0) {
            *(unsigned short *)(q + 0xC) = a0;
        }
    }
    D_800C0E08 = a0;
}
