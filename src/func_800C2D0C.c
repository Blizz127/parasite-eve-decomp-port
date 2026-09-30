typedef struct { short vx; short vy; short vz; } SVec;

extern unsigned char *D_800F34F4;
extern unsigned char *D_800E2248;

void func_800C2D0C(int a0, int a1, int a2) {
    SVec unused;
    unsigned int i;
    unsigned char *e;
    unsigned char *r;
    register int off asm("$2");
    register unsigned int m asm("$3");
    register int v asm("$4");

    i = a0 & 0xFFFF;
    D_800F34F4[i * 6 + 1] = 1;
    D_800F34F4[i * 6] = a1;
    e = D_800E2248;
    r = (unsigned char *)(i * 6 + (unsigned int)D_800F34F4);
    *(short *)(r + 2) = 0;
    off = *(short *)(e + 4);
    m = a2 & 0xFFFF;
    v = off;
    off = off + m;
    if ((unsigned int)off >= 0x80C) {
        v = 0;
    }
    *(short *)(r + 4) = v;
    *(short *)(e + 4) = v + a2;
    e[6] = e[6] + 1;
}
