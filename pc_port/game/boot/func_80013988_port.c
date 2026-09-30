/*
 * SEW8 — field-script wait/turn opcodes 18 / 92 / A5 / C3 / DB and the
 * camera pan 665A0 behind opcode 48.
 *
 * Native translations from the retail EXE (build/disc1.candidate.exe,
 * SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b), decoded with capstone
 * (local/live/mdis.py) and raw words for the GTE section. Not matching-C.
 *
 * VM conventions (func_80017018_port.c): D_8009D300 = current task,
 * D_8009D2F0 = current actor, D_8009CE00 = next PC (already past the
 * instruction). A handler that must run again next frame rewinds CE00 by
 * its own instruction size (8 + 4*argc), sets task+0x10 = 1 and returns 0.
 *
 * 1787C (op 18, 41w): terminate the task whose +0xA id == (u16)*a0. Self:
 *   task+8 |= 0x10, return 0 (yield, done). Otherwise walk the current
 *   actor's three task lists; the first match gets +8 |= 0x10, return 1.
 *   No match returns 3 (any nonzero continues).
 * 14FD8 (op 92, 76w): dest scale-fade. First visit: actor+0x98 &= ~0x40,
 *   task+8 |= 0x20, 3C5D8(actor+0x1B4, (s16)*a0), actor+0x250 |= 4; when
 *   the actor is D254 also D2E8 &= ~2, 3C5D8(B0CEC, arg), B0D88 |= 4; then
 *   rewind 12. Later visits: wait while actor+0x250 & 4 (rewind 12), else
 *   task+8 &= ~0x20 and return 1.
 * 155FC (op A5, 19w): (BCF88 & 7) == 4 or == 0 -> return 1; otherwise
 *   rewind 8 and wait.
 * 19B08 (op C3, 28w): first visit task+8 |= 0x20 and rewind 8; later wait
 *   while actor+0x98 & 8 (rewind 8), else task+8 &= ~0x20, return 1.
 * 13988 (op DB, 171w): walk the actor toward 16.16 (*a0, *a1) turning by at
 *   most *a2 per frame. speed = actor+0x20 (D254: 3708C(0x50000, +0x20)),
 *   then 3708C(speed, u16(+0x26) << 4). First visit latches target/rate in
 *   task+0x14/+0x18/+0x1C (already there -> return 1), normalizes a negative
 *   +0x3A by +0x1000 and sets task+8 |= 0x20. Every visit: heading =
 *   (0x1400 - 79FB4(z - tz, x - tx)) & 0xFFF (full 16.16 deltas);
 *   +0x68 = 3708C(-speed, sin << 4); +0x70 = 3708C(-speed, cos << 4);
 *   +0x3A turns toward heading by rate along the short arc; +0x3A &= 0xFFF;
 *   if |velocity|^2 (from s16 +0x6A/+0x72) < |delta|^2 (integer parts)
 *   rewind 20 and wait, else snap +0x28/+0x30 to the target, zero +0x68/
 *   +0x70, task+8 &= ~0x20, return 1.
 * 665A0 (150w): pan the field camera to centre a world point. Requires
 *   BCF88 & 0x40 (else -21). BCF98/9A = BCF8C/8E; V0 = ((s16)x, (s16)y -
 *   BCFFE, (s16)z) from the integer halves at pos+2/+6/+0xA; OFX/OFY =
 *   160/112; RT/TR from the matrix at BCFA4; RTPS; the screen point plus
 *   (w/2 - 160, h/2 - 112) of view record BCFFD (52-byte records at
 *   container + container[0x1C]; +0x28 w, +0x2A h) is clamped to the
 *   record's +0x2C/+0x2E (x) and +0x30/+0x32 (y) and stored in BCF9C/9E;
 *   BCFA2 = duration (or 30 when -1); BCFA0 = 1; BCF88 low nibble = mode|3
 *   (or (BCF88 & ~7) | 3 when mode == -1). Returns 0.
 * 17CE8 (op 48, matching C): 665A0(&D254->+0x28, -1, -1).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_800BCF88 0x800BCF88u

static int sew8_wait(uint32_t size)
{
    PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - size);
    PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, 1u);
    return 0;
}

int func_8001787C(pe_addr_t args)
{
    pe_addr_t task = PE_LoadU32(GA_D_8009D300);
    uint32_t id = PE_LoadU16(PE_LoadU32(args));
    pe_addr_t actor;
    unsigned int slot;
    if (PE_LoadU16(task + 0xAu) == id) {
        PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) | 0x10u));
        return 0;
    }
    actor = PE_LoadU32(GA_D_8009D2F0);
    for (slot = 0u; slot < 3u; slot++) {
        pe_addr_t t = PE_LoadU32(actor + 0xA0u + slot * 4u);
        while (t != 0u) {
            if (PE_LoadU16(t + 0xAu) == id) {
                PE_StoreU16(t + 8u, (uint16_t)(PE_LoadU16(t + 8u) | 0x10u));
                return 1;
            }
            t = PE_LoadU32(t + 0x24u);
        }
    }
    return 1;   /* src/func_8001787C.c: not found -> 1 */
}

int func_80014FD8(pe_addr_t args)
{
    pe_addr_t task = PE_LoadU32(GA_D_8009D300);
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
    uint16_t flags = PE_LoadU16(task + 8u);
    int value;
    if (flags & 0x20u) {
        if (PE_LoadU16(actor + 0x250u) & 4u)
            return sew8_wait(12u);
        PE_StoreU16(task + 8u, (uint16_t)(flags & 0xFFDFu));
        return 1;
    }
    PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) & ~0x40u);
    PE_StoreU16(task + 8u, (uint16_t)(flags | 0x20u));
    value = (int)(int16_t)PE_LoadU16(PE_LoadU32(args));
    func_8003C5D8(actor + 0x1B4u, value);
    PE_StoreU16(actor + 0x250u, (uint16_t)(PE_LoadU16(actor + 0x250u) | 4u));
    if (actor == PE_LoadU32(GA_D_8009D254)) {
        PE_StoreU32(GA_D_8009D2E8, PE_LoadU32(GA_D_8009D2E8) & ~2u);
        func_8003C5D8(0x800B0CECu, value);
        PE_StoreU16(0x800B0D88u, (uint16_t)(PE_LoadU16(0x800B0D88u) | 4u));
    }
    return sew8_wait(12u);
}

/* func_800155FC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800155FC_port.c (src/func_800155FC.c); hand port retired (port3 switch-over I). */

/* func_80019B08: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80019B08_port.c (src/func_80019B08.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

int func_80013988(pe_addr_t args)
{
    pe_addr_t actor = PE_LoadU32(GA_D_8009D2F0);
    pe_addr_t task = PE_LoadU32(GA_D_8009D300);
    uint32_t x = PE_LoadU32(actor + 0x28u), z = PE_LoadU32(actor + 0x30u);
    uint32_t speed, tx, tz, rate, heading;
    int32_t rot, d, dx, dz, vx, vz;
    if (actor == PE_LoadU32(GA_D_8009D254))
        speed = func_8003708C(0x50000u, PE_LoadU32(actor + 0x20u));
    else
        speed = PE_LoadU32(actor + 0x20u);
    speed = func_8003708C(speed, (uint32_t)PE_LoadU16(actor + 0x26u) << 4);
    if (PE_LoadU16(task + 8u) & 0x20u) {
        tx = PE_LoadU32(task + 0x14u);
        tz = PE_LoadU32(task + 0x18u);
        rate = PE_LoadU32(task + 0x1Cu);
    } else {
        tx = PE_LoadU32(PE_LoadU32(args));
        tz = PE_LoadU32(PE_LoadU32(args + 4u));
        if (x == tx && z == tz)
            return 1;
        rate = PE_LoadU32(PE_LoadU32(args + 8u));
        rot = (int32_t)(int16_t)PE_LoadU16(actor + 0x3Au);
        if (rot < 0)
            PE_StoreU16(actor + 0x3Au, (uint16_t)(rot + 0x1000));
        PE_StoreU32(task + 0x14u, tx);
        PE_StoreU32(task + 0x18u, tz);
        PE_StoreU32(task + 0x1Cu, rate);
        PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) | 0x20u));
    }
    heading = (uint32_t)(0x1400 - func_80079FB4((int32_t)(z - tz), (int32_t)(x - tx))) & 0xFFFu;
    speed = (uint32_t)(-(int32_t)speed);
    PE_StoreU32(actor + 0x68u, func_8003708C(speed, (uint32_t)func_80077CF4((int32_t)heading) << 4));
    PE_StoreU32(actor + 0x70u, func_8003708C(speed, (uint32_t)func_80077DC4((int32_t)heading) << 4));
    rot = (int32_t)(int16_t)PE_LoadU16(actor + 0x3Au);
    if (rot < (int32_t)heading) {
        d = (int32_t)heading - rot;
        if (d < 0x801) {
            if ((int32_t)rate < d) PE_StoreU16(actor + 0x3Au, (uint16_t)(rot + (int32_t)rate));
        } else if ((int32_t)rate < d) {
            PE_StoreU16(actor + 0x3Au, (uint16_t)(rot - (int32_t)rate));
        }
    } else {
        d = rot - (int32_t)heading;
        if (d < 0x801) {
            if ((int32_t)rate < d) PE_StoreU16(actor + 0x3Au, (uint16_t)(rot - (int32_t)rate));
        } else if ((int32_t)rate < d) {
            PE_StoreU16(actor + 0x3Au, (uint16_t)(rot + (int32_t)rate));
        }
    }
    dx = (int32_t)(tx - x) >> 16;
    dz = (int32_t)(tz - z) >> 16;
    vx = (int32_t)(int16_t)PE_LoadU16(actor + 0x6Au);
    vz = (int32_t)(int16_t)PE_LoadU16(actor + 0x72u);
    PE_StoreU16(actor + 0x3Au, (uint16_t)(PE_LoadU16(actor + 0x3Au) & 0xFFFu));
    if (vx * vx + vz * vz < dx * dx + dz * dz)
        return sew8_wait(20u);
    PE_StoreU32(actor + 0x28u, tx);
    PE_StoreU32(actor + 0x30u, tz);
    PE_StoreU32(actor + 0x68u, 0u);
    PE_StoreU32(actor + 0x70u, 0u);
    PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) & 0xFFDFu));
    return 1;
}

int func_800665A0(pe_addr_t pos, int duration, int mode)
{
    uint32_t flags = PE_LoadU32(GA_D_800BCF88);
    pe_addr_t container, view;
    uint32_t xy, sz;
    int32_t sx, sy, half_w, half_h, lo, hi;
    if (!(flags & 0x40u))
        return -21;
    PE_StoreU16(0x800BCF98u, PE_LoadU16(0x800BCF8Cu));
    PE_StoreU16(0x800BCF9Au, PE_LoadU16(0x800BCF8Eu));
    g_pe_gte.ofx = (int32_t)(0xA0u << 16);
    g_pe_gte.ofy = (int32_t)(0x70u << 16);
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    PE_GTE_SetV0((int16_t)PE_LoadU16(pos + 2u),
                 (int16_t)(PE_LoadU16(pos + 6u) - PE_LoadU16(0x800BCFFEu)),
                 (int16_t)PE_LoadU16(pos + 0xAu));
    PE_GTE_RTPS_coordinates(&xy, &sz);
    sx = (int32_t)(int16_t)(xy & 0xFFFFu);
    sy = (int32_t)(int16_t)(xy >> 16);
    container = PE_LoadU32(0x800B1624u);
    view = container + PE_LoadU32(container + 0x1Cu) + 52u * PE_LoadU8(0x800BCFFDu);
    half_w = (int32_t)(int16_t)PE_LoadU16(view + 0x28u) / 2 - 0xA0;
    half_h = (int32_t)(int16_t)PE_LoadU16(view + 0x2Au) / 2 - 0x70;
    sx += half_w;
    sy += half_h;
    lo = (int32_t)(int16_t)PE_LoadU16(view + 0x2Cu);
    hi = (int32_t)(int16_t)PE_LoadU16(view + 0x2Eu);
    if (sx < lo) sx = lo;
    if (hi < sx) sx = hi;
    lo = (int32_t)(int16_t)PE_LoadU16(view + 0x30u);
    hi = (int32_t)(int16_t)PE_LoadU16(view + 0x32u);
    if (sy < lo) sy = lo;
    if (hi < sy) sy = hi;
    PE_StoreU16(0x800BCF9Cu, (uint16_t)sx);
    PE_StoreU16(0x800BCF9Eu, (uint16_t)sy);
    PE_StoreU16(0x800BCFA2u, duration != -1 ? (uint16_t)duration : 0x1Eu);
    PE_StoreU16(0x800BCFA0u, 1u);
    flags = PE_LoadU32(GA_D_800BCF88);
    if (mode != -1)
        flags = (flags & ~0xFu) | ((uint32_t)mode | 3u);
    else
        flags = (flags & ~7u) | 3u;
    PE_StoreU32(GA_D_800BCF88, flags);
    return 0;
}

int func_80017CE8(pe_addr_t args)
{
    (void)args;
    (void)func_800665A0(PE_LoadU32(GA_D_8009D254) + 0x28u, -1, -1);
    return 1;
}
