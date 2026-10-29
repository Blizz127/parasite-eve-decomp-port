/* func_8002AA98, the battle mode-3 revive presentation, through the
 * production battle tick func_800299CC.  Mode 3 takes it when Aya dies
 * holding item 18, or with record+0x4C bit 0x800.  Retail then fades Aya
 * and the partner out and in, plays cue 0x69 and clip 0xF, restores half
 * HP and the HP gauge colours, and returns 1, so the tick consumes the item
 * (or clears the bit) and leaves mode 3.  The hand port this replaces ran
 * phase 0 only, so the port stayed in mode 3.
 * Fixture: the field menu idle (func_8005C498 returns 0), no enemies, an
 * empty effect descriptor table (cue 0x69 spawns nothing), models without
 * geometry (func_8003CAEC returns at once), D_8009D2A0 = 1 (phase 7 skips
 * func_8002F300), no ordering table (the last tick's func_80033A40 returns
 * at once; +0x28 is seeded anyway so its clamp could not change +0x08).
 * Two waits are opened by the fixture, so 106 is the phase-counter count of
 * the C, not a retail duration: phase 2 holds while either actor's +0x252 is
 * set (both start 0 here), and phase 6 waits for the clip to reach +0x1A
 * (done between ticks below).  No disc data or CPU interpreter. */
#define REV_AYA     0x80108600u
#define REV_REC     0x8010A000u
#define REV_PARTNER 0x800B0B38u
#define REV_ITEMS   0x800C0E48u   /* inventory slots, short item ids */
#define REV_FX_TBL  0x80150800u   /* effect descriptor table, all empty */
#define REV_TICKS   106u          /* 1+17+1+53+15+17+1+1, phases 0..7, both waits open */

static void REV_Seed(uint32_t rec_flags, int item18)
{
    ResetTestState();
    D_8009D1A0 |= 0x80u;                  /* modules initialised (func_8006914C) */
    PE_StoreU32(0x8009CED8u, 7u);         /* field menu idle */
    PE_StoreU32(0x8009D254u, REV_AYA);
    PE_StoreU32(REV_AYA, REV_REC);        /* the tick reloads D_8009D278 from Aya */
    PE_StoreU32(0x8009D28Cu, 3u);         /* mode 3: Aya died */
    PE_StoreU32(REV_REC + 0x4Cu, rec_flags);
    PE_StoreU16(REV_REC + 0x1Cu, 200u);   /* max HP */
    PE_StoreU16(REV_REC + 0x0Cu, 0u);     /* HP */
    PE_StoreU32(REV_REC + 0x08u, 0x400u);
    PE_StoreU32(REV_REC + 0x28u, 0x10000000u); /* +8's maximum (func_80033A40) */
    PE_StoreU8(REV_REC + 0x12u, 4u);      /* the clip phase 6 restores */
    PE_StoreU8(REV_AYA + 0x0Eu, 0x13u);   /* the death clip ... */
    PE_StoreU8(REV_AYA + 0x0Fu, 8u);      /* ... at its last frame */
    PE_StoreU16(REV_AYA + 0x16u, 8u);
    PE_StoreU16(REV_AYA + 0x210u, 0x0111u);
    PE_StoreU16(REV_AYA + 0x212u, 0x0222u);
    PE_StoreU8(0x8009D2A0u, 1u);
    PE_StoreU32(0x800942E0u, REV_FX_TBL);
    PE_StoreU8(0x800C0E0Cu, 2u);          /* two inventory slots (func_80052F70) */
    PE_StoreU16(REV_ITEMS + 0u, 5u);
    PE_StoreU16(REV_ITEMS + 2u, item18 ? 18u : 0u);
    PE_StoreU8(0x800C0E22u, 0xFFu);       /* no slot equipped */
    PE_StoreU32(0x8009D048u, REV_ITEMS);  /* the list func_80053E6C counts */
    PE_StoreU32(0x8009D050u, 2u);
    PE_StoreU32(0x8009D03Cu, 0x100u);
}

/* Production ticks until mode 3 ends.  Phase 6 waits for the clip phase 3
 * started to reach frame +0x1A; the actor animation does that between
 * ticks, so the fixture does it here. */
static unsigned REV_TickUntilModeLeaves3(unsigned limit)
{
    unsigned n;
    for (n = 0; n < limit && PE_LoadU32(0x8009D28Cu) == 3u; n++) {
        if (PE_LoadU8(0x8009CE74u) == 6u)
            PE_StoreU8(REV_AYA + 0x0Fu, (uint8_t)PE_LoadU16(REV_AYA + 0x1Au));
        func_800299CC();
        if (PE_Port_ShouldStop()) break;
    }
    return n;
}

/* Where a revive that did not finish stopped (printed on failure only). */
static void REV_Report(const char *name, unsigned ticks)
{
    if (ticks != REV_TICKS || PE_LoadU32(0x8009D28Cu) == 3u)
        fprintf(stderr, "%s: %u ticks, phase %u, mode %u, CE70 %u\n", name, ticks,
                PE_LoadU8(0x8009CE74u), PE_LoadU32(0x8009D28Cu), PE_LoadU8(0x8009CE70u));
}

static int REV_Revived(void)
{
    return PE_LoadU8(0x8009CE74u) == 0u && PE_LoadU32(0x8009D28Cu) == 0u &&
           PE_LoadU16(REV_REC + 0x0Cu) == 100u &&            /* max/2 */
           PE_LoadU16(REV_REC + 0x58u) == 100u &&
           PE_LoadU16(REV_REC + 0x5Au) == 0x0111u &&
           PE_LoadU16(REV_REC + 0x5Cu) == 0x0222u &&
           PE_LoadU8(REV_REC + 0x5Eu) == 30u && PE_LoadU8(REV_REC + 0x5Fu) == 1u &&
           PE_LoadU32(REV_REC + 0x08u) == 0x100u;            /* >>= 2 */
}

static void test_REVIVE_mode3_item_revives(void)
{
    unsigned ticks;
    TEST("REVIVE_mode3_item_revives");
    REV_Seed(0u, 1);
    ASSERT(func_80053E6C(18) == 1, "fixture: item 18 must be held");
    ticks = REV_TickUntilModeLeaves3(300u);
    REV_Report("REVIVE_mode3_item_revives", ticks);
    ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count && g_stub_count == 0,
           "the revive stopped or took a boundary");
    ASSERT(ticks == REV_TICKS, "mode 3 did not end after the 106 phase-counter ticks");
    ASSERT(REV_Revived(), "phase, mode, HP or the record's HUD fields differ from retail");
    ASSERT(PE_LoadU16(REV_ITEMS + 2u) == 0u && PE_LoadU16(REV_ITEMS + 0u) == 5u &&
           func_80053E6C(18) == 0, "item 18 was not the one consumed");
    ASSERT(PE_LoadU16(REV_AYA + 0x250u) == 0x26u && PE_LoadU8(REV_AYA + 0x252u) == 1u &&
           PE_LoadU16(REV_PARTNER + 0x250u) == 0x06u &&
           PE_LoadU8(REV_PARTNER + 0x252u) == 1u,
           "the fade flags of Aya and the partner differ from retail");
    ASSERT((PE_LoadU32(REV_AYA + 0x98u) & 0x100u) == 0u &&
           PE_LoadU8(REV_AYA + 0x0Eu) == 4u,
           "the +0x98 flag or the clip phase 6 restores differ from retail");
    ASSERT(PE_LoadU8(0x800B00E8u + 5u) == 0x46u && PE_LoadU8(0x800B00E8u + 13u) == 0xFFu &&
           PE_LoadU8(0x800B0130u + 5u) == 0x82u,
           "the HP gauge colours were not reset");
    PASS();
}

/* Bit 0x800 takes the arm first: the flag is cleared and the item kept. */
static void test_REVIVE_flag800_revives(void)
{
    unsigned ticks;
    TEST("REVIVE_flag800_revives");
    REV_Seed(0x800u, 1);
    ticks = REV_TickUntilModeLeaves3(300u);
    REV_Report("REVIVE_flag800_revives", ticks);
    ASSERT(!PE_Port_ShouldStop() && !g_bootstrap_arg4_call_count && g_stub_count == 0,
           "the revive stopped or took a boundary");
    ASSERT(ticks == REV_TICKS && REV_Revived(), "the bit-0x800 revive did not complete");
    ASSERT((PE_LoadU32(REV_REC + 0x4Cu) & 0x800u) == 0u, "bit 0x800 was not cleared");
    ASSERT(PE_LoadU16(REV_ITEMS + 2u) == 18u, "item 18 was consumed on the bit-0x800 path");
    PASS();
}

/* Control: behaviour the parent already had.  Without the item or the bit,
 * mode 3 takes func_8002B29C (its case 0 sets CE70 = 70, not 16). */
static void test_REVIVE_no_item_takes_2b29c(void)
{
    TEST("REVIVE_no_item_takes_2b29c");
    REV_Seed(0u, 0);
    func_800299CC();
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x8009D28Cu) == 3u &&
           PE_LoadU8(0x8009CE74u) == 1u && PE_LoadU8(0x8009CE70u) == 70u,
           "mode 3 without item 18 did not take func_8002B29C");
    PASS();
}

/* The legacy mode-3 cut (PE_BATTLE_TICK_LEGACY only) mirrors the tick: the
 * item arm consumes item 18 and the bit arm clears 0x800 from a reloaded
 * +0x4C, keeping phase 7's func_800293F4(1) update (bit 0x800000). */
static void test_REVIVE_legacy_cut_matches_tick(void)
{
    TEST("REVIVE_legacy_cut_matches_tick");
    for (unsigned bit = 0; bit < 2u; bit++) {
        unsigned n;
        REV_Seed(bit ? 0x800u : 0u, 1);
        PE_StoreU32(0x8009D278u, REV_REC);
        for (n = 0; n < 300u && PE_LoadU32(0x8009D28Cu) == 3u; n++) {
            if (PE_LoadU8(0x8009CE74u) == 6u)
                PE_StoreU8(REV_AYA + 0x0Fu, (uint8_t)PE_LoadU16(REV_AYA + 0x1Au));
            func_8002A7F8_mode3_cut();
            if (PE_Port_ShouldStop()) break;
        }
        ASSERT(!PE_Port_ShouldStop() && n == REV_TICKS && REV_Revived(),
               "the legacy cut did not complete the revive");
        if (bit)
            ASSERT((PE_LoadU32(REV_REC + 0x4Cu) & 0x800800u) == 0x800000u &&
                   PE_LoadU16(REV_ITEMS + 2u) == 18u,
                   "the bit arm wrote back stale flags or consumed the item");
        else
            ASSERT(PE_LoadU16(REV_ITEMS + 2u) == 0u && func_80053E6C(18) == 0,
                   "the item arm did not consume item 18");
    }
    PASS();
}
