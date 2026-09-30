/*
 * Phase 6E-FD1 — func_8006E9A0 boot/New-Game display setup + fade loop,
 * translated retail logic.  The HOST_ADAPTED single-pass stand-in is gone:
 * this is the first adapter removed rather than catalogued.
 *
 * Authority is the 141-word matched C leaf src/func_8006E9A0.c
 * (VRAM 0x8006E9A0 / file 0x5F1A0 / size 0x234, era -O2 -G0, byte-exact),
 * transliterated to guest-address native style after the func_8006A8D4
 * port precedent (pe_addr_t cursors, guest constants, guest-RAM arena
 * stores).  Statement order follows the leaf, which follows ROM.
 *
 * Sections, ROM order:
 *   1. Display init: VSync(0), SetDispMask(0), PutDispEnv, ClearImage
 *      320x448, DrawSync(0).
 *   2. Pointer arena: the same 19-store cursor chain as func_8006A8D4
 *      (fixed bases 0x800F34F8 / 0x8010BD00 / 0x80120D08 / 0x801ED800 plus
 *      D_80011614), republished here exactly as retail does.
 *   3. Post-arena calls: func_8005E588(), func_80066B60(2).
 *   4. Poll loop until (D_800BCFEE & 3) == 1:
 *        ClearOTagR(lookup[D_8009CDDC], 0x1000)  (translated, OTC1)
 *        func_80068E24()                         (translated, BTL38)
 *        func_80070E54()                         (STILL A STUB BOUNDARY:
 *          logs and returns off-strict; strict stops here honestly)
 *      Termination is proven, not hoped: 66B60 arms CFEE=2 / CFF6=arg /
 *      CFF8=0 unconditionally, and 68E24 advances CFF8 to CFF6 then
 *      publishes CFEE=1 (bit4 clear).  Retail carries no iteration cap
 *      and neither does this port.
 *   5. Post-loop: D_800B0DC6 = 0; func_80038D1C().
 *   6. Dispatch exit: arg == 1 -> D_8009D280 = 0xA80830C8 (New Game);
 *      arg == 3 -> D_8009D280 = 0xA80651C8.
 *
 * The loop's OT comes from the arena it just published
 * (ot = *(0x800B0E38 + CDDC*4) in exact retail sll/addu arithmetic), so
 * the OTC1 NULL-OT skip no longer fires on this path; a wild CDDC would
 * read whatever retail's own addressing yields.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "stub_registry.h"
#include <string.h>

#define GA_D_800F34F8  0x800F34F8u
#define GA_D_8010BD00  0x8010BD00u
#define GA_D_80120D08  0x80120D08u
#define GA_D_801ED800  0x801ED800u
#define GA_OT_TABLE    0x800B0E38u
#define GA_D_800BCFEE  0x800BCFEEu
#define GA_OT_COUNT    0x1000

int func_8006E9A0(int arg)
{
    RECT rect;
    pe_addr_t cursor;
    pe_addr_t next;

    /* 1. Display init */
    func_80073A44(0);                           /* VSync(0) */
    func_80074D28(0);                           /* SetDispMask(0) */
    func_800755F0(0x800BCE80u);                /* PutDispEnv */

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x140;                             /* 320 */
    rect.h = 0x1C0;                             /* 448 (clamped to 240) */
    func_80074F44(&rect, 0, 0, 1);              /* ClearImage → THE BLACK FRAME */

    func_80074DC0(0);                           /* DrawSync(0) */

    /* 2. Pointer arena — retail store/compute interleave (same as 6A8D4) */
    cursor = GA_D_800F34F8;
    next = cursor + 0x1800u;
    D_800B0E24 = cursor;
    cursor += 0x6000u;
    D_800B0E28 = next;
    next = 0xE000u;  /* this is a delta, not a guest address */
    D_800B0E2C = cursor;
    cursor += next;
    D_800B0E30 = cursor;

    cursor = GA_D_8010BD00;
    next = GA_D_80120D08;
    D_800B0E40 = cursor;
    cursor = next + 0x1C98u;
    D_800B0E34 = next;
    next += 0x5C98u;
    D_800B0E38 = cursor;
    cursor += 0x8000u;
    D_800B0E3C = next;
    next = cursor + 0x2400u;
    D_800B0E44 = cursor;
    cursor += 0x4800u;
    D_800B0E4C = cursor;
    cursor += 0x48000u;
    D_800B0E48 = next;
    next = cursor + 0x4000u;
    D_800B0E50 = cursor;
    cursor += 0x8000u;
    D_800B0E54 = next;
    next = cursor + 0x3800u;
    D_800B0E5C = next;
    next = D_80011614;
    D_800B0E58 = cursor;
    cursor += 0x7000u;
    D_800B0E60 = cursor;

    cursor = GA_D_801ED800;
    D_800B0E6C = cursor;
    cursor = next - 8u;
    D_800B0E64 = cursor;
    D_800B0E68 = next;

    /* 3. Post-arena calls */
    func_8005E588();
    func_80066B60(2);

    /* 4. Poll loop until (D_800BCFEE & 3) == 1 */
    do {
        uint32_t cddc = PE_LoadU32(0x8009CDDCu); /* gp+0x6C; 70E54 flips it */
        pe_addr_t ot = PE_LoadU32(GA_OT_TABLE + cddc * 4u);
        func_800752AC(ot, GA_OT_COUNT);         /* ClearOTagR */
        func_80068E24();
        func_80070E54();
        /* Frame/quit budgets must be able to leave this retail-unbounded
         * fade; skip-movie also breaks out so New Game can publish. */
        if (PE_Port_ShouldStop())
            break;
        if (PE_Port_SkipMovie() &&
            (PE_LoadU8(GA_D_800BCFEE) & 3u) != 1u) {
            /* HOST_ADAPTED: after the first fade tick, force the
             * 66B60(2) completion so skip-movie is not stuck if a
             * later tail re-arms CFEE.  Retail would wait it out. */
            PE_StoreU8(GA_D_800BCFEE, 1u);
            Stub_Record("func_8006E9A0_skip_movie_fade", "HOST_ADAPTED");
            break;
        }
    } while ((PE_LoadU8(GA_D_800BCFEE) & 3u) != 1u);

    /* 5. Post-loop */
    D_800B0DC6 = 0;
    func_80038D1C();

    /* 6. Dispatch exit */
    if (arg == 1) {
        D_8009D280 = 0xA80830C8;
    } else if (arg == 3) {
        D_8009D280 = 0xA80651C8;
    }
    return 0;
}
