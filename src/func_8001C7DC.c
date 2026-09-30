typedef struct {
    short f0;
    short len;
    int nx;
    int nz;
} Wall;

typedef struct {
    unsigned char pad0[0x28];
    int f28;
    int pad2C;
    int f30;
    unsigned char pad34[0xC];
    int f40;
    int pad44;
    int f48;
} Obj;

extern unsigned int D_8009D2E8;
extern unsigned short D_8009CE0C;
extern short D_8009CE0E;
extern short D_8009CE10;
extern short D_8009CE12;
extern Wall *D_8009CE14;
extern unsigned short D_8009CE18;
extern unsigned short D_8009CE2C;
extern int func_8003708C(int, int);

void func_8001C7DC(Obj *o)
{
    int px;
    int pz;
    int x;
    int z;
    int d;
    int k;
    int j;
    int x0;
    int x1;
    int dx;
    int dz;
    int dx2;
    int dz2;

    px = o->f40;
    pz = o->f48;
    x = D_8009CE14[D_8009CE18].nx;
    z = D_8009CE14[D_8009CE18].nz;
    D_8009D2E8 |= 8;
    d = func_8003708C(x, o->f28 - px);
    d += func_8003708C(z, o->f30 - pz);
    px = func_8003708C(x, d);
    pz = func_8003708C(z, d) + o->f48;
    asm("" : : "r"(pz));
    asm("" : : "r"(pz));
    asm("" : : "r"(pz));
    x0 = D_8009CE0C << 16;
    x1 = D_8009CE10;
    px += o->f40;
    asm("" : : "r"(px));
    dx = x1 - (x0 >> 16);
    dz = D_8009CE12 - D_8009CE0E;
    d = D_8009CE14[D_8009CE18].len;
    d = (((pz >> 16) - D_8009CE0E) * dx - ((px >> 16) - (x0 >> 16)) * dz) / d;
    if (d < 0) {
        d = -d;
    }
    if (D_8009CE2C < d) {
        o->f28 = px;
        o->f30 = pz;
        return;
    }
    k = 0;
    goto start;
found:
    o->f28 = x;
    o->f30 = z;
    return;
start:
    {
    int xb = x1;
    int xa = x0;
kloop:
    {
        for (j = 0; j < 4; j++) {
            x = px;
            z = pz;
            switch (j) {
            case 0:
                x += k << 16;
                break;
            case 1:
                x -= k << 16;
                break;
            case 2:
                z += k << 16;
                break;
            case 3:
                z -= k << 16;
                break;
            }
            dx2 = xb - (short)(xa >> 16);
            dz2 = D_8009CE12 - D_8009CE0E;
            {
                int t = (((z >> 16) - D_8009CE0E) * dx2 - ((x >> 16) - (xa >> 16)) * dz2);
                d = D_8009CE14[D_8009CE18].len;
                d = t / d;
            }
            if (d < 0) {
                d = -d;
            }
            if (D_8009CE2C < d) {
                goto found;
            }
        }
        k++;
        goto kloop;
    }
    }
}
