typedef struct {
    short f0;
    short x;
    short f4;
    short z;
} Pt;

typedef struct {
    int x;
    int y;
    int z;
} Vec;

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

extern unsigned short D_8009CE2C;
extern void func_80078134(Vec *, Vec *);
extern int func_8003708C(int, int);
extern int func_80078004(int);

void func_8001CBA0(Obj *o, Pt *p, unsigned short n, short i)
{
    Vec a;
    Vec b;
    Pt *q;
    int x1;
    int z1;
    int x0;
    int z0;
    int d;
    int px;
    int pz;
    int len;
    int x;
    int z;
    int k;
    int j;

    x1 = p[i].x;
    z1 = p[i].z;
    if (i > 0) {
        x0 = p[i - 1].x;
        z0 = p[i - 1].z;
    } else {
        x0 = p[n - 1].x;
        z0 = p[n - 1].z;
    }
    a.x = x1 - x0;
    a.y = 0;
    a.z = z1 - z0;
    func_80078134(&a, &b);
    b.x <<= 4;
    b.z <<= 4;
    px = o->f40;
    pz = o->f48;
    d = func_8003708C(b.x, o->f28 - px);
    d += func_8003708C(b.z, o->f30 - pz);
    px = func_8003708C(b.x, d);
    pz = func_8003708C(b.z, d);
    px += o->f40;
    pz += o->f48;
    a.x = x0 - x1;
    a.y = 0;
    a.z = z0 - z1;
    len = func_80078004(a.x * a.x + a.z * a.z);
    d = (((pz >> 16) - z1) * a.x - ((px >> 16) - x1) * a.z) / len;
    if (d < 0) {
        d = -d;
    }
    if (D_8009CE2C < d) {
        o->f28 = px;
        o->f30 = pz;
        return;
    }
    for (k = 0;; k++) {
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
            d = (((z >> 16) - z1) * a.x - ((x >> 16) - x1) * a.z) / len;
            if (d < 0) {
                d = -d;
            }
            if (D_8009CE2C < d) {
                o->f28 = x;
                o->f30 = z;
                return;
            }
        }
    }
}
