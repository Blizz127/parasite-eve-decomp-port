/*
 * Phase 6E-B54K-R/Y/T — func_801909B4 through MoveImage, display setup, the
 * one-time overlay-local initializer, Disc-1 FMV (func_80192CE8), post-
 * movie environment restore, title present (func_8018F2F4), freelist seed,
 * title-object alloc (func_8018FBC0), title-loop header (bank flip +
 * 425DC with HOST_ADAPTED TestEvent), and the input-pump frontier at
 * 0x801911F8 (jal func_8003EB04).
 *
 * Retail overlay body: [0x801909B4,0x801918F8), 977 words.
 * 0x801911C0 title main loop, 0x80191410 teardown, menu-choice dispatch
 * (0xA0 NEW GAME → 1; CONTINUE card screen → func_8005C498 result;
 * timeout → attract replay from 0x80190BCC) and 0x8019172C epilogue are
 * translated.  PE_Port_SetTitleLoopBudget cuts the loop for tests.
 * HOST_ADAPTED --skip-movie still short-circuits to FinishTitleMenu(1).
 * Without it, Disc 1's saved bit plays FMV001; both arms then run title
 * present + freelist + one object alloc + loop header + one 425DC pump.
 * --skip-opening-menu returns New-Game after present/freelist, before
 * alloc / loop.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "stub_registry.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GA_DRAWENV_SOURCE_0  0x800BCDC8u
#define GA_DRAWENV_SOURCE_1  0x800BCE24u
#define GA_DISPENV_SOURCE_0  0x800BCE80u
#define GA_DISPENV_SOURCE_1  0x800BCE94u
#define GA_DRAWENV_COPY_0    0x801D1498u
#define GA_DRAWENV_COPY_1    0x801D14F4u
#define GA_DISPENV_COPY_0    0x801D1550u
#define GA_DISPENV_COPY_1    0x801D1564u

extern void func_8005E57C(int value);
extern void func_8005C1EC(int enabled);
extern void func_8005E6E4(int value);
extern void func_80042538(void);
extern void func_800425DC(void);
extern void func_8003EB04(void);
extern int func_80190660(void);
extern int func_80192CE8(int index);
extern void func_8018F2F4(void);
extern pe_addr_t func_8018FBC0(int type);
extern void func_80190064(void);
extern void func_8018F468(void);
extern void func_80036E34(void);
extern void func_80036DF8(void);
extern int func_80042770(int a0);
extern void func_8004D084(int a0);
extern void func_8003FFAC(int value);
extern int func_8005C498(pe_addr_t result);
extern void func_8005E588(void);
extern void func_8005E6F0(void);
extern void func_8005E788(int32_t wait);

/* HOST_ADAPTED: retail libpad fills the BIOS pad buffer from the VSync
 * interrupt; the port fills it from the host pad source right before the
 * guest's func_8003EB04 read (same contract as func_8003F3C4's field
 * tick), so title/movie loops see window or scripted input. */
static void HostPadFill(void)
{
    if (PE_Port_HasPadSource()) {
        PE_StoreU16(0x800BE9A0u, 0x4100u);
        PE_StoreU16(0x800BE9A2u, PE_Port_ReadPadRaw());
    }
}

static void TitleListSweepAndMerge(void)
{
    pe_addr_t node = PE_LoadU32(0x801D1370u);
    pe_addr_t prev = 0u;
    pe_addr_t pending;
    pe_addr_t tail;

    while (node != 0u) {
        pe_addr_t next = PE_LoadU32(node);
        if (PE_LoadU32(node + 0x30u) != 0u) {
            if (prev != 0u)
                PE_StoreU32(prev, next);
            else
                PE_StoreU32(0x801D1370u, next);
            if (PE_LoadU32(0x801D1374u) == node)
                PE_StoreU32(0x801D1374u, prev);
            PE_StoreU32(node, PE_LoadU32(0x801D136Cu));
            PE_StoreU32(0x801D136Cu, node);
            node = prev != 0u ? PE_LoadU32(prev) : PE_LoadU32(0x801D1370u);
            continue;
        }
        prev = node;
        node = next;
    }

    pending = PE_LoadU32(0x801D1378u);
    if (pending == 0u)
        return;
    tail = PE_LoadU32(0x801D1374u);
    if (tail != 0u)
        PE_StoreU32(tail, pending);
    else
        PE_StoreU32(0x801D1370u, pending);
    PE_StoreU32(0x801D1374u, PE_LoadU32(0x801D137Cu));
    PE_StoreU32(0x801D137Cu, 0u);
    PE_StoreU32(0x801D1378u, 0u);
}

static void TitleLoopPresent(void)
{
    pe_addr_t active;

    (void)func_80074DC0(0);
    active = PE_LoadU32(0x801D11C4u);
    if ((int16_t)PE_LoadU16(active + 0x74u) > 0) {
        RECT blit;
        int32_t w;
        int32_t h;

        blit.x = (int16_t)PE_LoadU16(active + 0x70u);
        blit.y = (int16_t)PE_LoadU16(active + 0x72u);
        blit.w = (int16_t)PE_LoadU16(active + 0x74u);
        blit.h = (int16_t)PE_LoadU16(active + 0x76u);
        w = (int32_t)blit.x;
        blit.x = (int16_t)((w + w + w) >> 1);
        if (PE_LoadU32(0x801D11C8u) == 0u)
            blit.y = (int16_t)(blit.y + 240);
        h = (int32_t)blit.w;
        blit.w = (int16_t)((h + h + h) >> 1);
        (void)func_8007506C(&blit, active + 0x8080u);
    }
    (void)func_80073A44(2);
    (void)func_80074A44(1);
    active = PE_LoadU32(0x801D11C4u);
    (void)func_80075424(active);
    (void)func_800755F0(active + 0x5Cu);
}

#define GA_TITLE_NODE_POOL   0x801D11CCu
#define GA_TITLE_NODE_END    0x801D1338u /* pool + 364 */
#define GA_TITLE_NODE_STRIDE 52u
#define GA_TITLE_FREELIST    0x801D136Cu
#define GA_TITLE_CALLBACK    0x8019319Cu
#define GA_TITLE_SAVED_BIT   0x801D1380u

static void CopyGuestBytes(pe_addr_t destination, pe_addr_t source,
                           uint32_t size)
{
    memcpy(PE_Translate(destination, size),
           PE_TranslateConst(source, size), size);
}

/* Original nonnegative-selector exit, 0x801916DC..0x801918C8.
 * The movie-skip entry still needs the title's ordinary teardown: its
 * standalone renderer and card timer must not survive into the field.
 * The current screen Y survives the saved-environment restoration. */
static int FinishTitleMenu(int selector)
{
    RECT clear_rect = {0, 0, 320, 480};
    uint16_t screen_y;

    func_80074D28(0);
    func_8005E6E4(0);
    func_80074F44(&clear_rect, 0, 0, 0);
    func_80074DC0(0);
    if (PE_Port_ShouldStop())
        return -1;
    screen_y = PE_LoadU16(GA_DISPENV_SOURCE_0 + 10u);

    CopyGuestBytes(GA_DRAWENV_SOURCE_0, GA_DRAWENV_COPY_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_SOURCE_1, GA_DRAWENV_COPY_1, 0x5Cu);
    CopyGuestBytes(GA_DISPENV_SOURCE_0, GA_DISPENV_COPY_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_SOURCE_1, GA_DISPENV_COPY_1, 0x14u);
    PE_StoreU16(GA_DISPENV_SOURCE_1 + 10u, screen_y);
    PE_StoreU16(GA_DISPENV_SOURCE_0 + 10u, screen_y);
    func_8005C1EC(0);
    if (PE_Port_ShouldStop())
        return -1;
    func_8005E57C(0);
    return selector;
}

/* 0x80190DB4..0x8019111C: copy live DRAWENV/DISPENV into the title
 * arenas, clear the HUD strips, then PutDrawEnv/PutDispEnv. */
static void PostMovieRestoreEnvironments(void)
{
    pe_addr_t arena0 = PE_LoadU32(0x801D11BCu);
    pe_addr_t arena1 = PE_LoadU32(0x801D11C0u);
    RECT clear_rect;
    uint32_t bank;
    pe_addr_t active;

    CopyGuestBytes(arena0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(arena1, GA_DRAWENV_SOURCE_1, 0x5Cu);
    CopyGuestBytes(arena0 + 0x5Cu, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(arena1 + 0x5Cu, GA_DISPENV_SOURCE_1, 0x14u);

    PE_StoreU8(arena0 + 0x18u, 0u);
    PE_StoreU8(arena1 + 0x18u, 0u);
    PE_StoreU16(arena0 + 0x7Cu, 0u);
    PE_StoreU16(arena1 + 0x7Cu, 0u);
    PE_StoreU16(arena0 + 0x74u, 0u);
    PE_StoreU16(arena1 + 0x74u, 0u);

    bank = PE_LoadU32(0x8009CDDCu);
    PE_StoreU32(0x801D11C8u, bank);
    PE_StoreU16(arena0 + 0x74u, 0u);
    PE_StoreU16(arena1 + 0x74u, 0u);

    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = 480;
    clear_rect.h = 20;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.y = 224;
    clear_rect.h = 16;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.y = 240;
    clear_rect.h = 20;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.y = 464;
    clear_rect.h = 16;
    func_80074F44(&clear_rect, 0, 0, 0);

    bank = PE_LoadU32(0x801D11C8u) == 0u;
    PE_StoreU32(0x801D11C8u, bank);
    active = PE_LoadU32(0x801D11BCu + bank * 4u);
    PE_StoreU32(0x801D11C4u, active);

    (void)func_80074DC0(0);
    active = PE_LoadU32(0x801D11C4u);
    if ((int16_t)PE_LoadU16(active + 0x74u) > 0) {
        RECT blit;
        int32_t w;
        int32_t h;
        blit.x = (int16_t)PE_LoadU16(active + 0x70u);
        blit.y = (int16_t)PE_LoadU16(active + 0x72u);
        blit.w = (int16_t)PE_LoadU16(active + 0x74u);
        blit.h = (int16_t)PE_LoadU16(active + 0x76u);
        w = (int32_t)blit.x;
        blit.x = (int16_t)((w + w + w) >> 1);
        if (PE_LoadU32(0x801D11C8u) == 0u)
            blit.y = (int16_t)(blit.y + 240);
        h = (int32_t)blit.w;
        blit.w = (int16_t)((h + h + h) >> 1);
        (void)func_8007506C(&blit, active + 0x8080u);
        if (PE_Port_ShouldStop())
            return;
    }

    (void)func_80073A44(0);
    (void)func_80074A44(1);
    active = PE_LoadU32(0x801D11C4u);
    (void)func_80075424(active);
    if (PE_Port_ShouldStop())
        return;
    (void)func_800755F0(active + 0x5Cu);
}

/* 0x80191548..0x8019168C: dim the 24-bit load-screen still at
 * D_80193254+D_80193258 into 15-bit pixels at D_801D11C0+0x1C080
 * (four pixels per three source words, each channel quartered). */
static void TitleLoadScreenDim(void)
{
    const uint32_t s6 = 0x1F0000u;
    const uint32_t t4 = 0x3E00000u;
    const uint32_t t3 = 0x7C000000u;
    const uint32_t t2 = 0x1CE71CE7u;
    pe_addr_t header = PE_LoadU32(0x80193258u) + 0x80193254u;
    int32_t product = (int32_t)(int16_t)PE_LoadU16(header + 0x10u) *
                      (int32_t)(int16_t)PE_LoadU16(header + 0x12u);
    int32_t count = (int32_t)(((int64_t)product * 0x2AAAAAABll) >> 32) -
                    (product >> 31);
    pe_addr_t src = header + 0x14u;
    pe_addr_t dst = PE_LoadU32(0x801D11C0u) + 0x1C080u;

    while (count >= 0) {
        uint32_t a0 = PE_LoadU32(src);
        uint32_t a1 = PE_LoadU32(src + 4u);
        uint32_t a2 = PE_LoadU32(src + 8u);
        uint32_t v;

        src += 12u;
        count--;
        v = ((a0 >> 3) & 0x1Fu) | ((a0 >> 6) & 0x3E0u) |
            ((a0 >> 9) & 0x7C00u) | ((a0 >> 11) & s6) |
            ((a1 << 18) & t4) | ((a1 << 15) & t3);
        PE_StoreU32(dst, (v >> 2) & t2);
        v = ((a1 >> 19) & 0x1Fu) | ((a1 >> 22) & 0x3E0u) |
            ((a2 << 7) & 0x7C00u) | ((a2 << 5) & s6) |
            ((a2 << 2) & t4) | ((a2 >> 1) & t3);
        PE_StoreU32(dst + 4u, (v >> 2) & t2);
        dst += 8u;
    }
}

static pe_addr_t TitleFindType(uint32_t type)
{
    pe_addr_t node = PE_LoadU32(0x801D1370u);

    while (node != 0u && PE_LoadU32(node + 0x2Cu) != type)
        node = PE_LoadU32(node);
    return node;
}

int func_801909B4(void)
{
    pe_addr_t arena;
    pe_addr_t second;
    pe_addr_t environment0 = 0u;
    pe_addr_t environment1 = 0u;
    uint32_t saved_bit;
    RECT move_rect;
    RECT clear_rect;
    int selector = -1;           /* $s2 */
    uint16_t screen_y = 0u;      /* $s0 at 0x801918A4 */

    CopyGuestBytes(GA_DISPENV_COPY_0, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_COPY_1, GA_DISPENV_SOURCE_1, 0x14u);
    CopyGuestBytes(GA_DRAWENV_COPY_0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_COPY_1, GA_DRAWENV_SOURCE_1, 0x5Cu);

    /* 0x80190A90: retained in s4 for the later 0x80190D7C branch. */
    saved_bit = PE_LoadU8(0x800B0DCDu) & 1u;

    arena = PE_LoadU32(0x80011610u);
    second = arena + 0x1C080u;
    PE_StoreU32(0x800B0E50u, arena + 0x4080u);
    PE_StoreU32(0x801D11BCu, arena);
    PE_StoreU32(0x801D11C0u, second);
    PE_StoreU32(0x800B0E54u, second + 0x4080u);
    PE_StoreU32(0x800B0E38u, arena + 0x80u);
    PE_StoreU32(0x800B0E3Cu, second + 0x80u);

    func_8005E57C(1);

    /* 0x80190BCC: attract-loop head.  A title timeout (D_801D1380 reaches
     * 1000 with no confirm) leaves $s2 negative and branches back here,
     * replaying the opening movie. */
    for (;;) {
    func_8005C1EC(1);
    if (PE_Port_ShouldStop())
        return -1;
    func_80042538();
    func_80074D28(0);

    move_rect.x = 320;
    move_rect.y = 0;
    move_rect.w = 160;
    move_rect.h = 256; /* jal delay slot at 0x80190C0C */
    (void)func_8007512C(&move_rect, 0x2C0, 0);
    if (PE_Port_ShouldStop())
        return -1;

    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);

    environment0 = PE_LoadU32(0x801D11BCu);
    environment1 = PE_LoadU32(0x801D11C0u);
    (void)func_80074924(environment0, 0, 0, 320, 240);
    (void)func_80074924(environment1, 0, 240, 320, 240);
    (void)func_800749D8(environment0 + 0x5Cu, 0, 240, 320, 240);
    (void)func_800749D8(environment1 + 0x5Cu, 0, 0, 320, 240);

    PE_StoreU16(environment1 + 0x66u, 0u);
    PE_StoreU16(environment0 + 0x66u, 0u);
    PE_StoreU16(environment1 + 0x6Au, 240u);
    PE_StoreU16(environment0 + 0x6Au, 240u);
    PE_StoreU8(environment1 + 0x6Du, 1u);
    PE_StoreU8(environment0 + 0x6Du, 1u);
    PE_StoreU16(environment1 + 0x7Cu, 0u);
    PE_StoreU16(environment0 + 0x7Cu, 0u);
    PE_StoreU16(environment1 + 0x74u, 0u);
    PE_StoreU16(environment0 + 0x74u, 0u);

    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = 480;
    clear_rect.h = 480;
    func_80074F44(&clear_rect, 0, 0, 0);
    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);

    static int pe_title_entries;
    pe_title_entries++;
    /* PE_SKIP_MOVIE_ONCE (diagnostic): skip only the cold-boot title, so a
     * later entry (game over -> title) runs the real overlay. */
    if (PE_Port_SkipMovie() && !(getenv("PE_SKIP_MOVIE_ONCE") && pe_title_entries > 1)) {
        /* HOST_ADAPTED: skip logo fade, FMV001, and title. The real first
         * pass marks the once-per-power-on logos as shown (D_8009D1BC=1);
         * do the same so a later title entry (game over) skips them. */
        PE_StoreU32(0x8009D1BCu, 1u);
        Stub_Record("func_801909B4_skip_movie_new_game", "HOST_ADAPTED");
        Trace_Direct("skip_movie_new_game_selector");
        return FinishTitleMenu(1);
    }

    if (PE_LoadU32(0x8009D1BCu) == 0u) {
        PE_StoreU32(0x8009D1BCu, 1u);
        (void)func_80190660();
        if (PE_Port_ShouldStop())
            return -1;
    }

    if (saved_bit != 0u) {
        int movie = func_80192CE8(1);
        unsigned i;

        if (PE_Port_ShouldStop())
            return -1;
        /* Retail: nonzero movie result waits 60 frames (skip pressed). */
        if (movie != 0) {
            for (i = 0u; i < 60u; i++) {
                PE_Port_SetCardTestEventHostReturn(1);
                func_800425DC();
                PE_Port_SetCardTestEventHostReturn(0);
                if (PE_Port_ShouldStop())
                    return -1;
                (void)func_80073A44(0);
            }
        }
        PostMovieRestoreEnvironments();
        if (PE_Port_ShouldStop())
            return -1;
    }

    /* 0x80191120: title present (func_8018F2F4), then freelist. */
    func_8018F2F4();
    if (PE_Port_ShouldStop())
        return -1;
    (void)func_80073A44(0);
    func_80074D28(1);

    {
        pe_addr_t node = GA_TITLE_NODE_POOL;
        pe_addr_t end = GA_TITLE_NODE_END;
        pe_addr_t next;

        while (node < end) {
            next = node + GA_TITLE_NODE_STRIDE;
            PE_StoreU32(node, next);
            node = next;
        }
        PE_StoreU32(node, 0u);
        PE_StoreU32(GA_TITLE_FREELIST, GA_TITLE_NODE_POOL);
        PE_StoreU32(0x801D137Cu, 0u);
        PE_StoreU32(0x801D1378u, 0u);
        PE_StoreU32(0x801D1374u, 0u);
        PE_StoreU32(0x801D1370u, 0u);
    }

    if (PE_Port_SkipOpeningMenu()) {
        Stub_Record("func_801909B4_skip_opening_menu", "HOST_ADAPTED");
        Trace_Direct("skip_opening_menu_after_title_present");
        return FinishTitleMenu(1);
    }

    /* 0x80191198: jal func_8018FBC0(1), install 0x8019319C at +0x14. */
    {
        pe_addr_t object = func_8018FBC0(1);

        PE_StoreU32(object + 0x14u, GA_TITLE_CALLBACK);
        PE_StoreU32(GA_TITLE_SAVED_BIT, saved_bit);
    }

    /* 0x801911C0: title main loop.  Retail repeats while
     * D_801D1380 (+= saved_bit per frame) < 1000; func_80190064's Circle
     * confirm stores 1001. */
    while ((int32_t)PE_LoadU32(GA_TITLE_SAVED_BIT) < 1000) {
        {
            uint32_t bank = PE_LoadU32(0x801D11C8u) == 0u ? 1u : 0u;
            pe_addr_t active = PE_LoadU32(0x801D11BCu + bank * 4u);

            PE_StoreU32(0x801D11C8u, bank);
            PE_StoreU32(0x801D11C4u, active);
        }

        PE_Port_SetCardTestEventHostReturn(1);
        func_800425DC();
        PE_Port_SetCardTestEventHostReturn(0);
        if (PE_Port_ShouldStop()) {
            Bootstrap_ReturnVoid4Indirect(
                "func_801909B4_title_loop_425DC", "func_801909B4",
                0x801911F0u, saved_bit, environment0, environment1, 0u);
            return -1;
        }

        /* jal 0x8003EB04 — only when retail libpad dispatch is already
         * installed (PadInit / test seed).  B54KR's RAM canary path leaves
         * 9B738 unset and therefore still cuts here. */
        if (PE_LoadU32(0x8009B738u) != 0x80084B20u) {
            Bootstrap_ReturnVoid4Indirect(
                "func_801909B4_title_loop_3EB04", "func_801909B4",
                0x801911F8u, saved_bit, environment0, environment1, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }

        HostPadFill();
        func_8003EB04();
        if (PE_Port_ShouldStop())
            return -1;
        func_80190064();
        if (PE_Port_ShouldStop())
            return -1;
        func_8018F468();
        if (PE_Port_ShouldStop())
            return -1;

        TitleListSweepAndMerge();
        TitleLoopPresent();
        if (PE_Port_ShouldStop())
            return -1;

        PE_StoreU32(GA_TITLE_SAVED_BIT,
                    PE_LoadU32(GA_TITLE_SAVED_BIT) + saved_bit);
        if ((int32_t)PE_LoadU32(GA_TITLE_SAVED_BIT) >= 1000)
            break;
        if (!PE_Port_ConsumeTitleLoop()) {
            Bootstrap_ReturnVoid4Indirect(
                "func_801909B4_title_main_loop", "func_801909B4",
                0x801911C0u, saved_bit, environment0, environment1, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
    }

    /* 0x80191410: title teardown common to timeout and confirm. */
    func_80074D28(0);
    move_rect.x = 0x2C0;
    move_rect.y = 0;
    move_rect.w = 0xA0;
    move_rect.h = 0x100;
    (void)func_8007512C(&move_rect, 0x140, 0);
    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = 320;
    clear_rect.h = 480;
    func_80074F44(&clear_rect, 0, 0, 0);
    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);
    func_8005E588();
    screen_y = 8u; /* $s0 = 8 in the 0x80191488 delay slot */
    func_8005E6F0();
    func_8005E788(2);
    if (PE_Port_ShouldStop())
        return -1;

    if ((int32_t)PE_LoadU32(GA_TITLE_SAVED_BIT) > 1000) {
        /* 0x801914B4: the confirmed menu row is the type-7 node's +6. */
        pe_addr_t row = TitleFindType(7u);
        int16_t choice;
        int load_screen = 0;

        if (row == 0u) {
            /* Retail dereferences NULL+6 here; unreachable after a
             * confirm (func_80190064 only confirms on a type-7 node). */
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
        choice = (int16_t)PE_LoadU16(row + 6u);
        if (getenv("PE_FMV_DEBUG")) fprintf(stderr, "[FMV] title choice=%d\n", choice);
        if (choice == 0xA0) {
            selector = 1; /* NEW GAME */
            func_80036E34();
            func_80036DF8();
        } else if (choice >= 0xB5) {
            selector = 3;
        } else if (choice == 0xB4 && func_80042770(0) == 0 &&
                   func_80042770(1) == 0) {
            selector = 3;
        } else {
            load_screen = 1;
        }

        if (load_screen) {
            /* 0x80191548: CONTINUE — dimmed still behind the card menu. */
            func_8005E6E4((int)(PE_LoadU32(0x801D11C0u) + 0x1C080u));
            TitleLoadScreenDim();
            func_80074D28(1);
            func_8004D084(0);
            func_8003FFAC(choice == 0x8C);
            if (PE_Port_ShouldStop())
                return -1;
            do {
                HostPadFill();
                func_8003EB04();
                if (PE_Port_ShouldStop())
                    return -1;
                selector = func_8005C498(0u);
                if (PE_Port_ShouldStop())
                    return -1;
            } while (selector == 0);
            if (selector == 2 && choice == 0x8C)
                func_80036E34();
            func_80074D28(0);
            func_8005E6E4(0);
            clear_rect.x = 0;
            clear_rect.y = 0;
            clear_rect.w = 320;
            clear_rect.h = 480;
            func_80074F44(&clear_rect, 0, 0, 0);
            func_80074DC0(0);
            screen_y = PE_LoadU16(GA_DISPENV_SOURCE_0 + 10u);
        }
    }

    /* 0x80191724: bltz $s2 → replay from 0x80190BCC. */
    if (selector >= 0)
        break;
    Trace_Direct("title_attract_timeout_replay");
    }

    /* 0x8019172C: restore the saved environments, publish screen Y. */
    CopyGuestBytes(GA_DRAWENV_SOURCE_0, GA_DRAWENV_COPY_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_SOURCE_1, GA_DRAWENV_COPY_1, 0x5Cu);
    CopyGuestBytes(GA_DISPENV_SOURCE_0, GA_DISPENV_COPY_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_SOURCE_1, GA_DISPENV_COPY_1, 0x14u);
    PE_StoreU16(GA_DISPENV_SOURCE_1 + 10u, screen_y);
    PE_StoreU16(GA_DISPENV_SOURCE_0 + 10u, screen_y);
    func_8005C1EC(0);
    if (PE_Port_ShouldStop())
        return -1;
    func_8005E57C(0);
    return selector;
}
