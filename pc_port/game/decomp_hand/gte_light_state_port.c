/*
 * Hand adapters — the field lighting-state setters called by the room init
 * func_8003F074 (gfx2 lane).  Both leaves are inline-GTE matched C
 * (src/func_8003F758.c, src/func_8003F798.c), so the generator cannot emit
 * them; before this file the generated func_8003F074 TU reached them as
 * no-op boundaries (audit item 22: "drops the GTE lighting setup").
 * GTE control registers go through the pe_gte state (psx-spx COP2 ctrl
 * 13-15 = RBK/GBK/BBK, 16-20 = LCM).
 */
#include "pe_guest_decomp.h"
#include "pe_sdk.h"

/* src/func_8003F758.c: BK = (r,g,b) << 4 (ctc2 $13-$15), then zero the
 * nine halfwords at dst (the light-direction matrix of the state block). */
void func_8003F758(pe_addr_t dst, int r, int g, int b)
{
    unsigned k;

    PE_GTE_SetBK(r << 4, g << 4, b << 4);
    for (k = 0; k < 9u; k++)
        PE_StoreU16(dst + k * 2u, 0u);
}

/* src/func_8003F798.c: light `index` colour.  index < 3 (unsigned) writes
 * the colour-matrix column (+0x20/+0x26/+0x2C halfword rows); the byte
 * triple at +0x40 + index*4 is written for any index; then the five words
 * at +0x20 load the LCM (ctc2 $16-$20). */
void func_8003F798(pe_addr_t state, int index, int r, int g, int b)
{
    pe_addr_t slot;

    if ((unsigned int)index < 3u) {
        PE_StoreU16(state + 0x20u + (pe_addr_t)index * 2u, (uint16_t)r);
        PE_StoreU16(state + 0x26u + (pe_addr_t)index * 2u, (uint16_t)g);
        PE_StoreU16(state + 0x2Cu + (pe_addr_t)index * 2u, (uint16_t)(uint16_t)b);
    }
    slot = state + (pe_addr_t)index * 4u;
    PE_StoreU8(slot + 0x40u, (uint8_t)r);
    PE_StoreU8(slot + 0x41u, (uint8_t)g);
    PE_StoreU8(slot + 0x42u, (uint8_t)b);
    PE_GTE_LoadLCM(state + 0x20u);
}
