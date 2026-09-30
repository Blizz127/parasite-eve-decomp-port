/*
 * Hand adapters — field-VM actor-targeting handlers (port_absent lane).
 *
 * The four leaves share one search, written out identically in each
 * src/func_XXXXXXXX.c: operand *f0 == 0 selects the player (D_8009D254);
 * otherwise walk the D_8009D20C actor list (next at +4) for the first actor
 * with +0x0C == *f0, +0x0D == *f4 and +0x98 bit 4 clear.
 *
 * Their Node/Ctx structs carry pointer members, so they are guest layout
 * (generator rule E18): the offsets used below are the leaves' own annotated
 * guest offsets (Node.next +0x04, f0C/f0D, f28/f2C/f30, f3A, f98, f18C +0x18C,
 * f1B4 +0x1B4, f254/f256/f258) and Ctx is three 32-bit guest pointers.
 */
#include "pe_guest_decomp.h"

/* The shared actor search; returns the actor or 0 (the leaves' `fail`). */
static pe_addr_t actor_find_lo(pe_addr_t ctx)
{
    int v = (int)PE_LoadU32(PE_LoadU32(ctx + 0u));
    int key;
    pe_addr_t p;

    if (v == 0)
        return PE_LoadU32(0x8009D254u);
    key = v;
    p = PE_LoadU32(0x8009D20Cu);
    while (p != 0u) {
        if ((int)PE_LoadU8(p + 0x0Cu) == key &&
            (int)PE_LoadU8(p + 0x0Du) == (int)PE_LoadU32(PE_LoadU32(ctx + 4u)) &&
            !(PE_LoadU32(p + 0x98u) & 0x10u))
            return p;
        p = PE_LoadU32(p + 4u);
    }
    return 0u;
}

/* src/func_80015108.c: bind the current state's +0x1B4 anim to the target's,
 * reset it with the D_800B89F8 table, target it, and reload the position
 * words +0x28/+0x2C/+0x30 from the +0x254/+0x256/+0x258 shorts (<< 16);
 * +0x98 |= 0x2000. */
int func_80015108(pe_addr_t arg0)
{
    pe_addr_t p = actor_find_lo(arg0);
    pe_addr_t s;

    if (p == 0u)
        return 1;
    func_8003DF50(PE_LoadU32(0x8009D2F0u) + 0x1B4u, p + 0x1B4u,
                  (short)PE_LoadU16(PE_LoadU32(arg0 + 8u)));
    func_8003A6A8(PE_LoadU32(0x8009D2F0u) + 0x1B4u, 0x800B89F8u);
    s = PE_LoadU32(0x8009D2F0u);
    PE_StoreU32(s + 0x18Cu, p);
    PE_StoreU32(s + 0x28u, (uint32_t)(int)(short)PE_LoadU16(s + 0x254u) << 16);
    PE_StoreU32(s + 0x2Cu, (uint32_t)(int)(short)PE_LoadU16(s + 0x256u) << 16);
    PE_StoreU32(s + 0x30u, (uint32_t)(int)(short)PE_LoadU16(s + 0x258u) << 16);
    PE_StoreU32(s + 0x98u, PE_LoadU32(s + 0x98u) | 0x2000u);
    return 1;
}

/* src/func_80015648.c: *f8 = heading from the current state to the target
 * (0x1400 - ratan2(dx, dz) folded into 0..0xFFF, minus state +0x3A);
 * no target -> *f8 = -1. */
int func_80015648(pe_addr_t arg0)
{
    pe_addr_t p = actor_find_lo(arg0);
    int y;
    int x;
    int t;

    if (p == 0u) {
        PE_StoreU32(PE_LoadU32(arg0 + 8u), 0xFFFFFFFFu);
        return 1;
    }
    /* retail `subu` wraps; the difference is then shifted arithmetically */
    y = (int)(PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 0x28u) - PE_LoadU32(p + 0x28u)) >> 16;
    x = (int)(PE_LoadU32(PE_LoadU32(0x8009D2F0u) + 0x30u) - PE_LoadU32(p + 0x30u)) >> 16;
    t = 0x1400 - func_80079FB4(x, y);
    if (t > 0x1000)
        t -= 0x1000;
    t -= (short)PE_LoadU16(PE_LoadU32(0x8009D2F0u) + 0x3Au);
    if (t < 0)
        t += 0x1000;
    PE_StoreU32(PE_LoadU32(arg0 + 8u), (uint32_t)t);
    return 1;
}

/* src/func_80019170.c: copy the target's +0x1B4 anim binding onto the current
 * state's with func_8003E0A4(state+0x1B4, target+0x1B4, *f8) and target it.
 * func_8003E0A4 has no pc_port implementation yet: loud boundary with the
 * three guest arguments. */
int func_80019170(pe_addr_t arg0)
{
    pe_addr_t p = actor_find_lo(arg0);

    if (p == 0u)
        return 1;
    /* func_8003E0A4 (absent_lo2_port.c) stores its second argument, the
     * guest address p + 0x1B4, as a word. */
    func_8003E0A4(PE_LoadU32(0x8009D2F0u) + 0x1B4u, (int)(p + 0x1B4u),
                  (int)(short)PE_LoadU16(PE_LoadU32(arg0 + 8u)));
    PE_StoreU32(PE_LoadU32(0x8009D2F0u) + 0x18Cu, p);
    return 1;
}

/* src/func_80019F04.c: target the actor (state +0x18C), mark it
 * (+0x98 |= 0x100000) and the state (+0x98 |= 0x600000). */
int func_80019F04(pe_addr_t arg0)
{
    pe_addr_t p = actor_find_lo(arg0);
    pe_addr_t s;

    if (p == 0u)
        return 1;
    s = PE_LoadU32(0x8009D2F0u);
    PE_StoreU32(s + 0x18Cu, p);
    PE_StoreU32(p + 0x98u, PE_LoadU32(p + 0x98u) | 0x100000u);
    PE_StoreU32(s + 0x98u, PE_LoadU32(s + 0x98u) | 0x600000u);
    return 1;
}

/* src/func_8001A680.c: activate handler `id` of a body's class and cascade to
 * its 0x200000-linked children.  Body is guest layout (E18): classId +0x0C,
 * handlerId +0x0E, handlerCount +0x0F, value/value2 +0x14/+0x18, flags
 * +0x98, link +0x18C, handler +0x1B0; D_800B0E98 rows are 0xC0 bytes of
 * 32-bit handler addresses; the body list head is D_8009D20C[0]. */
void pe_anim_set_log(const char *who, pe_addr_t actor, unsigned int command, void *caller);
void func_8001A680(pe_addr_t body, unsigned int id)
{
    pe_addr_t h;
    pe_addr_t b;

    pe_anim_set_log("1A680h", body, id, __builtin_return_address(0));

    h = PE_LoadU32(0x800B0E98u + (uint32_t)PE_LoadU8(body + 0x0Cu) * 0xC0u +
                   (uint32_t)(unsigned short)id * 4u);
    PE_StoreU8(body + 0x0Eu, (unsigned char)id);
    PE_StoreU32(body + 0x14u, 0u);
    PE_StoreU32(body + 0x18u, 0u);
    PE_StoreU32(body + 0x1B0u, h);
    PE_StoreU32(body + 0x98u, PE_LoadU32(body + 0x98u) & ~0x200u);
    /* body->handler[2]: a 0 handler makes retail read address 2, i.e. low
     * main RAM through the KUSEG mirror; PE_Translate folds KUSEG onto
     * KSEG0 (PE_RamCanonical), so the plain load is retail's read. */
    PE_StoreU8(body + 0x0Fu,
               (unsigned char)(PE_LoadU8(PE_LoadU32(body + 0x1B0u) + 2u) - 1u));

    if ((PE_LoadU32(body + 0x98u) & 0x100000u) != 0u) {
        b = PE_LoadU32(0x8009D20Cu);
        while (b != 0u) {
            if (PE_LoadU32(b + 0x18Cu) == body &&
                (PE_LoadU32(b + 0x98u) & 0x200000u) != 0u)
                func_8001A680(b, id & 0xFFFFu);
            b = PE_LoadU32(b + 4u);
        }
    }
}
