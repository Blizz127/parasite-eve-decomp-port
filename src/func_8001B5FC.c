typedef struct {
    short x;
    short z;
} Vtx4;

typedef struct {
    short x;
    short y;
    short z;
} Vtx6;

typedef struct {
    unsigned char f0;
    unsigned char pad1;
    unsigned short v[3];
    unsigned short w[3];
    unsigned short nb[3];
    unsigned short pad14;
} Tri22;

typedef struct {
    unsigned char f0;
    unsigned char pad1[7];
    unsigned short v[3];
    unsigned short w[3];
    unsigned short nb[3];
    unsigned short pad1A;
} Tri28;

typedef struct {
    unsigned char pad0[0x18];
    void *verts;
    void *tris;
} Mesh;

typedef struct {
    int d;
    int nx;
    int nz;
} Wall;

extern int D_8009D1D8;
extern Mesh *D_8009D1FC;
extern unsigned int D_8009DFB0[];
extern unsigned short D_8009CE2C;
extern Wall *D_8009CE14;
extern short D_8009CE0C;
extern short D_8009CE0E;
extern short D_8009CE10;
extern short D_8009CE12;
extern unsigned short D_8009CE18;
extern short D_8009CE1C;
extern short D_8009CE20;
extern short D_8009CE24;
extern short D_8009CE28;
typedef struct {
    short a[3];
} S3;

extern S3 D_8009CD88;
extern int func_8003708C(int, int);

int func_8001B5FC(short x, short z, void *t)
{
    S3 list;
    int w;
    unsigned int vi;
    unsigned int ov;
    short px;
    short pz;
    short ox;
    short oz;
    unsigned short w0;
    unsigned int *p;
    unsigned int m;
    unsigned short nb;
    int r;
    int d;

    list = D_8009CD88;
    if (D_8009D1D8 == 0) {
        void *tp = t;
        unsigned int k;
        int X;
        int Z;
        int x0;
        int z0;
        int ax;
        int az;
        int bx;
        int bz;
        unsigned int i;

        X = x;
        Z = z;
        k = 0;
        vi = ((Tri22 *)t)->v[2];
        px = ((Vtx4 *)D_8009D1FC->verts)[vi].x;
        pz = ((Vtx4 *)D_8009D1FC->verts)[vi].z;
        do {
            ov = vi;
            ox = px;
            oz = pz;
            w0 = ((Tri22 *)tp)->w[k];
            p = &D_8009DFB0[w0 >> 5];
            m = 1 << (w0 & 0x1F);
            vi = ((Tri22 *)tp)->v[k];
            px = ((Vtx4 *)D_8009D1FC->verts)[vi].x;
            pz = ((Vtx4 *)D_8009D1FC->verts)[vi].z;
            if (*p & m) {
                goto next0;
            }
            *p |= m;
            r = D_8009CE2C;
            d = X - r;
            if (px < d && ox < d) {
                goto next0;
            }
            d = X + r;
            if (d < px && d < ox) {
                goto next0;
            }
            d = Z - r;
            if (pz < d && oz < d) {
                goto next0;
            }
            d = Z + r;
            if (d < pz && d < oz) {
                goto next0;
            }
            x0 = ox;
            az = Z - pz;
            z0 = oz;
            ax = X - px;
            w = ((Tri22 *)tp)->w[k];
            d = (short)(D_8009CE14[w].d >> 16);
            d = (az * (x0 - px) - ax * (z0 - pz)) / d;
            if (d < 0) {
                d = -d;
            }
            if (r < d) {
                goto next0;
            }
            if (vi < ov) {
                d = func_8003708C(D_8009CE14[w].nx, ax << 16);
                d += func_8003708C(D_8009CE14[w].nz, az << 16);
                if (d < 0) {
                    d = ax * ax + az * az;
                    if (D_8009CE2C * D_8009CE2C < d) {
                        goto next0;
                    }
                }
                if (D_8009CE14[w].d < d) {
                    d = (X - x0) * (X - x0) + (Z - z0) * (Z - z0);
                    if (D_8009CE2C * D_8009CE2C < d) {
                        goto next0;
                    }
                }
            } else {
                bx = X - x0;
                d = func_8003708C(D_8009CE14[w].nx, bx << 16);
                bz = Z - z0;
                d += func_8003708C(D_8009CE14[w].nz, bz << 16);
                if (d < 0) {
                    d = bx * bx + bz * bz;
                    if (D_8009CE2C * D_8009CE2C < d) {
                        goto next0;
                    }
                }
                if (D_8009CE14[w].d < d) {
                    d = ax * ax + az * az;
                    if (D_8009CE2C * D_8009CE2C < d) {
                        goto next0;
                    }
                }
            }
            nb = ((Tri22 *)tp)->nb[k];
            if (nb == 0xFFFF || (((Tri22 *)D_8009D1FC->tris)[nb].f0 & 0x80)) {
                D_8009CE0C = px;
                D_8009CE0E = pz;
                D_8009CE10 = ox;
                D_8009CE12 = oz;
                if (px > ox) {
                    D_8009CE1C = px;
                    D_8009CE20 = ox;
                } else {
                    D_8009CE1C = ox;
                    D_8009CE20 = px;
                }
                if (pz > oz) {
                    D_8009CE24 = pz;
                    D_8009CE28 = oz;
                } else {
                    D_8009CE24 = oz;
                    D_8009CE28 = pz;
                }
                D_8009CE18 = w;
                return 0;
            }
            list.a[k] = nb;
        next0:
            k++;
        } while (k < 3);
        for (i = 0; i < 3; i++) {
            asm("" : : "r"(i));
            if (list.a[i] >= 0) {
                if (!func_8001B5FC(x, z, &((Tri22 *)D_8009D1FC->tris)[list.a[i]])) {
                    return 0;
                }
            }
        }
        return 1;
    }
    {
        void *tp = t;
        unsigned int k;
        int X;
        int Z;
        int x0;
        int z0;
        int ax;
        int az;
        int bx;
        int bz;
        unsigned int i;

    X = x;
    Z = z;
    k = 0;
    vi = ((Tri28 *)t)->v[2];
    px = ((Vtx6 *)D_8009D1FC->verts)[vi].x;
    pz = ((Vtx6 *)D_8009D1FC->verts)[vi].z;
    do {
        ov = vi;
        ox = px;
        oz = pz;
        w0 = ((Tri28 *)tp)->w[k];
        p = &D_8009DFB0[w0 >> 5];
        m = 1 << (w0 & 0x1F);
        vi = ((Tri28 *)tp)->v[k];
        px = ((Vtx6 *)D_8009D1FC->verts)[vi].x;
        pz = ((Vtx6 *)D_8009D1FC->verts)[vi].z;
        if (*p & m) {
            goto next1;
        }
        *p |= m;
        r = D_8009CE2C;
        d = X - r;
        if (px < d && ox < d) {
            goto next1;
        }
        d = X + r;
        if (d < px && d < ox) {
            goto next1;
        }
        d = Z - r;
        if (pz < d && oz < d) {
            goto next1;
        }
        d = Z + r;
        if (d < pz && d < oz) {
            goto next1;
        }
        x0 = ox;
        az = Z - pz;
        z0 = oz;
        ax = X - px;
        w = ((Tri28 *)tp)->w[k];
        d = (short)(D_8009CE14[w].d >> 16);
            d = (az * (x0 - px) - ax * (z0 - pz)) / d;
        if (d < 0) {
            d = -d;
        }
        if (r < d) {
            goto next1;
        }
        if (vi < ov) {
            d = func_8003708C(D_8009CE14[w].nx, ax << 16);
            d += func_8003708C(D_8009CE14[w].nz, az << 16);
            if (d < 0) {
                d = ax * ax + az * az;
                if (D_8009CE2C * D_8009CE2C < d) {
                    goto next1;
                }
            }
            if (D_8009CE14[w].d < d) {
                d = (X - x0) * (X - x0) + (Z - z0) * (Z - z0);
                if (D_8009CE2C * D_8009CE2C < d) {
                    goto next1;
                }
            }
        } else {
            bx = X - x0;
            d = func_8003708C(D_8009CE14[w].nx, bx << 16);
            bz = Z - z0;
            d += func_8003708C(D_8009CE14[w].nz, bz << 16);
            if (d < 0) {
                d = bx * bx + bz * bz;
                if (D_8009CE2C * D_8009CE2C < d) {
                    goto next1;
                }
            }
            if (D_8009CE14[w].d < d) {
                d = ax * ax + az * az;
                if (D_8009CE2C * D_8009CE2C < d) {
                    goto next1;
                }
            }
        }
        nb = ((Tri28 *)tp)->nb[k];
        if (nb == 0xFFFF || (((Tri28 *)D_8009D1FC->tris)[nb].f0 & 0x80)) {
            D_8009CE0C = px;
            D_8009CE0E = pz;
            D_8009CE10 = ox;
            D_8009CE12 = oz;
            if (px > ox) {
                D_8009CE1C = px;
                D_8009CE20 = ox;
            } else {
                D_8009CE1C = ox;
                D_8009CE20 = px;
            }
            if (pz > oz) {
                D_8009CE24 = pz;
                D_8009CE28 = oz;
            } else {
                D_8009CE24 = oz;
                D_8009CE28 = pz;
            }
            D_8009CE18 = w;
            return 0;
        }
        list.a[k] = nb;
    next1:
        k++;
    } while (k < 3);
    i = 0;
    asm("" : : "r"(i));
    for (; i < 3; i++) {
        if (list.a[i] >= 0) {
            if (!func_8001B5FC(x, z, &((Tri28 *)D_8009D1FC->tris)[list.a[i]])) {
                return 0;
            }
        }
    }
    return 1;
    }
}
