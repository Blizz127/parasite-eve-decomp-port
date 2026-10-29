/* func_800C5538 (engine B5D38.s, 0x800C5538..0x800C5A40): chain/tail actor
 * segment update (Day 2 Central Park random battles, m0075i/m0080i via
 * func_80190884).  NON_MATCHING host port written from the retail asm.
 *
 * c = *(arg0+0) holds h(arg0+4) segments of 68 bytes from c+4.  Starting from
 * the root MATRIX at arg0+20, each segment i gets: its two side points
 * (+-h(arg0+10) along X, through the current matrix) at seg+20/+28; a joint
 * angle (segments 0-1 the stored seg+10/+12, segments >=2 a random Y swing of
 * 64..575 whose sign follows the turn toward the player D_8009D254, segments
 * >=6 none); the matrix advanced by that rotation and h(arg0+8) along -Z
 * (GTE MVMVA, IR saturation as retail) and stored at seg+36.  The stack frame
 * is guest-addressed (retail sp offsets) so the libgte callees see retail
 * layouts. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_guest_ram.h"

pe_addr_t func_80071A44(pe_addr_t dst, int32_t fillbyte, int32_t len);

static void gte_rt_from(pe_addr_t m)
{
    PE_GTE_WriteCtrl(0u, PE_LoadU32(m + 0u));
    PE_GTE_WriteCtrl(1u, PE_LoadU32(m + 4u));
    PE_GTE_WriteCtrl(2u, PE_LoadU32(m + 8u));
    PE_GTE_WriteCtrl(3u, PE_LoadU32(m + 12u));
    PE_GTE_WriteCtrl(4u, PE_LoadU32(m + 16u));
}

/* IR = RT * column(src) >> 12 (MVMVA 0x49E012), stored as column dst. */
static void gte_col(pe_addr_t src, pe_addr_t dst)
{
    PE_GTE_WriteData(9u, PE_LoadU16(src + 0u));
    PE_GTE_WriteData(10u, PE_LoadU16(src + 6u));
    PE_GTE_WriteData(11u, PE_LoadU16(src + 12u));
    PE_GTE_Command(0x49E012u);
    PE_StoreU16(dst + 0u, (uint16_t)PE_GTE_ReadData(9u));
    PE_StoreU16(dst + 6u, (uint16_t)PE_GTE_ReadData(10u));
    PE_StoreU16(dst + 12u, (uint16_t)PE_GTE_ReadData(11u));
}

static int32_t rand_swing(void)
{
    int32_t v = (int32_t)func_80071A54();
    return v % 512 + 64;
}

void func_800C5538(pe_addr_t a0)
{
    pe_addr_t sp = PE_GuestFrame_Enter(208u);
    pe_addr_t m = sp + 48u, m2 = sp + 88u, seg;
    uint32_t i, k;
    pe_addr_t chain = PE_LoadU32(a0);

    func_80071A44(sp + 24u, 0, 8u);
    PE_StoreU16(sp + 26u, 0u);
    PE_StoreU16(sp + 28u, 0u);
    PE_StoreU16(sp + 24u, (uint16_t)-PE_LoadU16(a0 + 10u));
    PE_StoreU32(sp + 16u, PE_LoadU32(sp + 24u));
    PE_StoreU32(sp + 20u, PE_LoadU32(sp + 28u));
    func_80071A44(sp + 40u, 0, 8u);
    PE_StoreU16(sp + 42u, 0u);
    PE_StoreU16(sp + 44u, 0u);
    PE_StoreU16(sp + 40u, PE_LoadU16(a0 + 10u));
    PE_StoreU32(sp + 32u, PE_LoadU32(sp + 40u));
    PE_StoreU32(sp + 36u, PE_LoadU32(sp + 44u));
    for (k = 0; k < 32u; k += 4u)
        PE_StoreU32(m + k, PE_LoadU32(a0 + 20u + k));
    PE_StoreU16(a0 + 14u, 0u);
    seg = chain + 36u;
    for (i = 0; i < (uint32_t)(int32_t)(int16_t)PE_LoadU16(a0 + 4u); i++, seg += 68u) {
        func_80078C34(m, sp + 16u, sp + 24u);
        func_80078C34(m, sp + 32u, sp + 40u);
        PE_StoreU16(seg - 16u, (uint16_t)(PE_LoadU16(sp + 40u) + PE_LoadU32(sp + 68u)));
        PE_StoreU16(seg - 14u, (uint16_t)(PE_LoadU16(sp + 42u) + PE_LoadU32(sp + 72u)));
        PE_StoreU16(seg - 12u, (uint16_t)(PE_LoadU16(sp + 44u) + PE_LoadU32(sp + 76u)));
        PE_StoreU16(seg - 8u, (uint16_t)(PE_LoadU16(sp + 24u) + PE_LoadU32(sp + 68u)));
        PE_StoreU16(seg - 6u, (uint16_t)(PE_LoadU16(sp + 26u) + PE_LoadU32(sp + 72u)));
        PE_StoreU16(seg - 4u, (uint16_t)(PE_LoadU16(sp + 28u) + PE_LoadU32(sp + 76u)));
        PE_StoreU8(a0 + 18u, i >= 2u ? 1u : 0u);
        if (PE_LoadU8(a0 + 18u) == 1u) {
            pe_addr_t pl = PE_LoadU32(0x8009D254u);
            int32_t ax, az, bx, bz;
            uint32_t l1;
            ax = (int32_t)(PE_LoadU32(seg - 48u) - PE_LoadU32(seg - 116u));
            az = (int32_t)(PE_LoadU32(seg - 40u) - PE_LoadU32(seg - 108u));
            bx = (int32_t)(int16_t)PE_LoadU16(pl + 42u) - (int32_t)PE_LoadU32(seg - 116u);
            bz = (int32_t)(int16_t)PE_LoadU16(pl + 50u) - (int32_t)PE_LoadU32(seg - 108u);
            PE_StoreU32(sp + 120u, (uint32_t)ax);
            PE_StoreU32(sp + 124u, 0u);
            PE_StoreU32(sp + 128u, (uint32_t)az);
            PE_StoreU32(sp + 136u, (uint32_t)bx);
            PE_StoreU32(sp + 140u, 0u);
            PE_StoreU32(sp + 144u, (uint32_t)bz);
            func_800791D0(sp + 120u, sp + 136u, sp + 152u);
            l1 = func_80078004((uint32_t)(ax * ax) + (uint32_t)(az * az));
            func_80079FB4((int32_t)l1, (int32_t)func_80078004((uint32_t)(bx * bx) + (uint32_t)(bz * bz)));
            if ((int32_t)PE_LoadU32(sp + 156u) > 0)
                PE_StoreU16(sp + 82u, (uint16_t)rand_swing());
            else
                PE_StoreU16(sp + 82u, (uint16_t)-rand_swing());
            PE_StoreU16(sp + 80u, 0u);
            PE_StoreU16(sp + 84u, 0u);
        } else {
            PE_StoreU16(sp + 80u, PE_LoadU16(seg - 26u));
            PE_StoreU16(sp + 84u, 0u);
            PE_StoreU16(sp + 82u, PE_LoadU16(seg - 24u));
        }
        if (i >= 6u) {
            PE_StoreU16(sp + 80u, 0u);
            PE_StoreU16(sp + 82u, 0u);
            PE_StoreU16(sp + 84u, 0u);
        }
        func_800794C4(sp + 80u, m2);
        PE_StoreU32(sp + 108u, 0u);
        PE_StoreU32(sp + 112u, 0u);
        PE_StoreU32(sp + 116u, (uint32_t)-(int32_t)(int16_t)PE_LoadU16(a0 + 8u));
        gte_rt_from(m);
        gte_col(m2 + 0u, m + 0u);
        gte_col(m2 + 2u, m + 2u);
        gte_col(m2 + 4u, m + 4u);
        PE_GTE_WriteCtrl(5u, PE_LoadU32(m + 20u));
        PE_GTE_WriteCtrl(6u, PE_LoadU32(m + 24u));
        PE_GTE_WriteCtrl(7u, PE_LoadU32(m + 28u));
        PE_GTE_WriteData(0u, (uint32_t)PE_LoadU16(sp + 108u) | ((uint32_t)PE_LoadU16(sp + 112u) << 16));
        PE_GTE_WriteData(1u, PE_LoadU32(sp + 116u));
        PE_GTE_Command(0x480012u);
        PE_StoreU32(sp + 68u, PE_GTE_ReadData(9u));
        PE_StoreU32(sp + 72u, PE_GTE_ReadData(10u));
        PE_StoreU32(sp + 76u, PE_GTE_ReadData(11u));
        for (k = 0; k < 32u; k += 4u)
            PE_StoreU32(seg + k, PE_LoadU32(m + k));
    }
    PE_GuestFrame_Leave(sp, 208u);
}
