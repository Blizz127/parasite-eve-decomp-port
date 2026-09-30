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

extern unsigned short D_8009CE2C;
extern void func_80078134(Vec *, Vec *);
extern int func_8003708C(int, int);
extern int func_80078004(int);

int func_8001CE88(short x, short z, Pt *p, unsigned short n)
{
    Vec a;
    unsigned short i;
    short x0;
    short z0;
    short ox;
    short oz;
    short x1;
    short z1;
    int dx;
    int dz;
    int len;
    int d;

    x0 = p[n - 1].x;
    z0 = p[n - 1].z;
    for (i = 0; i < n; i++) {
        ox = x0;
        oz = z0;
        { int t = p[i].x; x1 = t; x0 = t; }
        { int t = p[i].z; z1 = t; z0 = t; }
        d = x - D_8009CE2C;
        if (x1 < d && ox < d) {
            continue;
        }
        d = x + D_8009CE2C;
        if (d < x1 && d < ox) {
            continue;
        }
        d = z - D_8009CE2C;
        if (z1 < d && oz < d) {
            continue;
        }
        d = z + D_8009CE2C;
        if (d < z1 && d < oz) {
            continue;
        }
        a.x = ox - x1;
        a.y = 0;
        a.z = oz - z1;
        len = func_80078004(a.x * a.x + a.z * a.z);
        dz = z - z1;
        dx = x - x1;
        d = (dz * a.x - dx * a.z) / len;
        if (d < 0) {
            d = -d;
        }
        if (D_8009CE2C < d) {
            continue;
        }
        func_80078134(&a, &a);
        d = func_8003708C(a.x << 4, dx << 16);
        d += func_8003708C(a.z << 4, dz << 16);
        if (d < 0) {
            d = dx * dx + dz * dz;
            if (D_8009CE2C * D_8009CE2C < d) {
                continue;
            }
        }
        if (len < (d >> 16)) {
            d = (x - ox) * (x - ox) + (z - oz) * (z - oz);
            if (D_8009CE2C * D_8009CE2C < d) {
                continue;
            }
        }
        return i;
    }
    return -1;
}
