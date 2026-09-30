typedef struct { short vx; short vy; short vz; } SVec;
typedef struct { int vx; int vy; int vz; int pad; } Vec;

extern short *D_8009D254;

int func_800C6B90(SVec *a0, int a1) {
    Vec d;
    SVec s;
    short *b;
    int tx;
    int ty;
    int tz;
    int tr;
    int r;

    b = D_8009D254;
    tx = b[0x15];
    tr = b[0x112];
    s.vx = tx;
    ty = b[0x17];
    s.vy = ty;
    tz = b[0x19];
    s.vz = tz;
    d.vx = tx - a0->vx;
    d.vz = tz - a0->vz;
    r = tr + a1;
    return d.vx * d.vx + d.vz * d.vz < r * r;
}
