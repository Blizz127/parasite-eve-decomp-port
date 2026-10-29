/* room_m0017i (PE.IMG room m0017i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1F8 — blob offset 0x210, 0x388 bytes. Profile era_o2_g0_expand_div
 * (-O2 -G0, MASPSX_EXPAND_DIV=1; the two guarded `div`s need it).
 * Line-projection helper: foot of the perpendicular from the object's (x0,z0)
 * onto the actor's segment (BC,BE)-(C0,C2), writes it to +0x48/+0x4C/+0x50,
 * picks a facing quadrant, builds the rotation (func_800794C4) and multiplies
 * it by the room's constant matrix D_8018FB60 (inline MulMatrix0).
 * Levers: the second distance term is a separate local (e); the quadrant angle
 * sum goes through a temporary that is the same variable as the negated angle
 * (ry); r = -r is not reused.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldclmv(r0) __asm__ volatile ( \
    "lhu $12,0(%0)\n\tlhu $13,6(%0)\n\tlhu $14,12(%0)\n\t" \
    "mtc2 $12,$9\n\tmtc2 $13,$10\n\tmtc2 $14,$11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtir() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A49E012")
#define gte_stclmv(r0) __asm__ volatile ( \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%0)\n\tsh $13,6(%0)\n\tsh $14,12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")
#define ABS(x) ((x) >= 0 ? (x) : -(x))

extern char D_8018FB60[];
extern int func_80078004();
extern int func_80079FB4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800794C4();
extern void func_80039B74();

void func_8018F1F8(char *a)
{
    char *o = *(char **)(a + 0x60);
    char *q = o + 0x1B4;
    short dx;
    short dz;
    int k;
    int d;
    int x0;
    int z0;
    int ang;
    int s;
    int x1;
    int z1;
    int r;
    char *m;
    int ry, rz;
    int t;
    int e;
    short ax, az, bx, bz;
    int c;
    int rx;
    int adx;

    ax = *(short *)(a + 0xBC);
    bx = *(short *)(a + 0xC0);
    az = *(short *)(a + 0xBE);
    bz = *(short *)(a + 0xC2);
    dx = bx - ax;
    dz = bz - az;
    if (dx == 0) {
        dx = 1;
    }
    k = dz / dx;
    x0 = *(int *)(o + 0x1FC);
    z0 = *(int *)(o + 0x204);
    e = bz - k * bx;
    d = (k * x0 - z0) + e;
    if (d < 0) {
        d = -d;
    }
    d /= func_80078004(k * k + 1);
    ang = func_80079FB4(dz, dx);
    s = (func_80077CF4(ang) * d) >> 12;
    c = func_80077DC4(ang) * d;
    x1 = x0 - s * 2;
    *(int *)(a + 0x48) = x1;
    *(int *)(a + 0x4C) = *(int *)(o + 0x200);
    z1 = z0 + (c >> 12) * 2;
    *(int *)(a + 0x50) = z1;
    rx = -*(short *)(o + 0x1E0);
    adx = ABS(dx);
    if (dz >= 0 ? dz < adx : -dz < adx) {
        if (z0 < z1) {
            ry = ang + 0x800;
            r = *(short *)(q + 0x2E) + ry;
        } else {
            ry = ang;
            r = *(short *)(q + 0x2E) + ry;
        }
    } else {
        if (x1 < x0) {
            ry = ang + 0xC00;
            r = *(short *)(q + 0x2E) + ry;
        } else {
            ry = ang + 0x400;
            r = *(short *)(q + 0x2E) + ry;
        }
    }
    m = a + 0x34;
    ry = -r;
    rz = -*(short *)(q + 0x30);
    *(short *)(a + 0x2C) = rx;
    *(short *)(a + 0x2E) = ry;
    *(short *)(a + 0x30) = rz;
    func_800794C4(a + 0x2C, m);
    gte_SetRotMatrix(m);
    gte_ldclmv(D_8018FB60);
    gte_rtir();
    gte_stclmv(m);
    gte_ldclmv(D_8018FB60 + 2);
    gte_rtir();
    gte_stclmv(a + 0x36);
    gte_ldclmv(D_8018FB60 + 4);
    gte_rtir();
    gte_stclmv(a + 0x38);
    func_80039B74(a, *(int *)(o + 0x1B0), *(short *)(o + 0x16), 1);
}
