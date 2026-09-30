/* ovl_0700 (PE.IMG map/camera subsystem overlay)
 * func_8018F55C — blob offset 0x56C, 0x3D0 bytes. Profile era_o2_g0_expand_div;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl14-lane-2026-09-28/REPORT.md).
 * Camera-path spline sample: path record = tab + tab[idx] ({pad[3], short count, SVECTOR pts[]}),
 * t = segment<<8 | frac.  Interpolates the position between pts[i0] and pts[i1] and derives a
 * yaw (ratan2 of the two segment directions, with 0x1000 wrap) plus a clamped roll into *rot.
 * Levers: count read through a volatile ushort (retail lhu + sll/sra), `seg %= m` reusing seg,
 * `(idx << 2)` byte offset (tab + off operand order), short ya/yb, pinned sa ($2) / pr ($3). */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { int vx, vy, vz, pad; } VECTOR;

extern unsigned char D_8019BFCC;
extern int func_80079FB4();

int func_8018F55C(unsigned int t, int idx, int *tab, VECTOR *out, SVECTOR *rot)
{
    SVECTOR *pts;
    int n;
    int frac;
    int seg;
    int m;
    int i0, i1, i2;
    VECTOR a, b, c, d, e;
    SVECTOR v;
    int ya0;
    short ya;
    short yb;
    int yb0;

    pts = (SVECTOR *)((char *)tab + *(int *)((char *)tab + (idx << 2)));
    seg = t >> 8;
    frac = t & 0xFF;
    n = (short)((volatile unsigned short *)pts)[3];
    pts++;
    m = n - 2;
    if (m < seg) {
        D_8019BFCC = 1;
    }
    seg %= m;
    if (seg < 0) {
        seg += n - 2;
    }
    i0 = seg;
    i1 = (i0 + 1) % m;
    i2 = (i0 + 2) % m;
    v = pts[i0];
    a.vx = v.vx;
    a.vy = v.vy;
    a.vz = v.vz;
    v = pts[i1];
    b.vx = v.vx;
    b.vy = v.vy;
    b.vz = v.vz;
    v = pts[i2];
    c.vx = v.vx;
    c.vy = v.vy;
    c.vz = v.vz;
    d.vx = b.vx - a.vx;
    d.vy = b.vy - a.vy;
    d.vz = b.vz - a.vz;
    e.vx = c.vx - b.vx;
    e.vy = c.vy - b.vy;
    e.vz = c.vz - b.vz;
    ya0 = -func_80079FB4(d.vz, d.vx);
    ya = ya0;
    yb0 = -func_80079FB4(e.vz, e.vx);
    yb = yb0;
    if ((short)ya0 - (short)yb0 > 0x800) {
        yb = yb0 + 0x1000;
    }
    if ((short)yb - (short)ya0 > 0x800) {
        ya = ya0 + 0x1000;
    }
    {
        register int sa asm("$2") = ya;
        int dz = sa - yb;
        register int pr asm("$3");
        rot->vx = 0;
        pr = (yb - sa) * frac;
        rot->vy = ya + (pr >> 8);
        rot->vz = dz >> 3;
    }
    if (rot->vz > 0x80) {
        rot->vz = 0x80;
    }
    if (rot->vz < -0x80) {
        rot->vz = -0x80;
    }
    if (rot->vz > -4 && rot->vz < 4) {
        rot->vz = 0;
    }
    a.vx <<= 8;
    a.vy <<= 8;
    a.vz <<= 8;
    b.vx <<= 8;
    b.vy <<= 8;
    b.vz <<= 8;
    a.vx += d.vx * frac;
    a.vy += d.vy * frac;
    a.vz += d.vz * frac;
    a.vx >>= 8;
    a.vy >>= 8;
    a.vz >>= 8;
    out->vx = a.vx;
    out->vy = a.vy;
    out->vz = a.vz;
    return n;
}
