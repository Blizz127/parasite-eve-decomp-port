/*
 * Phase 6E-B54K-T — func_8018FBC0 title-object freelist allocator.
 *
 * Retail overlay: [0x8018FBC0, 0x8018FD04), 81 words / 0x144.
 * Pops a 52-byte node from D_801D136C, links it into the live list at
 * D_801D137C/1378, zeroes the body, stamps type a0, binds the TIM base
 * from D_80193258[a0] + D_80193254, copies geometry from the 12-byte
 * descriptor at D_801D0D5C + a0*12, and installs draw/update fn ptrs
 * 0x80192FE8 / 0x8018F7F0. Returns the node.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_TITLE_FREELIST     0x801D136Cu
#define GA_TITLE_LIVE_TAIL    0x801D137Cu
#define GA_TITLE_LIVE_HEAD    0x801D1378u
#define GA_TITLE_DESC_TABLE   0x801D0D5Cu
#define GA_OVERLAY_DATA_ANCHOR 0x80193254u
#define GA_TITLE_STRIP_OFFSET  0x80193258u
#define GA_TITLE_DRAW_FN       0x80192FE8u
#define GA_TITLE_UPDATE_FN     0x8018F7F0u

pe_addr_t func_8018FBC0(int type)
{
    pe_addr_t node;
    pe_addr_t next;
    pe_addr_t live_tail;
    pe_addr_t desc;
    pe_addr_t tim;
    int32_t width;
    int32_t scaled;

    node = PE_LoadU32(GA_TITLE_FREELIST);
    live_tail = PE_LoadU32(GA_TITLE_LIVE_TAIL);
    next = PE_LoadU32(node);
    PE_StoreU32(node, 0u);
    PE_StoreU32(GA_TITLE_FREELIST, next);

    desc = GA_TITLE_DESC_TABLE + (uint32_t)type * 12u;
    if (live_tail != 0u) {
        PE_StoreU32(live_tail, node);
        PE_StoreU32(GA_TITLE_LIVE_TAIL, node);
    } else {
        PE_StoreU32(GA_TITLE_LIVE_TAIL, node);
        PE_StoreU32(GA_TITLE_LIVE_HEAD, node);
    }

    PE_StoreU32(node + 0x0Cu, 0u);
    PE_StoreU32(node + 0x10u, 0u);
    PE_StoreU32(node + 0x14u, 0u);
    PE_StoreU32(node + 0x18u, 0u);
    PE_StoreU16(node + 0x04u, 0u);
    PE_StoreU16(node + 0x06u, 0u);
    PE_StoreU16(node + 0x08u, 0u);
    PE_StoreU16(node + 0x0Au, 0u);
    PE_StoreU32(node + 0x1Cu, 0u);
    PE_StoreU32(node + 0x20u, 0u);
    PE_StoreU32(node + 0x24u, 0u);
    PE_StoreU32(node + 0x28u, 0u);
    PE_StoreU32(node + 0x2Cu, 0u);
    PE_StoreU32(node + 0x30u, 0u);

    PE_StoreU32(node + 0x2Cu, (uint32_t)type);
    tim = PE_LoadU32(GA_TITLE_STRIP_OFFSET + (uint32_t)type * 4u) +
          GA_OVERLAY_DATA_ANCHOR;
    PE_StoreU32(node + 0x18u, tim);

    PE_StoreU16(node + 0x04u, (uint16_t)PE_LoadU32(desc + 0u));
    PE_StoreU16(node + 0x06u, (uint16_t)PE_LoadU32(desc + 4u));

    width = (int32_t)(int16_t)PE_LoadU16(tim + 0x10u);
    /* Retail: (2*width) * 0x55555556 → mfhi − sign(2*width) ≡ (2*width)/3. */
    scaled = (int32_t)((((int64_t)(width << 1) * (int64_t)0x55555556) >> 32) -
                       ((width << 1) >> 31));
    PE_StoreU16(node + 0x08u, (uint16_t)scaled);
    PE_StoreU16(node + 0x0Au, PE_LoadU16(tim + 0x12u));

    PE_StoreU32(node + 0x0Cu, GA_TITLE_DRAW_FN);
    PE_StoreU32(node + 0x10u, GA_TITLE_UPDATE_FN);
    PE_StoreU32(node + 0x20u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(node + 0x06u));
    PE_StoreU32(node + 0x28u, PE_LoadU32(desc + 8u));
    PE_StoreU32(node + 0x24u, (uint32_t)-16);

    return node;
}
