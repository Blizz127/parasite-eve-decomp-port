extern unsigned char D_800B89D0[];
extern int D_800B89D4;
extern int D_800B89D8;
extern int D_800B89DC;
extern int D_800B89E0;
extern void func_8008A92C(unsigned char *a0, unsigned char *a1, unsigned char *a2);

void func_80090C88(unsigned char *a0) {
    unsigned char *p;
    unsigned char *r1;
    unsigned char *r2;
    unsigned int o;

    p = *(unsigned char **)a0;
    o = (p[1] << 8) | p[0];
    if (o != 0) {
        r1 = p + o + 2;
    } else {
        r1 = 0;
    }
    p += 2;
    o = (p[1] << 8) | p[0];
    if (o != 0) {
        r2 = p + o + 2;
    } else {
        r2 = 0;
    }
    D_800B89D4 = 0;
    D_800B89D8 = 0;
    D_800B89DC = *(unsigned short *)(a0 + 0x76) >> 8;
    D_800B89E0 = *(int *)(a0 + 0x44) >> 23;
    func_8008A92C(D_800B89D0, r1, r2);
    *(unsigned char **)a0 = *(unsigned char **)a0 + 4;
}
