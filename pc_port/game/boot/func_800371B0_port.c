/*
 * PE-BTL9 — func_800371B0: four 56-byte window records + two
 * double-buffered GPU packets (translated retail, not matching src/).
 *
 * 169 words 0x800371B0..0x80037454, SHA-256 83a0b015…ba366.
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. All eight callees are
 * already translated GPU/BIOS leaves.
 *
 * Incoming a0 is stored at 0x120($gp) = D_8009CE90 and is not
 * dereferenced here. 3F074 @ 0x8003F244 selects
 *   (D_800B0CD8 & 0x40000000) ? D_800B162C : D_800B1628
 * (overlay +0x954 / +0x950; 6B4F8 writer family). USA boot
 * func_800527C8 ORs that bit, so the live m0005i path uses 162C.
 * Do not invent a battle-specific name for +0x950/+0x954 or
 * gp+0x120; TXT0 already consumes gp+0x120 as the 37870 scan base.
 *
 * Loop 1 (s4 = 0..3): stride 56 at D_800BCEA8. Two sb zeros at
 * +0x0C/+0x0D, lw, then +0x00/+0x04/+0x08/+0x09 clears, sh -1 at
 * +0x10, AND the loaded word with 0xFFF0FFFF / 0xFFEFFFFF /
 * 0xFFDFFFFF / 0xFE3FFFFF / 0xFDFFFFFF, sw back. Net: keep bits
 * 26-31 of +0x0C.
 *
 * Then sb 0 → gp+0x130 (D_8009CEA0), sb 0 → gp+0x160 (D_8009CED0),
 * sw 0 → gp+0x164 (D_8009CED4), sw a0 → gp+0x120.
 *
 * Loop 2 (s4 = 0..1):
 *   DR_MODE+setSprt+append at D_8009EC70 + i*28, mode lhu D_80091680
 *   SetShadeTex(sprt,1); u=0x70 w=0x18 h=0x0C; v=lhu D_8009167A;
 *     clut=lhu D_80091682 at +0x0E
 *   GetTPage(0,0,0,0) then DR_MODE+SetTile+append at D_8009ECA8+i*24
 *   TILE rgb=2,2,2; xy=0,0xAA; wh=0x140,0x36; SetSemiTrans(1)
 * 77CB4 fail jals 719E4(-1). Live packets fit the 17-word budget.
 *
 * Void return. Second TEXT caller is 0x80069C2C (a0=lw D_800B0E6C),
 * not the 3F074 live path. This is EXE-resident field-window init,
 * not battle overlay entry and not M2.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CE90 0x8009CE90u
#define GA_D_8009CEA0 0x8009CEA0u
#define GA_D_8009CED0 0x8009CED0u
#define GA_D_8009CED4 0x8009CED4u
#define GA_D_8009167A 0x8009167Au
#define GA_D_80091680 0x80091680u
#define GA_D_80091682 0x80091682u
#define GA_D_800BCEA8 0x800BCEA8u
#define GA_D_8009EC70 0x8009EC70u
#define GA_D_8009EC78 0x8009EC78u
#define GA_D_8009EC86 0x8009EC86u
#define GA_D_8009ECA8 0x8009ECA8u
#define GA_D_8009ECB0 0x8009ECB0u

#define REC_STRIDE 56u
#define SPRT_STRIDE 28u
#define TILE_STRIDE 24u

void func_800371B0(pe_addr_t a0)
{
    unsigned int i;
    uint32_t word;
    pe_addr_t rec;
    pe_addr_t sprt_head;
    pe_addr_t sprt;
    pe_addr_t tile_head;
    pe_addr_t tile;
    uint32_t tpage;

    for (i = 0; i < 4u; i++) {
        rec = GA_D_800BCEA8 + i * REC_STRIDE;
        PE_StoreU8(rec + 0x0Cu, 0u);
        PE_StoreU8(rec + 0x0Du, 0u);
        word = PE_LoadU32(rec + 0x0Cu);
        PE_StoreU8(rec, 0u);
        PE_StoreU32(rec + 4u, 0u);
        PE_StoreU8(rec + 8u, 0u);
        PE_StoreU8(rec + 9u, 0u);
        PE_StoreU16(rec + 0x10u, 0xFFFFu);
        word &= 0xFFF0FFFFu;
        word &= 0xFFEFFFFFu;
        word &= 0xFFDFFFFFu;
        word &= 0xFE3FFFFFu;
        word &= 0xFDFFFFFFu;
        PE_StoreU32(rec + 0x0Cu, word);
    }

    PE_StoreU8(GA_D_8009CEA0, 0u);
    PE_StoreU8(GA_D_8009CED0, 0u);
    PE_StoreU32(GA_D_8009CED4, 0u);
    PE_StoreU32(GA_D_8009CE90, a0);

    for (i = 0; i < 2u; i++) {
        sprt_head = GA_D_8009EC70 + i * SPRT_STRIDE;
        func_80077C84(sprt_head, 0u, 1u, PE_LoadU16(GA_D_80091680));
        sprt = sprt_head + 8u;
        func_80077C04(sprt);
        if (func_80077CB4(sprt_head, sprt) != 0)
            func_800719E4((uint32_t)-1);

        sprt = GA_D_8009EC78 + i * SPRT_STRIDE;
        func_80077B34(sprt, 1u);
        PE_StoreU8(sprt + 0x0Cu, 0x70u);
        PE_StoreU16(sprt + 0x10u, 0x18u);
        PE_StoreU16(sprt + 0x12u, 0x0Cu);
        PE_StoreU8(sprt + 0x0Du, (uint8_t)PE_LoadU16(GA_D_8009167A));
        PE_StoreU16(GA_D_8009EC86 + i * SPRT_STRIDE, PE_LoadU16(GA_D_80091682));

        tpage = func_80077A64(0u, 0u, 0u, 0u);
        tile_head = GA_D_8009ECA8 + i * TILE_STRIDE;
        func_80077C84(tile_head, 0u, 1u, tpage & 0xFFFFu);
        tile = tile_head + 8u;
        func_80077C44(tile);
        if (func_80077CB4(tile_head, tile) != 0)
            func_800719E4((uint32_t)-1);

        tile = GA_D_8009ECB0 + i * TILE_STRIDE;
        PE_StoreU8(tile + 4u, 2u);
        PE_StoreU8(tile + 5u, 2u);
        PE_StoreU8(tile + 6u, 2u);
        PE_StoreU16(tile + 0x0Cu, 0x140u);
        PE_StoreU16(tile + 0x0Eu, 0x36u);
        PE_StoreU16(tile + 8u, 0u);
        PE_StoreU16(tile + 0x0Au, 0xAAu);
        func_80077B04(tile, 1u);
    }
}
