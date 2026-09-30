typedef struct { short vx; short vy; short vz; } SVec;
typedef struct { int vx; int vy; int vz; int pad; } Vec;

int func_800C6148(unsigned char *a0, SVec *a1, int a2) {
    Vec d;
    unsigned int r;

    d.vx = *(short *)(a0 + 0x2A) - a1->vx;
    d.vz = *(short *)(a0 + 0x32) - a1->vz;
    r = a2 & 0xFFFF;
    return d.vx * d.vx + d.vz * d.vz < (int)(r * r);
}
