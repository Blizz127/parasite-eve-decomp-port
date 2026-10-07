/*
 * Phase 6C — Native entry through translated Parasite Eve main.
 *
 * Default path: func_8001220C → full PE boot chain → black frame.
 * Direct-clear shortcut requires explicit --direct-clear-test flag.
 */

#include "pe_gpu.h"
#include "host_audio.h"
#include "pe_audio_driver.h"
#include "pe_spu.h"
#include "pe_guestcode.h"
#include "psx_compat.h"
#include "host_framebuffer.h"
#include "host_vram.h"
#include "host_window.h"
#include "stub_registry.h"
#include "pe_bootstrap.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_disc.h"
#include "pe_guest_image.h"
#include "pe_disc_check.h"
#include "pe_route_pad.h"
#include "pe_port_compat.h"
#include "pe_guest_ram.h"
#include "pe_plat/cheats.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

extern void func_8001220C(void);
extern int  func_8006E9A0(int);
extern pe_addr_t D_80011614;
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern int  func_80070DD0(int, int);

/* ── CLI ────────────────────────────────────────────────────────────── */
static struct {
    int headless, bootstrap_disc, strict_stubs;
    const char *screenshot, *vram_screenshot, *vram_raw, *trace_path;
    int hold_ms, scale, hold_until_close, debug_overlay;
    int fullscreen;
    const char *window_title;
    int direct_clear_test;
    const char *stop_after_event;
    int max_frames, max_main_iterations;
    const char *disc_image;
    int disc_load_test;
    int rng_oracle_dump;
    int lzcr_oracle_dump;
    int callback_oracle_dump;
    int dma_checkpoint_report;
    int skip_movie;
    int skip_opening_menu;
    int boundary_report;
    int route_pad;
    int auto_quit;
    /* Autopilot hand-over: after `hand_over_at` route frames (0 = immediately)
     * the route pad is dropped so the player owns the controller.  With
     * `hand_over_key` the release additionally waits for a button press, so
     * "autopilot to the fight, then grab it when I'm ready" works. */
    int hand_over_at;
    int hand_over_key;
} g_opts = {
    .headless = 0, .bootstrap_disc = 0, .strict_stubs = 0,
    .screenshot = NULL, .vram_screenshot = NULL, .vram_raw = NULL,
    .trace_path = NULL,
    .hold_ms = 0, .scale = 2, .hold_until_close = 1, .debug_overlay = 0,
    .window_title = "Parasite Eve Native Port",
    .direct_clear_test = 0, .stop_after_event = NULL,
    .max_frames = 0, .max_main_iterations = 0,
    .disc_image = NULL, .disc_load_test = 0, .rng_oracle_dump = 0,
    .lzcr_oracle_dump = 0, .callback_oracle_dump = 0,
    .dma_checkpoint_report = 0,
    .skip_movie = 0, .boundary_report = 0,
    .route_pad = 0, .auto_quit = 0,
    .hand_over_at = 0, .hand_over_key = 0,
};

/* ── Deterministic route pad (interactive autopilot) ────────────────────
 * The interactive entry point has no SIO/pad, so without an installed
 * source the guest never sees a button (func_8003F3C4's idle-zero
 * normalize fills 0xFFFF) and cold boot parks at the field prefix waiting
 * for input.  This drives the SAME four-stage pad the boot -> Day-2 route
 * harness uses (pc_port/tests/test_route_boot_day2.c) via the shared
 * pc_port/include/pe_route_pad.h table, so the SAME Day-1 field route the
 * harness proves is also reachable from the windowed binary.  It is a host
 * input source only; it never writes guest state that the pad path would
 * not. */

static PeRoutePadConfig g_route_pad;
static int g_route_frame;
#define GA_TOKEN D_8009D280
static int g_frame;
static unsigned g_sewer_victories, g_sewer_enemy_peak[3];
static int g_pulse_end = 33620, g_pulse_resume = 35300;
static int g_exact_pad_begin = 42713, g_exact_pad_end = 45041;
static int g_sewer_pad_begin = 50500, g_sewer_pad_end = 51200;
static int g_second_sewer_pad_begin = 52344, g_second_sewer_pad_end = 54500;
static int g_supply_pad_begin = 54500, g_supply_pad_end = 62000;
static struct { int frame; uint16_t mask; } g_pad_sequence[4096];
static unsigned g_pad_sequence_count;
#include "route_rehearsal_pads.h"
#include "route_reward_sewer_pilot.h"
#include "route_ammo_trace.h"

/* m0004i module-4 type-4 task PC: the documented executed-route frontier
 * (same FRONTIER_PC the harness pins).  Matched on the task PC rather than
 * the room token because the m0004i token (0xA8000248) is also live earlier
 * in the route, before the m0378i/m0377i bounce.  The frontier additionally
 * requires persist[1]==0x17A, which only the post-bounce m0004i visit holds;
 * the first m0004i visit has persist[1]==3 and also parks module 4 on the
 * same task PC. */
#define ROUTE_FRONTIER_PC       0x801B6CC8u
#define ROUTE_FRONTIER_TOKEN    0xA8000248u
#define ROUTE_FRONTIER_PERSIST1 0x0000017Au

static void RoutePadAutoQuitIfDone(void)
{
    pe_addr_t actor;
    int guard = 0;

    if (!g_opts.auto_quit) return;
    /* The m0377i module-5 transfer bounces through m0378i back to m0004i. */
    if (D_8009D280 != ROUTE_FRONTIER_TOKEN) return;
    if (PE_LoadU32(0x800A77F4u) != ROUTE_FRONTIER_PERSIST1) return;
    actor = PE_LoadU32(0x8009D20Cu);
    while (actor != 0u && guard < 64) {
        pe_addr_t task = PE_LoadU32(actor + 0xA8u);
        if (task != 0u && PE_LoadU32(task) == ROUTE_FRONTIER_PC) {
            fprintf(stderr,
                    "[ROUTE] reached m0004i frontier (pc=0x%08X) at "
                    "route frame %d; quitting (--auto-quit)\n",
                    (unsigned)ROUTE_FRONTIER_PC, g_route_frame);
            PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
            return;
        }
        actor = PE_LoadU32(actor + 4u);
        guard++;
    }
}

static void RecordSewerVictory(void)
{
    unsigned room;
    unsigned enemies=0;
    pe_addr_t actor,aya,record;
    uint32_t flags;
    if (GA_TOKEN==0xA80023C8u) room=0;
    else if (GA_TOKEN==0xA8002448u) room=1;
    else if (GA_TOKEN==0xA8003148u) room=2;
    else return;
    actor=PE_LoadU32(0x8009D20Cu);
    for (unsigned i=0;actor && i<64u;i++,actor=PE_LoadU32(actor+4u)) {
        unsigned type=PE_LoadU8(actor+12u);
        if ((room==0?type==3u:room==1?(type==7u || type==8u):type==6u) && PE_LoadU32(actor)) enemies++;
    }
    flags=PE_LoadU32(0x8009D1A0u);
    if ((flags&2u) && enemies>g_sewer_enemy_peak[room]) g_sewer_enemy_peak[room]=enemies;
    aya=PE_LoadU32(0x8009D254u); record=aya?PE_LoadU32(aya):0u;
    if (g_sewer_enemy_peak[room]==(room==2?2u:3u) && !enemies && !(flags&6u) &&
        PE_LoadU32(0x8009D28Cu)==9u && record && PE_LoadU16(record+12u)>0u &&
        !(g_sewer_victories&(1u<<room))) {
        g_sewer_victories|=1u<<room;
        fprintf(stderr,"route: sewer victory room=%u frame=%d HP=%u\n",room+1u,g_frame,PE_LoadU16(record+12u));
    }
}

static void RoutePadLoadSequence(void)
{
    const char *s = getenv("PE_ROUTE_PAD_SEQUENCE");
    if (!s) s=kDay1RoutePads;
    g_pad_sequence_count=0;
    while (s && s[0]) {
        char *end;
        unsigned long frame, mask;
        errno=0;
        frame=strtoul(s,&end,10);
        if (errno || end==s || *end!=':' || frame>INT_MAX
            || g_pad_sequence_count==sizeof(g_pad_sequence)/sizeof(g_pad_sequence[0])
            || (g_pad_sequence_count && frame<=(unsigned)g_pad_sequence[g_pad_sequence_count-1].frame))
            break;
        s=end+1;
        errno=0;
        mask=strtoul(s,&end,16);
        if (errno || end==s || mask>0xFFFFu || (*end && *end!=',')) break;
        g_pad_sequence[g_pad_sequence_count].frame=(int)frame;
        g_pad_sequence[g_pad_sequence_count++].mask=(uint16_t)mask;
        s=*end?end+1:end;
    }
}

static struct { int frame; uint16_t mask; } g_script_pad[1024];
static unsigned g_script_pad_count;

static void ScriptPadLoad(const char *s)
{
    g_script_pad_count = 0;
    while (s && *s && g_script_pad_count < 1024u) {
        char *end;
        unsigned long frame = strtoul(s, &end, 10), mask;
        if (end == s || *end != ':') break;
        s = end + 1;
        mask = strtoul(s, &end, 16);
        if (end == s) break;
        g_script_pad[g_script_pad_count].frame = (int)frame;
        g_script_pad[g_script_pad_count++].mask = (uint16_t)mask;
        s = *end == ',' ? end + 1 : end;
    }
    if (s && *s) {
        fprintf(stderr, "[PAD] PE_PAD_SCRIPT: more than %u changes (or a malformed entry at \"%.20s\"); refusing to truncate\n",
                (unsigned)(sizeof(g_script_pad) / sizeof(g_script_pad[0])), s);
        exit(2);
    }
}

/* PE_PAD_SCRIPT frames count host presents by default.  The game presents
 * nothing while paused (retail's pause only waits VSync), so a present-keyed
 * script can never send the unpausing Start.  PE_PAD_SCRIPT_CLOCK=vblank
 * keys the script on emulated VBlanks (PE_GPU_VSyncQuery: one per VBlank,
 * not per VSync() call), the same unit as the emulator oracle's .pad files. */
static uint16_t ScriptPadSource(void)
{
    static int by_vblank = -1;
    int presented = 0;
    uint16_t mask = 0xFFFFu;
    if (by_vblank < 0) {
        const char *c = getenv("PE_PAD_SCRIPT_CLOCK");
        by_vblank = c && !strcmp(c, "vblank");
    }
    HostFB_GetState(NULL, NULL, &presented, NULL);
    if (by_vblank) presented = (int)PE_GPU_VSyncQuery();
    for (unsigned i = 0; i < g_script_pad_count && presented >= g_script_pad[i].frame; i++)
        mask = g_script_pad[i].mask;
    return mask;
}

static int      g_handed_over;
static int      g_hand_over_announced;
static int      g_hand_over_frame;

/* Hand-over: the autopilot drives the route until the player takes the pad.
 * `--hand-over-at <frame>` gates on the route frame; `--hand-over-key` then
 * waits for a real button press so the release happens when the player is
 * ready (with neither flag the autopilot keeps the pad, as before).
 * HostWindow_PadRaw() is idle 0xFFFF when no window is open, so a headless
 * hand-over simply stops feeding route input. */
static int RouteHandOverReady(void)
{
    uint16_t player;

    if (g_handed_over) return 1;
    if (g_opts.hand_over_at == 0 && !g_opts.hand_over_key) return 0;
    if (g_opts.hand_over_at > 0 && g_frame < g_opts.hand_over_at) return 0;

    player = HostWindow_PadRaw();
    if (g_opts.hand_over_key) {
        if (player == 0xFFFFu) {
            if (!g_hand_over_announced) {
                g_hand_over_announced = 1;
                fprintf(stderr,
                        "[ROUTE] autopilot reached frame %d; press a button to take control\n",
                        g_frame);
            }
            return 0;
        }
        fprintf(stderr, "[ROUTE] hand-over at frame %d (button pressed): pad is the player's\n",
                g_frame);
    } else {
        fprintf(stderr, "[ROUTE] hand-over at frame %d (--hand-over-at): pad is the player's\n",
                g_frame);
    }
    g_handed_over = 1;
    g_hand_over_frame = g_frame;
    return 1;
}

/* Event-anchored pad clock.  The recorded pads (kDay1RoutePads, the exact
 * battle windows, the pulse gating) were captured on one build's timing.  Port
 * changes can move a battle start by a few frames (9dd1bd1c: func_800144FC
 * retail TU made every battle entry 2-3 frames earlier), and a replayed fight
 * then desyncs and is lost.  So the replay runs on a route clock
 * rf = g_frame - offset, re-anchored at every battle start: when D1A0&0x2
 * rises in room T, offset = g_frame - (the next recorded start for T).
 * Anchors were recorded on b04c438e (build/lanes/day1/anchors-good.log).
 * PE_ROUTE_NO_ANCHOR=1 restores absolute frames. */
static const struct { uint32_t token; int frame; } kRouteBattleAnchors[] = {
#include "route_battle_anchors.h"
};
/* Room-entry anchors: {token, nth entry, recorded route-clock frame}. A battle
 * that ends faster/slower (e.g. a battle-code swap) shifts the room entry after
 * it; re-anchoring on each entry keeps the recorded field pads aligned. */
static const struct { uint32_t token; int nth; int frame; } kRouteRoomAnchors[] = {
#include "route_room_anchors.h"
};
/* Battle-end anchors: the recorded pads after a battle are field walking, so
 * they are keyed to the battle's END (D1A0&2 1->0), not its start. A battle
 * that ends earlier (e.g. a crit from the matched battle tick's RNG, port3
 * 09-28: M0013I ends ~184 frames early) would otherwise desync them. */
static const struct { uint32_t token; int nth; int frame; } kRouteBattleEndAnchors[] = {
#include "route_battle_end_anchors.h"
};
static int g_pad_offset;
static int RoutePadClock(void)
{
    static int init = -1;
    static uint32_t last_b;
    static unsigned used;          /* bitmask of consumed anchors */
    uint32_t b;
    if (init < 0) init = !getenv("PE_ROUTE_NO_ANCHOR");
    if (init) {
        static uint32_t last_tok;
        static struct { uint32_t tok; int n; } cnt[64]; static unsigned ncnt;
        uint32_t tok = (uint32_t)D_8009D280;
        if (tok != last_tok) {
            unsigned k; int nth = 0;
            last_tok = tok;
            for (k = 0; k < ncnt && cnt[k].tok != tok; k++) {}
            if (k == ncnt && ncnt < 64u) { cnt[ncnt].tok = tok; cnt[ncnt].n = 0; ncnt++; }
            if (k < ncnt) nth = cnt[k].n++;
            for (unsigned i = 0; i < sizeof(kRouteRoomAnchors)/sizeof(kRouteRoomAnchors[0]); i++) {
                if (kRouteRoomAnchors[i].token == tok && kRouteRoomAnchors[i].nth == nth &&
                    abs(g_frame - kRouteRoomAnchors[i].frame - g_pad_offset) < 900) {
                    int o = g_frame - kRouteRoomAnchors[i].frame;
                    if (o != g_pad_offset)
                        fprintf(stderr, "[ROUTE] anchor room token=%08X nth=%d recorded=%d actual=%d offset=%d\n",
                                (unsigned)tok, nth, kRouteRoomAnchors[i].frame, g_frame, o);
                    g_pad_offset = o;
                    break;
                }
            }
        }
    }
    b = PE_LoadU32(0x8009D1A0u) & 0x2u;
    if (init && b && !last_b) {
        uint32_t tok = (uint32_t)D_8009D280;
        for (unsigned i = 0; i < sizeof(kRouteBattleAnchors)/sizeof(kRouteBattleAnchors[0]) && i < 32u; i++) {
            if (!(used & (1u << i)) && kRouteBattleAnchors[i].token == tok &&
                abs(g_frame - kRouteBattleAnchors[i].frame - g_pad_offset) < 600) {
                used |= 1u << i;
                g_pad_offset = g_frame - kRouteBattleAnchors[i].frame;
                fprintf(stderr, "[ROUTE] anchor battle token=%08X recorded=%d actual=%d offset=%d\n",
                        (unsigned)tok, kRouteBattleAnchors[i].frame, g_frame, g_pad_offset);
                break;
            }
        }
    }
    if (init && !b && last_b) {
        static struct { uint32_t tok; int n; } ec[32]; static unsigned nec;
        uint32_t tok = (uint32_t)D_8009D280; unsigned k; int nth = 0;
        for (k = 0; k < nec && ec[k].tok != tok; k++) {}
        if (k == nec && nec < 32u) { ec[nec].tok = tok; ec[nec].n = 0; nec++; }
        if (k < nec) nth = ec[k].n++;
        for (unsigned i = 0; i < sizeof(kRouteBattleEndAnchors)/sizeof(kRouteBattleEndAnchors[0]); i++) {
            if (kRouteBattleEndAnchors[i].token == tok && kRouteBattleEndAnchors[i].nth == nth &&
                abs(g_frame - kRouteBattleEndAnchors[i].frame - g_pad_offset) < 900) {
                int o = g_frame - kRouteBattleEndAnchors[i].frame;
                fprintf(stderr, "[ROUTE] anchor battle-end token=%08X nth=%d recorded=%d actual=%d offset=%d\n",
                        (unsigned)tok, nth, kRouteBattleEndAnchors[i].frame, g_frame, o);
                g_pad_offset = o;
                break;
            }
        }
    }
    last_b = b;
    return g_frame - g_pad_offset;
}

static uint16_t RoutePadSource(void)
{
    uint16_t mask;
    int rf;

    g_route_frame = g_frame;
    if (RouteHandOverReady()) {
        /* PE_PAD_SCRIPT with --route-pad: after the hand-over the script
         * drives the pad, its frames counted from the hand-over frame (a
         * headless route can then e.g. answer a save prompt). */
        if (g_script_pad_count) {
            uint16_t m = 0xFFFFu;
            for (unsigned i = 0; i < g_script_pad_count &&
                 g_frame - g_hand_over_frame >= g_script_pad[i].frame; i++)
                m = g_script_pad[i].mask;
            return m;
        }
        return HostWindow_PadRaw();
    }
    /* PE_ROUTE_PLAY_MOVIES: no buttons while a movie shows (a held
     * route button would skip it). */
    if (getenv("PE_ROUTE_PLAY_MOVIES") && PE_LoadU8(0x800B0DBAu) != 0u)
        return 0xFFFFu;
    rf = RoutePadClock();
    /* Harness-only (--route-pad; PE_ROUTE_BATTLE_NO_SHIM=1 disables): HP lock in
     * the three sewer fights. Under the retail battle tick enemies act as in
     * retail and the autopilot can lose (bt-route 09-28: Aya died in M0032I at
     * 60250 after entering at 28 HP). The sewer pilot still moves/attacks. */
    if (g_route_battle_shim && (PE_LoadU32(0x8009D1A0u) & 2u) &&
        (D_8009D280 == 0xA80023C8u || D_8009D280 == 0xA8002448u || D_8009D280 == 0xA8003148u)) {
        pe_addr_t rec = PE_LoadU32(0x8009D278u);
        if (rec) {
            unsigned mx = PE_LoadU16(rec + 28u);
            if (mx && PE_LoadU16(rec + 12u) < mx) PE_StoreU16(rec + 12u, (uint16_t)mx);
        }
    }
    {
        /* Diagnostic (harness-only): PE_FORCE_DEATH_TOKEN=<hex token> sets Aya's
         * HP to 0 once when a battle is live in that room, to exercise the
         * death -> Game Over path. */
        static int done; const char *ft = getenv("PE_FORCE_DEATH_TOKEN");
        if (ft && !done && (uint32_t)D_8009D280 == (uint32_t)strtoul(ft, NULL, 16) &&
            (PE_LoadU32(0x8009D1A0u) & 2u) && PE_LoadU32(0x8009D28Cu) == 0u) {
            pe_addr_t rec = PE_LoadU32(0x8009D278u);
            if (rec && PE_LoadU16(rec + 12u)) {
                PE_StoreU16(rec + 12u, 0u); done = 1;
                fprintf(stderr, "[ROUTE] FORCE_DEATH frame=%d token=%08X\n", g_frame, (unsigned)D_8009D280);
            }
        }
    }
    if (getenv("PE_ROUTE_FORCE_P0_2") && D_8009D280 == 0xA8000148u && !(PE_LoadU32(0x800A77F0u) & 2u)) {
        PE_StoreU32(0x800A77F0u, PE_LoadU32(0x800A77F0u) | 2u);   /* diagnostic only */
        fprintf(stderr, "FORCE persist[0]|=2 at frame %d\n", g_frame);
    }
    if (getenv("PE_ROUTE_REAL_NAME_ENTRY") && (PE_LoadU32(0x800B0CD8u) & 0x1000u)) {
        /* Diagnostic: accept the default name in the real name-entry menu. */
        static int ne_log;
        if (g_frame - ne_log >= 60) { ne_log = g_frame;
            fprintf(stderr, "NAME_ENTRY %d cd8=%08X focus=%08X\n", g_frame, PE_LoadU32(0x800B0CD8u), PE_LoadU32(0x8009D15Cu)); }
        return (g_frame % 40) == 3 ? 0xFFF7u : (g_frame % 40) == 23 ? 0xBFFFu : 0xFFFFu;
    }
    mask=PeRoutePad_Mask(&g_route_pad,rf);
    for (unsigned i=0;i<g_pad_sequence_count && rf>=g_pad_sequence[i].frame;i++)
        mask=g_pad_sequence[i].mask;
    if (!(rf>=g_exact_pad_begin && rf<g_exact_pad_end) &&
        !(rf>=g_sewer_pad_begin && rf<g_sewer_pad_end) &&
        !(rf>=g_second_sewer_pad_begin && rf<g_second_sewer_pad_end) &&
        !(rf>=g_supply_pad_begin && rf<g_supply_pad_end) &&
        (rf<g_pulse_end || rf>=g_pulse_resume) && (rf%g_route_pad.period)==3)
        mask&=g_route_pad.pulse;
    mask=RouteRewardSewerPilot(mask);
    if (getenv("PE_ROUTE_DEBUG") && (g_frame % 500) == 0)
        fprintf(stderr, "[ROUTE] padf=%d token=%08X story=%08X held=%08X pad=%04X\n",
                g_frame, (unsigned)D_8009D280,
                (unsigned)PE_LoadU32(0x800A7918u), (unsigned)PE_LoadU32(0x8009D26Cu),
                (unsigned)mask);
    if (getenv("PE_ROUTE_POS_TRACE")) {
        static uint16_t last_mask = 0; static int last_f = -1000;
        if (mask != last_mask || g_frame - last_f >= 60) {
            pe_addr_t aya = PE_LoadU32(0x8009D254u);
            fprintf(stderr, "POS %d rf=%d token=%08X mode=%u d1a0=%X pad=%04X aya=%d,%d focus=%08X\n",
                    g_frame, rf, (unsigned)D_8009D280, PE_LoadU32(0x8009D28Cu),
                    PE_LoadU32(0x8009D1A0u), (unsigned)mask,
                    aya ? (int32_t)PE_LoadU32(aya + 40u) >> 16 : 0,
                    aya ? (int32_t)PE_LoadU32(aya + 48u) >> 16 : 0,
                    (unsigned)PE_LoadU32(0x8009D15Cu));
            last_mask = mask; last_f = g_frame;
        }
    }
    RoutePadAutoQuitIfDone();
    return mask;
}

/* Phase 6E-PRS1 live-window present hook: blit the newest host pixels and
 * poll for close/Escape.  Host data only; never touches guest state.
 * Headless --route-pad also uses this hook so g_frame advances; without it
 * the sewer/M34 pilot sees frame 0 forever and cannot time inputs. */
static void PresentHook_BlitWindow(void)
{
    pe_plat_cheats_frame();   /* opt-in cheat console (pe_plat/cheats.h); no-op while all off */
    {   /* Harness aid (non-route runs): PE_SHOT_DIR=<dir> + PE_SHOT_EVERY=N write
         * <dir>/p<present>.ppm every N presents (menu / load verification). */
        static const char *sdir = (const char *)1; static int sev;
        if (sdir == (const char *)1) { sdir = getenv("PE_SHOT_DIR"); sev = getenv("PE_SHOT_EVERY") ? atoi(getenv("PE_SHOT_EVERY")) : 60; }
        if (sdir && sev > 0 && !g_opts.route_pad) {
            int presented = 0; HostFB_GetState(NULL, NULL, &presented, NULL);
            if (presented % sev == 0) {
                char path[512]; snprintf(path, sizeof path, "%s/p%06d.ppm", sdir, presented);
                (void)HostFB_WritePPM(path);
            }
        }
    }
    if (g_opts.route_pad) {
        /* fmv2 harness aid, PE_ROUTE_PLAY_MOVIES=1 only: the route pad
         * schedule was recorded with --skip-movie, so hold the route
         * clock while a field movie owns the display (D_800B0DBA != 0:
         * func_801216C4 set, func_80121A00 restore clears it).  The
         * movies then play for real without shifting the pad script. */
        static int play_movies = -1;
        if (play_movies < 0) play_movies = getenv("PE_ROUTE_PLAY_MOVIES") != NULL;
        if (play_movies && PE_LoadU8(0x800B0DBAu) != 0u)
            return;
        g_frame++;
        RecordSewerVictory();
        RouteAmmoTrace();
        if (getenv("PE_GO_TRACE")) {
            /* Game Over phase trace: battle mode, mode-3 phase CE74, timer CE70, D1A0, token, VSync. */
            static uint32_t sig = 0xFFFFFFFFu;
            uint32_t m = PE_LoadU32(0x8009D28Cu), ph = PE_LoadU8(0x8009CE74u), tm = PE_LoadU8(0x8009CE70u);
            uint32_t ns = (m << 16) ^ (ph << 8) ^ (uint32_t)D_8009D280 ^ (PE_LoadU32(0x8009D1A0u) & 0x6u);
            if (ns != sig || (m == 3u && g_frame % 10 == 0)) {
                PeGpuState st; PE_GPU_GetState(&st);
                fprintf(stderr, "GO frame=%d vbl=%u token=%08X mode=%d phase=%u timer=%u d1a0=%X\n",
                        g_frame, st.vsync_count, (unsigned)D_8009D280, (int)m, ph, tm, PE_LoadU32(0x8009D1A0u));
                sig = ns;
            }
        }
        {   /* PE_ACTOR_TRACE=TT:II: log that actor's anim (+0xE) and x/z every 15 frames. */
            const char *at = getenv("PE_ACTOR_TRACE");
            if (at && g_frame % 15 == 0) {
                unsigned t = (unsigned)strtoul(at, NULL, 16), id = (unsigned)strtoul(strchr(at, ':') ? strchr(at, ':') + 1 : "0", NULL, 16);
                pe_addr_t a;
                for (a = PE_LoadU32(0x8009D20Cu); a; a = PE_LoadU32(a + 4u))
                    if (PE_LoadU8(a + 0xCu) == t && PE_LoadU8(a + 0xDu) == id) {
                        fprintf(stderr, "ACT frame=%d token=%08X anim=%02X pos=%d,%d\n", g_frame, (unsigned)D_8009D280,
                                PE_LoadU8(a + 0xEu), (int32_t)PE_LoadU32(a + 0x28u) >> 16, (int32_t)PE_LoadU32(a + 0x30u) >> 16);
                        break;
                    }
            }
        }
        {
            /* PE_ROUTE_SHOT_DIR=<dir>: write <dir>/<frame>_<token>.ppm on every
             * room change (+120 frames, so the room is drawn) during a route run. */
            static const char *dir = (const char *)1;
            static uint32_t shot_tok; static int shot_at = -1;
            if (dir == (const char *)1) dir = getenv("PE_ROUTE_SHOT_DIR");
            if (dir) {
                if ((uint32_t)D_8009D280 != shot_tok) { shot_tok = (uint32_t)D_8009D280; shot_at = g_frame + 120; }
                {
                    /* PE_ROUTE_SHOT_EVERY=N: also shoot every N frames while in A9400048. */
                    const char *ev = getenv("PE_ROUTE_SHOT_EVERY");
                    int n = ev ? atoi(ev) : 0;
                    if (n > 0 && (uint32_t)D_8009D280 == 0xA9400048u && g_frame > 100 && g_frame % n == 0)
                        shot_at = g_frame;
                    {   /* PE_ROUTE_SHOT_FROM/TO: also shoot every N frames in a frame window. */
                        const char *fr = getenv("PE_ROUTE_SHOT_FROM"), *to = getenv("PE_ROUTE_SHOT_TO");
                        if (n > 0 && ((fr && to && g_frame >= atoi(fr) && g_frame <= atoi(to) && g_frame % n == 0) ||
                                      (g_handed_over && ((g_frame - g_hand_over_frame) % n) == 0)))
                            shot_at = g_frame;
                    }
                }
                if (g_frame == shot_at) {
                    char path[512];
                    snprintf(path, sizeof path, "%s/%06d_%08X.ppm", dir, g_frame, (unsigned)shot_tok);
                    if (HostFB_WritePPM(path) == 0) fprintf(stderr, "[ROUTE] shot %s\n", path);
                }
            }
        }
        {
            /* gfx debug aids (off unless set):
             * PE_ROUTE_SHOT_RANGE=<dir>:first:last:step writes <dir>/f<frame>.ppm
             * for every step-th frame in [first,last]; PE_GPU_PRIM_LOG (pe_gpu.c)
             * gates its per-primitive log on this frame counter. */
            static int init, first, last, step = 1; static char rdir[400];
            if (!init) {
                const char *e = getenv("PE_ROUTE_SHOT_RANGE"); init = 1;
                if (e && sscanf(e, "%399[^:]:%d:%d:%d", rdir, &first, &last, &step) >= 3) {
                    if (step < 1) step = 1;
                } else rdir[0] = 0;
            }
            if (rdir[0] && g_frame >= first && g_frame <= last && (g_frame - first) % step == 0) {
                char path[512];
                snprintf(path, sizeof path, "%s/f%06d.ppm", rdir, g_frame);
                (void)HostFB_WritePPM(path);
            }
            PE_GPU_PrimLogFrame(g_frame);
        }
        {
            static uint32_t last_tok = 0xFFFFFFFFu, last_story = 0xFFFFFFFFu;
            uint32_t tok = (uint32_t)D_8009D280, st = PE_LoadU32(0x800A7918u);
            if (tok != last_tok || st != last_story) {
                fprintf(stderr, "[ROUTE] change frame=%d token=%08X story=%08X\n",
                        g_frame, (unsigned)tok, (unsigned)st);
                last_tok = tok; last_story = st;
            }
        }
        {
            /* PE_DEBUG_SAVE_MENU_AT=<frame>: verification aid for the memory
             * card.  At that route frame, run the field VM opcode E7 body
             * (func_80015AF0's second stage: 67CBC, the save-menu
             * constructor 4D18C, scene bits 9000, input bit 4) as a phone
             * would, so a headless run can reach the in-game save menu. */
            static int at = -2;
            if (at == -2) at = getenv("PE_DEBUG_SAVE_MENU_AT") ? atoi(getenv("PE_DEBUG_SAVE_MENU_AT")) : -1;
            if (at >= 0 && g_frame == at && !(PE_LoadU32(0x800B0CD8u) & 0x1000u)) {
                fprintf(stderr, "[ROUTE] debug: opening the save menu (E7 body) at frame %d token=%08X story=%08X\n",
                        g_frame, (unsigned)D_8009D280, (unsigned)PE_LoadU32(0x800A7918u));
                (void)func_80067CBC();
                func_8004D18C();
                PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) | 0x9000u);
                PE_StoreU32(0x8009D1A0u, PE_LoadU32(0x8009D1A0u) | 4u);
            }
        }
        if ((g_frame % 500) == 0)
            fprintf(stderr, "[ROUTE] frame=%d token=%08X story=%08X victories=%u\n",
                    g_frame, (unsigned)D_8009D280,
                    (unsigned)PE_LoadU32(0x800A7918u), g_sewer_victories);
    }
    if (g_opts.headless)
        return;
    if (HostWindow_Pace()) {PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);return;}
    HostWindow_Blit(pe_plat_cheats_overlay(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT),
                    PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
    (void)HostWindow_Poll();
}

/* VBlank scan-out refresh (host_framebuffer.c): show what the GPU displays
 * between PutDispEnv calls.  Display only — input is already polled by the
 * per-VBlank pacer, and nothing here counts as a present. */
static void ScanoutHook_BlitWindow(void)
{
    HostWindow_Blit(pe_plat_cheats_overlay(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT),
                    PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
}

static int ParsePositiveLimit(const char *option, const char *value) {
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (!value[0] || !end || *end || parsed < 1 || parsed > INT_MAX) {
        fprintf(stderr, "%s requires a positive integer\n", option);
        exit(1);
    }
    return (int)parsed;
}

static void ParseArgs(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if      (!strcmp(a, "--headless"))            g_opts.headless = 1;
        else if (!strcmp(a, "--bootstrap-disc"))       g_opts.bootstrap_disc = 1;
        else if (!strcmp(a, "--strict-stubs"))         g_opts.strict_stubs = 1;
        else if (!strcmp(a, "--windowed"))             g_opts.headless = 0;
        else if (!strcmp(a, "--fullscreen"))           g_opts.fullscreen = 1;
        else if (!strcmp(a, "--hold-until-close"))     g_opts.hold_until_close = 1;
        else if (!strcmp(a, "--debug-overlay"))        g_opts.debug_overlay = 1;
        else if (!strcmp(a, "--direct-clear-test"))    g_opts.direct_clear_test = 1;
        else if (!strcmp(a, "--disc-load-test"))       g_opts.disc_load_test = 1;
        else if (!strcmp(a, "--rng-oracle-dump"))      g_opts.rng_oracle_dump = 1;
        else if (!strcmp(a, "--lzcr-oracle-dump"))     g_opts.lzcr_oracle_dump = 1;
        else if (!strcmp(a, "--callback-oracle-dump")) g_opts.callback_oracle_dump = 1;
        else if (!strcmp(a, "--dma-checkpoint-report")) g_opts.dma_checkpoint_report = 1;
        else if (!strcmp(a, "--skip-movie"))           g_opts.skip_movie = 1;
        else if (!strcmp(a, "--skip-opening-menu"))    g_opts.skip_opening_menu = 1;
        else if (!strcmp(a, "--route-pad"))            g_opts.route_pad = 1;
        else if (!strcmp(a, "--auto-quit"))            g_opts.auto_quit = 1;
        else if (!strcmp(a, "--boundary-report"))      g_opts.boundary_report = 1;
        else if (!strcmp(a, "--hand-over-key"))        g_opts.hand_over_key = 1;
        else if (i+1<argc && !strcmp(a, "--hand-over-at"))     g_opts.hand_over_at = ParsePositiveLimit(a, argv[++i]);
        else if (i+1<argc && !strcmp(a, "--screenshot"))      g_opts.screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--vram-screenshot")) g_opts.vram_screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--vram-raw"))        g_opts.vram_raw = argv[++i];
        else if (i+1<argc && !strcmp(a, "--trace"))           g_opts.trace_path = argv[++i];
        else if (i+1<argc && !strcmp(a, "--hold-ms"))         g_opts.hold_ms = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--scale"))           g_opts.scale = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--window-title"))    g_opts.window_title = argv[++i];
        else if (i+1<argc && !strcmp(a, "--stop-after-event")) g_opts.stop_after_event = argv[++i];
        else if (i+1<argc && !strcmp(a, "--max-frames"))
            g_opts.max_frames = ParsePositiveLimit(a, argv[++i]);
        else if (i+1<argc && !strcmp(a, "--max-main-iterations"))
            g_opts.max_main_iterations = ParsePositiveLimit(a, argv[++i]);
        else if (i+1<argc && !strcmp(a, "--disc-image"))      g_opts.disc_image = argv[++i];
        else { fprintf(stderr, "Unknown: %s\n", a); exit(1); }
    }
    if (g_opts.hold_ms > 0) g_opts.hold_until_close = 0;
    if (g_opts.bootstrap_disc && g_opts.disc_image) {
        fprintf(stderr, "--bootstrap-disc and --disc-image are mutually exclusive\n");
        exit(1);
    }
    if (g_opts.disc_load_test && !g_opts.disc_image) {
        fprintf(stderr, "--disc-load-test requires --disc-image\n");
        exit(1);
    }
    if (g_opts.rng_oracle_dump && !g_opts.disc_image) {
        fprintf(stderr, "--rng-oracle-dump requires --disc-image\n");
        exit(1);
    }
}

/* ── Trace ──────────────────────────────────────────────────────────── */
static FILE *g_trace_fp = NULL; static int g_trace_seq = 0;
static void TraceInit(void) { if (g_opts.trace_path) g_trace_fp = fopen(g_opts.trace_path, "w"); }
static void TraceEvent(const char *e) {
    if (g_trace_fp) { fprintf(g_trace_fp, "%04d %s\n", ++g_trace_seq, e); fflush(g_trace_fp); }
    fprintf(stderr, "[TRACE %04d] %s\n", g_trace_seq, e);
}
static void TraceClose(void) { if (g_trace_fp) { fclose(g_trace_fp); g_trace_fp = NULL; } }
void Trace_Direct(const char *e) { TraceEvent(e); }

/* ── Real-disc load test ────────────────────────────────────────────── */
/* Explicit verification driver (like --direct-clear-test): runs the real
 * Phase 6E-A disc byte path against --disc-image — PVD verify, DsSearchFile
 * "\PE.IMG;1", CdPosToInt, bounded guest-RAM load at the D_80011614
 * destination, poll — and traces every value.  Deterministic for a given
 * image; used for the three-run real-disc trace gate. */
#define PE_LOADTEST_CDLFILE  0x801FFEC0u   /* documented guest scratch */
#define PE_LOADTEST_MAX      0x8000u       /* 32 KiB load cap          */

static uint64_t PeFnv1a64(const uint8_t *p, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    size_t i;
    for (i = 0; i < n; i++) { h ^= p[i]; h *= 1099511628211ULL; }
    return h;
}

static int RunDiscLoadTest(void) {
    char buf[160];
    int r;
    int lba;
    uint32_t size, load_size, load_sectors;
    uint64_t h;

    TraceEvent("disc_load_test_begin");
    /* Drive reset first, as the retail boot path does (CdInit/reset bring
     * the synchronous drive model to the idle lane before any verify). */
    func_8007EC14();
    func_8007ED58();
    r = func_80082314();
    snprintf(buf, sizeof(buf), "disc_load_pvd_verify=%d", r);
    TraceEvent(buf);
    if (r != 4) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_80081414(PE_LOADTEST_CDLFILE, "\\PE.IMG;1");
    snprintf(buf, sizeof(buf), "disc_load_search_pe_img=%d", r);
    TraceEvent(buf);
    if (r != 1) { TraceEvent("disc_load_test_fail"); return 1; }

    lba = func_80080C48(PE_LOADTEST_CDLFILE);
    size = PE_LoadU32(PE_LOADTEST_CDLFILE + 4);
    load_size = size < PE_LOADTEST_MAX ? size : PE_LOADTEST_MAX;
    load_sectors = (load_size + PE_DISC_USER_SECTOR - 1u) /
                   PE_DISC_USER_SECTOR;
    load_size = load_sectors * PE_DISC_USER_SECTOR;
    snprintf(buf, sizeof(buf), "disc_load_pe_img_lba=%d_size=%u", lba, size);
    TraceEvent(buf);
    if (lba <= 0 || size == 0) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_8006E6D4(lba, 0, D_80011614, (int)load_sectors);
    snprintf(buf, sizeof(buf), "disc_load_issue=%d_dest=0x%08X_len=%u",
             r, (unsigned)D_80011614, load_size);
    TraceEvent(buf);
    if (r != 1) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_800811E4(PE_LOADTEST_CDLFILE);
    snprintf(buf, sizeof(buf), "disc_load_poll=%d", r);
    TraceEvent(buf);
    if (r != 0) { TraceEvent("disc_load_test_fail"); return 1; }

    h = PeFnv1a64(PE_Translate(D_80011614, load_size), load_size);
    snprintf(buf, sizeof(buf), "disc_load_fnv1a64=%016llX",
             (unsigned long long)h);
    TraceEvent(buf);
    TraceEvent("disc_load_test_ok");
    return 0;
}

/* ── RNG oracle dump ────────────────────────────────────────────────── */
/* Phase 6E-B2 verification driver (requires --disc-image so the retail
 * exe bytes are in guest RAM): seeds via func_80070D10, runs the 2000-call
 * warm-up, and prints checkpoints + func_80070DD0 samples to stdout in
 * exactly the format produced by pc_port/tools/rng_oracle.py.  The phase
 * gate diffs the two outputs; they must be identical. */
#define GA_DUMP_INDEX1  0x80070E04u
#define GA_DUMP_INDEX2  0x80070E08u
#define GA_DUMP_TABLE   0x80070E0Cu

static int RunRngOracleDump(void) {
    static const int k_checkpoints[] = { 1, 2, 16, 17, 64, 256, 2000 };
    static const int k_ranges[][2] = {
        { 0, 100 }, { 1, 4 }, { 0, 65536 }, { 5, 5 }, { 10, 0 }, { -3, 3 }
    };
    func_80070D10();
    for (int call = 1; call <= 2000; call++) {
        int pre1 = (int)PE_LoadU32(GA_DUMP_INDEX1);
        int pre2 = (int)PE_LoadU32(GA_DUMP_INDEX2);
        unsigned int v0 = func_80070D6C();
        for (size_t k = 0; k < sizeof(k_checkpoints)/sizeof(k_checkpoints[0]); k++) {
            if (call == k_checkpoints[k]) {
                printf("checkpoint call=%5d v0=0x%08X i1=%4d i2=%4d "
                       "addr1=0x%08X addr2=0x%08X\n",
                       call, v0,
                       (int)PE_LoadU32(GA_DUMP_INDEX1),
                       (int)PE_LoadU32(GA_DUMP_INDEX2),
                       GA_DUMP_TABLE + (pe_addr_t)pre1,
                       GA_DUMP_TABLE + (pe_addr_t)pre2);
            }
        }
    }
    for (size_t k = 0; k < sizeof(k_ranges)/sizeof(k_ranges[0]); k++) {
        int a0 = k_ranges[k][0], a1 = k_ranges[k][1];
        int out = func_80070DD0(a0, a1);
        printf("70DD0(%d,%d) = %u (0x%08X)\n", a0, a1,
               (unsigned int)out, (unsigned int)out);
    }
    return 0;
}

/* ── LZCR oracle dump ───────────────────────────────────────────────── */
/* Phase 6E-B4 verification driver (pure arithmetic + guest RAM — no
 * --disc-image required): calls the REAL func_8003EAC8 for the oracle
 * input set and prints one line per input in exactly the format produced
 * by pc_port/tools/lzcr_oracle.py (a tiny MIPS interpreter over the
 * verified retail words).  `lzcr`/`idx` come from PE_GTE_LZCR and the
 * translated branch; `dest` is found by SCANNING guest RAM for the unique
 * sentinel — independent of the formula — so an inconsistent store path
 * cannot hide.  The phase gate diffs the two outputs; must be identical. */
#define LZCR_DUMP_SCAN_LO  0x800A76E0u   /* 3 words below D_800A76F0-4   */
#define LZCR_DUMP_SCAN_HI  0x800A7770u   /* 1 word past D_800A76F0+0x7C  */

static int RunLzcrOracleDump(void) {
    static const uint32_t k_first[] = {
        0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
        0x00000008u, 0x00008000u, 0x40000000u, 0x7FFFFFFFu,
        0x80000000u, 0x80000001u, 0xC0000000u, 0xFFFFFFFFu
    };
    static const uint32_t k_skip[] = {
        0x00000001u, 0x00000002u, 0x00000008u,
        0x00008000u, 0x40000000u, 0x80000000u
    };
    uint32_t inputs[12 + 26];
    int n = 0;
    for (size_t i = 0; i < sizeof(k_first)/sizeof(k_first[0]); i++)
        inputs[n++] = k_first[i];
    for (int k = 0; k < 32; k++) {
        uint32_t m = 1u << k, skip = 0;
        for (size_t j = 0; j < sizeof(k_skip)/sizeof(k_skip[0]); j++)
            if (k_skip[j] == m) skip = 1;
        if (!skip) inputs[n++] = m;
    }

    for (int i = 0; i < n; i++) {
        uint32_t mask = inputs[i];
        uint32_t sentinel = 0xEAC80000u | (uint32_t)i;
        pe_addr_t dest = 0;
        int found = 0;
        for (pe_addr_t a = LZCR_DUMP_SCAN_LO; a <= LZCR_DUMP_SCAN_HI; a += 4)
            PE_StoreU32(a, 0);
        func_8003EAC8((int)mask, (int)sentinel);
        for (pe_addr_t a = LZCR_DUMP_SCAN_LO; a <= LZCR_DUMP_SCAN_HI; a += 4) {
            if (PE_LoadU32(a) == sentinel) { dest = a; found++; }
        }
        if (found != 1) {
            printf("EAC8 a0=0x%08X FATAL sentinel found %d times\n", mask, found);
            return 1;
        }
        int32_t idx = (mask == 0x80000000u) ? 31
                                            : 31 - (int32_t)PE_GTE_LZCR(mask);
        printf("EAC8 a0=0x%08X lzcr=%u idx=%d dest=0x%08X stored=0x%08X\n",
               mask, PE_GTE_LZCR(mask), idx, dest, PE_LoadU32(dest));
    }
    return 0;
}

/* ── Callback oracle dump ───────────────────────────────────────────── */
/* Phase 6E-B6 verification driver (pure guest RAM + host bindings — no
 * --disc-image required): mirrors the fixed operation script of
 * pc_port/tools/callback_oracle.py op-for-op through the production
 * func_80073D24 / PE_Callback_SetSlot / PE_Callback_Dispatch path and
 * prints byte-identical lines.  The phase gate diffs the two outputs. */
static pe_addr_t  g_cb_dump_visits[16];
static int        g_cb_dump_visit_count;
static void CbDumpVisitA(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x80010000u; }
static void CbDumpVisitB(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x80010004u; }
static void CbDumpVisitC(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x80010008u; }
static void CbDumpVisitD(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x8003E91Cu; }

static void CbDumpWset(pe_addr_t handler) {
    uint32_t prev = func_80073D24(handler);
    printf("wset handler=0x%08X prev=0x%08X slot4=0x%08X\n",
           handler, prev, PE_LoadU32(0x8009569Cu));
}
static void CbDumpSset(uint32_t slot, pe_addr_t handler) {
    uint32_t prev = PE_Callback_SetSlot(slot, handler);
    printf("sset slot=%u handler=0x%08X prev=0x%08X\n", slot, handler, prev);
}
static void CbDumpDispatch(void) {
    g_cb_dump_visit_count = 0;
    PE_Callback_Dispatch();
    printf("dispatch counter=%u visits=",
           PE_LoadU32(0x800956ACu));
    if (g_cb_dump_visit_count == 0) {
        printf("-");
    } else {
        for (int i = 0; i < g_cb_dump_visit_count; i++) {
            printf("%s0x%08X", i ? "," : "", g_cb_dump_visits[i]);
        }
    }
    printf("\n");
}

static int RunCallbackOracleDump(void) {
    PE_Callback_Bind(0x80010000u, CbDumpVisitA);
    PE_Callback_Bind(0x80010004u, CbDumpVisitB);
    PE_Callback_Bind(0x80010008u, CbDumpVisitC);
    PE_Callback_Bind(0x8003E91Cu, CbDumpVisitD);

    CbDumpWset(0x00000000u);            /* clear slot 4                    */
    CbDumpWset(0x8003E91Cu);            /* install boot callback           */
    CbDumpWset(0x8003E91Cu);            /* repeated install: no store      */
    CbDumpWset(0x80010000u);            /* replacement                     */
    CbDumpWset(0x00000000u);            /* removal                         */
    CbDumpWset(0x00000000u);            /* repeated removal                */
    CbDumpSset(0, 0x80010000u);
    CbDumpSset(2, 0x80010004u);
    CbDumpSset(7, 0x80010008u);
    CbDumpDispatch();                   /* counter=1, visits slots 0,2,7   */
    CbDumpWset(0x8003E91Cu);            /* re-install slot 4               */
    CbDumpDispatch();                   /* counter=2, visits 0,2,4,7       */
    CbDumpSset(8, 0x11111111u);         /* slot 8 aliases counter (retail) */

    printf("snapshot slots=");
    for (uint32_t i = 0; i < 8; i++) {
        printf("%s0x%08X", i ? "," : "", PE_Callback_GetSlot(i));
    }
    printf(" counter=0x%08X\n", PE_LoadU32(0x800956ACu));
    return 0;
}

/* ── Title overlay ──────────────────────────────────────────────────── */
static void UpdateTitle(const char *phase, const char *func) {
    if (!g_opts.debug_overlay || !g_host_window_open) return;
    char buf[256];
    int vs, ds, pr, mk;
    HostFB_GetState(&vs, &ds, &pr, &mk);
    snprintf(buf, sizeof(buf),
        "%s | %s | %s | iter %d | frame %d | vsync %d | stubs %d",
        g_opts.window_title, phase, func ? func : "-",
        g_port_main_iterations, pr, vs, g_stub_bootstrap_invocations);
    HostWindow_SetTitle(buf);
}

/* ── main ───────────────────────────────────────────────────────────── */
static void AudioStatsAtExit(void)
{
    PeAudioDriverStats d;
    PeSpuStats sp;
    PE_AudioDriver_GetStats(&d);
    PE_Spu_GetStats(&sp);
    fprintf(stderr, "[AUDIO] summary: vblanks=%llu ticks=%llu skipped=%llu "
            "faults=%llu insns=%llu | KON=%llu KOFF=%llu regwrites=%llu "
            "fifo=%llu frames=%llu\n",
            (unsigned long long)d.vblanks, (unsigned long long)d.ticks,
            (unsigned long long)d.skipped_ticks, (unsigned long long)d.faults,
            (unsigned long long)d.instructions,
            (unsigned long long)sp.kon_events, (unsigned long long)sp.koff_events,
            (unsigned long long)sp.register_writes,
            (unsigned long long)sp.fifo_bytes,
            (unsigned long long)sp.rendered_frames);
    PE_Spu_DebugSummary(stderr);
}

#if !defined(_WIN32)
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
/* Debug aid (fmv lane): PE_WATCHDOG_SEC=N dumps a backtrace every N s so a
 * spinning guest loop can be located with addr2line (no debugger here). */
static unsigned g_watchdog_sec;
static void WatchdogHandler(int sig)
{
    void *frames[48];
    int n = backtrace(frames, 48);
    (void)sig;
    (void)!write(2, "[WATCHDOG] backtrace\n", 21);
    backtrace_symbols_fd(frames, n, 2);
    alarm(g_watchdog_sec);
}
static void WatchdogArm(void)
{
    const char *w = getenv("PE_WATCHDOG_SEC");
    if (!w || !(g_watchdog_sec = (unsigned)atoi(w))) return;
    signal(SIGALRM, WatchdogHandler);
    signal(SIGABRT, WatchdogHandler);
    alarm(g_watchdog_sec);
}

/* Host termination.  The Linux launcher runs the port as
 * `parasite-eve-port ... 2>&1 | tee run-*.log`, and a Steam / gamescope
 * "exit game" is SIGTERM (then SIGKILL).  With default dispositions the
 * port died without a line in the log, which reads the same as a crash
 * (build/lanes/legion/run-20260929T151702Z.log).
 *  - SIGTERM/SIGINT/SIGHUP: logged, then the normal host-quit stop (the
 *    path Escape / window close take), so the log ends with the usual
 *    summary; the signal is re-raised at exit so the parent still sees it.
 *    A second signal, or 5 s without reaching exit, terminates at once.
 *  - SIGPIPE ignored: if tee goes away first, stderr writes fail with EPIPE
 *    instead of killing the game.
 *  - SIGSEGV/SIGBUS/SIGILL/SIGFPE: one line + backtrace, then the default
 *    action, so a crash is distinguishable from a kill in a user's log. */
static volatile sig_atomic_t g_host_term_sig;

static const char *HostSigName(int sig)
{
    switch (sig) {
    case SIGTERM: return "SIGTERM";
    case SIGINT:  return "SIGINT";
    case SIGHUP:  return "SIGHUP";
    case SIGSEGV: return "SIGSEGV";
    case SIGBUS:  return "SIGBUS";
    case SIGILL:  return "SIGILL";
    case SIGFPE:  return "SIGFPE";
    default:      return "signal";
    }
}
static void HostSigWrite(const char *a, const char *b, const char *c)
{
    (void)!write(2, a, strlen(a));
    (void)!write(2, b, strlen(b));
    (void)!write(2, c, strlen(c));
}
static void HostTermDeadline(int sig)
{
    int term = (int)g_host_term_sig;
    (void)sig;
    HostSigWrite("[HOST] shutdown did not finish within 5 s of ", HostSigName(term),
                 "; exiting now\n");
    signal(term, SIG_DFL);
    raise(term);
}
static void HostTermHandler(int sig)
{
    /* SA_RESETHAND: a second signal takes the default action. */
    g_host_term_sig = sig;
    HostSigWrite("[HOST] ", HostSigName(sig), " received: stopping (host-quit)\n");
    if (!g_watchdog_sec) {
        signal(SIGALRM, HostTermDeadline);
        alarm(5);
    }
}
static void HostCrashHandler(int sig)
{
    void *frames[48];
    int n = backtrace(frames, 48);
    HostSigWrite("[HOST] fatal ", HostSigName(sig), ": backtrace\n");
    backtrace_symbols_fd(frames, n, 2);
    raise(sig);   /* SA_RESETHAND: default action (core) on return */
}
static void HostSignalsInstall(void)
{
    static char altstack[64 * 1024];
    static const int term[] = {SIGTERM, SIGINT, SIGHUP};
    static const int crash[] = {SIGSEGV, SIGBUS, SIGILL, SIGFPE};
    struct sigaction sa;
    stack_t ss;
    void *warm[1];

    (void)backtrace(warm, 1);     /* load libgcc now, not inside a handler */
    signal(SIGPIPE, SIG_IGN);
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sa.sa_handler = HostTermHandler;
    for (unsigned i = 0; i < sizeof(term) / sizeof(term[0]); i++) sigaction(term[i], &sa, NULL);
    memset(&ss, 0, sizeof(ss));
    ss.ss_sp = altstack;
    ss.ss_size = sizeof(altstack);
    sa.sa_flags = SA_RESETHAND | (sigaltstack(&ss, NULL) == 0 ? SA_ONSTACK : 0);
    sa.sa_handler = HostCrashHandler;
    for (unsigned i = 0; i < sizeof(crash) / sizeof(crash[0]); i++) sigaction(crash[i], &sa, NULL);
}
static void HostSignalsExit(void)
{
    int sig = (int)g_host_term_sig;
    if (!sig) return;
    fprintf(stderr, "[HOST] exiting on %s\n", HostSigName(sig));
    fflush(stderr);
    signal(sig, SIG_DFL);
    raise(sig);
}
#else
static void WatchdogArm(void) {}
static const int g_host_term_sig = 0;
static void HostSignalsInstall(void) {}
static void HostSignalsExit(void) {}
#endif
static int QuitPoll_Window(void) { return g_host_term_sig || HostWindow_Poll(); }
static int QuitPoll_Signal(void) { return g_host_term_sig != 0; }

int main(int argc, char **argv) {
    HostSignalsInstall();
    WatchdogArm();
    ParseArgs(argc, argv); TraceInit();
    g_bootstrap_disc = g_opts.bootstrap_disc;
    g_strict_stubs   = g_opts.strict_stubs;

    /* Phase 6D-S: initialize host-safe subsystems */
    PE_RamInit();
    PE_GuestCode_InstallGenerated();   /* guest-code dispatch registry */
    PE_Callback_Init();
    Bootstrap_Init();
    HostFB_Init();
    PE_Port_RunControlReset();
    PE_Port_SetFrameLimit(g_opts.max_frames);
    PE_Port_SetMainIterationLimit(g_opts.max_main_iterations);
    PE_Port_SetSkipMovie(g_opts.skip_movie);
    PE_Port_SetSkipOpeningMenu(g_opts.skip_opening_menu);
    if (g_strict_stubs) Bootstrap_EnableStrict();
    /* Native only (owner, 2026-10-07): a call into retail code the port has
     * no C for is a CPU_BOUNDARY/REFUSED STOP, never emulated and never
     * silently answered with 0.  PE_BOUNDARY_POLICY=record restores the old
     * log-and-continue behaviour for diagnosis only. */
    {
        extern void PE_Decomp_SetBoundaryStop(int stop);
        const char *bp = getenv("PE_BOUNDARY_POLICY");
        int stop = !(bp && strcmp(bp, "record") == 0);
        PE_Decomp_SetBoundaryStop(stop);
        fprintf(stderr, "[NATIVE] boundary policy=%s (no MIPS interpreter, "
                "no CPU emulation in this binary)\n", stop ? "stop" : "record");
    }

    /* Phase 6E-A: real Disc 1 image, read-only (never copied or staged). */
    PE_Disc *disc = NULL;
    if (g_opts.disc_image) {
        char err[256];
        disc = PE_Disc_Open(g_opts.disc_image, err, sizeof(err));
        if (!disc) {
            fprintf(stderr, "[DISC] failed to open disc image: %s\n", err);
            TraceClose();
            PE_RamDestroy();
            return 1;
        }
        PE_Disc_SetActive(disc);
        {
            /* First-run disc check (pe_disc_check.h): SYSTEM.CNF boot serial
             * + boot-EXE SHA-1 against the known-disc table.  The game boots
             * from Disc 1; Disc 2 is recognised and rejected with a clear
             * message until disc swapping lands. */
            PE_DiscIdentity id;
            char why[384];
            char vol[40];
            if (PE_DiscCheck_Identify(disc, 1u, &id, why, sizeof(why)) != 0) {
                fprintf(stderr, "[DISC] %s: %s\n", g_opts.disc_image, why);
                fprintf(stderr, "[DISC] The game reads everything from your own disc image: "
                                "pass the Parasite Eve (USA) Disc 1 .bin (BIN/CUE dump).\n");
                TraceClose();
                PE_Disc_Close(disc);
                PE_RamDestroy();
                return 1;
            }
            if (!PE_Disc_VolumeId(disc, vol, sizeof(vol)))
                vol[0] = '\0';
            fprintf(stderr,
                    "[DISC] opened '%s' (%u user sectors, vol='%s'): Parasite Eve %s "
                    "Disc %d (%s), EXE sha1 %s verified\n",
                    g_opts.disc_image, PE_Disc_UserSectorCount(disc), vol,
                    id.known->region, id.known->disc, id.known->serial, id.exe_sha1);
        }

        /* Phase 6E-B2: load the retail boot executable into guest RAM —
         * retail code reads its own text as data (the func_80070D6C RNG
         * read cursor cycles through 14 code words below its table).
         * A disc that cannot supply its boot exe is rejected here. */
        if (PE_GuestImage_LoadExe(disc, err, sizeof(err)) != 0) {
            fprintf(stderr, "[DISC] boot executable load failed: %s\n", err);
            TraceClose();
            PE_Disc_Close(disc);
            PE_RamDestroy();
            return 1;
        }
        if (PE_Globals_AdoptRetailImage() != 0) {
            fprintf(stderr, "[DISC] retail overlay authority is invalid\n");
            TraceClose();
            PE_Disc_Close(disc);
            PE_RamDestroy();
            return 1;
        }
        fprintf(stderr, "[DISC] boot executable loaded into guest RAM\n");
    }

    TraceEvent("native_executable_start");

    if (g_opts.lzcr_oracle_dump) {
        int rc = RunLzcrOracleDump();
        TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    if (g_opts.callback_oracle_dump) {
        int rc = RunCallbackOracleDump();
        TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    if (g_opts.rng_oracle_dump) {
        int rc = RunRngOracleDump();
        TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    if (g_opts.disc_load_test) {
        int rc = RunDiscLoadTest();
        TraceEvent("shutdown_end"); TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    pe_plat_cheats_init();   /* PE_CHEATS=0 hard-disables */
    int use_window = !g_opts.headless;
    if (use_window) {
        const char *dpy = getenv("DISPLAY") ? getenv("DISPLAY") : ":10.0";
        int w = PE_PORT_FB_WIDTH * g_opts.scale;
        int h = PE_PORT_FB_HEIGHT * g_opts.scale;
        if (HostWindow_Open(dpy, w, h, g_opts.window_title, g_opts.scale,
                            g_opts.fullscreen) != 0)
            use_window = 0;
        else {
            PE_Port_SetQuitPoll(QuitPoll_Window);
            PE_Port_SetPresentHook(PresentHook_BlitWindow);
            HostFB_SetScanoutHook(ScanoutHook_BlitWindow);
            PE_Port_SetPadSource(HostWindow_PadRaw);
        }
    }

    if (!use_window) PE_Port_SetQuitPoll(QuitPoll_Signal);
    /* PE_CHEAT_KEYS (scripted cheat keys) needs the present hook headless too. */
    if (g_opts.headless && (getenv("PE_CHEAT_KEYS") || getenv("PE_SHOT_DIR")))
        PE_Port_SetPresentHook(PresentHook_BlitWindow);
    /* Autopilot-only aids (M34 HP lock etc.) are test-harness only: they may
     * be armed solely by --route-pad.  A normal start must never see them. */
    if (!g_opts.route_pad) {
        if (g_route_m34_shim || g_route_battle_shim) {
            fprintf(stderr, "[ROUTE_SHIM] FATAL: route aid armed without --route-pad\n");
            abort();
        }
        fprintf(stderr, "[ROUTE_SHIM] disabled (normal start: no --route-pad)\n");
    }
    if (g_opts.route_pad) {
        /* Interactive Day-1 autopilot: same four-stage table + switch frames
         * the route harness proves (shared pe_route_pad.h).  A route-pad run
         * implies both documented HOST_ADAPTED skips, otherwise the
         * untranslated title/menu is still in front of the field and no pad
         * can get past it. */
        PeRoutePad_ConfigFromEnv(&g_route_pad);
        g_route_frame = 0;
        g_frame = 0;
        g_sewer_victories = 0;
        g_sewer_enemy_peak[0]=g_sewer_enemy_peak[1]=g_sewer_enemy_peak[2]=0;
        RoutePadLoadSequence();
        g_route_m34_shim = !getenv("PE_ROUTE_M34_NO_SHIM");
        g_route_battle_shim = !getenv("PE_ROUTE_BATTLE_NO_SHIM");
        if (g_route_m34_shim)
            fprintf(stderr, "[ROUTE_SHIM] M34 HP lock + simple tail controller enabled "
                            "(PE_ROUTE_M34_NO_SHIM=1 disables; see DAY1_FIDELITY_GAPS.md)\n");
        /* PE_ROUTE_PLAY_MOVIES=1 (fmv2 harness aid) keeps this skip for
         * the boot/title but lets field opcode 0x35 play its movie; the
         * present hook holds the route clock while one is showing. */
        PE_Port_SetSkipMovie(1);
        /* PE_ROUTE_REAL_NAME_ENTRY=1: run the original name-entry handshake
         * instead of the HOST_ADAPTED skip (diagnostic). */
        PE_Port_SetSkipOpeningMenu(getenv("PE_ROUTE_REAL_NAME_ENTRY") ? 0 : 1);
        PE_Port_SetPadSource(RoutePadSource);
        /* Headless runs never open a window; still need the present hook so
         * g_frame (and therefore the pilot) advances with HostFB presents. */
        if (g_opts.headless)
            PE_Port_SetPresentHook(PresentHook_BlitWindow);
        fprintf(stderr,
                "[ROUTE] --route-pad: full Day-1/Day-2 pad sequence (%u pairs) "
                "+ sewer/M34 pilot + skip-movie + skip-opening-menu\n",
                g_pad_sequence_count);
    }

    /* fmv lane: PE_PAD_SCRIPT="present:mask,..." (active-low raw masks,
     * e.g. FFF7 Start, DFFF Circle) drives a headless non-route run by
     * host present count, so title/movie/menu paths can be verified
     * without --route-pad (which forces --skip-movie). */
    if (g_opts.route_pad && getenv("PE_PAD_SCRIPT")) {
        ScriptPadLoad(getenv("PE_PAD_SCRIPT"));
        fprintf(stderr, "[PAD] PE_PAD_SCRIPT: %u scripted pad changes after the route hand-over\n",
                g_script_pad_count);
    }
    if (!g_opts.route_pad && getenv("PE_PAD_SCRIPT")) {
        ScriptPadLoad(getenv("PE_PAD_SCRIPT"));
        PE_Port_SetPadSource(ScriptPadSource);
        fprintf(stderr, "[PAD] PE_PAD_SCRIPT: %u scripted pad changes\n",
                g_script_pad_count);
    }

    /* Audio lane: native SPU + retail AKAO driver tick + host output.
     * PE_AUDIO=0 keeps the historical silent command-only behavior.  The
     * sound device opens with a window (or PE_AUDIO_DEVICE=1); the WAV dump
     * (PE_AUDIO_WAV) works headless. */
    {
        const char *pa = getenv("PE_AUDIO");
        const char *pd = getenv("PE_AUDIO_DEVICE");
        if (!(pa && pa[0] == '0')) {
            int dev = pd ? pd[0] != '0' : use_window;
            (void)HostAudio_Open(dev);
            PE_AudioDriver_SetSink(HostAudio_Submit);
            PE_AudioDriver_Enable(1);
            atexit(AudioStatsAtExit);
            fprintf(stderr, "[AUDIO] native SPU + AKAO driver enabled "
                    "(device=%d, PE_AUDIO=0 disables)\n", HostAudio_DeviceOpen());
        }
    }

    if (g_opts.direct_clear_test) {
        /* Explicit test path — requires --direct-clear-test flag */
        TraceEvent("direct_clear_test_begin");
        UpdateTitle("Direct clear test", "func_8006E9A0");
        func_8006E9A0(0);
        TraceEvent("direct_clear_test_done");
    } else {
        /* Normal path: translated Parasite Eve main */
        TraceEvent("call_func_8001220C");
        UpdateTitle("Main entry", "func_8001220C");
        func_8001220C();
        TraceEvent("func_8001220C_returned");
    }

    TraceEvent("shutdown_begin");

    if (use_window) {
        TraceEvent("window_blit");
        HostWindow_Blit(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
        UpdateTitle("Black frame", "waiting");
        if (PE_Port_GetStopReason() != PE_PORT_STOP_NONE) {
            /* A quit request or explicit budget already ended execution. */
        } else if (g_opts.hold_until_close) {
            fprintf(stderr, "[WINDOW] Open until close/Escape...\n");
            HostWindow_Run(-1);
        } else if (g_opts.hold_ms > 0) {
            HostWindow_Run(g_opts.hold_ms);
        } else {
            HostWindow_Run(2000);
        }
        PE_Port_SetQuitPoll(NULL);
        PE_Port_SetPresentHook(NULL);
        HostWindow_Close();
    }

    const char *sp = g_opts.screenshot ? g_opts.screenshot : "/tmp/pe-port-black.ppm";
    if (HostFB_WritePPM(sp) == 0)
        fprintf(stderr, "[SCREENSHOT] %s (%dx%d)\n", sp, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);

    if (g_opts.vram_raw) {
        if (HostVRAM_WriteRaw(g_opts.vram_raw) == 0) {
            fprintf(stderr, "[VRAM-RAW] %s (%ux%u RGB555/STP little-endian)\n",
                    g_opts.vram_raw, PE_GPU_VRAM_WIDTH, PE_GPU_VRAM_HEIGHT);
        } else {
            fprintf(stderr, "[VRAM-RAW] failed to write '%s'\n", g_opts.vram_raw);
        }
    }
    if (g_opts.vram_screenshot) {
        if (HostVRAM_WritePPM(g_opts.vram_screenshot) == 0) {
            fprintf(stderr, "[VRAM-SCREENSHOT] %s (%ux%u RGB555 diagnostic)\n",
                    g_opts.vram_screenshot,
                    PE_GPU_VRAM_WIDTH, PE_GPU_VRAM_HEIGHT);
        } else {
            fprintf(stderr, "[VRAM-SCREENSHOT] failed to write '%s'\n",
                    g_opts.vram_screenshot);
        }
    }

    Stub_PrintSummary();
    int vs, ds, pr, mk; HostFB_GetState(&vs, &ds, &pr, &mk);
    /* vsyncs = VSync() CALLS (counter queries included), not elapsed time;
     * vblanks = emulated VBlanks (PE_GPU_VSyncQuery), the unit the emulator
     * oracle and retail timings use. */
    fprintf(stderr, "[FB] vsyncs=%d drawsyncs=%d presents=%d mask=%d main_iters=%d vblanks=%u\n",
            vs, ds, pr, mk, g_port_main_iterations, (unsigned)PE_GPU_VSyncQuery());
    fprintf(stderr, "[HOST] stop_reason=%s\n",
            PE_Port_StopReasonName(PE_Port_GetStopReason()));
    fprintf(stderr, "[HOST] final room token=%08X story persist[74]=%08X\n",
            (unsigned)PE_LoadU32(0x8009D280u), (unsigned)PE_LoadU32(0x800A7918u));
    {
        PeGpuState gpu;

        PE_GPU_GetState(&gpu);
        fprintf(stderr,
                "[GPU] fills=%llu mono_rects=%llu tex_rects=%llu moves=%llu polygons=%llu polygon_pixels=%llu "
                "draw_mode_writes=%llu last_mono=0x%08X@0x%08X size=0x%08X\n",
                (unsigned long long)gpu.fill_count,
                (unsigned long long)gpu.mono_rectangle_count,
                (unsigned long long)gpu.rectangle_count,
                (unsigned long long)gpu.move_count,
                (unsigned long long)gpu.polygon_count,
                (unsigned long long)gpu.polygon_pixel_count,
                (unsigned long long)gpu.draw_mode_count,
                gpu.mono_rectangle_command, gpu.mono_rectangle_position,
                gpu.mono_rectangle_size);
    }
    if (g_opts.dma_checkpoint_report) {
        PEPortDmaIrqCheckpointTrace checkpoint;

        PE_Port_GetDmaIrqCheckpointTrace(&checkpoint);
        fprintf(stderr,
                "[DMA_CHECKPOINT] calls=%llu queries=%llu services=%llu "
                "captured=%llu serviced=%llu\n",
                (unsigned long long)checkpoint.checkpoint_calls,
                (unsigned long long)checkpoint.token_queries,
                (unsigned long long)checkpoint.service_calls,
                (unsigned long long)checkpoint.last_captured_token,
                (unsigned long long)checkpoint.last_serviced_token);
    }

    if (g_opts.boundary_report) {
        /* Recorded boundary/indirect-call arguments, oldest first.  This is
         * the census view of exactly which guest shape a named cut refused
         * (e.g. the GP0 word a DrawOTag node carried), so frontier work can
         * be scoped from evidence rather than guessed. */
        fprintf(stderr, "[BOUNDARY_REPORT] %d recorded arg4 call(s)%s\n",
                g_bootstrap_arg4_call_count,
                g_bootstrap_arg4_call_count >= BOOTSTRAP_MAX_ARG4_CALLS
                    ? " (log full; later calls dropped)" : "");
        for (int i = 0; i < g_bootstrap_arg4_call_count; i++) {
            const BootstrapArgCall4 *c = &g_bootstrap_arg4_calls[i];
            fprintf(stderr,
                    "[BOUNDARY_REPORT] %3d %s <- %s target=0x%08lX "
                    "a0=0x%08lX a1=0x%08lX a2=0x%08lX a3=0x%08lX",
                    i, c->symbol, c->caller,
                    (unsigned long)c->target, (unsigned long)c->arg0,
                    (unsigned long)c->arg1, (unsigned long)c->arg2,
                    (unsigned long)c->arg3);
            if (c->payload_size) {
                fprintf(stderr, " payload=");
                for (uint32_t k = 0; k < c->payload_size; k++)
                    fprintf(stderr, "%02X", c->payload[k]);
            }
            fprintf(stderr, "\n");
        }
    }

    TraceEvent("shutdown_end"); TraceClose();
    PE_Disc_Close(disc);
    PE_RamDestroy();
    HostSignalsExit();
    return 0;
}
