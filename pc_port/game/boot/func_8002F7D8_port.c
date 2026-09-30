/*
 * PE-CH1 — func_8002F7D8: opcode 0x6F slot alloc (translated retail).
 *
 * Complete retail body (102 words / 0x198, exe 0x8002F7D8–0x8002F96C,
 * file offset 0x1FFD8). Words dumped from SHA-1-exact
 * build/disc1.candidate.exe (452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * yaml places this as the [0x11718, asm] prefix before matching
 * func_8002F970 at 0x20170. This worktree has no era/asm split, so the
 * leaf cannot yet be a matching src/ unit. Native port follows the
 * 6536C / 2F9CC SlotRecord family.
 *
 * Opcode 0x6F jump table D_800910A0[0x6F] @ 0x8009125C = wrapper
 * func_80018954, which loads a0 = *(D_8009D2F0) (current actor) and
 * jal this leaf. Sole jal of 2F7D8 is 0x80018964. Internal jal
 * func_8001A680 @ 0x8002F924 now starts the native command clip.
 *
 * ROM (same 7×220 SlotRecord as matching 2F9CC/2F970):
 *
 *   copy 216B D_800109B0 -> stack (0xD0 loop + 8-byte tail)
 *   for i = 0..6 (andi 0xFF, sltiu 7):
 *     if inUse(D_800A5D58[i]) != 0: continue
 *     inUse = 1
 *     copy 216B stack -> body (D_800A5D5C + i*220)
 *     *actor = body
 *     D_8009D2EC = (lbu + 1) as byte; body+7 = that byte
 *     body+8 = 1 << i
 *     if (actor+0x98 & 0x2000) == 0:
 *       body+0x18 = body+0x1C
 *       jal func_8001A680(actor, 2)
 *       D_8009D2A0 = (lbu + 1) as byte
 *     return
 *   return                         # table full: no store
 *
 * Stack staging cannot alias rodata/BSS, so the port copies
 * template → body in the same 16+8 store order. ATK21 restores the
 * full command selection, including linked child animation.
 *
 * BTL1: D_800A5D58 inUse==1 after 0x6F
 * (docs/evidence/pe-btl0-field-battle-handoff/).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800109B0 0x800109B0u
#define GA_D_800A5D58 0x800A5D58u
#define GA_D_8009D2A0 0x8009D2A0u
#define GA_D_8009D2EC 0x8009D2ECu
#define SLOT_STRIDE   220u

static void copy_216(pe_addr_t dst, pe_addr_t src)
{
    unsigned int off;

    for (off = 0; off < 0xD0u; off += 16u) {
        PE_StoreU32(dst + off + 0u, PE_LoadU32(src + off + 0u));
        PE_StoreU32(dst + off + 4u, PE_LoadU32(src + off + 4u));
        PE_StoreU32(dst + off + 8u, PE_LoadU32(src + off + 8u));
        PE_StoreU32(dst + off + 12u, PE_LoadU32(src + off + 12u));
    }
    PE_StoreU32(dst + 0xD0u, PE_LoadU32(src + 0xD0u));
    PE_StoreU32(dst + 0xD4u, PE_LoadU32(src + 0xD4u));
}

void func_8002F7D8(pe_addr_t actor)
{
    unsigned char i;

    for (i = 0; i < 7; i++) {
        pe_addr_t rec;
        pe_addr_t body;
        unsigned int serial;

        rec = GA_D_800A5D58 + (unsigned int)i * SLOT_STRIDE;
        if (PE_LoadU32(rec) != 0u)
            continue;

        PE_StoreU32(rec, 1u);
        body = rec + 4u;
        copy_216(body, GA_D_800109B0);
        PE_StoreU32(actor, body);

        serial = (PE_LoadU8(GA_D_8009D2EC) + 1u) & 0xFFu;
        PE_StoreU8(GA_D_8009D2EC, (uint8_t)serial);
        PE_StoreU8(body + 7u, (uint8_t)serial);
        PE_StoreU32(body + 8u, 1u << i);

        if ((PE_LoadU32(actor + 0x98u) & 0x2000u) == 0u) {
            PE_StoreU32(body + 0x18u, body + 0x1Cu);
            func_8001A680_command_cut(actor,2u);
            PE_StoreU8(GA_D_8009D2A0,
                       (uint8_t)((PE_LoadU8(GA_D_8009D2A0) + 1u) & 0xFFu));
        }
        return;
    }
}
