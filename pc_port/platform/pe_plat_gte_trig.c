/* Integer trig and rotation-matrix helpers (guest 0x80077CF4 rsin, 0x80077D30
 * sin_index, 0x80077DC4 rcos, 0x800794C4 RotMatrix, 0x800799E4 RotMatrixZYX,
 * 0x80079E14 RotMatrixZ, 0x80079754 RotMatrixYXZ), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_gte_trig_vectors.py),
 * with no SDK source.
 * Evidence: docs/evidence/native-gte-trig-20261030/.
 *
 * The matrix kinds share one host-array core each (angles in, nine halfwords
 * out); the guest forms load the angles, call it and store the result.
 *
 * Angles are 12-bit (0x1000 = one turn).  Values are 4.12 fixed point.  The
 * tables stay in guest memory (the game's own data) and are read from there. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"

/* Quarter-wave table: 0x401 signed halfwords; entry i is the 4.12 sine of
 * angle i for i = 0..0x400 (a quarter turn).  The rest of the circle is
 * mirrored and negated. */
#define SIN_QUARTER_TABLE   0x8009589Cu
#define QUARTER_MASK        0x3FFu
#define QUARTER_STEPS       0x400u

/* Full-circle table: 0x1000 words indexed by the 12-bit angle.  Low halfword
 * is sin, high halfword is cos, both signed 4.12. */
#define SIN_COS_TABLE       0x800966ECu

#define ANGLE_MASK          0xFFFu
#define ANGLE_TURN          0x1000
#define FIX_SHIFT           12

#define MATRIX_ELEMS        9u
#define ROW_ELEMS           3u
#define ANGLE_COUNT         3u

static int16_t load_s16(uint32_t addr)
{
    return (int16_t)PE_LoadU16(addr);
}

/* Table value for an angle reduced to 0..0xFFF, folded into the first
 * quadrant (no sign applied). */
static int quarter_value(uint32_t phase)
{
    uint32_t low = phase & QUARTER_MASK;
    uint32_t idx = (phase & QUARTER_STEPS) ? QUARTER_STEPS - low : low;

    return load_s16(SIN_QUARTER_TABLE + 2u * idx);
}

/* Magnitude of a signed angle, as an unsigned value. */
static uint32_t angle_abs(int a)
{
    return a < 0 ? 0u - (uint32_t)a : (uint32_t)a;
}

/* The sign flips for the upper half circle, but the boundaries are not
 * symmetric: phase 0x800 keeps the table sign (only phases above it flip). */
#define SIN_FLIP_ABOVE      0x800u

/* Sine is odd: the sign of a negative angle is flipped after the lookup. */
int pe_plat_gte_rsin(int a)
{
    uint32_t phase = angle_abs(a) & ANGLE_MASK;
    int v = quarter_value(phase);
    int neg = (phase > SIN_FLIP_ABOVE) != (a < 0);

    return neg ? -v : v;
}

/* Cosine is even.  The table is read at phase + a quarter turn, but the sign
 * flips for phases in (0x400, 0xC00]: the boundary at 0xC00 flips, the one at
 * 0x400 does not. */
#define COS_FLIP_ABOVE      0x400u
#define COS_FLIP_THROUGH    0xC00u

int pe_plat_gte_rcos(int a)
{
    uint32_t phase = angle_abs(a) & ANGLE_MASK;
    int v = quarter_value((phase + QUARTER_STEPS) & ANGLE_MASK);

    return (phase > COS_FLIP_ABOVE && phase <= COS_FLIP_THROUGH) ? -v : v;
}

/* Inside 0..0xFFF this is rsin.  Outside that range the angle is not
 * reduced: the table is read as a plain halfword array, unmirrored, at index
 * a (negative a, value returned as is) or 0x1000 - a (a >= 0x1000, value
 * negated). */
int pe_plat_gte_sin_index(int a)
{
    uint32_t idx;
    int v;

    if (a >= 0 && a < ANGLE_TURN) {
        v = quarter_value((uint32_t)a);
        return (uint32_t)a > SIN_FLIP_ABOVE ? -v : v;
    }
    if (a < 0)
        return load_s16(SIN_QUARTER_TABLE + 2u * (uint32_t)a);
    idx = (uint32_t)ANGLE_TURN - (uint32_t)a;
    return -load_s16(SIN_QUARTER_TABLE + 2u * idx);
}

/* Sine and cosine of a signed angle from the full-circle table.  A negative
 * angle reads the entry for -angle and flips the sine. */
static void sin_cos(int32_t angle, int *s, int *c)
{
    uint32_t neg = angle < 0;
    uint32_t idx = (neg ? 0u - (uint32_t)angle : (uint32_t)angle) & ANGLE_MASK;
    uint32_t e = PE_LoadU32(SIN_COS_TABLE + 4u * idx);
    int sv = (int16_t)(e & 0xFFFFu);

    *s = neg ? -sv : sv;
    *c = (int16_t)(e >> 16);
}

static int mul12(int a, int b)
{
    return (a * b) >> FIX_SHIFT;
}

static void store_matrix(uint32_t out, const int16_t m[MATRIX_ELEMS])
{
    uint32_t i;

    for (i = 0; i < MATRIX_ELEMS; i++)
        PE_StoreU16(out + 2u * i, (uint16_t)m[i]);
}

static void load_angles(uint32_t angles, int16_t a[ANGLE_COUNT])
{
    uint32_t i;

    for (i = 0; i < ANGLE_COUNT; i++)
        a[i] = load_s16(angles + 2u * i);
}

/* Matrix = Rx * Ry * Rz (row-major, 3x3) from the angle triple (x, y, z). */
void pe_plat_gte_rot_matrix_values(const int16_t angles[ANGLE_COUNT],
                                   int16_t out[MATRIX_ELEMS])
{
    int sx, cx, sy, cy, sz, cz, w1, w2;

    sin_cos(angles[0], &sx, &cx);
    sin_cos(angles[1], &sy, &cy);
    sin_cos(angles[2], &sz, &cz);
    w1 = mul12(-sy, cz);
    w2 = mul12(-sy, sz);
    out[0] = (int16_t)mul12(cy, cz);
    out[1] = (int16_t)mul12(cy, -sz);
    out[2] = (int16_t)sy;
    out[3] = (int16_t)(mul12(cx, sz) - mul12(sx, w1));
    out[4] = (int16_t)(mul12(cx, cz) + mul12(sx, w2));
    out[5] = (int16_t)mul12(sx, -cy);
    out[6] = (int16_t)(mul12(sx, sz) + mul12(cx, w1));
    out[7] = (int16_t)(mul12(sx, cz) - mul12(cx, w2));
    out[8] = (int16_t)mul12(cx, cy);
}

void pe_plat_gte_rot_matrix(uint32_t angles, uint32_t out)
{
    int16_t a[ANGLE_COUNT], m[MATRIX_ELEMS];

    load_angles(angles, a);
    pe_plat_gte_rot_matrix_values(a, m);
    store_matrix(out, m);
}

/* Matrix = Rz * Ry * Rx (row-major, 3x3) from the angle triple (x, y, z). */
static void rot_matrix_zyx_values(const int16_t angles[ANGLE_COUNT],
                                  int16_t out[MATRIX_ELEMS])
{
    int sx, cx, sy, cy, sz, cz, w1, w2;

    sin_cos(angles[0], &sx, &cx);
    sin_cos(angles[1], &sy, &cy);
    sin_cos(angles[2], &sz, &cz);
    w1 = mul12(sy, sx);
    w2 = mul12(sy, cx);
    out[0] = (int16_t)mul12(cy, cz);
    out[1] = (int16_t)(mul12(cz, w1) - mul12(sz, cx));
    out[2] = (int16_t)(mul12(cz, w2) + mul12(sz, sx));
    out[3] = (int16_t)mul12(cy, sz);
    out[4] = (int16_t)(mul12(cz, cx) + mul12(sz, w1));
    out[5] = (int16_t)(mul12(sz, w2) - mul12(cz, sx));
    out[6] = (int16_t)-sy;
    out[7] = (int16_t)mul12(cy, sx);
    out[8] = (int16_t)mul12(cx, cy);
}

/* Returns out. */
uint32_t pe_plat_gte_rot_matrix_zyx(uint32_t angles, uint32_t out)
{
    int16_t a[ANGLE_COUNT], m[MATRIX_ELEMS];

    load_angles(angles, a);
    rot_matrix_zyx_values(a, m);
    store_matrix(out, m);
    return out;
}

/* Matrix = Ry * Rx * Rz (row-major, 3x3) from the angle triple (x, y, z).
 * The Y-times-X partial products are rounded once before they meet the Z
 * terms (same shape as the ZYX form). */
void pe_plat_gte_rot_matrix_yxz_values(const int16_t angles[ANGLE_COUNT],
                                       int16_t out[MATRIX_ELEMS])
{
    int sx, cx, sy, cy, sz, cz, w1, w2;

    sin_cos(angles[0], &sx, &cx);
    sin_cos(angles[1], &sy, &cy);
    sin_cos(angles[2], &sz, &cz);
    w1 = mul12(sy, sx);
    w2 = mul12(cy, sx);
    out[0] = (int16_t)(mul12(cy, cz) + mul12(sz, w1));
    out[1] = (int16_t)(mul12(w1, cz) - mul12(cy, sz));
    out[2] = (int16_t)mul12(sy, cx);
    out[3] = (int16_t)mul12(cx, sz);
    out[4] = (int16_t)mul12(cx, cz);
    out[5] = (int16_t)-sx;
    out[6] = (int16_t)(mul12(w2, sz) - mul12(sy, cz));
    out[7] = (int16_t)(mul12(w2, cz) + mul12(sy, sz));
    out[8] = (int16_t)mul12(cy, cx);
}

/* Returns out. */
uint32_t pe_plat_gte_rot_matrix_yxz(uint32_t angles, uint32_t out)
{
    int16_t a[ANGLE_COUNT], m[MATRIX_ELEMS];

    load_angles(angles, a);
    pe_plat_gte_rot_matrix_yxz_values(a, m);
    store_matrix(out, m);
    return out;
}

/* Rotate the first two rows of an existing matrix about Z.  Each pair is
 * combined before a single shift. */
void pe_plat_gte_rot_matrix_z(int32_t angle, uint32_t out)
{
    int s, c;
    uint32_t j;

    sin_cos(angle, &s, &c);
    for (j = 0; j < ROW_ELEMS; j++) {
        int r0 = load_s16(out + 2u * j);
        int r1 = load_s16(out + 2u * (j + ROW_ELEMS));
        PE_StoreU16(out + 2u * j, (uint16_t)((c * r0 - s * r1) >> FIX_SHIFT));
        PE_StoreU16(out + 2u * (j + ROW_ELEMS),
                    (uint16_t)((s * r0 + c * r1) >> FIX_SHIFT));
    }
}
