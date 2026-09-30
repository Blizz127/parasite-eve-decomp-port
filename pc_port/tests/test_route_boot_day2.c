/*
 * test_route_boot_day2.c — end-to-end boot -> Day-2 route harness.
 *
 * PURPOSE
 * -------
 * Drive func_8001220C (the translated retail main loop) with the retail Disc 1
 * image and a deterministic scripted pad, then assert the address-exact
 * persist[]/token transitions documented in
 * docs/ai_context/DAY1_DAY2_TRANSITIONS.md and re-derived by
 * pc_port/tools/pe_day1_day2_transitions.py.
 *
 * This is a CONTROL-FLOW / STORY-STATE traversal proof.  It does NOT prove
 * audio fidelity (XA is not decoded), pixel/timing fidelity (no hardware
 * rasterizer), or that the pad sequence is the unique player solution.  A run
 * that advances part of the chain and then stops at a named, evidenced
 * frontier is the expected result.
 *
 * WHAT IS MEASURED
 * ----------------
 *   * persist[74]  (0x800A7918) — story/progress word, source of the exit
 *     selector at 0x80192030.
 *   * persist[1]   (0x800A77F4) — the selector's companion write.
 *   * D_8009D280   — the current field room token (packed ascii).
 *   * PE_Port_ShouldStop() / PE_Port_GetStopReason() — loud boundaries and
 *     iteration/frame limits are honored, never swallowed.
 *   * g_stub_order_log — the registered HOST_ADAPTED/UNSUPPORTED boundary
 *     stubs the run actually invoked.
 *
 * The walk records observed milestones. Every milestone that is reached
 * is pinned to the retail PC that writes it, so a regression that changes the
 * route is visible immediately.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "stub_registry.h"
#include "host_framebuffer.h"
#include "pe_disc.h"
#include "pe_guest_image.h"
#include "pe_route_pad.h"
#include "pe_memcard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

extern void func_8001220C(void);
/* D_800A7918: guest-memory macro from psx_compat.h */
extern void PE_Disc_SetActive(PE_Disc *disc);

/* The library's translated code calls Trace_Direct; the CLI host normally
 * provides it, so the harness supplies the same no-op sink. */
void Trace_Direct(const char *event) { (void)event; }

/* Story word storage (psx_compat.h exposes D_8009D280 as a guest lvalue). */
#define GA_PERSIST74 0x800A7918u
#define GA_PERSIST1  0x800A77F4u
#define GA_PERSIST24 0x800A7850u
#define GA_TOKEN     D_8009D280

#define MAX_TRACE 256
#define ROUTE_FRAME_LIMIT 62000

typedef struct RouteTrace {
    int            frames;
    uint32_t       story;
    uint32_t       persist1;
    uint32_t       flags24;
    uint32_t       token;
} RouteTrace;

static RouteTrace g_trace[MAX_TRACE];
static int        g_trace_count;
static int        g_frame;
static int        g_frame_limit;
static const char *g_ram_window;
static int g_ram_window_begin,g_ram_window_end,g_ram_window_failed;
static uint32_t   g_last_story = 0xDEADBEEFu;
static uint32_t   g_last_tok;
static uint32_t   g_last_flags24;
static int        g_first_story_frame = -1;
static int        g_milestone_hits;
static unsigned   g_sewer_enemy_peak[3], g_sewer_victories, g_supply_observations;

/* Deterministic four-stage pad source.
 *
 * Stage 1 (frames < SWITCH=5400): hold 0xFFEF + periodic Cross.  This is the
 * input that carries m0002i -> m0003i -> m0091i -> m0004i, matching the
 * documented first-play prefix (m0003i @801AA828 -> m0003i).
 * Stage 2 (5400 <= frames < SWITCH2=6040): hold 0xFFBF + periodic Cross, which
 * releases the m0004i walk-to-bench gate and transfers to m0378i at 0x801B69D0.
 * Stage 3 (6040 <= frames < SWITCH3): hold 0xFF9F (held bits 0x30 = 0x10|0x20) +
 * periodic Cross.  Holding stage 2 (0xFFBF, held 0x20) leaves Aya parked just
 * outside every rectangle in m0378i, so the room's module-4 gates keep
 * failing; the 0xFF9F hold walks Aya into m0378i module-4 rectangle #1
 * (x in (-1067, 533), z in (-560, -300)) at 0x801957A8, which sets actor
 * local[4]=1 and fires the m0378i -> m0377i room_transfer at 0x80195728
 * (token 0xA80673C8).
 * Stage 4 (frames >= SWITCH3=6989, the frame m0377i is entered): hold 0xFFAF
 * (0xFF9F with the 0x10 bit also pressed) + periodic Cross.  m0377i module 1's
 * op-77 rectangle at 0x801953C4 (x in (-211, 241), z in (-4607, -4114)) is an
 * interior trigger that the 0xFF9F hold never enters.  With 0xFFAF the op-77
 * test returns a hit, local[4]=1, and the guard at 0x80195410 falls through
 * instead of looping at 0x8019544C.  m0377i module 5 then writes
 * persist[1]=0x179 and transfers back to m0378i at 0x80195768.
 *
 * The probe sweep (PE_ROUTE_PAD4=0xFFBF/0xFFDF/0xFF9F/0xFF7F) leaves the m0377i
 * gate looping exactly as the old three-stage default did; only masks that add
 * the 0x10 bit (0xFFAF, 0xFF2F, 0xFF8F, 0xFFCF) make the gate report a hit.
 * 0xFFAF is the one that then carries the bounce furthest: m0378i module 0's
 * transfer at 0x80195384 returns the route to m0004i, whose module-4 op-77
 * volumes (0x801B6940/0x801B6A08/0x801B6B74) are the next input-gated stop.
 *
 * Bits are active-low Sony pad bits (0 = pressed). 0xFF9F = 0xFFBF & 0xFFDF,
 * 0xFFAF = 0xFF9F & 0xFFEF.
 *
 * The four masks / switch frames / pulse period are overridable from the
 * environment so the route can be probed without a rebuild; the defaults are
 * the documented first-play sequence and are what the assertions run with.
 *
 * The table itself lives in pc_port/include/pe_route_pad.h so the interactive
 * --route-pad autopilot in src/port_main.c drives the IDENTICAL sequence
 * (shared source of truth; no parallel copy). */
static int g_route_frames = 0;   /* 0 = use the ROUTE_FRAME_LIMIT default */
static PeRoutePadConfig g_pad;

#define PAD_CROSS  PE_ROUTE_PAD_CROSS
#define PAD_DOWN   PE_ROUTE_PAD_DOWN
#define PAD_UP     PE_ROUTE_PAD_UP
#define PAD_DN_LEFT PE_ROUTE_PAD_DN_LEFT
#define PAD_DN_LEFT_MASK2 PE_ROUTE_PAD_DN_LEFT_MASK2

/* Optional opcode-PC histogram (PE_ROUTE_PCTRACE=1).  It answers "which script
 * word is the frontier task parked on" without a rebuild or a debugger. */
#define PCTRACE_MAX 256
typedef struct PcSlot {
    pe_addr_t pc;
    uint32_t  op;
    uint32_t  count;
} PcSlot;
static PcSlot g_pc_slots[PCTRACE_MAX];
static int    g_pc_slot_count;
static int    g_pc_trace_on;
static int    g_pos_dump;
static int    g_pos_dump_every = 120;
static int    g_battle_dump_begin;
static int    g_pulse_end = 33620; /* Release Cross after the second-key pickup. */
static int    g_pulse_resume = 35300; /* Open the ammunition cabinet. */
static int    g_exact_pad_begin = 42713;
static int    g_exact_pad_end = 45041;
static int    g_sewer_pad_begin = 50500;
static int    g_sewer_pad_end = 51200;
static int    g_second_sewer_pad_begin = 52344;
static int    g_second_sewer_pad_end = 54500;
static int    g_supply_pad_begin = 54500;
static int    g_supply_pad_end = 62000;
#include "route_rehearsal_pads.h"
/* Optional input-only continuation: decimal-frame:hex-pad pairs. */
static struct { int frame; uint16_t mask; } g_pad_sequence[4096];
static unsigned g_pad_sequence_count;

static void PadConfigFromEnv(void)
{
    const char *s;

    PeRoutePad_ConfigFromEnv(&g_pad);
    const struct { const char *name; int *value; } pulse_options[] = {
        {"PE_ROUTE_PULSE_END", &g_pulse_end},
        {"PE_ROUTE_PULSE_RESUME", &g_pulse_resume},
        {"PE_ROUTE_EXACT_PAD_BEGIN", &g_exact_pad_begin},
        {"PE_ROUTE_EXACT_PAD_END", &g_exact_pad_end},
        {"PE_ROUTE_SEWER_PAD_BEGIN", &g_sewer_pad_begin},
        {"PE_ROUTE_SEWER_PAD_END", &g_sewer_pad_end},
        {"PE_ROUTE_SECOND_SEWER_PAD_BEGIN", &g_second_sewer_pad_begin},
        {"PE_ROUTE_SECOND_SEWER_PAD_END", &g_second_sewer_pad_end},
        {"PE_ROUTE_SUPPLY_PAD_BEGIN", &g_supply_pad_begin},
        {"PE_ROUTE_SUPPLY_PAD_END", &g_supply_pad_end}
    };
    for (unsigned i=0;i<sizeof(pulse_options)/sizeof(pulse_options[0]);i++) {
        s=getenv(pulse_options[i].name);
        if (s && s[0]) {
            char *end;
            unsigned long frame;
            errno=0;
            frame=strtoul(s,&end,10);
            if (errno || end==s || *end || frame>INT_MAX) {
                fprintf(stderr,"route: invalid %s (use a nonnegative frame)\n",pulse_options[i].name);
                exit(2);
            }
            *pulse_options[i].value=(int)frame;
        }
    }
    fprintf(stderr,"route: Cross pulse end=%d resume=%d\n",g_pulse_end,g_pulse_resume);
    s = getenv("PE_ROUTE_PAD_SEQUENCE");
    if (!s) s=kDay1RoutePads;
    while (s && s[0]) {
        char *end;
        unsigned long frame, mask;
        errno=0;
        frame=strtoul(s,&end,10);
        if (errno || end==s || *end!=':' || frame>INT_MAX
            || g_pad_sequence_count==sizeof(g_pad_sequence)/sizeof(g_pad_sequence[0])
            || (g_pad_sequence_count && frame<=(unsigned)g_pad_sequence[g_pad_sequence_count-1].frame))
            goto bad_sequence;
        s=end+1;
        errno=0;
        mask=strtoul(s,&end,16);
        if (errno || end==s || mask>0xFFFFu || (*end && *end!=',')) goto bad_sequence;
        g_pad_sequence[g_pad_sequence_count].frame=(int)frame;
        g_pad_sequence[g_pad_sequence_count++].mask=(uint16_t)mask;
        fprintf(stderr,"route: pad sequence frame=%lu mask=%04lX\n",frame,mask);
        s=*end?end+1:end;
        if (*end && !*s) goto bad_sequence;
    }
    s = getenv("PE_ROUTE_AYA_EVERY");
    if (s && s[0]) g_pos_dump_every = atoi(s);
    if (g_pos_dump_every < 1) g_pos_dump_every = 1;
    s = getenv("PE_ROUTE_FRAMES");
    if (s && s[0]) g_route_frames = atoi(s);
    g_pc_trace_on = getenv("PE_ROUTE_PCTRACE") != NULL;
    g_pos_dump = getenv("PE_ROUTE_AYA_DUMP") != NULL;
    s = getenv("PE_ROUTE_BATTLE_DUMP_BEGIN");
    g_battle_dump_begin = s ? atoi(s) : 0;
    return;
bad_sequence:
    fprintf(stderr,"route: invalid PE_ROUTE_PAD_SEQUENCE (use increasing frame:hex-pad pairs)\n");
    exit(2);
}

#include "route_reward_sewer_pilot.h"

static uint16_t RoutePadSource(void)
{
    uint16_t mask=PeRoutePad_Mask(&g_pad,g_frame);
    for (unsigned i=0;i<g_pad_sequence_count && g_frame>=g_pad_sequence[i].frame;i++)
        mask=g_pad_sequence[i].mask;
    if (!(g_frame>=g_exact_pad_begin && g_frame<g_exact_pad_end) &&
        !(g_frame>=g_sewer_pad_begin && g_frame<g_sewer_pad_end) &&
        !(g_frame>=g_second_sewer_pad_begin && g_frame<g_second_sewer_pad_end) &&
        !(g_frame>=g_supply_pad_begin && g_frame<g_supply_pad_end) &&
        (g_frame<g_pulse_end || g_frame>=g_pulse_resume) && (g_frame%g_pad.period)==3) mask&=g_pad.pulse;
    if (getenv("PE_ROUTE_REWARD_PILOT")) mask=RouteRewardSewerPilot(mask);
    return mask;
}

static void RecordTrace(void)
{
    if (g_trace_count >= MAX_TRACE)
        return;
    g_trace[g_trace_count].frames   = g_frame;
    g_trace[g_trace_count].story    = PE_LoadU32(GA_PERSIST74);
    g_trace[g_trace_count].persist1 = PE_LoadU32(GA_PERSIST1);
    g_trace[g_trace_count].flags24  = PE_LoadU32(GA_PERSIST24);
    g_trace[g_trace_count].token    = GA_TOKEN;
    g_trace_count++;
}

/* Optional opcode-PC histogram (PE_ROUTE_PCTRACE=1).  It answers "which script
 * word is the frontier task parked on" without a rebuild or a debugger. */
static void PcTraceSample(void)
{
    pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
    int guard = 0;

    if (!g_pc_trace_on)
        return;
    while (actor != 0u && guard < 64) {
        pe_addr_t task = PE_RangeIsRam(actor + 0xA8u, 4u)
            ? PE_LoadU32(actor + 0xA8u) : 0u;
        pe_addr_t pc;
        uint32_t word;
        int i;

        if (PE_RangeIsRam(task, 4u)) {
            pc = PE_LoadU32(task);
            if (PE_RangeIsRam(pc, 4u)) {
                word = PE_LoadU32(pc);
                for (i = 0; i < g_pc_slot_count; i++) {
                    if (g_pc_slots[i].pc == pc) {
                        g_pc_slots[i].count++;
                        goto next;
                    }
                }
                if (g_pc_slot_count < PCTRACE_MAX) {
                    g_pc_slots[g_pc_slot_count].pc  = pc;
                    g_pc_slots[g_pc_slot_count].op  = word & 0x1FFFu;
                    g_pc_slots[g_pc_slot_count].count = 1u;
                    g_pc_slot_count++;
                }
            }
        }
    next:
        actor = PE_LoadU32(actor + 4u);
        guard++;
    }
}

/* Read-only battle diagnostics. Set PE_ROUTE_BATTLE_DUMP_BEGIN to the first
 * frame of interest; HP reaching zero distinguishes defeat from progression. */
static void BattleTraceSample(void)
{
    static uint32_t last_mode = UINT32_MAX, last_hp = UINT32_MAX;
    static uint32_t last_enemy_hp = UINT32_MAX, last_queue = UINT32_MAX;
    pe_addr_t aya, record, actor, enemy = 0;
    uint32_t mode, queue, hp, enemy_hp;
    unsigned guard = 0;
    if (!g_battle_dump_begin || g_frame < g_battle_dump_begin ||
        !(PE_LoadU32(0x8009D1A0u) & 2u))
        return;
    aya = PE_LoadU32(0x8009D254u);
    record = PE_LoadU32(0x8009D278u);
    for (guard = 0; guard < 45u; guard++) {
        pe_addr_t body;
        actor = PE_LoadU32(0x8009E000u + guard * 12u);
        if (!actor) break;
        if (actor == aya || !PE_RangeIsRam(actor, 640u)) continue;
        body = PE_LoadU32(actor);
        if (!PE_RangeIsRam(body, 24u) || (int32_t)PE_LoadU32(body + 16u) <= 0) continue;
        if (!enemy) enemy = actor;
        if ((GA_TOKEN == 0xA8003148u || GA_TOKEN == 0xA8003248u) && g_frame % 60 == 0)
            fprintf(stderr, "route: %s enemy f=%d actor=%08X type=%u hp=%u pos=%d,%d,%d flags=%08X\n",
                    GA_TOKEN==0xA8003248u?"m34":"m32",g_frame, actor, PE_LoadU8(actor + 12u), PE_LoadU32(body + 16u),
                    (int32_t)PE_LoadU32(actor + 40u) >> 16,
                    (int32_t)PE_LoadU32(actor + 44u) >> 16,
                    (int32_t)PE_LoadU32(actor + 48u) >> 16, PE_LoadU32(actor + 152u));
    }
    mode = PE_LoadU32(0x8009D28Cu);
    queue = PE_LoadU8(0x8009CE3Cu);
    hp = record ? PE_LoadU16(record + 12u) : 0u;
    enemy_hp = enemy && PE_LoadU32(enemy) ?
        PE_LoadU32(PE_LoadU32(enemy) + 16u) : 0u;
    if (mode != last_mode || queue != last_queue || hp != last_hp ||
        enemy_hp != last_enemy_hp || g_frame % 120 == 0) {
        pe_addr_t task = enemy ? PE_LoadU32(enemy + 0xA8u) : 0u;
        pe_addr_t weapon = record ? PE_LoadU32(record + 104u) : 0u;
        fprintf(stderr, "route: battle f=%d token=%08X mode=%u queue=%u hp=%u enemyhp=%u AT=%u pos=%d,%d enemy=%d,%d flags=%X enemy_pc=%08X PE=%u loaded=%u bullet_reserve=%u\n",
                g_frame, (unsigned)GA_TOKEN, (unsigned)mode, (unsigned)queue,
                (unsigned)hp, (unsigned)enemy_hp,
                record ? (unsigned)PE_LoadU16(record + 16u) : 0u,
                aya ? (int32_t)PE_LoadU32(aya + 40u) >> 16 : 0,
                aya ? (int32_t)PE_LoadU32(aya + 48u) >> 16 : 0,
                enemy ? (int32_t)PE_LoadU32(enemy + 40u) >> 16 : 0,
                enemy ? (int32_t)PE_LoadU32(enemy + 48u) >> 16 : 0,
                (unsigned)PE_LoadU32(0x8009D1A0u),
                task ? (unsigned)PE_LoadU32(task) : 0u,
                record ? (unsigned)(PE_LoadU32(record + 8u) >> 16) : 0u,
                weapon ? (unsigned)(PE_LoadU32(weapon + 12u) & 1023u) : 0u,
                (unsigned)PE_LoadU16(0x800A1E6Eu));
        last_mode = mode; last_queue = queue;
        last_hp = hp; last_enemy_hp = enemy_hp;
    }
}

/* Per-present hook: count frames, capture every token is not needed — a
 * token/story change is enough to reconstruct the path. */
static void RecordSewerVictory(void)
{
    unsigned room;
    if (GA_TOKEN==0xA80023C8u) room=0;
    else if (GA_TOKEN==0xA8002448u) room=1;
    else if (GA_TOKEN==0xA8003148u) room=2;
    else return;
    unsigned enemies=0;
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu);
    for (unsigned i=0;actor && i<64u;i++,actor=PE_LoadU32(actor+4u)) {
        unsigned type=PE_LoadU8(actor+12u);
        if ((room==0?type==3u:room==1?(type==7u || type==8u):type==6u) && PE_LoadU32(actor)) enemies++;
    }
    uint32_t flags=PE_LoadU32(0x8009D1A0u);
    if ((flags&2u) && enemies>g_sewer_enemy_peak[room]) g_sewer_enemy_peak[room]=enemies;
    pe_addr_t aya=PE_LoadU32(0x8009D254u),record=aya?PE_LoadU32(aya):0u;
    if (g_sewer_enemy_peak[room]==(room==2?2u:3u) && !enemies && !(flags&6u) &&
        PE_LoadU32(0x8009D28Cu)==9u && record && PE_LoadU16(record+12u)>0u &&
        !(g_sewer_victories&(1u<<room))) {
        g_sewer_victories|=1u<<room;
        fprintf(stderr,"route: sewer victory room=%u frame=%d HP=%u\n",room+1u,g_frame,PE_LoadU16(record+12u));
    }
}

/* Observe awards and menu results; never supply gameplay state. */
static void RecordSupplies(void)
{
    unsigned before=g_supply_observations;
    if(GA_TOKEN==0xA8063248u) {
        if((PE_LoadU32(0x800A7940u)&0x80u) && PE_LoadU16(0x800A1E6Eu)>=15u)g_supply_observations|=1u;
        if(PE_LoadU32(0x800A789Cu)&4u)g_supply_observations|=2u;
        if((PE_LoadU32(0x800A7940u)&0x200u) && PE_LoadU16(0x800C0E50u)==7u)g_supply_observations|=4u;
    }
    pe_addr_t aya=PE_LoadU32(0x8009D254u),record=aya?PE_LoadU32(aya):0u;
    /* Milestone 55 — item7 consumed through the field Items/Use menu restores
     * Aya to full HP at token m0031i.  The old pin compared against the
     * absolute 45, which was Aya's max HP in the recorded-pad run that authored
     * this row.  The route now wins both sewer battles, so it reaches m0031i at
     * max HP 53 (func_80023E14 heals item7 by 90 and then clamps HP to
     * record+28), and the absolute pin could never fire.  The route-independent
     * invariant is "HP is full", which is strictly stronger.  Traced live:
     * LOOT_PILOT_HEAL_DONE 58044 hp=53 item4=0 (record+28==53). */
    if(GA_TOKEN==0xA80030C8u && (g_supply_observations&7u)==7u && record &&
        PE_LoadU16(record+12u)==PE_LoadU16(record+28u) &&
        !PE_LoadU16(0x800C0E50u)) {
        g_supply_observations|=8u;
    }
    /* Milestone 56 — the battle equipment command 407 selected carried slot 0.
     * The old observation sampled the gun slot only on the item-use frame, but
     * on every recorded route the field item use (m0031i) precedes the equip
     * (m0032i battle): the pilot runs Items/Use at 58044 with gun=2 and queues
     * command 407 slot0 at 58917.  Observe the command itself, after the item7
     * pickup (bit 4), so the earlier battle equip command 407 slot2 (51701)
     * does not count. */
    if((g_supply_observations&4u) && PE_LoadU16(0x800BE834u)==407u &&
        PE_LoadU8(0x800C0E20u)==0u)
        g_supply_observations|=16u;
    if(before!=g_supply_observations)fprintf(stderr,"route: supplies frame=%d observations=%02X\n",g_frame,g_supply_observations);
}

/* Optional item-use trace (PE_ROUTE_ITEM_TRACE=1).  Records every change of
 * room token / Aya HP / item slot / equipped gun slot so the Items/Use consume
 * window (milestone 55) can be read frame by frame.  Read-only. */
static void ItemTrace(void)
{
    static int init = 0;
    static uint32_t last_tok = 0xFFFFFFFFu, last_hp = 0xFFFFu;
    static uint32_t last_item = 0xFFFFu, last_gun = 0xFFFFu;
    pe_addr_t aya = PE_LoadU32(0x8009D254u);
    pe_addr_t record = aya ? PE_LoadU32(aya) : 0u;
    uint32_t tok = GA_TOKEN;
    uint32_t hp = record ? PE_LoadU16(record + 12u) : 0xFFFFu;
    uint32_t item = PE_LoadU16(0x800C0E50u);
    uint32_t gun = PE_LoadU8(0x800C0E20u);

    if (init && tok == last_tok && hp == last_hp && item == last_item && gun == last_gun)
        return;
    init = 1;
    fprintf(stderr,
        "ITEM_TRACE %d token=%08X hp=%u item=%u gun=%u obs=%02X mode=%u focus=%08X\n",
        g_frame, tok, hp, item, gun, g_supply_observations,
        PE_LoadU32(0x8009D28Cu), (unsigned)PE_LoadU32(0x8009D15Cu));
    last_tok = tok; last_hp = hp; last_item = item; last_gun = gun;
}

static void RouteHook(void)
{
    uint32_t story;
    uint32_t tok;

    g_frame++;
    if (g_frame > g_frame_limit)
        return;
    if (getenv("PE_ROUTE_FRAME_TRACE") && (g_frame % 10) == 0)
        fprintf(stderr,"route: frame %d\n",g_frame);

    /* Optional read-only research window at the present hook. These files
     * capture this exact phase, before the remaining frame-tail instructions;
     * they are never restored into the connected runtime. */
    if(g_ram_window && g_frame>=g_ram_window_begin && g_frame<=g_ram_window_end) {
        char path[PATH_MAX];FILE *out=NULL;
        int length=snprintf(path,sizeof(path),"%s/frame-%06d.bin",g_ram_window,g_frame);
        if(length>=0 && (size_t)length<sizeof(path))out=fopen(path,"wb");
        size_t written=out?fwrite(PE_TranslateConst(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,out):0;
        int closed=out?fclose(out):-1;
        if(written!=PE_RAM_SIZE || closed) {
            fprintf(stderr,"route: RAM window write failed at frame %d\n",g_frame);
            g_ram_window_failed=1;PE_Port_RequestStop(PE_PORT_STOP_EXPLICIT);return;
        }
    }

    BattleTraceSample();
    RecordSewerVictory();
    RecordSupplies();
    {
        static int item_trace = -1;
        if (item_trace < 0) item_trace = getenv("PE_ROUTE_ITEM_TRACE") ? 1 : 0;
        if (item_trace) ItemTrace();
    }

    if (g_first_story_frame < 0)
        g_first_story_frame = g_frame;

    story = PE_LoadU32(GA_PERSIST74);
    tok   = GA_TOKEN;
    if (story != g_last_story || tok != g_last_tok ||
        PE_LoadU32(GA_PERSIST24) != g_last_flags24) {
        g_last_story = story;
        g_last_tok   = tok;
        g_last_flags24 = PE_LoadU32(GA_PERSIST24);
        RecordTrace();
    }
    if (g_pos_dump && (tok == 0xA8067448u || tok == 0xA80673C8u ||
                       tok == 0xA8000248u || tok == 0xA8002048u ||
                       tok == 0xA8001148u || tok == 0xA8001448u) &&
        (g_frame % g_pos_dump_every) == 0) {
        pe_addr_t aya = PE_LoadU32(0x8009D254u);
        pe_addr_t res = PE_LoadU32(0x8009D2F0u);
        if (aya)
            printf("route: ayadump f=%d x=%08X y=%08X z=%08X held=%08X edge=%08X res=%08X",
                   g_frame, (unsigned)PE_LoadU32(aya + 0x28u),
                   (unsigned)PE_LoadU32(aya + 0x2Cu),
                   (unsigned)PE_LoadU32(aya + 0x30u),
                   (unsigned)PE_LoadU32(0x8009D26Cu),
                   (unsigned)PE_LoadU32(0x8009D1F4u), (unsigned)res);
        if (aya)
            printf(" ayafl=%08X cb=%08X code=%02X d2e8=%08X d1a0=%08X",
                   (unsigned)PE_LoadU32(aya + 0x98u),
                   (unsigned)PE_LoadU32(aya + 0x190u),
                   (unsigned)PE_LoadU8(aya + 0x0Eu),
                   (unsigned)PE_LoadU32(0x8009D2E8u),
                   (unsigned)PE_LoadU32(0x8009D1A0u));
        if (res != 0u)
            printf(" resloc=%08X %08X %08X resfl=%08X",
                   (unsigned)PE_LoadU32(res + 0xACu + 8u),
                   (unsigned)PE_LoadU32(res + 0xACu + 12u),
                   (unsigned)PE_LoadU32(res + 0xACu),
                   (unsigned)PE_LoadU32(res + 0x98u));
        printf("\n");
        if (tok == 0xA8067448u) {
            pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
            int n2 = 0;
            while (actor != 0u && n2 < 64) {
                pe_addr_t task = PE_LoadU32(actor + 0xA8u);
                pe_addr_t pc = task ? PE_LoadU32(task) : 0u;
                if (pc != 0u)
                    printf("    actor=%08X type=%u pc=%08X task+10=%u L=%08X %08X %08X %08X %08X\n",
                           (unsigned)actor, (unsigned)PE_LoadU8(actor + 0xCu),
                           (unsigned)pc,
                           (unsigned)PE_LoadU32(task + 0x10u),
                           (unsigned)PE_LoadU32(actor + 0xACu),
                           (unsigned)PE_LoadU32(actor + 0xB0u),
                           (unsigned)PE_LoadU32(actor + 0xB4u),
                           (unsigned)PE_LoadU32(actor + 0xB8u),
                           (unsigned)PE_LoadU32(actor + 0xBCu));
                actor = PE_LoadU32(actor + 4u);
                n2++;
            }
        }
    }
    if ((g_frame % 30) == 0)
        PcTraceSample();
}

static PE_Disc *OpenRouteDisc(char *err, size_t err_size)
{
    const char *env = getenv("PE_DISC1_BIN");
    char path[1024];
    FILE *fp;
    size_t n;
    PE_Disc *disc;

    if (env && env[0])
        return PE_Disc_Open(env, err, err_size);

    fp = fopen("local/pe_disc1.path", "r");
    if (fp) {
        if (!fgets(path, (int)sizeof(path), fp)) {
            fclose(fp);
            if (err && err_size)
                snprintf(err, err_size, "empty local/pe_disc1.path");
            return NULL;
        }
        fclose(fp);
        n = strlen(path);
        while (n > 0 && (path[n - 1u] == '\n' || path[n - 1u] == '\r'))
            path[--n] = 0;
        disc = PE_Disc_Open(path, err, err_size);
        if (disc)
            return disc;
    }

    /* Same fallback as the other retail-disc fixtures: a locally extracted
     * Disc-1 executable can stand in when the image path is unavailable. */
    disc = PE_Disc_Open("build/extracted/disc1/SLUS_006.62", err, err_size);
    return disc;
}

static int TraceSawToken(uint32_t token)
{
    int i;
    for (i = 0; i < g_trace_count; i++)
        if (g_trace[i].token == token)
            return g_trace[i].frames;
    return -1;
}

static int TraceSawStory(uint32_t story)
{
    int i;
    for (i = 0; i < g_trace_count; i++)
        if (g_trace[i].story == story)
            return g_trace[i].frames;
    return -1;
}

/*
 * Run the boot spine once and return the stop reason.  The caller owns the
 * trace buffers; this only guarantees deterministic cleanup.
 */
static PEPortStopReason RunRoute(int frame_limit)
{
    PE_Disc *disc;
    char err[256];
    PE_RamInit();
    PE_Callback_Init();
    Bootstrap_Init();
    PE_Port_RunControlReset();
    HostFB_Init();
    PadConfigFromEnv();
    if (g_route_frames > 0)
        frame_limit = g_route_frames;

    g_frame = 0;
    g_frame_limit = frame_limit;
    g_ram_window=getenv("PE_ROUTE_RAM_WINDOW");
    g_ram_window_begin=getenv("PE_ROUTE_RAM_WINDOW_BEGIN")?atoi(getenv("PE_ROUTE_RAM_WINDOW_BEGIN")):0;
    g_ram_window_end=getenv("PE_ROUTE_RAM_WINDOW_END")?atoi(getenv("PE_ROUTE_RAM_WINDOW_END")):g_ram_window_begin;
    g_ram_window_failed=0;
    if(!g_ram_window || !g_ram_window[0] || g_ram_window_begin<=0 || g_ram_window_end<g_ram_window_begin)g_ram_window=NULL;
    g_trace_count = 0;
    g_last_story = 0xDEADBEEFu;
    g_last_tok = 0u;
    g_last_flags24 = 0xDEADBEEFu;
    g_first_story_frame = -1;
    g_milestone_hits = 0;
    g_sewer_enemy_peak[0]=g_sewer_enemy_peak[1]=g_sewer_enemy_peak[2]=g_sewer_victories=0u;
    err[0] = 0;
    disc = OpenRouteDisc(err, sizeof(err));
    if (!disc) {
        fprintf(stderr, "route: disc open failed: %s\n", err);
        return PE_PORT_STOP_NONE;
    }
    PE_Disc_SetActive(disc);
    {
        int kind = PE_Disc_BootKind(disc);
        char vol[40];
        if (!PE_Disc_VolumeId(disc, vol, sizeof(vol)))
            vol[0] = 0;
        fprintf(stderr, "route: disc boot=%d vol='%s'\n", kind, vol);
        if (kind == 2) {
            fprintf(stderr, "route: Disc 2 image is not the disc-1 gameplay baseline\n");
            PE_Disc_SetActive(NULL);
            return PE_PORT_STOP_NONE;
        }
    }
    if (PE_GuestImage_LoadExe(disc, err, sizeof(err))) {
        fprintf(stderr, "route: EXE load failed: %s\n", err);
        PE_Disc_SetActive(NULL);
        return PE_PORT_STOP_NONE;
    }
    if (PE_Globals_AdoptRetailImage()) {
        fprintf(stderr, "route: globals adopt failed\n");
        PE_Disc_SetActive(NULL);
        return PE_PORT_STOP_NONE;
    }

    PE_Port_SetSkipMovie(1);         /* documented HOST_ADAPTED boundary */
    PE_Port_SetSkipOpeningMenu(1);   /* documented HOST_ADAPTED boundary */
    PE_Port_SetFrameLimit(frame_limit);
    PE_Port_SetPadSource(RoutePadSource);
    PE_Port_SetPresentHook(RouteHook);

    func_8001220C();

    PE_Disc_SetActive(NULL);
    return PE_Port_GetStopReason();
}

/*
 * Milestone table.  Each entry is a retail-proven transition:
 *   kind 0 = persist[74] value, kind 1 = room token, kind 2 = persist[1],
 *   kind 3 = persist[24] contains every bit in value,
 *   kind 5 = observed sewer victories, kind 6 = observed supply/menu results.
 */
typedef struct Milestone {
    int         kind;
    uint32_t    value;
    const char *what;
    const char *evidence;
} Milestone;

static const Milestone kMilestones[] = {
    { 0, 0x00000001u, "persist[74]=0x01 (m0010i profile/name entry)",
      "m0010i module EF path" },
    { 0, 0x00000008u, "persist[74]=0x08",
      "first field-prefix story step" },
    { 1, 0xA8001048u, "token m0010i (0xA8001048)",
      "field prefix first room" },
    { 1, 0xA8000148u, "token m0002i (0xA8000148)",
      "m0010i -> m0002i" },
    { 0, 0x00000009u, "persist[74]=0x09 (m0002i)",
      "m0002i story write" },
    { 1, 0xA80001C8u, "token m0003i (0xA80001C8)",
      "m0002i @801AA828 room_transfer A80001C8" },
    { 1, 0xA8067148u, "token m0372i (0xA8067148)",
      "m0003i -> m0372i (FMV003 prefix)" },
    { 0, 0x00000011u, "persist[74]=0x11",
      "m0372i story write" },
    { 0, 0x00000012u, "persist[74]=0x12",
      "m0372i -> m0004i story write" },
    { 1, 0xA8000248u, "token m0004i (0xA8000248)",
      "m0372i @801D7844 room_transfer A8000248" },
    { 0, 0x00000018u, "persist[74]=0x18 (m0004i)",
      "m0004i @801B5FD4 assign [74,0x18]" },
    { 1, 0xA8067448u, "token m0378i (0xA8067448)",
      "m0004i @801B69D0 room_transfer A8067448" },
    { 1, 0xA80673C8u, "token m0377i (0xA80673C8)",
      "m0378i @80195728 room_transfer A80673C8 (mod4 rect #1 "
      "(-1067,-560)-(-300,-300) at 0x801957A8)" },
    { 2, 0x00000179u, "persist[1]=0x179 (m0377i module 5)",
      "m0377i @80195758 assign [1,0x179]; module 5 @80195768 "
      "room_transfer A8067448 (mod1 op77 rect (-211,-4607)-(241,-4114) "
      "at 0x801953C4 hit under PAD4=0xFFAF)" },
    { 1, 0xA80002C8u, "token m0005i (0xA80002C8)",
      "m0004i left-door op77 at 801B6B74 sends mailbox payload 4; "
      "module 0 @801B614C transfers to m0005i" },
    { 0, 0x00000028u, "persist[74]=0x28 (m0005i encounter)",
      "m0005i module 4 central rectangle at 801B1C14 starts the encounter" },
    { 0, 0x00000026u, "persist[74]=0x26 (battle exit)",
      "m0005i encounter exit leads into m0367i" },
    { 1, 0xA80663C8u, "token m0367i (0xA80663C8)",
      "m0005i battle exit cutscene" },
    { 0, 0x00000027u, "persist[74]=0x27 (return from m0367i)",
      "m0367i returns to m0005i after the encounter" },
    { 1, 0xA80004C8u, "token m0009i (0xA80004C8)",
      "m0005i right doorway after battle completion" },
    { 0, 0x00000030u, "persist[74]=0x30 (m0009i exit choice)",
      "m0009i hole interaction advances story before jumping to m0011i" },
    { 1, 0xA80010C8u, "token m0011i (0xA80010C8)",
      "m0009i module 0 at 80192B38 transfers to m0011i" },
    { 0, 0x00000038u, "persist[74]=0x38 (m0011i)",
      "m0011i initial event completes after arrival" },
    { 1, 0xA8001148u, "token m0012i (0xA8001148)",
      "m0011i module 3 at 801A0890 transfers to m0012i" },
    { 0, 0x00000039u, "persist[74]=0x39 (m0012i)",
      "m0012i arrival starts its initial event" },
    { 0, 0x00000040u, "persist[74]=0x40 (m0013i event)",
      "m0012i initial event transfers to m0013i" },
    { 1, 0xA80011C8u, "token m0013i (0xA80011C8)",
      "m0012i initial event visits m0013i" },
    { 0, 0x00000048u, "persist[74]=0x48 (return to m0012i)",
      "m0012i event completes after returning from m0013i" },
    { 1, 0xA8002048u, "token m0020i (0xA8002048)",
      "m0012i module 0 at 801A28B8 enters the open dressing-room door" },
    { 3, 0x200u, "persist[24] bit 0x200 (first NPC contact)",
      "m0020i module 2 at 801A1770 completes the first contact dialogue" },
    { 3, 0x20u, "persist[24] bit 0x20 (first key)",
      "m0020i Aya payload 3 at 801A0C00 records the C8 award" },
    { 1, 0xA8001448u, "token m0018i (0xA8001448)",
      "m0012i's side door accepts the first-key flag" },
    { 0, 0x54u, "persist[74]=0x54 (diary)",
      "m0018i module 0 at 8019D834 advances story during the diary reading" },
    { 3, 0x80000u, "persist[24] bit 0x80000 (second key)",
      "m0018i module 0 at 8019D90C records the C9 award" },
    { 3, 0x80000000u, "persist[24] bit 0x80000000 (cabinet ammunition)",
      "m0018i module 0 at 8019DD20 records the item1 award" },
    { 1, 0xA80614C8u, "token m0319i (rehearsal room)",
      "m0012i's rehearsal door accepts the C9 pickup flag" },
    { 0, 0x5Bu, "persist[74]=0x5B (rehearsal battle)",
      "m0319i module6 encounter trigger enters m0023i" },
    { 1, 0xA80021C8u, "token m0023i (rehearsal battle)",
      "original room transfer initializes Eve and the required encounter" },
    { 0, 0x5Eu, "persist[74]=0x5E (rehearsal victory)",
      "Eve's HP threshold triggers m0023i's exit and m0367i scene" },
    { 0, 0x5Fu, "persist[74]=0x5F (return from victory scene)",
      "m0367i returns to the rehearsal room after the encounter" },
    { 0, 0x60u, "persist[74]=0x60 (rehearsal control restored)",
      "m0319i completes the post-victory event" },
    { 1, 0xA8002348u, "token m0026i (sewer entrance)",
      "m0319i module7's doorway enters the sewer" },
    { 0, 0x68u, "persist[74]=0x68 (sewers)",
      "m0026i arrival completes the sewer entrance event" },
    { 1, 0xA80023C8u, "token m0027i (first sewer hallway)",
      "m0026i exit rectangle transfers to the first enemy hallway" },
    { 2, 0x1Au, "persist[1]=0x1A (m0027i entry)",
      "m0026i's original transfer records the arrival direction" },
    { 5, 1u, "first sewer victory with three enemies retired",
      "three spawned bodies, no remaining enemy bodies, living Aya, battle mode9 and field control" },
    { 1, 0xA8002448u, "token m0028i (second sewer hallway)",
      "m0027i's original forward transfer" },
    { 2, 0x1Bu, "persist[1]=0x1B (m0028i entry)",
      "m0027i's original transfer records the arrival direction" },
    { 5, 2u, "second sewer victory with three enemies retired",
      "three spawned bodies, no remaining enemy bodies, living Aya, battle mode9 and field control" },
    { 1, 0xA80030C8u, "token m0031i (sewer junction)", "m0028i original forward transfer" },
    { 1, 0xA8063248u, "token m0334i (supply room)", "m0031i original upper exit at 8019EB40" },
    { 6, 1u, "first chest awards 15 reserve rounds", "m0334i A7/E8 award at 80192AC8/80192B00" },
    { 6, 2u, "supply switch activated", "m0334i module2 at 80192384 sets persist[2B] bit4" },
    { 6, 4u, "second chest awards item7", "m0334i module7 fixed item and A7/E8 award" },
    { 2, 0x14Eu, "return to sewer junction", "m0334i transfer8019249C writes arrival14E" },
    { 6, 8u, "normal item use restores 45 HP", "item7 consumed from slot4 through Items/Use" },
    { 6, 16u, "normal equipment menu selects pistol", "equipment command407 selects carried slot0" },
};

#define MILESTONE_COUNT ((int)(sizeof(kMilestones) / sizeof(kMilestones[0])))

static int MilestoneSatisfied(const Milestone *m)
{
    switch (m->kind) {
    case 0: return TraceSawStory(m->value) >= 0;
    case 1: return TraceSawToken(m->value) >= 0;
    case 2: {
        int i;
        for (i = 0; i < g_trace_count; i++)
            if (g_trace[i].persist1 == m->value)
                return 1;
        return 0;
    }
    case 3:
        for (int i=0;i<g_trace_count;i++)
            if ((g_trace[i].flags24&m->value)==m->value) return 1;
        return 0;
    case 5: return (g_sewer_victories&m->value)==m->value;
    case 6: return (g_supply_observations&m->value)==m->value;
    default: return 0;
    }
}

static int BoundaryStubCount(const char *symbol)
{
    int n = 0;
    int i;
    for (i = 0; i < g_stub_order_count; i++)
        if (strcmp(g_stub_order_log[i], symbol) == 0)
            n++;
    return n;
}

/* Fixed input intervals preserve the recorded menu and battle button edges:
 * [42713,45041) rehearsal; [50500,51200) first sewer battle;
 * [52344,54500) second sewer battle; [54500,62000) supplies and menus.
 * Other frames retain periodic Cross. Each sewer victory requires three
 * observed enemy bodies, retirement, living Aya, mode9 and field control.
 *
 * The current endpoint is m0031i after visiting m0334i: 15 reserve rounds,
 * switch activation, item7 pickup/use, and normal pistol selection. The next
 * required m0032i fight is beyond this passing regression. Full Day2 and
 * whole-route timing/audio/rendering fidelity remain unproved.
 * See docs/ai_context/DAY1_SEWER_MOVEMENT.md for original PCs and evidence.
 */
#define FRONTIER_TOKEN 0xA80030C8u
#define FRONTIER_PC    0x8019E8B8u

/* The retail carried-item table has at most 50 halfword slots. Read-only. */
static int InventoryContains(uint16_t item)
{
    for (unsigned i=0;i<50u;i++)
        if (PE_LoadU16(0x800C0E48u+i*2u)==item) return 1;
    return 0;
}

/* Retail removes retired enemies from the active list and later resets slots.
 * Require the observed battles and absence of the second room's enemy types. */
static int SewerEnemiesDefeated(void)
{
    /* Other rooms reuse actor types7/8 for chests; prior victory observations
     * already require every enemy to be retired before leaving each room. */
    if(GA_TOKEN!=0xA8002448u)return (g_sewer_victories&3u)==3u;
    pe_addr_t actor=PE_LoadU32(0x8009D20Cu);
    for (unsigned i=0;actor && i<64;i++,actor=PE_LoadU32(actor+4u)) {
        unsigned type=PE_LoadU8(actor+0xCu);
        if (type==7u || type==8u) return 0;
    }
    return (g_sewer_victories&3u)==3u;
}

/* Return the frontier actor's task PC if the list is parked at FRONTIER_PC,
 * else 0.  Used to pin the stop, not to influence it. */
static pe_addr_t FindFrontierPc(void)
{
    pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
    int n = 0;
    while (actor != 0u && n < 64) {
        pe_addr_t task = PE_RangeIsRam(actor + 0xA8u, 4u)
            ? PE_LoadU32(actor + 0xA8u) : 0u;
        pe_addr_t pc = PE_RangeIsRam(task, 4u) ? PE_LoadU32(task) : 0u;
        if (pc == FRONTIER_PC)
            return pc;
        actor = PE_LoadU32(actor + 4u);
        n++;
    }
    return 0u;
}

/*
 * Frontier report: walk the live actor list and print each actor's script PC
 * and its first 8 script-local words (actor+0xAC + i*4, the VM's mode-1
 * operand base).  This is what names the blocker when the route stops short
 * of a milestone — the room token alone does not say which task is parked,
 * and the locals show the op77 hit-test inputs/result.
 */
static void PrintFrontier(void)
{
    pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
    int n = 0;

    printf("route: frontier actors (D_8009D20C list):\n");
    while (actor != 0u && n < 64) {
        pe_addr_t task = PE_RangeIsRam(actor + 0xA8u, 4u)
            ? PE_LoadU32(actor + 0xA8u) : 0u;
        pe_addr_t pc = PE_RangeIsRam(task, 4u) ? PE_LoadU32(task) : 0u;
        uint32_t word = PE_RangeIsRam(pc, 4u) ? PE_LoadU32(pc) : 0u;
        printf("  actor=%08X type=%u id=%u task=%08X pc=%08X op=%02X delay=%u\n",
               (unsigned)actor, PE_LoadU8(actor + 0xCu), PE_LoadU8(actor + 0xDu),
               (unsigned)task, (unsigned)pc, (unsigned)(word & 0x1FFFu),
               PE_RangeIsRam(task + 0x10u, 4u)
                   ? (unsigned)PE_LoadU32(task + 0x10u) : 0u);
        if (PE_RangeIsRam(actor + 0xACu, 32u)) {
            printf("    locals:");
            {
                int i;
                for (i = 0; i < 8; i++)
                    printf(" [%d]=%08X", i, (unsigned)PE_LoadU32(actor + 0xACu + (pe_addr_t)i * 4u));
            }
            printf("\n");
        }
        actor = PE_LoadU32(actor + 4u);
        n++;
    }
}

int main(void)
{
    PEPortStopReason reason;
    int reached = 0;
    int i;

    if (access("local/pe_disc1.path", R_OK) != 0 &&
        !(getenv("PE_DISC1_BIN") && getenv("PE_DISC1_BIN")[0])) {
        printf("SKIP (requires PE_DISC1_BIN or local/pe_disc1.path)\n");
        return 0;
    }

    /* Never touch the user's memory-card image: in-memory card. */
    PE_Memcard_SetPath(0, "");
    PE_Memcard_SetPath(1, NULL);
    reason = RunRoute(ROUTE_FRAME_LIMIT);
    if(g_ram_window_failed)return 1;
    /* Optional read-only checkpoint for original/native differential probes.
     * Captures are local research inputs, never a way to advance the route. */
    {
        const char *path = getenv("PE_ROUTE_RAM_DUMP");
        if (path && path[0]) {
            FILE *out = fopen(path, "wb");
            if (!out) {
                perror("route: RAM checkpoint");
                return 1;
            }
            size_t written = fwrite(PE_TranslateConst(PE_RAM_BASE, PE_RAM_SIZE),
                                    1, PE_RAM_SIZE, out);
            int closed = fclose(out);
            if (written != PE_RAM_SIZE || closed != 0) {
                fprintf(stderr, "route: RAM checkpoint write failed\n");
                return 1;
            }
        }
    }
    printf("route: frames=%d stop=%s story=0x%08X persist1=0x%08X token=0x%08X\n",
           g_frame, PE_Port_StopReasonName(reason),
           PE_LoadU32(GA_PERSIST74), PE_LoadU32(GA_PERSIST1), GA_TOKEN);

    for (i = 0; i < g_trace_count; i++) {
        const RouteTrace *t = &g_trace[i];
        int j;
        int marks = 0;
        for (j = 0; j < MILESTONE_COUNT; j++) {
            const Milestone *m = &kMilestones[j];
            if ((m->kind == 0 && m->value == t->story) ||
                (m->kind == 1 && m->value == t->token) ||
                (m->kind == 2 && m->value == t->persist1) ||
                (m->kind == 3 && (t->flags24&m->value)==m->value))
                marks = 1;
        }
        printf("  [%c f=%d] story=0x%08X persist1=0x%08X flags24=0x%08X token=0x%08X\n",
               marks ? 'X' : ' ', t->frames, t->story, t->persist1, t->flags24, t->token);
    }

    for (i = 0; i < MILESTONE_COUNT; i++) {
        const Milestone *m = &kMilestones[i];
        int ok = MilestoneSatisfied(m);
        printf("  %s %s  (%s)\n", ok ? "OK  " : "MISS", m->what, m->evidence);
        if (ok)
            reached++;
    }

    printf("route: %d/%d milestones observed\n", reached, MILESTONE_COUNT);

    printf("route: boundary stubs invoked:\n");
    for (i = 0; i < g_stub_count; i++) {
        printf("  %s %s x%d\n", g_stub_registry[i].classification,
               g_stub_registry[i].symbol, g_stub_registry[i].invoked);
    }

    /* Loud-boundary contract: an UNSUPPORTED stub would silently stand in for
     * missing retail behavior, so the route must never have invoked one. */
    for (i = 0; i < g_stub_count; i++) {
        if (strcmp(g_stub_registry[i].classification, "UNSUPPORTED") == 0 &&
            g_stub_registry[i].invoked > 0) {
            printf("FAIL: UNSUPPORTED boundary stub %s invoked\n",
                   g_stub_registry[i].symbol);
            return 1;
        }
    }

    /* Name the frontier: the room token says where, the script PC says what. */
    PrintFrontier();

    if (g_pc_trace_on) {
        printf("route: frontier opcode-PC histogram:\n");
        for (i = 0; i < g_pc_slot_count; i++)
            printf("  pc=%08X op=%02X frames=%u\n", (unsigned)g_pc_slots[i].pc,
                   (unsigned)g_pc_slots[i].op, g_pc_slots[i].count);
    }

    if (getenv("PE_ROUTE_AYA_DUMP")) {
        pe_addr_t aya = PE_LoadU32(0x8009D254u);
        printf("route: Aya D_8009D254=%08X", (unsigned)aya);
        if (aya) {
            printf(" pos x=%08X y=%08X z=%08X", (unsigned)PE_LoadU32(aya + 0x28u),
                   (unsigned)PE_LoadU32(aya + 0x2Cu),
                   (unsigned)PE_LoadU32(aya + 0x30u));
        }
        printf("\nroute: pad raw D_800BE9A2=%04X held D_8009D26C=%08X edge D_8009D1F4=%08X prev D_8009D1E4=%08X\n",
               (unsigned)PE_LoadU16(0x800BE9A2u),
               (unsigned)PE_LoadU32(0x8009D26Cu),
               (unsigned)PE_LoadU32(0x8009D1F4u),
               (unsigned)PE_LoadU32(0x8009D1E4u));
    }

    /* Loud-boundary contract: an UNSUPPORTED stub would silently stand in for
     * missing retail behavior, so the route must never have invoked one. */
    for (i = 0; i < g_stub_count; i++) {
        if (strcmp(g_stub_registry[i].classification, "UNSUPPORTED") == 0 &&
            g_stub_registry[i].invoked > 0) {
            printf("FAIL: UNSUPPORTED boundary stub %s invoked\n",
                   g_stub_registry[i].symbol);
            return 1;
        }
    }

    /* The four documented HOST_ADAPTED skips must actually have been taken —
     * otherwise the harness is not exercising the same host-adapted boot it
     * claims (movie + opening-menu skip), and the route numbers are suspect. */
    if (BoundaryStubCount("func_801909B4_skip_movie_new_game") == 0 ||
        BoundaryStubCount("func_8006E9A0_skip_movie_fade") == 0 ||
        BoundaryStubCount("func_80014E30_skip_movie") == 0 ||
        BoundaryStubCount("func_80016F10_skip_opening_menu") == 0) {
        printf("FAIL: a documented HOST_ADAPTED skip stub was never invoked\n");
        return 1;
    }

    /* The route cannot claim more than it proved.  If it stopped before the
     * documented frontier, say exactly where so a regression is visible.
     *
     * Two accepted endpoints:
     *  (a) the frozen m0031i supply frontier (the recorded-pad route), with the
     *      frozen task PC; or
     *  (b) the reward-pilot endpoint, which advances one room further into the
     *      m0032i fight.  Milestone 56 (equipment command 407 selects carried
     *      slot 0) is only issued in that fight — the field Items/Use and the
     *      pistol selection never share a frame (pilot: Items/Use 58044 gun=2,
     *      command407 slot0 58917) — so the pilot cannot park at m0031i.  The
     *      durable story/inventory state is asserted instead and the observed
     *      endpoint is reported loudly.
     *
     * The old endpoint pinned HP/status-copy at the absolute 45; that was Aya's
     * max HP in the route variant that authored this block.  The current route
     * levels Aya to max HP 53, so those absolute pins are gone; milestone 55
     * still requires the item use to restore HP to full (record+28). */
    if (reached == MILESTONE_COUNT) {
        pe_addr_t aya=PE_LoadU32(0x8009D254u), record=aya?PE_LoadU32(aya):0u;
        int frozen = (GA_TOKEN == FRONTIER_TOKEN && FindFrontierPc() != 0u);
        int durable;
        fprintf(stderr,
            "route: endpoint token=%08X pc=%08X frozen=%d hp=%u/%u statuscopy=%u gun=%u ammo=%u "
            "persist1=%X persist74=%X p24=%08X chests=%X switch=%X mode=%u d1a0=%X d244=%u "
            "f7854=%X aya98=%X c0dca=%u c0cd8=%X inv6=%d inv7=%d invC8=%d invC9=%d\n",
            (unsigned)GA_TOKEN, (unsigned)FindFrontierPc(), frozen,
            record?PE_LoadU16(record+12u):0u, record?PE_LoadU16(record+28u):0u,
            (unsigned)PE_LoadU16(0x800C0E08u), (unsigned)PE_LoadU8(0x800C0E20u),
            (unsigned)PE_LoadU16(0x800A1E6Eu),
            (unsigned)PE_LoadU32(GA_PERSIST1), (unsigned)PE_LoadU32(GA_PERSIST74),
            (unsigned)PE_LoadU32(GA_PERSIST24), (unsigned)PE_LoadU32(0x800A7940u),
            (unsigned)PE_LoadU32(0x800A789Cu), (unsigned)PE_LoadU32(0x8009D28Cu),
            (unsigned)PE_LoadU32(0x8009D1A0u), (unsigned)PE_LoadU8(0x8009D244u),
            (unsigned)PE_LoadU32(0x800A7854u), aya?(unsigned)PE_LoadU32(aya+0x98u):0u,
            (unsigned)PE_LoadU8(0x800B0DCAu), (unsigned)PE_LoadU32(0x800B0CD8u),
            InventoryContains(6u), InventoryContains(7u),
            InventoryContains(0xC8u), InventoryContains(0xC9u));
        durable =
            aya != 0u &&
            !(PE_LoadU32(0x8009D1A0u)&6u) &&
            !PE_LoadU8(0x8009D244u) &&
            PE_LoadU32(GA_PERSIST74)==0x68u &&
            (PE_LoadU32(GA_PERSIST24)&0x80080220u)==0x80080220u &&
            InventoryContains(0xC8u) && InventoryContains(0xC9u) &&
            !InventoryContains(6u) && !InventoryContains(7u) &&
            (PE_LoadU32(0x800A7940u)&0x280u)==0x280u &&
            (PE_LoadU32(0x800A789Cu)&4u) &&
            PE_LoadU8(0x800C0E20u)==0u &&
            (PE_LoadU32(0x800A7854u)&0x4000u) &&
            SewerEnemiesDefeated() &&
            PE_LoadU8(0x800B0DCAu)==0u &&
            (PE_LoadU32(0x800B0CD8u)&0x44u)==0x40u &&
            PE_LoadU32(0x8009D28Cu)==9u;
        /* persist1 (0x14E = the m0031i arrival, milestone 17) and
         * Aya+0x98 bit 0x200 (a room-local actor flag) only pin the frozen
         * m0031i endpoint; both legitimately differ in the next room after the
         * m0032i fight (observed: persist1=0x20, aya+0x98=0x600).  They are
         * reported above but not required of the advanced endpoint. */
        if (durable && (!frozen || PE_LoadU32(GA_PERSIST1)==0x14Eu)) {
            printf("PASS: boot -> theater keys -> rehearsal -> two sewer victories route (%d milestones, "
                   "endpoint=%s token=0x%08X pc=0x%08X, story68, C8/C9 retained, item7 consumed, "
                   "pistol equipped, two victories and supply pickups, player control)\n",
                   MILESTONE_COUNT, frozen?"m0031i":"advanced", (unsigned)GA_TOKEN,
                   (unsigned)FindFrontierPc());
            return 0;
        }
        printf("FAIL: %d milestones reached but the end state is wrong "
               "(token=0x%08X pc=0x%08X durable=%d)\n",
               MILESTONE_COUNT, (unsigned)GA_TOKEN, (unsigned)FindFrontierPc(), durable);
        return 1;
    }

    printf("FAIL: route stopped at milestone %d (%s)\n", reached,
           reached < MILESTONE_COUNT ? kMilestones[reached].what : "?");
    return 1;
}
