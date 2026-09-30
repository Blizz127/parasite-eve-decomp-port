typedef struct { short x; short y; short z; } Vec;
extern short *D_8009D254;
extern int func_800C62DC(Vec *a0, unsigned char *a1);

int func_800C6B20(unsigned char *a0) {
    Vec v;
    Vec *p;
    short *s;
    int tx;
    int ty;
    int tz;
    int r1;
    int r2;

    s = D_8009D254;
    p = &v;
    tx = s[0x15];
    p->x = tx;
    ty = s[0x17];
    p->y = ty;
    tz = s[0x19];
    p->z = tz;
    r1 = func_800C62DC(p, a0);
    r2 = func_800C62DC(p, a0 + 8);
    return r1 | r2;
}
