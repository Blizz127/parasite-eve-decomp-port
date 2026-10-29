/* Day-3 route pilot (harness-only: --route-pad with PE_PAD_SCRIPT and
 * PE_ROUTE_WAYPOINTS; never active in a normal start).
 *
 * After the hand-over the pad script supplies the text/menu pad (Cross /
 * Circle). This pilot overrides it when there is something to do:
 *   * a battle is live (0x8009D1A0 & 2): RouteBattleShim (shared Day-1 battle
 *     aid: HP lock + keep range + Cross on full ATB / default target);
 *   * field control (mode 0) in a room with waypoints: walk to the next
 *     waypoint with the learned pad->world map (RouteM34ShimMove).
 * It only emits controller buttons, apart from the battle aid's HP lock.
 *
 * PE_ROUTE_WAYPOINTS = entry|entry|...
 *   entry  = TOKEN[@MINSTORY][#VISIT]=wp;wp;...  (hex token, hex story,
 *            VISIT: only on the VISIT-th entry into that room, 1-based;
 *            TOKEN@MINSTORY&MASK: only while persist[0x2C] has all MASK bits;
 *            TOKEN@MINSTORY&SLOT:MASK: the same test on persist[SLOT])
 *   wp     = X,Z[,R][!N]                      (world units; R arrive radius,
 *            default 100; !N: on arrival press Cross for N frames;
 *            ^N: pulse Down for N frames instead, e.g. to move a menu cursor;
 *            %N: wait N frames with no buttons; *N: pulse Circle (raw 0x2000,
 *            the field check edge 0x20000000, e.g. the m0129i elevator);
 *            $HEX[:N]: hold raw pad HEX for N frames (default 12), e.g. turn in place at a wall;
 *            +: run there (hold Circle while walking), e.g. the timed m0381i roof escape;
 *            R >= 5000: chained, its act starts the moment the previous act ends, even
 *            while a menu window is open (as the first waypoint: from room entry), e.g. Down
 *            then Cross in the m0176i panel menu)
 * The entry used is the one for the current token with the highest MINSTORY
 * <= story; a matching #VISIT entry wins, then a matching &MASK entry. Its waypoints run in order; after the last one the pilot holds
 * still (the pad script resumes) until the room or story changes.
 *
 * World map (token A8000048): the entry's first wp is TARGET,CONFIRM[,STEP]
 * where TARGET is the location index (0x8019CC52), CONFIRM an active-low
 * pad mask (0 = Cross) and STEP the active-low mask that steps the selection
 * (default 0xFF7F, internal edge 0x40 = next location).  The pilot pulses Down until the selection matches, then
 * pulses CONFIRM (first press previews, second leaves). */
#ifndef PC_PORT_ROUTE_DAY3_PILOT_H
#define PC_PORT_ROUTE_DAY3_PILOT_H

typedef struct { int x, z, r, act, tmo, dn, run; unsigned pm; } D3Wp;   /* tmo: give up after N frames (~N); dn: 1 = ^N pulses Down, 2 = %N waits, 3 = *N pulses Circle, 4 = $HEX[:N] holds raw pad HEX N frames (e.g. turn in place) */
typedef struct { uint32_t token, story, flags, slot; int visit, n; D3Wp wp[24]; } D3Entry;
static D3Entry g_d3[160];
static int g_d3_n = -1;

static void D3Parse(void)
{
    const char *s = getenv("PE_ROUTE_WAYPOINTS");
    g_d3_n = 0;
    while (s && *s && g_d3_n < (int)(sizeof g_d3 / sizeof g_d3[0])) {
        D3Entry *e = &g_d3[g_d3_n];
        char *end;
        memset(e, 0, sizeof *e);
        e->token = (uint32_t)strtoul(s, &end, 16); s = end;
        if (*s == '@') { e->story = (uint32_t)strtoul(s + 1, &end, 16); s = end; }
        e->slot = 0x2Cu;
        if (*s == '&') {
            e->flags = (uint32_t)strtoul(s + 1, &end, 16); s = end;
            if (*s == ':') { e->slot = e->flags & 0x1FFu; e->flags = (uint32_t)strtoul(s + 1, &end, 16); s = end; }
        }
        if (*s == '#') { e->visit = (int)strtol(s + 1, &end, 10); s = end; }
        if (*s != '=') {   /* malformed entry (e.g. a stray '|'): skip it, keep parsing */
            while (*s && *s != '|') s++;
            if (*s) s++;
            continue;
        }
        s++;
        while (*s && *s != '|' && e->n < 24) {
            D3Wp *w = &e->wp[e->n];
            w->x = (int)strtol(s, &end, 10); s = end;
            if (*s != ',') break;
            w->z = (int)strtol(s + 1, &end, 10); s = end;
            w->r = 100;
            if (*s == ',') { w->r = (int)strtol(s + 1, &end, 10); s = end; }
            w->run = 0;
            while (*s == '!' || *s == '~' || *s == '^' || *s == '%' || *s == '*' || *s == '$' || *s == '+') {   /* suffixes in any order */
                int v;
                if (*s == '+') { w->run = 1; s++; continue; }
                v = *s == '$' ? 12 : (int)strtol(s + 1, &end, 10);
                if (*s == '$') { w->pm = (unsigned)strtoul(s + 1, &end, 16); w->dn = 4; w->act = v; if (*end == ':') w->act = (int)strtol(end + 1, &end, 10); s = end; continue; }
                if (*s == '!') w->act = v; else if (*s == '^') { w->act = v; w->dn = 1; }
                else if (*s == '%') { w->act = v; w->dn = 2; }
                else if (*s == '*') { w->act = v; w->dn = 3; } else w->tmo = v;
                s = end;
            }
            e->n++;
            if (*s == ';') s++;
        }
        g_d3_n++;
        while (*s && *s != '|') s++;
        if (*s == '|') s++;
    }
    fprintf(stderr, "[D3PILOT] %d waypoint entries\n", g_d3_n);
}

static uint16_t RouteDay3Pilot(uint16_t script_mask)
{
    static uint32_t cur_tok = 0, cur_story = 0xFFFFFFFFu;
    static int ent = -1, idx = 0, act_until = 0, act_dn = 0, last_log = 0, done_logged = 0;
    static unsigned act_pm = 0;
    uint32_t tok = (uint32_t)GA_TOKEN, story = PE_LoadU32(0x800A7918u);
    unsigned mode = PE_LoadU32(0x8009D28Cu), d1a0 = PE_LoadU32(0x8009D1A0u);
    pe_addr_t aya = PE_LoadU32(0x8009D254u);
    int ax, az;
    {   /* PE_D3_TASKS="KEY": harness probe, log the VM task list of actor KEY (u16 at +0xC) on change */
        const char *q = getenv("PE_D3_TASKS");
        static char last[1024];
        if (q) {
            unsigned key = (unsigned)strtoul(q, 0, 16); char cur[1024]; size_t n = 0;
            for (pe_addr_t a = PE_LoadU32(0x8009D20Cu); a && n < 900; a = PE_LoadU32(a + 4u)) {
                if (PE_LoadU16(a + 0xCu) != key) continue;
                for (unsigned i = 0; i < 3u; i++)
                    for (pe_addr_t t = PE_LoadU32(a + 0xA0u + i * 4u); t && n < 900; t = PE_LoadU32(t + 0x24u))
                        n += (size_t)snprintf(cur + n, sizeof cur - n, " [%u]%08X pc=%08X f=%X d=%X", i, (unsigned)t,
                                              (unsigned)PE_LoadU32(t), (unsigned)PE_LoadU16(t + 8u), (unsigned)PE_LoadU32(t + 0x10u));
                n += (size_t)snprintf(cur + n, sizeof cur - n, " f98=%X d1a0=%X", (unsigned)PE_LoadU32(a + 0x98u), d1a0);
            }
            cur[n] = 0;
            if (strcmp(cur, last)) { fprintf(stderr, "[D3TASK] frame=%d%s\n", g_frame, cur); strcpy(last, cur); }
        }
    }
    {   /* PE_D3_RAM_DUMP="FRAME:PATH,...": harness probe, 2 MB guest RAM at those frames */
        const char *q = getenv("PE_D3_RAM_DUMP");
        while (q && *q) {
            char *e, path[512]; int f = (int)strtol(q, &e, 10); size_t n = 0;
            if (*e != ':') break;
            for (q = e + 1; *q && *q != ',' && n < sizeof path - 1; q++) path[n++] = *q;
            path[n] = 0;
            if (f == g_frame) {
                FILE *o = fopen(path, "wb");
                if (o) { for (uint32_t i = 0; i < 0x200000u; i++) fputc(PE_LoadU8(0x80000000u + i), o); fclose(o); }
            }
            if (*q == ',') q++;
        }
    }

    if (g_d3_n < 0) D3Parse();
    if (!g_d3_n) return script_mask;
    {   /* the Day-2 gate's text pad, unbounded: Cross, release, Circle, release (40f each) */
        static const uint16_t seq[4] = {0xBFFFu, 0xFFFFu, 0xDFFFu, 0xFFFFu};
        script_mask = seq[((g_frame - g_hand_over_frame) / 40) % 4];
    }
    if (d1a0 & 2u) {
        static int last_dump;
        if (getenv("PE_D3_ACTOR_DUMP") && g_frame - last_dump >= 120) {   /* battle: actor table */
            last_dump = g_frame;
            for (unsigned i = 0; i < 24u; i++) {
                pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
                pe_addr_t b = p && PE_RangeIsRam(p, 640u) ? PE_LoadU32(p) : 0u;
                fprintf(stderr, "[D3ACT] frame=%d i=%u w=%08X,%08X,%08X body=%08X hp=%d type=%u x=%d z=%d\n", g_frame, i,
                        (unsigned)p, (unsigned)PE_LoadU32(0x8009E004u + i * 12u), (unsigned)PE_LoadU32(0x8009E008u + i * 12u),
                        (unsigned)b, b && PE_RangeIsRam(b, 24u) ? (int)PE_LoadU32(b + 16u) : -1,
                        p ? PE_LoadU8(p + 12u) : 0u, p ? (int)((int32_t)PE_LoadU32(p + 40u) >> 16) : 0,
                        p ? (int)((int32_t)PE_LoadU32(p + 48u) >> 16) : 0);
                if (!p) break;
            }
        }
        {   /* The shim aims only at actors already in the target list
             * (0x8009E000), which the game fills when targeting opens.
             * With no target yet and a full AT bar, Cross opens it. */
            pe_addr_t rec = PE_LoadU32(0x8009D278u);
            static int last_hp = -1;
            /* Harness fixture (opt-in): a warp sets the story but keeps Day-1
             * stats (45 HP), and Day-3 enemies hit for 35-45+.  PE_ROUTE_MAXHP
             * raises max HP in battle so the warped route can be fought;
             * the shim's existing HP refill then tops up to it. */
            if (rec && getenv("PE_ROUTE_MAXHP")) {
                unsigned want = (unsigned)atoi(getenv("PE_ROUTE_MAXHP"));
                if (want > PE_LoadU16(rec + 28u) && want < 1000u) PE_StoreU16(rec + 28u, (uint16_t)want);
            }
            if (rec && (int)PE_LoadU16(rec + 12u) != last_hp) {
                fprintf(stderr, "[D3HP] frame=%d hp=%u/%u mode=%u\n", g_frame,
                        PE_LoadU16(rec + 12u), PE_LoadU16(rec + 28u), mode);
                last_hp = (int)PE_LoadU16(rec + 12u);
            }
            uint16_t m = RouteBattleShim();
            {   /* PE_D3_BTL_SEQ="MASK:FRAMES,...": scripted pads from battle start (harness probe, e.g. Escape) */
                static int b0 = -1; static uint32_t btok;
                const char *q = getenv("PE_D3_BTL_SEQ");
                if (q && (b0 < 0 || btok != (uint32_t)GA_TOKEN || g_frame - b0 > 20000)) { b0 = g_frame; btok = (uint32_t)GA_TOKEN; }
                if (q) {
                    int t = g_frame - b0, acc = 0;
                    while (*q) {
                        char *e; unsigned mk = (unsigned)strtoul(q, &e, 16); int n = 1;
                        if (*e == ':') n = (int)strtol(e + 1, &e, 10);
                        if (t < acc + n) { m = (uint16_t)mk; break; }
                        acc += n; q = *e == ',' ? e + 1 : e;
                    }
                    if ((g_frame % 30) == 0)
                        fprintf(stderr, "[D3BTL] frame=%d t=%d mode=%u cmd=%X focus=%X out=%04X\n", g_frame, t,
                                mode, (unsigned)PE_LoadU32(0x8009D2A4u), (unsigned)PE_LoadU32(0x8009D15Cu), (unsigned)m);
                }
            }
            if (getenv("PE_D3_ACTORS") && (g_frame % 600) == 9) {   /* debug: chain in battle */
                pe_addr_t a = PE_LoadU32(0x8009D20Cu);
                for (unsigned i = 0; a && i < 64u && PE_RangeIsRam(a, 64u); i++, a = PE_LoadU32(a + 4u)) {
                    pe_addr_t b = PE_LoadU32(a);
                    fprintf(stderr, "[D3BACT] frame=%d a=%08X type=%u pos=%d,%d y=%d body=%08X hp=%d m=%04X\n", g_frame, (unsigned)a,
                            PE_LoadU16(a + 12u), (int32_t)PE_LoadU32(a + 40u) >> 16, (int32_t)PE_LoadU32(a + 48u) >> 16,
                            (int32_t)PE_LoadU32(a + 44u) >> 16, (unsigned)b,
                            b && PE_RangeIsRam(b, 24u) ? (int)PE_LoadU32(b + 16u) : -1, m);
                }
            }
            int no_target = 1;   /* list holds no live enemy; stale entries seen: Aya (type 0),
                                  * a bodiless type-4 actor (m0121i) */
            for (unsigned i = 0; i < 24u; i++) {
                pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
                if (!p) break;
                pe_addr_t b = PE_RangeIsRam(p, 64u) ? PE_LoadU32(p) : 0u;
                if (p != aya && PE_LoadU16(p + 12u) != 0u && b && PE_RangeIsRam(b, 24u) &&
                    (int32_t)PE_LoadU32(b + 16u) > 0) { no_target = 0; break; }
            }
            if (m == 0xFFFFu && mode == 0u && no_target && aya) {
                /* No target in range: close in on the nearest live actor of
                 * the field chain (0x8009D20C, next at +4, type at +12 with 0 = Aya,
                 * body hp at +16). */
                int ax2 = (int32_t)PE_LoadU32(aya + 40u) >> 16, az2 = (int32_t)PE_LoadU32(aya + 48u) >> 16;
                long long bd = -1; int bx = 0, bz = 0;
                pe_addr_t a = PE_LoadU32(0x8009D20Cu);
                for (unsigned i = 0; a && i < 64u; i++, a = PE_LoadU32(a + 4u)) {
                    pe_addr_t b = PE_LoadU32(a);
                    if (a == aya || !PE_RangeIsRam(a, 64u) || PE_LoadU16(a + 12u) == 0u || !b || !PE_RangeIsRam(b, 24u) ||
                        (int32_t)PE_LoadU32(b + 16u) <= 0) continue;
                    int x = (int32_t)PE_LoadU32(a + 40u) >> 16, z = (int32_t)PE_LoadU32(a + 48u) >> 16;
                    long long d = (long long)(x - ax2) * (x - ax2) + (long long)(z - az2) * (z - az2);
                    if (bd < 0 || d < bd) { bd = d; bx = x; bz = z; }
                }
                if (bd > 22500) m = RouteM34ShimMove(ax2, az2, bx - ax2, bz - az2, 0);
                if (rec && PE_LoadU16(rec + 16u) >= 9000u && (g_frame % 8) == 3) m &= 0xBFFFu;
            }
            if (story >= 0x220u) {
                /* Day 5+: an enemy still emerging (m0187i centipede) carries
                 * +0x98 0x4000 "not targetable".  Opening targeting then sets
                 * D1A0&4, which pauses every actor script, so it never finishes
                 * emerging.  Hold Cross until one is targetable; if targeting
                 * opened on an empty list (D2B0 == 0), back out with Triangle. */
                int ready = 0;
                pe_addr_t a = PE_LoadU32(0x8009D20Cu);
                for (unsigned i = 0; a && i < 64u && PE_RangeIsRam(a, 0xA0u); i++, a = PE_LoadU32(a + 4u)) {
                    pe_addr_t b = PE_LoadU32(a);
                    if (a == aya || PE_LoadU16(a + 12u) == 0u || !b || !PE_RangeIsRam(b, 24u) ||
                        (int32_t)PE_LoadU32(b + 16u) <= 0) continue;
                    if (!(PE_LoadU32(a + 0x98u) & 0x44000u)) { ready = 1; break; }
                }
                if (!ready) {
                    if (mode == 0u) m |= 0x4000u;
                    else if (mode == 1u && PE_LoadU32(0x8009D2B0u) == 0u) m = (g_frame & 8) ? 0xEFFFu : 0xFFFFu;
                }
            }
            return m;
        }
    }
    {   /* log: item/door flag word persist[0x41] (keys, fuses, fuse box) */
        static uint32_t last41 = 0xFFFFFFFFu;
        uint32_t p41 = PE_LoadU32(0x800A77F0u + 0x41u * 4u);
        if (p41 != last41) {
            fprintf(stderr, "[D3PILOT] frame=%d token=%08X p41=%08X\n", g_frame, (unsigned)tok, (unsigned)p41);
            last41 = p41;
        }
    }
    if (tok != cur_tok || story != cur_story) {
        static uint32_t vtok[64]; static int vcnt[64], nv;
        int best = -1, visit = 0, k;
        if (tok != cur_tok) {
            for (k = 0; k < nv && vtok[k] != tok; k++) ;
            if (k == nv && nv < 64) { vtok[nv] = tok; vcnt[nv++] = 0; }
            if (k < 64) vcnt[k]++;
        }
        for (k = 0; k < nv; k++) if (vtok[k] == tok) visit = vcnt[k];
        for (int i = 0; i < g_d3_n; i++) {
            if (g_d3[i].token != tok || g_d3[i].story > story) continue;
            if (g_d3[i].visit && g_d3[i].visit != visit) continue;
            if ((PE_LoadU32(0x800A77F0u + g_d3[i].slot * 4u) & g_d3[i].flags) != g_d3[i].flags) continue;
            int si = (g_d3[i].visit ? 2 : 0) + (g_d3[i].flags ? 1 : 0);
            int sb = best < 0 ? -1 : (g_d3[best].visit ? 2 : 0) + (g_d3[best].flags ? 1 : 0);
            if (si > sb || (si == sb && g_d3[i].story >= g_d3[best].story)) best = i;
        }
        if (tok != cur_tok || best != ent) {
            ent = best; idx = 0; act_until = 0; done_logged = 0;
            g_m34s_init = 0;              /* new room: reseed the pad map */
            fprintf(stderr, "[D3PILOT] frame=%d token=%08X story=%X visit=%d entry=%d p2C=%08X\n",
                    g_frame, (unsigned)tok, (unsigned)story, visit, ent, (unsigned)PE_LoadU32(0x800A78A0u));
        }
        {   /* debug: PE_D3_PERSIST_DUMP=<dir> writes the persist block (0x800A77F0,
             * 0x200 words) on each story change, for PE_WARP_PERSIST_FILE (scratch only) */
            const char *pd = getenv("PE_D3_PERSIST_DUMP");
            if (pd && story != cur_story) {
                char path[512]; snprintf(path, sizeof path, "%s/persist_%X.bin", pd, (unsigned)story);
                FILE *f = fopen(path, "wb");
                if (f) {
                    for (uint32_t k = 0; k < 0x200u; k++) { uint32_t w = PE_LoadU32(0x800A77F0u + k * 4u); fwrite(&w, 4u, 1u, f); }
                    fclose(f);
                }
            }
            if (pd && tok != cur_tok) {   /* and per room: persist_<story>_<token>.bin */
                char path[512]; snprintf(path, sizeof path, "%s/persist_%X_%08X.bin", pd, (unsigned)story, (unsigned)tok);
                FILE *f = fopen(path, "wb");
                if (f) {
                    for (uint32_t k = 0; k < 0x200u; k++) { uint32_t w = PE_LoadU32(0x800A77F0u + k * 4u); fwrite(&w, 4u, 1u, f); }
                    fclose(f);
                }
            }
        }
        cur_tok = tok; cur_story = story;
    }
    if (getenv("PE_D3_ACTORS") && (g_frame % 600) == 7) {   /* debug: field actor chain */
        pe_addr_t a = PE_LoadU32(0x8009D20Cu);
        for (unsigned i = 0; a && i < 64u && PE_RangeIsRam(a, 64u); i++, a = PE_LoadU32(a + 4u))
            fprintf(stderr, "[D3ACT] frame=%d a=%08X type=%u pos=%d,%d r=%u\n", g_frame, (unsigned)a,
                    PE_LoadU16(a + 12u), (int32_t)PE_LoadU32(a + 40u) >> 16, (int32_t)PE_LoadU32(a + 48u) >> 16,
                    PE_RangeIsRam(a, 0x228u) ? PE_LoadU16(a + 0x224u) : 0u);
    }
    {   /* debug: PE_D3_FLOOR_DUMP=<dir> writes the live floor mesh per room (scratch only) */
        static uint32_t dumped; static int seen_at; const char *dd = getenv("PE_D3_FLOOR_DUMP");
        if (dd && tok != dumped) {
            if (!seen_at) seen_at = g_frame;
            pe_addr_t mesh = PE_LoadU32(0x8009D1FCu);
            if (g_frame - seen_at > 120 && mesh && PE_RangeIsRam(mesh, 32u)) {
                int dim = PE_LoadU32(0x8009D1D8u) != 0, nv = PE_LoadU16(mesh), nt = PE_LoadU16(mesh + 4u);
                pe_addr_t vp = PE_LoadU32(mesh + 24u), tp = PE_LoadU32(mesh + 28u);
                char path[512]; snprintf(path, sizeof path, "%s/floor_%08X.txt", dd, (unsigned)tok);
                FILE *f = fopen(path, "w");
                if (f && PE_RangeIsRam(vp, (uint32_t)nv * 6u) && PE_RangeIsRam(tp, (uint32_t)nt * 28u)) {
                    fprintf(f, "dim %d nv %d nt %d\n", dim, nv, nt);
                    for (int i = 0; i < nv; i++) {
                        pe_addr_t v = vp + (uint32_t)i * (dim ? 6u : 4u);
                        fprintf(f, "v %d %d %d\n", (int16_t)PE_LoadU16(v), (int16_t)PE_LoadU16(v + (dim ? 4u : 2u)), dim ? (int16_t)PE_LoadU16(v + 2u) : 0);   /* x z y */
                    }
                    for (int i = 0; i < nt; i++) {
                        pe_addr_t t = tp + (uint32_t)i * (dim ? 28u : 22u) + (dim ? 8u : 2u);
                        fprintf(f, "t %u %u %u n %d %d %d\n", PE_LoadU16(t), PE_LoadU16(t + 2u), PE_LoadU16(t + 4u),
                                (int16_t)PE_LoadU16(t + 12u), (int16_t)PE_LoadU16(t + 14u), (int16_t)PE_LoadU16(t + 16u));
                    }
                }
                if (f) fclose(f);
                dumped = tok; seen_at = 0;
                fprintf(stderr, "[D3PILOT] floor dump %s nv=%d nt=%d dim=%d\n", path, nv, nt, dim);
            }
        }
    }
    if (g_frame >= act_until && ent >= 0 && idx < g_d3[ent].n &&
        g_d3[ent].wp[idx].r >= 5000 && g_d3[ent].wp[idx].act) {   /* R >= 5000: chained act, runs even inside a window */
        D3Wp *c = &g_d3[ent].wp[idx];
        fprintf(stderr, "[D3PILOT] frame=%d token=%08X chained wp%d\n", g_frame, (unsigned)tok, idx);
        act_until = g_frame + c->act; act_dn = c->dn; act_pm = c->pm; idx++;
    }
    if (g_frame < act_until && act_dn == 4) return (uint16_t)act_pm;   /* $HEX: steady raw hold */
    if (g_frame < act_until)   /* Cross / Down pulse, or a plain wait (%N) */
        return (g_frame % 8) == 3 && act_dn != 2 ? (act_dn == 3 ? 0xDFFFu : act_dn ? 0xFFBFu : 0xBFFFu) : 0xFFFFu;
    if (tok == 0xA8000048u && ent >= 0 && g_d3[ent].n > 0) {
        int target = g_d3[ent].wp[0].x, cur = (int16_t)PE_LoadU16(0x8019CC52u);
        uint16_t confirm = g_d3[ent].wp[0].z ? (uint16_t)g_d3[ent].wp[0].z : 0xBFFFu;
        if (g_frame - last_log >= 60) {
            last_log = g_frame;
            fprintf(stderr, "[D3PILOT] frame=%d map sel=%d target=%d c024=%d c026=%d c034=%d c044=%d c045=%d bff8=%08X f4=%08X\n",
                    g_frame, cur, target, (int16_t)PE_LoadU16(0x8019C024u), (int16_t)PE_LoadU16(0x8019C026u),
                    (int16_t)PE_LoadU16(0x8019C034u), PE_LoadU8(0x8019C044u), PE_LoadU8(0x8019C045u),
                    (unsigned)PE_LoadU32(0x8019BFF8u), (unsigned)PE_LoadU32(0x8009D1F4u));
        }
        if (PE_LoadU16(0x8019C024u) || PE_LoadU16(0x8019C026u)) return 0xFFFFu;
        uint16_t step = g_d3[ent].wp[0].r > 0xFF ? (uint16_t)g_d3[ent].wp[0].r : 0xFF7Fu;
        if (cur != target) return (g_frame % 24) < 4 ? step : 0xFFFFu;
        return (g_frame % 48) < 4 ? confirm : 0xFFFFu;
    }
    if (getenv("PE_D3_FIELD_ACTORS") && !(d1a0 & 2u) && (g_frame % 300) == 7) {   /* debug: field actor chain */
        pe_addr_t a = PE_LoadU32(0x8009D20Cu);
        for (unsigned i = 0; a && i < 64u && PE_RangeIsRam(a, 64u); i++, a = PE_LoadU32(a + 4u))
            fprintf(stderr, "[D3FACT] frame=%d token=%08X a=%08X type=%u pos=%d,%d y=%d\n", g_frame, (unsigned)tok,
                    (unsigned)a, PE_LoadU16(a + 12u), (int32_t)PE_LoadU32(a + 40u) >> 16,
                    (int32_t)PE_LoadU32(a + 48u) >> 16, (int32_t)PE_LoadU32(a + 44u) >> 16);
    }
    if (ent < 0 && aya && g_frame - last_log >= 120) {   /* no entry: still trace where Aya is */
        last_log = g_frame;
        fprintf(stderr, "[D3PILOT] frame=%d token=%08X idle pos=%d,%d mode=%X d1a0=%X\n", g_frame, (unsigned)tok,
                (int)((int32_t)PE_LoadU32(aya + 40u) >> 16), (int)((int32_t)PE_LoadU32(aya + 48u) >> 16), mode, d1a0);
    }
    if (ent < 0 || !aya || (mode != 0u && mode != 9u) || (d1a0 & 4u)) return script_mask;    /* window/menu: text pad; 9 = field after a battle */
    if (idx >= g_d3[ent].n) {
        if (!done_logged) { done_logged = 1;
            fprintf(stderr, "[D3PILOT] frame=%d token=%08X waypoints done\n", g_frame, (unsigned)tok); }
        return script_mask;
    }
    ax = (int32_t)PE_LoadU32(aya + 40u) >> 16;
    az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    {
        D3Wp *w = &g_d3[ent].wp[idx];
        int wx = w->x - ax, wz = w->z - az;
        if (g_frame - last_log >= 60) {
            last_log = g_frame;
            fprintf(stderr, "[D3PILOT] frame=%d token=%08X pos=%d,%d rot=%X wp%d=%d,%d d1a0=%X held=%08X cam=%d,%d\n",
                    g_frame, (unsigned)tok, ax, az, (unsigned)PE_LoadU16(aya + 0x3Au), idx, w->x, w->z, d1a0,
                    (unsigned)PE_LoadU32(0x8009D26Cu),
                    (int16_t)PE_LoadU16(0x800BD000u), (int16_t)PE_LoadU16(0x800BD004u));
        }
        static int wp_since, wp_key = -1;
        if (wp_key != ent * 64 + idx) { wp_key = ent * 64 + idx; wp_since = g_frame; }
        if (w->tmo && g_frame - wp_since >= w->tmo) {
            fprintf(stderr, "[D3PILOT] frame=%d token=%08X wp%d timeout at %d,%d p2C=%08X\n", g_frame, (unsigned)tok, idx, ax, az,
                    (unsigned)PE_LoadU32(0x800A78A0u));
            if (w->act) { act_until = g_frame + w->act; act_dn = w->dn; act_pm = w->pm; }
            idx++;
            return 0xFFFFu;
        }
        if ((long long)wx * wx + (long long)wz * wz <= (long long)w->r * w->r) {
            fprintf(stderr, "[D3PILOT] frame=%d token=%08X reached wp%d at %d,%d\n",
                    g_frame, (unsigned)tok, idx, ax, az);
            if (w->act) { act_until = g_frame + w->act; act_dn = w->dn; act_pm = w->pm; }
            idx++;
            return 0xFFFFu;
        }
        {   /* Cross pulse while walking: advances text the field mode does not flag */
            uint16_t m = RouteM34ShimMove(ax, az, wx, wz, 0);
            if ((g_frame % 16) == 3 || (g_frame % 16) == 4) m &= 0xBFFFu;
            if (w->run) m &= 0xDFFFu;           /* active-low Circle: run */
            return m;
        }
    }
}

#endif /* PC_PORT_ROUTE_DAY3_PILOT_H */
