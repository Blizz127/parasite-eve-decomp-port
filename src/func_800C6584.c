typedef struct { short vx; short vy; short vz; } SVec;
typedef struct { int vx; int vy; int vz; int pad; } Vec;

int func_800C6584(SVec *a0, int a1, SVec *a2, int a3) {
    Vec d;
    int r;

    d.vx = a0->vx - a2->vx;
    d.vz = a0->vz - a2->vz;
    r = a1 + a3;
    return d.vx * d.vx + d.vz * d.vz < r * r;
}
