/* func_800CF844 (EXE 0x800CF844, 0x264): orbit point.  Hand port of the
 * retail instruction graph (asm/disc1/C0044.s; inline GTE, no C candidate):
 *   out = centre + T,  T = R(ang) * (Z(yaw) * D_800C2270) * (r2, 0, 0)
 *                          + R(ang) * (0, 0, r)
 * with R(ang) = RotMatrixYXZ (func_80079754), Z(yaw) the z-axis rotation
 * func_80079E14 applies to a copy of D_800C2270, and the R(ang) product
 * from func_800786E4 (it multiplies by the GTE rotation, still R(ang)).  Both
 * products are GTE rt (MVMVA sf=1, mx=RT, v=V0, cv=TR, lm=0) through
 * the COP2 register model; the first one's MAC1..3 become the TR of the
 * second.  The camera RT (D_800BCFA4) is reloaded at the end, as retail does.
 *   centre, out  SVECTOR (out gets the 16-bit sums)
 *   r            z length rotated by R(ang)
 *   ang          SVECTOR angles
 *   r2, yaw      x length, z-axis angle
 * Retail frame: sp+0x10 V(0,0,r), sp+0x18 V(r2,0,0), sp+0x20 VECTOR T,
 * sp+0x30 MATRIX R(ang), sp+0x50 MATRIX Z(yaw); the vectors' pad halfwords are
 * never written (only VZ0's low half reaches the GTE). */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define CF844_LOC_SZ 0x60u
#define CF844_RT     0x0480012u   /* mvmva 1, 0, 0, 0, 0 (gte_rt) */

static void cf844_set_rot(pe_addr_t m)
{
    unsigned k;
    for (k = 0; k < 5u; k++) PE_GTE_WriteCtrl(k, PE_LoadU32(m + k * 4u));
}

static void cf844_set_trans(pe_addr_t m)
{
    unsigned k;
    for (k = 0; k < 3u; k++) PE_GTE_WriteCtrl(5u + k, PE_LoadU32(m + 20u + k * 4u));
}

void func_800CF844(pe_addr_t center, pe_addr_t out, int32_t r, pe_addr_t ang, int32_t r2,
                   int32_t yaw)
{
    const pe_addr_t loc = PE_GuestFrame_Enter(CF844_LOC_SZ);
    const pe_addr_t v1 = loc, v2 = loc + 8u, t = loc + 0x10u, m1 = loc + 0x20u, m2 = loc + 0x40u;
    pe_addr_t cam;
    int16_t angles[3];
    int32_t s;
    unsigned k;

    for (k = 0; k < 8u; k++) PE_StoreU32(m2 + k * 4u, PE_LoadU32(0x800C2270u + k * 4u));
    PE_StoreU16(v1 + 2u, 0u);
    PE_StoreU16(v1, 0u);
    PE_StoreU16(v1 + 4u, (uint16_t)r);
    for (k = 0; k < 3u; k++) angles[k] = (int16_t)PE_LoadU16(ang + k * 2u);
    PE_RotMatrix79754(angles, m1);                     /* func_80079754(ang, &m1) */
    cf844_set_rot(m1);
    for (k = 0; k < 3u; k++) PE_StoreU32(m1 + 20u + k * 4u, 0u);
    cf844_set_trans(m1);
    PE_GTE_WriteData(0u, PE_LoadU32(v1));
    PE_GTE_WriteData(1u, PE_LoadU32(v1 + 4u));
    PE_GTE_Command(CF844_RT);
    PE_StoreU16(v2 + 4u, 0u);
    PE_StoreU16(v2 + 2u, 0u);
    PE_StoreU16(v2, (uint16_t)r2);
    for (k = 0; k < 3u; k++)                           /* gte_stlvnl(&m1.t) */
        PE_StoreU32(m1 + 20u + k * 4u, PE_GTE_ReadData(25u + k));
    func_80079E14(yaw, m2);
    func_800786E4(m2);
    cf844_set_rot(m2);
    cf844_set_trans(m1);
    PE_GTE_WriteData(0u, PE_LoadU32(v2));
    PE_GTE_WriteData(1u, PE_LoadU32(v2 + 4u));
    PE_GTE_Command(CF844_RT);
    for (k = 0; k < 3u; k++)                           /* gte_stlvnl(&T) */
        PE_StoreU32(t + k * 4u, PE_GTE_ReadData(25u + k));
    cam = PE_LoadU32(0x800BCFA4u);
    cf844_set_rot(cam);
    cf844_set_trans(cam);
    for (k = 0; k < 3u; k++) {
        s = (int32_t)PE_LoadU32(t + k * 4u) + (int16_t)PE_LoadU16(center + k * 2u);
        PE_StoreU32(t + k * 4u, (uint32_t)s);
    }
    for (k = 0; k < 3u; k++) PE_StoreU16(out + k * 2u, (uint16_t)PE_LoadU32(t + k * 4u));
    PE_GuestFrame_Leave(loc, CF844_LOC_SZ);
}
