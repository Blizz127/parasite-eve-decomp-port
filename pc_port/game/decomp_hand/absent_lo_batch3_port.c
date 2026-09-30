/*
 * Hand adapters — leaves below 0x80060000 that became matched in batch 3
 * (port_absent lane, batch 10).  Conventions as in absent_lo_small_port.c.
 */
#include "pe_guest_decomp.h"

/* decomp-derived callee (pc_port/game/decomp/func_80030640_port.c;
 * src/func_80030640.c: `void func_80030640(void)`). */
void func_80030640(void);

/* src/func_80014E30.c: field-VM opcode that loads and starts the two-part
 * overlay.  D_800B0CD8 |= 0x8200; reset drawing/callbacks; read section 0
 * (sectors D_8009315E[0..1] from the base at D_800B0CD8+0x100) into
 * D_8001160C[0], retrying the issue while it returns -1 and restarting it
 * when the completion poll reports -1; flush; the same for section 1 into
 * D_8001160C[1]; then hand the overlay the { end-of-section-1, 0 } request
 * (the leaf's stack `Req`, here the lane's guest temp) and start it with
 * the operand **a0.  Returns 1. */
int func_80014E30(pe_addr_t a0)
{
    const pe_addr_t s = 0x800B0CD8u;
    const pe_addr_t req = PE_HAND_LO_STACK_TEMP;
    int r;
    int k;

    PE_StoreU32(s, PE_LoadU32(s) | 0x8200u);
    (void)func_80074DC0(0);
    func_80073A44(0);
    func_80074D28(0);
    func_80074A44(1);
    for (k = 0; k < 2; k++) {
        for (;;) {
            while (func_8006E6A8(
                       (int)PE_LoadU32(s + 0x100u) +
                           (int)PE_LoadU16(0x8009315Eu + (uint32_t)k * 2u),
                       (int)PE_LoadU32(0x8001160Cu + (uint32_t)k * 4u),
                       (int)PE_LoadU16(0x8009315Eu + (uint32_t)(k + 1) * 2u) -
                           (int)PE_LoadU16(0x8009315Eu + (uint32_t)k * 2u)) == -1) {
            }
            while ((r = func_8006E7E8()) != 0) {
                if (r == -1)
                    break;
            }
            if (r != -1)
                break;
        }
        func_80072714();
        func_800726C4();
        func_80072724();
    }
    PE_StoreU32(req + 4u, 0u);
    PE_StoreU32(req + 0u, PE_LoadU32(0x8001160Cu + 4u) +
                ((uint32_t)((int)PE_LoadU16(0x8009315Eu + 4u) -
                            (int)PE_LoadU16(0x8009315Eu + 2u)) << 11));
    func_801216C4(1, req);
    func_80121C04((short)PE_LoadU16(PE_LoadU32(a0)));
    func_801223A8(1);
    return 1;
}

/* src/func_80029810.c: field (re)entry reset for room `id`.  Clears the
 * scene state words, binds D_8009D278 to the player's record (player+0),
 * resets the 10 D_800A7FF0 {0, -1} pairs and the 7 D_800B8A90 words,
 * refreshes the player (func_800293F4), clamps the record's HP (+8) into
 * (0, +0x28] — 0 or less becomes 0x10000, the cap also sets +0x34 = 0xF0 —
 * installs the player update 0x8001D268 at +0x194, sets D_8009D27C from the
 * stats block (+0x238)->+0x18 - 100, then enters the room and binds the
 * record's +0x12 handler.  Actor/Rec are guest layout (E18): the offsets
 * are the leaf's struct offsets.  `*(u8 *)&D_8009D1AC = 0` clears the low
 * byte (little-endian) before the & ~0x300. */
void func_80029810(unsigned char id)
{
    unsigned char i;
    pe_addr_t r;

    PE_StoreU32(0x8009D1E8u, 0u);
    PE_StoreU32(0x8009D290u, 0u);
    PE_StoreU32(0x8009D28Cu, 0u);
    PE_StoreU8(0x8009CE7Cu, 0u);
    PE_StoreU8(0x8009CE78u, 0u);
    PE_StoreU8(0x8009D288u, 0u);
    PE_StoreU8(0x8009CE74u, 0u);
    PE_StoreU32(0x8009D278u, PE_LoadU32(PE_LoadU32(0x8009D254u)));
    func_80020EFC();
    PE_StoreU8(0x8009D1ACu, 0u);
    PE_StoreU32(0x8009D1A8u, 0u);
    PE_StoreU8(0x8009D1CEu, 0u);
    PE_StoreU8(0x8009D235u, 0u);
    PE_StoreU32(0x8009D304u, 0u);
    PE_StoreU16(0x8009D21Cu, 0u);
    PE_StoreU32(0x8009D1ACu, PE_LoadU32(0x8009D1ACu) & ~0x300u);
    for (i = 0; i < 10u; i++) {
        PE_StoreU16(0x800A7FF0u + (uint32_t)i * 4u + 0u, 0u);
        PE_StoreU16(0x800A7FF0u + (uint32_t)i * 4u + 2u, 0xFFFFu);
    }
    for (i = 0; i < 7u; i++)
        PE_StoreU32(0x800B8A90u + (uint32_t)i * 4u, 0u);
    func_80071A64((int)PE_LoadU32(0x8009D250u));
    func_800293F4(0u);
    func_800209F0();
    r = PE_LoadU32(0x8009D278u);
    if ((int)PE_LoadU32(r + 8u) <= 0) {
        PE_StoreU32(r + 8u, 0x10000u);
    } else if ((int)PE_LoadU32(r + 8u) >= (int)PE_LoadU32(r + 0x28u)) {
        PE_StoreU32(r + 8u, PE_LoadU32(r + 0x28u));
        PE_StoreU32(r + 0x34u, 0xF0u);
    }
    func_80030640();
    PE_StoreU32(PE_LoadU32(0x8009D254u) + 0x194u, 0x8001D268u);
    PE_StoreU16(0x8009D27Cu, (unsigned short)(
        (int)PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D254u) + 0x238u) + 0x18u) - 100));
    func_800339A0(id);
    func_8001A680(PE_LoadU32(0x8009D254u), PE_LoadU8(PE_LoadU32(0x8009D278u) + 0x12u));
}

/* src/func_8005F698.c: draw 0xFF-terminated text as wrapped lines of at most
 * `width` pixels.  Each line pushes the pen (D_8009D124/128) on the
 * D_8009D12C stack (limit D_800A22B0), emits glyphs through func_8005EED4
 * while summing their advance (func_8005DC28 width nibble + spacing
 * D_8009CDB0: 2 for digits and code 0xF, else 1; codes >= 0x100 use -0x13;
 * bytes >= 0xFA are page prefixes carried in D_8009D0D8), then pops the pen
 * (floor D_800A2270) and moves it down 0xE.  Stack over/underflow calls the
 * empty func_800527C0 stub — its argument (2 / 3) is unobservable because
 * that leaf's matched C is `void func_800527C0(void) {}`. */
void func_8005F698(pe_addr_t p, int width)
{
    pe_addr_t sp;
    pe_addr_t sq;
    int w;
    int c;
    int v;
    int t;

    while (PE_LoadU8(p) != 0xFFu) {
        sp = PE_LoadU32(0x8009D12Cu);
        if (sp < 0x800A22B0u) {
            int x0 = (int)PE_LoadU32(0x8009D124u);
            int y0 = (int)PE_LoadU32(0x8009D128u);
            PE_StoreU32(0x8009D12Cu, sp + 8u);
            PE_StoreU32(sp + 0u, (uint32_t)x0);
            PE_StoreU32(sp + 4u, (uint32_t)y0);
        } else {
            func_800527C0();
        }
        for (w = 0; w < width;) {
            if (PE_LoadU8(p) == 0xFFu)
                break;
            func_8005EED4(PE_LoadU8(p));
            c = PE_LoadU8(p);
            p++;
            v = c & 0xFF;
            if ((int)PE_LoadU32(0x8009D0D8u) != 0) {
                v = v + ((int)PE_LoadU32(0x8009D0D8u) << 8);
                PE_StoreU32(0x8009D0D8u, 0u);
            }
            if ((unsigned int)(c & 0xFF) >= 0xFAu) {
                PE_StoreU32(0x8009D0D8u, (uint32_t)((c & 0xFF) - 0xFA));
                v = -1;
            }
            if (v >= 0) {
                t = 0;
                if (v < 0xA || v == 0xF)
                    t = 1;
                PE_StoreU32(0x8009CDB0u, (uint32_t)(t + 1));
                if (v >= 0x100)
                    v -= 0x13;
                w += (int)((func_8005DC28((uint32_t)v) >> 4) & 0xFu) +
                     (int)PE_LoadU32(0x8009CDB0u);
            }
        }
        sq = PE_LoadU32(0x8009D12Cu);
        if (0x800A2270u < sq) {
            PE_StoreU32(0x8009D12Cu, sq - 8u);
            PE_StoreU32(0x8009D124u, PE_LoadU32(sq - 8u));
            PE_StoreU32(0x8009D128u, PE_LoadU32(sq - 4u));
        } else {
            func_800527C0();
        }
        PE_StoreU32(0x8009D128u, PE_LoadU32(0x8009D128u) + 0xEu);
    }
}
