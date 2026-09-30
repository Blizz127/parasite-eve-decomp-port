/*
 * PE-BTL38 — func_80068E24 fade tick (translated retail, not
 * matching src/). Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 202 words 0x80068E24..0x8006914C, SHA-256 bee869b9…4267.
 * Zero jal. v0=0.
 *
 * Callers: 3F3C4 @ 0x8003F588 (field tick) and 6E9A0 @ 0x8006EB4C
 * (boot display). CFEE&3==0 returns. CFEE&3==2 interpolates
 * CFE8/EA/EC toward CFF0/F2/F4 by CFF8/(CFF6-1), writes the
 * CDDC slot, then CFF8++. When CFF8>=CFF6, CFEE=0 if CFEE&4
 * else CFEE=1. Other CFEE&3 values copy CFE8/EA/EC and skip
 * the timer. Retail clamps the denominator positive, making div-break unreachable.
 *
 * Live type-0 0x86 sets CFEE=6, CFF6=60, CFF8=0. 0x9C waits
 * until (CFEE&3)<2.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCF88 0x800BCF88u
#define GA_D_800BCFE8 0x800BCFE8u
#define GA_D_800BCFEA 0x800BCFEAu
#define GA_D_800BCFEC 0x800BCFECu
#define GA_D_800BCFEE 0x800BCFEEu
#define GA_D_800BCFF0 0x800BCFF0u
#define GA_D_800BCFF2 0x800BCFF2u
#define GA_D_800BCFF4 0x800BCFF4u
#define GA_D_800BCFF6 0x800BCFF6u
#define GA_D_800BCFF8 0x800BCFF8u
#define GA_D_8009CDDC 0x8009CDDCu
#define GA_D_800B0E38 0x800B0E38u

static uint8_t pe_68e24_lerp(pe_addr_t cur_a, pe_addr_t dest_a,
                             int32_t t, int32_t den)
{
    int32_t cur;
    int32_t dest;

    cur = (int32_t)(int16_t)PE_LoadU16(cur_a);
    dest = (int32_t)(int16_t)PE_LoadU16(dest_a);
    /* Original mult/mflo wraps before signed div. den is clamped to at
     * least1 by the caller, so neither retail division trap is reachable. */
    uint32_t product = (uint32_t)(cur - dest) * (uint32_t)t;
    int64_t signed_product = product < 0x80000000u
        ? (int64_t)product : (int64_t)product - INT64_C(0x100000000);
    return (uint8_t)(dest + signed_product / den);
}

static void pe_68e24_link(unsigned int cddc)
{
    pe_addr_t base = GA_D_800BCF88;
    pe_addr_t slot = base + (cddc << 4);
    pe_addr_t rec8 = base + (cddc << 3);
    pe_addr_t row = GA_D_800B0E38 + cddc * 4u;
    pe_addr_t pkt;
    uint32_t t0 = 0x00FFFFFFu;
    uint32_t t1 = 0xFF000000u;
    uint32_t slot30;
    uint32_t phys30;
    uint32_t cmd;
    uint32_t rec50;
    uint32_t phys50;

    phys30 = (base + (cddc << 4) + 0x30u) & t0;
    pkt = PE_LoadU32(row);
    slot30 = PE_LoadU32(slot + 0x30u);
    PE_StoreU32(slot + 0x30u,
                (slot30 & t1) | (PE_LoadU32(pkt + 0xCu) & t0));
    pkt = PE_LoadU32(row);
    PE_StoreU32(pkt + 0xCu,
                (PE_LoadU32(pkt + 0xCu) & t1) | phys30);
    PE_StoreU8(rec8 + 0x53u, 1u);
    cmd = 0xE1000400u
        | ((unsigned int)(PE_LoadU8(base + 0x67u) & 3u) << 5);
    PE_StoreU32(rec8 + 0x54u, cmd);
    pkt = PE_LoadU32(row);
    rec50 = PE_LoadU32(rec8 + 0x50u);
    PE_StoreU32(rec8 + 0x50u,
                (rec50 & t1) | (PE_LoadU32(pkt + 0xCu) & t0));
    pkt = PE_LoadU32(row);
    phys50 = (base + (cddc << 3) + 0x50u) & t0;
    PE_StoreU32(pkt + 0xCu,
                (PE_LoadU32(pkt + 0xCu) & t1) | phys50);
}

int func_80068E24(void)
{
    unsigned int cfee;
    unsigned int low;
    unsigned int bit4;
    unsigned int cddc;
    pe_addr_t slot;
    int32_t den;
    int32_t t;
    uint16_t next;

    cfee = PE_LoadU8(GA_D_800BCFEE);
    low = cfee & 3u;
    bit4 = cfee & 4u;
    if (low == 0u)
        return 0;

    cddc = PE_LoadU32(GA_D_8009CDDC);
    slot = GA_D_800BCF88 + (cddc << 4);

    if (low == 2u) {
        den = (int32_t)PE_LoadU16(GA_D_800BCFF6) - 1;
        if (den <= 0)
            den = 1;
        t = (int32_t)PE_LoadU16(GA_D_800BCFF8);
        PE_StoreU8(slot + 0x34u,
                   pe_68e24_lerp(GA_D_800BCFE8, GA_D_800BCFF0, t, den));
        PE_StoreU8(slot + 0x35u,
                   pe_68e24_lerp(GA_D_800BCFEA, GA_D_800BCFF2, t, den));
        PE_StoreU8(slot + 0x36u,
                   pe_68e24_lerp(GA_D_800BCFEC, GA_D_800BCFF4, t, den));
    } else {
        PE_StoreU8(slot + 0x34u, (uint8_t)PE_LoadU16(GA_D_800BCFE8));
        PE_StoreU8(slot + 0x35u, (uint8_t)PE_LoadU16(GA_D_800BCFEA));
        PE_StoreU8(slot + 0x36u, (uint8_t)PE_LoadU16(GA_D_800BCFEC));
    }

    pe_68e24_link(cddc);

    if (low != 2u)
        return 0;
    next = (uint16_t)(PE_LoadU16(GA_D_800BCFF8) + 1u);
    PE_StoreU16(GA_D_800BCFF8, next);
    if (next < PE_LoadU16(GA_D_800BCFF6))
        return 0;
    PE_StoreU8(GA_D_800BCFEE, (bit4 == 0u) ? 1u : 0u);
    return 0;
}
