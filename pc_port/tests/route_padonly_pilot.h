/* PE_ROUTE_AIDS_PAD_ONLY=1 only (g_route_aids_pad_only, set in port_main's main under --route-pad): the pad-driven
 * healing and the read-only route trace of the pad-only Day-1 probe.  Nothing here runs with the switch unset, and
 * nothing here writes guest memory (R9): every function below reads the guest through PE_LoadU* and answers with a
 * controller mask, or logs to stderr.
 *
 * What a player does that the recorded route and the pilots do not: open the field menu and heal when HP is low
 * (Triangle -> root menu -> Parasite Energy list -> Heal -> confirm), instead of relying on the aids' HP lock.
 *
 * FIELD HEAL (RoutePadOnlyFieldHeal, called from RoutePadSource after RouteRewardSewerPilot, so it can take the pad
 * over that call's dismiss/walk logic): while Aya stands in a free field room (battle-mode word 0, 9 or 12, D1A0 low byte 0,
 * no menu focus, no movie, story >= 0x5E) and HP <= PE_PADONLY_FHEAL_PCT % of max (default 60), the controller opens the
 * menu with Triangle and walks the menu by the observed (id,row,col) of the menu object at *0x8009D15C
 * (+36 id, +72 row, +68 col; the same fields RoutePilotItemHealMask reads):
 *   id 0  root       -> row root_row (first 1, the battle-proven Parasite Energy row), Cross
 *   id 8  PE list    -> row 0 / col 0 (Heal), Cross once
 *   id 41 confirm    -> col 0, Cross
 *   anything else    -> Circle (back one level); a root row that did not open id 8 advances root_row
 * and leaves with Circle presses until the menu focus is gone.  Success = HP rose by >= 8 over the attempt.
 * Presses are 2 frames long and 28 frames apart (the oracle cadence; faster taps desync the lists).  While an
 * attempt owns the pad the route clock is held (g_pad_offset advances with g_frame), so the recorded field walking
 * resumes where it stopped; the hold is capped at 600 frames per attempt, well inside the 900-frame anchor window.
 * Limits: at most 2 attempts per room visit, a cooldown after each, a hard stop 900 frames after the start. */
#ifndef PC_PORT_ROUTE_PADONLY_PILOT_H
#define PC_PORT_ROUTE_PADONLY_PILOT_H

static uint16_t g_po_last_mask = 0xFFFFu;       /* last controller mask this module saw (trace only) */
static int g_po_fh_pct = 60;                    /* start the field heal at or below this % of max HP */
static int g_po_fh_enabled = 1;                 /* PE_PADONLY_FHEAL=0 turns the field heal off (read once) */
static unsigned g_po_fh_story = 0x5Eu;          /* the field heal arms only from this story value on (PE_PADONLY_FHEAL_STORY, hex) */
static int g_po_env_read;

static struct {
    int on;                 /* an attempt is running: this module owns the pad */
    int start;              /* frame the attempt began */
    int next_act;           /* first frame the next menu decision may be taken */
    int hold_until;         /* output hold_mask while g_frame < hold_until */
    uint16_t hold_mask;
    int opened, open_frame; /* menu focus seen during this attempt */
    int root_row, tries;    /* root-menu row being tried for the PE entry; rows tried so far */
    int wrong;              /* Cross pressed on a root row, id 8 not seen yet */
    int pe_seen, pe_cross;  /* the PE list was reached; Cross presses made on it */
    int exiting;            /* leave with Circle presses */
    int hp0, pe0, presses;
    int stable, last_stable_frame;
    int cooldown, attempts;
    uint32_t room, att_room;   /* room of the attempt counter; room the running attempt began in */
    int last_hold_frame, held, nofocus_since, last_blocked_log;
    unsigned lid, lrow, lcol;
    int ok_count, fail_count;
} g_po_fh;

static int g_po_fight_keep;                     /* PE_PADONLY_FIGHT=keep: keep-range fight policy in M0027I/M0028I */
static int g_po_near = 400, g_po_far = 700;     /* keep-range band, world units (PE_PADONLY_NEAR / PE_PADONLY_FAR) */
static unsigned g_po_heal_pe = 60u;             /* PE needed to start an in-fight PE heal (PE_PADONLY_HEAL_PE) */
static struct { int on, start, last; } g_po_bh; /* in-fight heal of the keep-range policy */
/* M34 alligator arena (A8003248) policy, pad only (PE_PADONLY_M34=shim keeps RouteM34Shim, which relies on the HP lock) */
static int g_po_m34 = 1;
static int g_po_m34_flee = 1000;     /* with HP low and no heal yet, keep at least this far from the head */
static int g_po_m34_fire_d = 1100;   /* fire at the tail from at most this distance (hit chance 100% to 1071, 80% to 1285, 50% to 1607, 25% to 1928) */
static int g_po_m34_fire_h = 850;    /* ... and only with the head at least this far away */
static int g_po_m34_appr_h = 1000;   /* approach the tail only while the head is at least this far away */
static int g_po_m34_stop_d = 1040;   /* engage at this distance from the tail (the sure-hit band ends at 1071) */
static int g_po_m34_strafe = 0;      /* strafe amplitude in z (0 = stand still), PE_PADONLY_M34_STRAFE: run 4 showed strafing does not dodge the head */
static int g_po_m34_stage = 0;       /* PE_PADONLY_M34_STAGE=1: stage at the spawn, heal to full and bank PE first.  Off by default: run 5 showed the head does not sleep (it walks and hits from 1548 units on its own) */
static int g_po_m34_heal_hp = 30;    /* PE heal at or below this HP once engaged (a Heal gives 30) */
static int g_po_m34_engage_hp = 12;  /* at or below this HP stay out of the head's reach instead of engaging (PE_PADONLY_M34_ENGAGE_HP) */
static int g_po_m34_v2;              /* PE_PADONLY_M34=v2: PoM34 (the run 2-7 policy) instead of the v3 wave-dodging policy */
static int g_po_m34_wall_x = -2700;  /* v3 firing line, world x (PE_PADONLY_M34_WALL_X) */
static int g_po_m34_v3_fire_d = 1040;/* v3 fires at the tail from at most this world distance (PE_PADONLY_M34_V3_FIRE_D) */
static int g_po_m34_speed = 22;      /* Aya's walking speed per frame in the v3 wave predictor (PE_PADONLY_M34_SPEED) */
static int g_po_m34_keep = 1000;    /* v3: keep at least this far from the alligator's body root (its tail spin reaches ~930) (PE_PADONLY_M34_KEEP) */
static int g_po_p2_shim = 1;         /* M34 phase 2: RouteBattleShim's movement, its Cross held back while the alligator attacks, plus the
                                      * PE heal (default); PE_PADONLY_P2=own: the PoM34Phase2 mover below (runs 8-9: it lost the target) */
static int g_po_m34_fire_root = 1000;/* v3: outside the post-attack window fire only with the body root this far away (PE_PADONLY_M34_FIRE_ROOT) */
/* PE_PADONLY_M34_WFLEE=<w> (pe-day2pad, opt-in; 0 = off = the v3j policy unchanged): while the alligator winds up a wave
 * attack (kind 4 = the 5-wave fan, kind 5 = 1-2 waves) the mover adds w x (distance from the body root, capped at 1800) to
 * every candidate move, so Aya runs from the launch instead of holding still near it.  Movies-on run 1 (binary c0438943):
 * she held at the north edge 1044 from the root during a kind-5 windup, the waves launched ~510 from her, all nine moves
 * were predicted hit ("dodge-fail") and a two-wave burst took her last 31 HP at frame 63435. */
static int g_po_m34_wflee = 0;
/* PE_PADONLY_P2_HEAL_HP=<hp> (pe-day2pad, opt-in): M34 phase 2's PE-heal gate in the default RouteBattleShim-mover mode;
 * the merged value 34 (pe-m34 run 10) unless set.  The PE_PADONLY_P2=own mover keeps its own 34. */
static int g_po_p2_heal_hp = 34;
/* PE_PADONLY_M34_SAFEFIRE=<d> (pe-day2pad, opt-in; 0 = off): v3's post-attack fire window opens only after a sequence that
 * really struck (action byte 1 or a strike anim 9/11/12/13), and no shot starts with the body root closer than d (a stomp
 * reaches ~320 and the shot pins Aya ~50 frames).  PE_PADONLY_M34_FLIPFAR=<d> (opt-in; 0 = off): the racetrack direction
 * rule described at its use. */
static int g_po_m34_safefire = 0, g_po_m34_flipfar = 0;

static void PoReadEnv(void)
{
    const char *e;
    g_po_env_read = 1;
    if ((e = getenv("PE_PADONLY_FHEAL_PCT")) != NULL && atoi(e) > 0 && atoi(e) <= 100) g_po_fh_pct = atoi(e);
    if ((e = getenv("PE_PADONLY_FHEAL")) != NULL && e[0] == '0') g_po_fh_enabled = 0;
    if ((e = getenv("PE_PADONLY_FHEAL_STORY")) != NULL) g_po_fh_story = (unsigned)strtoul(e, NULL, 16);
    if ((e = getenv("PE_PADONLY_FIGHT")) != NULL && !strcmp(e, "keep")) g_po_fight_keep = 1;
    if ((e = getenv("PE_PADONLY_NEAR")) != NULL && atoi(e) > 0) g_po_near = atoi(e);
    if ((e = getenv("PE_PADONLY_FAR")) != NULL && atoi(e) > 0) g_po_far = atoi(e);
    if ((e = getenv("PE_PADONLY_HEAL_HP")) != NULL && atoi(e) > 0) g_po_heal_hp = (unsigned)atoi(e);
    if ((e = getenv("PE_PADONLY_HEAL_ATB")) != NULL && atoi(e) > 0) g_po_heal_atb = (unsigned)atoi(e);
    if ((e = getenv("PE_PADONLY_HEAL_PE")) != NULL && atoi(e) > 0) g_po_heal_pe = (unsigned)atoi(e);
    if ((e = getenv("PE_PADONLY_M34")) != NULL && !strcmp(e, "shim")) g_po_m34 = 0;
    if ((e = getenv("PE_PADONLY_M34")) != NULL && !strcmp(e, "v2")) g_po_m34_v2 = 1;
    if ((e = getenv("PE_PADONLY_M34_WALL_X")) != NULL && atoi(e) < 0) g_po_m34_wall_x = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_V3_FIRE_D")) != NULL && atoi(e) > 0) g_po_m34_v3_fire_d = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_SPEED")) != NULL && atoi(e) > 0) g_po_m34_speed = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_KEEP")) != NULL && atoi(e) >= 0) g_po_m34_keep = atoi(e);
    if ((e = getenv("PE_PADONLY_P2")) != NULL && !strcmp(e, "own")) g_po_p2_shim = 0;
    if ((e = getenv("PE_PADONLY_M34_FIRE_ROOT")) != NULL && atoi(e) >= 0) g_po_m34_fire_root = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_FLEE")) != NULL && atoi(e) > 0) g_po_m34_flee = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_FIRE_D")) != NULL && atoi(e) > 0) g_po_m34_fire_d = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_FIRE_H")) != NULL && atoi(e) > 0) g_po_m34_fire_h = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_APPR_H")) != NULL && atoi(e) > 0) g_po_m34_appr_h = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_STOP_D")) != NULL && atoi(e) > 0) g_po_m34_stop_d = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_HEAL_HP")) != NULL && atoi(e) > 0) g_po_m34_heal_hp = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_STRAFE")) != NULL && atoi(e) >= 0) g_po_m34_strafe = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_ENGAGE_HP")) != NULL && atoi(e) >= 0) g_po_m34_engage_hp = atoi(e);
    if ((e = getenv("PE_PADONLY_M34_STAGE")) != NULL) g_po_m34_stage = e[0] != '0';
    fprintf(stderr, "[PADONLY] knobs fheal=%d fheal_pct=%d fheal_story=%X fight=%s near=%d far=%d heal_hp=%u heal_atb=%u heal_pe=%u\n",
            g_po_fh_enabled, g_po_fh_pct, g_po_fh_story, g_po_fight_keep ? "keep" : "pilot", g_po_near, g_po_far, g_po_heal_hp,
            g_po_heal_atb, g_po_heal_pe);
    fprintf(stderr, "[PADONLY] knobs m34=%s flee=%d fire_d=%d fire_h=%d appr_h=%d stop_d=%d heal_hp=%d strafe=%d engage_hp=%d stage=%d\n",
            !g_po_m34 ? "shim" : g_po_m34_v2 ? "v2" : "v3", g_po_m34_flee, g_po_m34_fire_d, g_po_m34_fire_h, g_po_m34_appr_h, g_po_m34_stop_d,
            g_po_m34_heal_hp, g_po_m34_strafe, g_po_m34_engage_hp, g_po_m34_stage);
    fprintf(stderr, "[PADONLY] knobs m34v3 wall_x=%d fire_d=%d speed=%d keep=%d fire_root=%d\n", g_po_m34_wall_x, g_po_m34_v3_fire_d,
            g_po_m34_speed, g_po_m34_keep, g_po_m34_fire_root);
    if ((e = getenv("PE_PADONLY_M34_WFLEE")) != NULL && atoi(e) > 0) {   /* printed only when set: knob-off logs stay byte-identical */
        g_po_m34_wflee = atoi(e);
        fprintf(stderr, "[PADONLY] knobs m34v3 wflee=%d\n", g_po_m34_wflee);
    }
    if ((e = getenv("PE_PADONLY_P2_HEAL_HP")) != NULL && atoi(e) > 0) {   /* printed only when set, as above */
        g_po_p2_heal_hp = atoi(e);
        fprintf(stderr, "[PADONLY] knobs p2 heal_hp=%d\n", g_po_p2_heal_hp);
    }
    if ((e = getenv("PE_PADONLY_M34_SAFEFIRE")) != NULL && atoi(e) > 0) {
        g_po_m34_safefire = atoi(e);
        fprintf(stderr, "[PADONLY] knobs m34v3 safefire=%d\n", g_po_m34_safefire);
    }
    if ((e = getenv("PE_PADONLY_M34_FLIPFAR")) != NULL && atoi(e) > 0) {
        g_po_m34_flipfar = atoi(e);
        fprintf(stderr, "[PADONLY] knobs m34v3 flipfar=%d\n", g_po_m34_flipfar);
    }
}

static int PoHp(pe_addr_t rec) { return (int)(int16_t)PE_LoadU16(rec + 12u); }
static unsigned PoPe(pe_addr_t rec) { return PE_LoadU32(rec + 8u) >> 16; }

static uint16_t PoPress(uint16_t mask, const char *name, int hp, unsigned pe, unsigned id, unsigned row, unsigned col)
{
    g_po_fh.hold_mask = mask;
    g_po_fh.hold_until = g_frame + 2;
    g_po_fh.next_act = g_frame + 28;
    g_po_fh.presses++;
    fprintf(stderr, "[PADONLY] fheal f=%d press=%s pad=%04X id=%d row=%u col=%u hp=%d pe=%u n=%d\n",
            g_frame, name, (unsigned)mask, id == 0xFFFFFFFFu ? -1 : (int)id, row, col, hp, pe, g_po_fh.presses);
    return mask;
}

static void PoFhFinish(const char *why, int hp, unsigned pe)
{
    int ok = hp >= g_po_fh.hp0 + 8;
    fprintf(stderr, "[PADONLY] fheal END f=%d result=%s why=%s frames=%d presses=%d hp=%d->%d pe=%u->%u room=%08X\n",
            g_frame, ok ? "healed" : "no_heal", why, g_frame - g_po_fh.start, g_po_fh.presses, g_po_fh.hp0, hp,
            g_po_fh.pe0, pe, (unsigned)GA_TOKEN);
    if (ok) g_po_fh.ok_count++; else g_po_fh.fail_count++;
    g_po_fh.on = 0;
    g_po_fh.attempts++;
    g_po_fh.cooldown = g_frame + (ok ? 600 : 900);
    g_po_fh.stable = 0;
}

static int PoFieldHealRoomOk(uint32_t tok)
{
    /* The rooms with their own item-7 supply/forward logic in RouteRewardSewerPilot (M0031I, M0334, and the boss
     * rooms) keep it; everything else with field control is eligible. */
    return tok != 0xA80030C8u && tok != 0xA8063248u && tok != 0xA8003248u && tok != 0xA8064448u &&
           tok != 0xA80031C8u && tok != 0xA8003148u;
}

static uint16_t PoFieldHealCore(uint16_t mask)
{
    pe_addr_t rec = PE_LoadU32(0x8009D278u), aya = PE_LoadU32(0x8009D254u);
    pe_addr_t focus = PE_LoadU32(0x8009D15Cu);
    unsigned mode = PE_LoadU32(0x8009D28Cu), d1a0 = PE_LoadU32(0x8009D1A0u);
    unsigned id = focus ? PE_LoadU32(focus + 36u) : 0xFFFFFFFFu;
    unsigned row = focus ? PE_LoadU32(focus + 72u) : 0u, col = focus ? PE_LoadU32(focus + 68u) : 0u;
    uint32_t tok = (uint32_t)GA_TOKEN;
    int hp, new_frame;
    unsigned mx, pe;

    if (!g_route_aids_pad_only) return mask;
    if (!g_po_env_read) PoReadEnv();
    if (!g_po_fh_enabled || !rec || !aya) return mask;
    hp = PoHp(rec); mx = PE_LoadU16(rec + 28u); pe = PoPe(rec);
    new_frame = g_frame != g_po_fh.last_stable_frame;
    if (tok != g_po_fh.room) { g_po_fh.room = tok; g_po_fh.attempts = 0; }

    if (!g_po_fh.on) {
        /* Field control: the battle-mode word 0x8009D28C reads 0 before the first battle, 12 after one and 9 later (the
         * first run of this module waited for 9 and never armed: the rooms after M0023I sit at 12), and D1A0's low byte is
         * 0 in a free room (0x4000) but 0x80 in the theater/cutscene rooms (0x4080), 2 in a battle and 4 with a window. */
        int hurt = mx > 0u && mx < 1000u && hp > 0 && hp * 100 <= (int)mx * g_po_fh_pct &&
                   PE_LoadU32(0x800A7918u) >= g_po_fh_story && PE_LoadU32(0x800A7918u) < 0x1000u;
        int want = (mode == 9u || mode == 12u || mode == 0u) && (d1a0 & 0xFFu) == 0u && !focus &&
                   PE_LoadU8(0x800B0DBAu) == 0u && hurt && PoFieldHealRoomOk(tok) &&
                   g_po_fh.attempts < 2 && g_frame >= g_po_fh.cooldown;
        if (hurt && !want && g_frame - g_po_fh.last_blocked_log >= 300) {   /* why a hurt Aya is not being healed */
            g_po_fh.last_blocked_log = g_frame;
            fprintf(stderr, "[PADONLY] fheal blocked f=%d room=%08X story=%X hp=%d/%u mode=%u d1a0=%X focus=%X movie=%u attempts=%d "
                    "cooldown=%d\n", g_frame, (unsigned)tok, PE_LoadU32(0x800A7918u), hp, mx, mode, d1a0, (unsigned)focus,
                    PE_LoadU8(0x800B0DBAu), g_po_fh.attempts, g_po_fh.cooldown);
        }
        if (new_frame) { g_po_fh.last_stable_frame = g_frame; g_po_fh.stable = want ? g_po_fh.stable + 1 : 0; }
        if (!want || g_po_fh.stable < 20) return mask;
        g_po_fh.on = 1; g_po_fh.start = g_frame; g_po_fh.next_act = g_frame; g_po_fh.hold_until = 0;
        g_po_fh.opened = 0; g_po_fh.root_row = 1; g_po_fh.tries = 0; g_po_fh.wrong = 0;
        g_po_fh.pe_seen = 0; g_po_fh.pe_cross = 0; g_po_fh.exiting = 0; g_po_fh.presses = 0;
        g_po_fh.hp0 = hp; g_po_fh.pe0 = (int)pe; g_po_fh.held = 0; g_po_fh.lid = 0xFFFFFFFEu;
        g_po_fh.att_room = tok; g_po_fh.nofocus_since = 0;
        fprintf(stderr, "[PADONLY] fheal START f=%d room=%08X story=%X hp=%d/%u pe=%u mode=%u d1a0=%X attempt=%d\n",
                g_frame, (unsigned)tok, PE_LoadU32(0x800A7918u), hp, mx, pe, mode, d1a0, g_po_fh.attempts + 1);
    }

    /* ---- an attempt is running ---- */
    {
        int age = g_frame - g_po_fh.start;
        /* hold the route clock (once per frame, at most 600 frames) so the recorded walking resumes after the heal */
        if (g_frame != g_po_fh.last_hold_frame) {
            g_po_fh.last_hold_frame = g_frame;
            if (g_po_fh.held < 600) { g_pad_offset++; g_po_fh.held++; }
        }
        if ((d1a0 & 2u) || tok != g_po_fh.att_room || age > 900 || mode == 3u) {
            PoFhFinish((d1a0 & 2u) ? "battle" : tok != g_po_fh.att_room ? "room_change" : mode == 3u ? "game_over" : "timeout",
                       hp, pe);
            return mask;
        }
        if (g_frame < g_po_fh.hold_until) return g_po_fh.hold_mask;
        if (focus && (id != g_po_fh.lid || row != g_po_fh.lrow || col != g_po_fh.lcol)) {
            g_po_fh.lid = id; g_po_fh.lrow = row; g_po_fh.lcol = col;
            fprintf(stderr, "[PADONLY] fheal OBS f=%d id=%u row=%u col=%u mode=%u d1a0=%X hp=%d pe=%u\n",
                    g_frame, id, row, col, mode, d1a0, hp, pe);
        }
        if (!focus) {
            if (g_po_fh.opened) {   /* the menu is gone (8 frames in a row): the attempt is over */
                if (!g_po_fh.nofocus_since) g_po_fh.nofocus_since = g_frame;
                if (g_frame - g_po_fh.nofocus_since >= 8) { PoFhFinish(g_po_fh.exiting ? "closed" : "closed_early", hp, pe); return mask; }
                return 0xFFFFu;
            }
            /* open the menu: Triangle held 4 frames of every 40 until a focus object appears */
            if (age > 260) { PoFhFinish("no_menu", hp, pe); return mask; }
            return (age % 40) < 4 ? 0xEFFFu : 0xFFFFu;
        }
        g_po_fh.nofocus_since = 0;
        if (!g_po_fh.opened) { g_po_fh.opened = 1; g_po_fh.open_frame = g_frame; g_po_fh.next_act = g_frame + 24; }
        if (g_frame < g_po_fh.next_act) return 0xFFFFu;
        if (age > 650) g_po_fh.exiting = 1;
        if (g_po_fh.exiting) return PoPress(0xDFFFu, "Circle(exit)", hp, pe, id, row, col);
        switch (id) {
        case 0u:
            if (g_po_fh.pe_seen) { g_po_fh.exiting = 1; return PoPress(0xDFFFu, "Circle(exit)", hp, pe, id, row, col); }
            if (g_po_fh.wrong) {
                g_po_fh.wrong = 0; g_po_fh.root_row++; g_po_fh.tries++;
                if (g_po_fh.tries >= 4) {
                    g_po_fh.exiting = 1;
                    return PoPress(0xDFFFu, "Circle(no_pe_row)", hp, pe, id, row, col);
                }
            }
            if ((int)row < g_po_fh.root_row) return PoPress(0xFFBFu, "Down", hp, pe, id, row, col);
            if ((int)row > g_po_fh.root_row) return PoPress(0xFFEFu, "Up", hp, pe, id, row, col);
            g_po_fh.wrong = 1;
            return PoPress(0xBFFFu, "Cross(root)", hp, pe, id, row, col);
        case 8u:
            g_po_fh.pe_seen = 1; g_po_fh.wrong = 0;
            if (g_po_fh.pe_cross >= 1) { g_po_fh.exiting = 1; return PoPress(0xDFFFu, "Circle(exit)", hp, pe, id, row, col); }
            if (row > 0u) return PoPress(0xFFEFu, "Up", hp, pe, id, row, col);
            if (col > 0u) return PoPress(0xFF7Fu, "Left", hp, pe, id, row, col);
            g_po_fh.pe_cross++;
            return PoPress(0xBFFFu, "Cross(heal)", hp, pe, id, row, col);
        case 41u:
            if (col > 0u) return PoPress(0xFF7Fu, "Left", hp, pe, id, row, col);
            return PoPress(0xBFFFu, "Cross(confirm)", hp, pe, id, row, col);
        default:
            /* a submenu the PE hypothesis did not expect: back out one level */
            return PoPress(0xDFFFu, "Circle(back)", hp, pe, id, row, col);
        }
    }
}

static uint16_t RoutePadOnlyFieldHeal(uint16_t mask)
{
    uint16_t out = PoFieldHealCore(mask);
    if (g_route_aids_pad_only) g_po_last_mask = out;
    return out;
}

/* ---- keep-range fight policy for M0027I / M0028I (opt-in, PE_PADONLY_FIGHT=keep; the default is the sewer pilot's own
 * rush-and-mash).  Pad only.  Like RouteBattleShim (which won M0023I without its HP lock): stay between the near and far
 * radius of the nearest living enemy with the learned pad->world walker, Cross every 8 frames (opens the target list when
 * the ATB is full, then confirms the default target).  Unlike RouteBattleShim it heals: when HP <= PE_PADONLY_HEAL_HP, the
 * ATB >= PE_PADONLY_HEAL_ATB and PE >= PE_PADONLY_HEAL_PE (or item 7 in slot 4), it opens the command menu with Triangle and
 * walks it every 16 frames (the cadence that finished the old M0027I heal in 54 frames): id 0 -> row 1, id 8 -> row 0/col 0,
 * id 41 -> col 0, anything else Circle.  The queued-command flag 0x8009CE3C (the one the sewer pilot clears its heal on) ends it. */
static uint16_t PoBattleHeal(int hp, unsigned pe)
{
    int age = g_frame - g_po_bh.start;
    pe_addr_t focus = PE_LoadU32(0x8009D15Cu);
    unsigned id = focus ? PE_LoadU32(focus + 36u) : 0xFFFFFFFFu;
    unsigned row = focus ? PE_LoadU32(focus + 72u) : 0u, col = focus ? PE_LoadU32(focus + 68u) : 0u;
    uint16_t m = 0xFFFFu;
    if (PE_LoadU8(0x8009CE3Cu) || age > 240) {
        fprintf(stderr, "[PADONLY] bheal END f=%d why=%s age=%d hp=%d pe=%u\n", g_frame,
                PE_LoadU8(0x8009CE3Cu) ? "queued" : "timeout", age, hp, pe);
        g_po_bh.on = 0;
        g_po_bh.last = g_frame;
        return 0xFFFFu;
    }
    if (age < 2) return 0xEFFFu;
    if (!focus) return (age % 20) < 2 ? 0xEFFFu : 0xFFFFu;
    if (g_frame % 16 != 3) return 0xFFFFu;
    switch (id) {
    case 0u:  m = row < 1u ? 0xFFBFu : row > 1u ? 0xFFEFu : 0xBFFFu; break;
    case 8u:  m = row > 0u ? 0xFFEFu : col > 0u ? 0xFF7Fu : 0xBFFFu; break;
    case 41u: m = col > 0u ? 0xFF7Fu : 0xBFFFu; break;
    default:  m = 0xDFFFu; break;
    }
    fprintf(stderr, "[PADONLY] bheal f=%d id=%d row=%u col=%u pad=%04X hp=%d pe=%u\n", g_frame,
            id == 0xFFFFFFFFu ? -1 : (int)id, row, col, (unsigned)m, hp, pe);
    return m;
}

static uint16_t PoFight(uint16_t base)
{
    pe_addr_t aya = PE_LoadU32(0x8009D254u), rec = PE_LoadU32(0x8009D278u), best = 0;
    unsigned mode = PE_LoadU32(0x8009D28Cu), atb, pe;
    uint16_t mask = 0xFFFFu;
    int hp, ax, az, ex = 0, ez = 0;
    long long bd = 0;
    if (!aya || !rec) return base;
    hp = PoHp(rec); atb = PE_LoadU16(rec + 16u); pe = PoPe(rec);
    ax = (int32_t)PE_LoadU32(aya + 40u) >> 16; az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    for (unsigned i = 0; i < 45u; i++) {
        pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
        int x, z; long long d;
        if (!p) break;
        if (p == aya || !PE_RangeIsRam(p, 640u) || !PE_RangeIsRam(PE_LoadU32(p), 24u) ||
            (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u) <= 0) continue;
        x = (int32_t)PE_LoadU32(p + 40u) >> 16; z = (int32_t)PE_LoadU32(p + 48u) >> 16;
        d = (long long)(x - ax) * (x - ax) + (long long)(z - az) * (z - az);
        if (!best || d < bd) { best = p; bd = d; ex = x; ez = z; }
    }
    if (g_po_bh.on) {
        if (mode == 0u || mode == 1u) return PoBattleHeal(hp, pe);
        g_po_bh.on = 0;
    }
    if (mode == 1u) { if (g_frame % 8 == 3) mask = 0xBFFFu; return mask; }
    if (mode != 0u) { if (g_frame % 32 == 3) mask = 0xBFFFu; return mask; }
    if (hp > 0 && hp <= (int)g_po_heal_hp && atb >= g_po_heal_atb && !PE_LoadU8(0x8009CE3Cu) &&
        (PE_LoadU16(0x800C0E50u) == 7u || pe >= g_po_heal_pe) && g_frame - g_po_bh.last > 120) {
        g_po_bh.on = 1; g_po_bh.start = g_frame;
        fprintf(stderr, "[PADONLY] bheal START f=%d tok=%08X hp=%d atb=%u pe=%u item4=%u near_d2=%lld\n", g_frame,
                (unsigned)GA_TOKEN, hp, atb, pe, PE_LoadU16(0x800C0E50u), best ? bd : -1LL);
        return 0xEFFFu;
    }
    if (best) {
        int wx = ex - ax, wz = ez - az;
        if (bd > (long long)g_po_far * g_po_far) mask = RouteM34ShimMove(ax, az, wx, wz, 0);
        else if (bd < (long long)g_po_near * g_po_near) mask = RouteM34ShimMove(ax, az, -wx, -wz, 0);
        if (g_frame % 8 == 3) mask &= 0xBFFFu;
    }
    return mask;
}

static uint16_t RoutePadOnlyFight(uint16_t mask)
{
    uint32_t tok = (uint32_t)GA_TOKEN;
    if (!g_route_aids_pad_only || !g_po_fight_keep) return mask;
    if (!(PE_LoadU32(0x8009D1A0u) & 2u)) { g_po_bh.on = 0; return mask; }
    if (tok != 0xA80023C8u && tok != 0xA8002448u) return mask;
    mask = PoFight(mask);
    g_po_last_mask = mask;
    return mask;
}

static int g_po_strafe_tz, g_po_strafe_f;       /* M34 strafe target z and the frame it was set */

static int PoIsqrt(long long v)
{
    long long r = 0, b = 1LL << 40;
    if (v <= 0) return 0;
    while (b > v) b >>= 2;
    while (b) { if (v >= r + b) { v -= r + b; r = (r >> 1) + b; } else r >>= 1; b >>= 2; }
    return (int)r;
}

/* ---- M34 alligator arena (token A8003248), pad only ---------------------------------------------------------------
 * RouteM34Shim walks straight at the tail and relies on the HP lock (run 2 of this lane: it died 274 frames in, one head hit
 * of >= 17 HP).  Facts used here (run 2 trace + the retail hit rule in func_80021278, hit chance by distance to the target
 * over the weapon range 1071: <= 1071 sure, <= 1285 80%, <= 1607 50%, <= 1928 25%): the head (type 3) wakes up and walks at
 * Aya at roughly 5 units/frame once she is within about 1300 units (Aya walks about 22), its bite reaches at least 459; the
 * tail (type 4, hit by shots for 9 each, 80 HP to take) is stationary at (-3354,-109); PE regenerates only in battle, 60 PE
 * pay one Heal (+30 HP), the ATB fills in 180 frames and every command uses all of it.
 * Policy: heal first (stand at the spawn side while PE and ATB come up), then edge toward the tail only while the head is
 * far, fire from the 50-80% band when the ATB is full and the head is not close, and run from the head when it is.  Target
 * menu handling is the shim's (cycle the selection onto the type-4 tail, Cross to commit, Circle when out of range). */
static uint16_t PoM34(uint16_t base)
{
    pe_addr_t aya = PE_LoadU32(0x8009D254u), rec = PE_LoadU32(0x8009D278u), head = 0, tail = 0;
    unsigned mode = PE_LoadU32(0x8009D28Cu), atb, pe, tail_idx = 0xFFu;
    uint16_t mask = 0xFFFFu;
    int hp, ax, az, hx = 0, hz = 0, tx = 0, tz = 0, headhp = 0, tailhp = 0;
    long long h2, t2;
    static int last_log; static const char *state = "init";
    int staging = 0, want_heal_hp;
    if (!aya || !rec) return base;
    hp = PoHp(rec); atb = PE_LoadU16(rec + 16u); pe = PoPe(rec);
    ax = (int32_t)PE_LoadU32(aya + 40u) >> 16; az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    for (unsigned i = 0; i < 45u; i++) {
        pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
        if (!p) break;
        if (p == aya || !PE_RangeIsRam(p, 640u) || !PE_RangeIsRam(PE_LoadU32(p), 24u)) continue;
        if (PE_LoadU8(p + 12u) == 4u) { tail = p; tail_idx = i; tailhp = (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u); }
        else if (PE_LoadU8(p + 12u) == 3u) { head = p; headhp = (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u); }
    }
    if (head && headhp > 0) { hx = (int32_t)PE_LoadU32(head + 40u) >> 16; hz = (int32_t)PE_LoadU32(head + 48u) >> 16; }
    else head = 0;
    if (tail && tailhp > 0) { tx = (int32_t)PE_LoadU32(tail + 40u) >> 16; tz = (int32_t)PE_LoadU32(tail + 48u) >> 16; }
    else tail = 0;
    h2 = head ? (long long)(hx - ax) * (hx - ax) + (long long)(hz - az) * (hz - az) : 1LL << 40;
    t2 = tail ? (long long)(tx - ax) * (tx - ax) + (long long)(tz - az) * (tz - az) : 1LL << 40;
    #define PO_D2(v) ((long long)(v) * (long long)(v))

    if (mode == 1u) {
        static const uint16_t cyc[4] = {0xFFBFu, 0xFFDFu, 0xFFEFu, 0xFF7Fu};
        static unsigned last_sel = 0xFFu, tries, dir;
        unsigned sel = PE_LoadU8(0x8009CE44u);
        if (g_po_bh.on) return PoBattleHeal(hp, pe);
        if (sel != last_sel) { last_sel = sel; tries = 0; }
        if (t2 > PO_D2(1928)) { if (g_frame % 8 == 3) mask = 0xDFFFu; state = "menu-cancel"; return mask; }
        if (tail_idx != 0xFFu && sel != tail_idx) {
            if (g_frame % 8 == 3) { if (++tries > 3) { tries = 0; dir = (dir + 1) % 4u; } mask = cyc[dir]; }
            state = "menu-select";
        } else {
            if (g_frame % 8 == 3) mask = 0xBFFFu;
            state = "menu-commit";
        }
        return mask;
    }
    if (mode != 0u) { if (g_frame % 32 == 3) mask = 0xBFFFu; return mask; }   /* intro / outro text: Cross to advance */
    if (g_po_bh.on) return PoBattleHeal(hp, pe);

    {   /* staging: while the head sleeps (it only wakes when Aya comes within ~1300), heal to full and bank PE for one more heal */
        unsigned mxhp = PE_LoadU16(rec + 28u);
        static int engaged;
        if (g_po_m34_stage && !engaged && (hp < (int)mxhp - 19 || pe < 60u) && (!head || h2 > PO_D2(1500)) && g_frame < 70000) staging = 1;
        else if (!engaged) { engaged = 1; fprintf(stderr, "[PADONLY] m34 engage f=%d hp=%d/%u pe=%u atb=%u\n", g_frame, hp, mxhp, pe, atb); }
        want_heal_hp = staging ? (int)mxhp - 20 : g_po_m34_heal_hp;
    }
    if (hp > 0 && hp <= want_heal_hp && atb >= 9000u && pe >= 60u && !PE_LoadU8(0x8009CE3Cu) && g_frame - g_po_bh.last > 120) {
        g_po_bh.on = 1; g_po_bh.start = g_frame;
        fprintf(stderr, "[PADONLY] bheal START f=%d tok=%08X hp=%d atb=%u pe=%u head_d=%d\n", g_frame, (unsigned)GA_TOKEN, hp, atb, pe,
                head ? PoIsqrt(h2) : -1);
        state = "heal";
        return 0xEFFFu;
    }
    if (staging) {
        state = "stage";                              /* hold still at the spawn side: nothing to do until HP and PE are up */
    } else if (hp <= g_po_m34_engage_hp) {
        /* weak and no heal yet (PE < 60 or ATB not full): keep out of the head's reach, it bites every ~150 frames for 19 */
        if (head && h2 < PO_D2(g_po_m34_flee)) {
            int wx = ax - hx, wz = az - hz;      /* run from the head; never push into a wall, slide along it instead */
            if (az > 650 && wz > 0) wz = -1; else if (az < -650 && wz < 0) wz = 1;
            if (ax > -800 && wx > 0) wx = -1;
            if (!wx && !wz) wz = az < 0 ? 1000 : -1000;
            mask = RouteM34ShimMove(ax, az, wx, wz, 0);
            state = "retreat";
        } else state = "weak-hold";
    } else if (atb >= 9000u && tail && t2 <= PO_D2(g_po_m34_fire_d)) {
        if (g_frame % 8 == 3) mask = 0xBFFFu;      /* ATB full and the tail in range: open the target menu (the world is frozen in menus and animations) */
        state = "fire";
    } else if (tail) {
        int xe = tx + g_po_m34_stop_d;             /* engage line */
        if (t2 > PO_D2(g_po_m34_fire_d - 40)) {    /* walk in until the tail is inside the firing distance (runs 4 and 6 stopped 70 units short and never fired) */
            mask = RouteM34ShimMove(ax, az, xe - ax, -az / 2, 0);
            state = "approach";
        } else if (g_po_m34_strafe > 0) {
            /* strafe in z while the ATB charges: does moving avoid the head's bite? */
            if (!g_po_strafe_tz) g_po_strafe_tz = g_po_m34_strafe;
            if (abs(az - g_po_strafe_tz) < 90 || g_frame - g_po_strafe_f > 90) {
                g_po_strafe_tz = -g_po_strafe_tz; g_po_strafe_f = g_frame;
            }
            mask = RouteM34ShimMove(ax, az, xe - ax, g_po_strafe_tz - az, 0);
            state = "strafe";
        } else state = "hold";
    } else state = "hold";
    if (g_frame - last_log >= 10) {
        last_log = g_frame;
        fprintf(stderr, "[PADONLY] m34 f=%d mode=%u hp=%d atb=%u pe=%u aya=%d,%d head=%d,%d,hp%d,d%d tail_d=%d tailhp=%d state=%s pad=%04X\n",
                g_frame, mode, hp, atb, pe, ax, az, hx, hz, headhp, head ? PoIsqrt(h2) : -1,
                tail ? PoIsqrt(t2) : -1, tailhp, state, (unsigned)mask);
    }
    #undef PO_D2
    return mask;
}

/* ---- M34 policy v3 (the default under the switch; PE_PADONLY_M34=v2 runs PoM34 above, =shim the shim) -------------
 * What the earlier runs could not see: the head's 19-23 HP "big hit" is not a bite but the M34 effect's wave
 * projectile (pc_port/game/boot/m34_boss_effect_port.c: child update 0x8018FC54, draw + hit test 0x8018F830).  Its
 * record sits in the effect pool *0x800942E4 (11 slots, stride 0xA0C; 64 six-byte child records at slot+128, byte 0 =
 * callback index into the update table 0x8018FF5C / draw table 0x8018FF48, byte 1 = 1 while live; data at
 * slot+512+(s16)rec[4]).  Every frame the update adds the velocity (data+24/26/28) to the position (data+8/10/12) and 30
 * to the size (data+4); the draw tests Aya's x/z point (aya+42/50) against the quad (+-1024,+-512,0)
 * (func_800C2FF0(64,32)) rotated by the record's matrix (data+36) with its columns scaled by (size,1096,1096) and moved
 * to the position (func_800CEB8C_port.c PE_EffectPointQuadC61A8, two triangles, x/z only).  So the wave is a strip
 * that widens by about 7.5 units a frame while Aya walks about 22: a sidestep at launch gets out of it.
 * The shot rule (func_80021278): a shot always hits only within range (weapon+2, 1071 for the pistol) of the target;
 * beyond that it may graze for 30% (func_80028574).  The tail sits at about (-3354,-109) behind the west wall, so the
 * west part of the arena (x < about -2290) is the only place a shot does full damage.
 * This policy, every frame of free movement (mode 0): reads the live waves, emulates that exact quad test 1..H frames
 * ahead for the eight pad directions and standing still, and takes the move with no predicted hit that best serves the
 * goal (the west wall in the firing band, away from the head).  With the ATB full it heals (HP low, PE >= 60) or fires
 * at the tail (in the band), but only while no wave is in flight, because Aya stands still in the shot animation
 * while the world keeps running (run 4: a contact hit landed mid-shot).  Pad only: guest memory is only read. */
#define PO_WMAX 8
#define PO_WH 26                                 /* frames of wave look-ahead */
typedef struct { pe_addr_t data; int x, z, vx, vz, size, age; int16_t r[9]; } PoWave;

static int PoSat16(int64_t v) { return v < -32768 ? -32768 : v > 32767 ? 32767 : (int)v; }

/* func_800CEB8C_port.c effect_triangle: same sign of the three edge cross products (GTE OP, 16-bit operands) */
static int PoTri(int px, int pz, const int16_t vx[3], const int16_t vz[3])
{
    int32_t c[3];
    for (int e = 0; e < 3; e++) {
        int n = (e + 1) % 3;
        int16_t dx = (int16_t)(vx[n] - vx[e]), dz = (int16_t)(vz[n] - vz[e]);
        int16_t qx = (int16_t)(px - vx[e]), qz = (int16_t)(pz - vz[e]);
        c[e] = (int32_t)(uint32_t)((int64_t)dz * qx - (int64_t)dx * qz);
    }
    return (int32_t)((c[0] ^ c[1]) | (c[1] ^ c[2])) > 0;
}

/* the draw's hit test k frames after the state read now (k = 1: the next draw), Aya at (px,pz) */
static int PoWaveHit(const PoWave *w, int k, int px, int pz)
{
    static const int16_t q[4][2] = {{-1024, -512}, {1024, -512}, {-1024, 512}, {1024, 512}};
    int16_t m[9], ax[3], az[3], bx[3], bz[3], wx[4], wz[4];
    int32_t scale[3];
    int tx = (int16_t)(uint16_t)(w->x + k * w->vx), tz = (int16_t)(uint16_t)(w->z + k * w->vz);
    scale[0] = (int16_t)(uint16_t)(w->size + 30 * k); scale[1] = 1096; scale[2] = 1096;
    for (int i = 0; i < 9; i++) m[i] = (int16_t)((int32_t)((uint32_t)(int32_t)w->r[i] * (uint32_t)scale[i % 3]) >> 12);
    for (int v = 0; v < 4; v++) {
        int ix = PoSat16(((int64_t)m[0] * q[v][0] + (int64_t)m[1] * q[v][1]) >> 12);
        int iz = PoSat16(((int64_t)m[6] * q[v][0] + (int64_t)m[7] * q[v][1]) >> 12);
        wx[v] = (int16_t)((uint32_t)(uint16_t)ix + (uint32_t)tx);
        wz[v] = (int16_t)((uint32_t)(uint16_t)iz + (uint32_t)tz);
    }
    for (int i = 0; i < 3; i++) { ax[i] = wx[i]; az[i] = wz[i]; bx[i] = wx[i + 1]; bz[i] = wz[i + 1]; }
    return PoTri(px, pz, ax, az) || PoTri(px, pz, bx, bz);
}

/* the live wave projectiles of the M34 boss effect (read only) */
static pe_addr_t PoKseg0(pe_addr_t p) { return p && p < 0x200000u ? p | 0x80000000u : p; }   /* the game stores physical pointers too */
static int PoM34Waves(PoWave *w, int maxn)
{
    static int dumped;
    pe_addr_t pool = PoKseg0(PE_LoadU32(0x800942E4u));
    int n = 0;
    if (!dumped) {   /* once: why a scan finds nothing, if it does */
        dumped = 1;
        fprintf(stderr, "[PADONLY] m34w pool raw=%08X canon=%08X ram=%d tables init=%08X,%08X,%08X,%08X,%08X draw=%08X,%08X,%08X,%08X,%08X "
                "update=%08X,%08X,%08X,%08X,%08X\n", PE_LoadU32(0x800942E4u), (unsigned)pool, (int)PE_RangeIsRam(pool, 11u * 0xA0Cu),
                PE_LoadU32(0x8018FF34u), PE_LoadU32(0x8018FF38u), PE_LoadU32(0x8018FF3Cu), PE_LoadU32(0x8018FF40u), PE_LoadU32(0x8018FF44u),
                PE_LoadU32(0x8018FF48u), PE_LoadU32(0x8018FF4Cu), PE_LoadU32(0x8018FF50u), PE_LoadU32(0x8018FF54u), PE_LoadU32(0x8018FF58u),
                PE_LoadU32(0x8018FF5Cu), PE_LoadU32(0x8018FF60u), PE_LoadU32(0x8018FF64u), PE_LoadU32(0x8018FF68u), PE_LoadU32(0x8018FF6Cu));
        if (PE_RangeIsRam(pool, 11u * 0xA0Cu))
            for (unsigned s = 0; s < 11u; s++) {
                pe_addr_t slot = pool + s * 0xA0Cu;
                if (PE_LoadU8(slot))
                    fprintf(stderr, "[PADONLY] m34w slot %u at %08X used=%u code=%02X owner=%08X\n", s, (unsigned)slot, PE_LoadU8(slot),
                            PE_LoadU8(slot + 1u), PE_LoadU32(slot + 8u));
            }
    }
    if (!PE_RangeIsRam(pool, 11u * 0xA0Cu) || !PE_RangeIsRam(0x8018FF48u, 0x30u)) return 0;
    for (unsigned s = 0; s < 11u; s++) {
        pe_addr_t slot = pool + s * 0xA0Cu;
        if (!PE_LoadU8(slot)) continue;
        for (unsigned j = 0; j < 64u; j++) {
            pe_addr_t rec = slot + 128u + j * 6u, data;
            unsigned code;
            if ((int8_t)PE_LoadU8(rec + 1u) != 1) continue;
            code = PE_LoadU8(rec);
            if (code > 4u || PoKseg0(PE_LoadU32(0x8018FF5Cu + code * 4u)) != 0x8018FC54u ||
                PoKseg0(PE_LoadU32(0x8018FF48u + code * 4u)) != 0x8018F830u) continue;
            data = slot + 512u + (uint32_t)(int32_t)(int16_t)PE_LoadU16(rec + 4u);
            if (!PE_RangeIsRam(data, 64u) || (int8_t)PE_LoadU8(data) != 1 || n >= maxn) continue;
            w[n].data = data;
            w[n].x = (int16_t)PE_LoadU16(data + 8u); w[n].z = (int16_t)PE_LoadU16(data + 12u);
            w[n].vx = (int16_t)PE_LoadU16(data + 24u); w[n].vz = (int16_t)PE_LoadU16(data + 28u);
            w[n].size = (int16_t)PE_LoadU16(data + 4u); w[n].age = PE_LoadU8(data + 2u);
            for (int i = 0; i < 9; i++) w[n].r[i] = (int16_t)PE_LoadU16(data + 36u + (unsigned)i * 2u);
            n++;
        }
    }
    return n;
}

/* read-only diagnostic: every actor on the D20C list whose attack action is live (action byte 0 == 1, the state
 * func_8001D268 hits with), with its part spheres (actor+0x234, 12 bytes: x,y,z,part,radius; count = model desc +3,
 * radius scaled by actor+38) — the melee contact pass func_80036448 tests these against Aya's spheres */
static void PoM34Parts(const char *why, int all)
{
    pe_addr_t aya = PE_LoadU32(0x8009D254u);
    int n = 0;
    for (pe_addr_t a = PoKseg0(PE_LoadU32(0x8009D20Cu)); a && n < 24 && PE_RangeIsRam(a, 0x280u); a = PoKseg0(PE_LoadU32(a + 4u)), n++) {
        pe_addr_t rec = PoKseg0(PE_LoadU32(a)), act = 0, desc = PoKseg0(PE_LoadU32(a + 0x1B4u)), sp = PoKseg0(PE_LoadU32(a + 0x234u));
        unsigned cnt = desc && PE_RangeIsRam(desc, 4u) ? PE_LoadU8(desc + 3u) : 0u, scale = PE_LoadU16(a + 38u);
        int live;
        if (rec && PE_RangeIsRam(rec, 0x90u)) act = PoKseg0(PE_LoadU32(rec + 24u));
        live = act && PE_RangeIsRam(act, 16u) && PE_LoadU8(act) == 1u;
        if (!all && !live) continue;
        fprintf(stderr, "[PADONLY] m34a %s f=%d actor=%08X aya=%d type=%u/%u pos=%d,%d rec=%08X recw=%08X act=%d,%u,pw%u kind=%u atkparts=%d,%d,%d,%d "
                "scale=%u parts=%u:", why, g_frame, (unsigned)a, a == aya, PE_LoadU8(a + 12u), PE_LoadU8(a + 13u),
                (int32_t)PE_LoadU32(a + 40u) >> 16, (int32_t)PE_LoadU32(a + 48u) >> 16, (unsigned)rec, rec ? PE_LoadU32(rec) : 0u,
                act && PE_RangeIsRam(act, 16u) ? (int)PE_LoadU8(act) : -1, act && PE_RangeIsRam(act, 16u) ? PE_LoadU8(act + 1u) : 0u,
                act && PE_RangeIsRam(act, 16u) ? PE_LoadU16(act + 12u) : 0u, rec ? (PE_LoadU32(rec) >> 21u) & 7u : 0u,
                rec ? (int8_t)PE_LoadU8(rec + 0x7Cu + ((PE_LoadU32(rec) >> 19u) & 28u)) : -9,
                rec ? (int8_t)PE_LoadU8(rec + 0x7Du + ((PE_LoadU32(rec) >> 19u) & 28u)) : -9,
                rec ? (int8_t)PE_LoadU8(rec + 0x7Eu + ((PE_LoadU32(rec) >> 19u) & 28u)) : -9,
                rec ? (int8_t)PE_LoadU8(rec + 0x7Fu + ((PE_LoadU32(rec) >> 19u) & 28u)) : -9, scale, cnt);
        for (unsigned i = 0; sp && i < cnt && i < 24u && PE_RangeIsRam(sp + i * 12u, 12u); i++)
            fprintf(stderr, " %d:%d,%d,%d,r%d", (int16_t)PE_LoadU16(sp + i * 12u + 6u), (int16_t)PE_LoadU16(sp + i * 12u),
                    (int16_t)PE_LoadU16(sp + i * 12u + 4u), (int16_t)PE_LoadU16(sp + i * 12u + 2u),
                    (int16_t)PE_LoadU16(sp + i * 12u + 8u) * (int)scale / 4096);
        fprintf(stderr, "\n");
    }
}

/* pad -> world direction from the live camera matrix (the transposed form route_reward_sewer_pilot.h proved in M34:
 * Up (sn,c), Right (c,-sn), Down (-sn,-c), Left (-c,sn)); diagonals are two buttons.  Unit length 4096. */
static const uint16_t kPoPad8[8] = {0xFFEFu, 0xFFCFu, 0xFFDFu, 0xFF9Fu, 0xFFBFu, 0xFF3Fu, 0xFF7Fu, 0xFF6Fu};
static void PoPadDirs(int dx[8], int dz[8])
{
    int c = (int16_t)PE_LoadU16(0x800BD000u), sn = (int16_t)PE_LoadU16(0x800BD004u);
    int ux, uz, rx, rz;
    if (!c && !sn) c = 4096;
    ux = sn; uz = c; rx = c; rz = -sn;
    dx[0] = ux; dz[0] = uz; dx[2] = rx; dz[2] = rz; dx[4] = -ux; dz[4] = -uz; dx[6] = -rx; dz[6] = -rz;
    dx[1] = (ux + rx) * 2896 / 4096; dz[1] = (uz + rz) * 2896 / 4096;
    dx[3] = (rx - ux) * 2896 / 4096; dz[3] = (rz - uz) * 2896 / 4096;
    dx[5] = (-ux - rx) * 2896 / 4096; dz[5] = (-uz - rz) * 2896 / 4096;
    dx[7] = (ux - rx) * 2896 / 4096; dz[7] = (uz - rz) * 2896 / 4096;
}

static int PoClampI(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* v3g racetrack: a rectangle inset from the arena walls; its west edge (x = PE_PADONLY_M34_WALL_X) lies in the firing band
 * for every z on it.  Perimeter parameter u runs A(x0,z0) -> B(x1,z0) -> C(x1,z1) -> D(x0,z1) -> A. */
static const int kPoLx0 = -2700, kPoLx1 = -950, kPoLz0 = -600, kPoLz1 = 600;
static int PoLoopLen(void) { return 2 * ((kPoLx1 - kPoLx0) + (kPoLz1 - kPoLz0)); }
static void PoLoopPoint(int u, int *x, int *z)
{
    int w = kPoLx1 - kPoLx0, h = kPoLz1 - kPoLz0, L = PoLoopLen();
    u %= L; if (u < 0) u += L;
    if (u < w) { *x = kPoLx0 + u; *z = kPoLz0; return; }
    u -= w; if (u < h) { *x = kPoLx1; *z = kPoLz0 + u; return; }
    u -= h; if (u < w) { *x = kPoLx1 - u; *z = kPoLz1; return; }
    u -= w; *x = kPoLx0; *z = kPoLz1 - u;
}
static int PoLoopProject(int x, int z)   /* perimeter parameter of the loop point nearest (x,z) */
{
    int best = 0; long long bd = -1;
    for (int u = 0; u < PoLoopLen(); u += 25) {
        int px, pz; long long d;
        PoLoopPoint(u, &px, &pz);
        d = (long long)(px - x) * (px - x) + (long long)(pz - z) * (pz - z);
        if (bd < 0 || d < bd) { bd = d; best = u; }
    }
    return best;
}

static uint16_t PoM34v3(uint16_t base)
{
    static int last_log, last_choice = 8, fire_on, fire_start, fire_tries, fire_dir, last_sel = -1, last_wave_f = -100000,
               last_wave_n, waves_seen, first, wave_ids_n, goal_z = -560, goal_z_f, last_ax, last_az, last_pos_f, hits_seen,
               last_hp = -1, cmd_wait, h_anim_last = -9, h_act_last = -9, h_was_threat, h_strike_end = -100000, atb_full_f,
               loop_dir = 1, loop_flip_f, h_struck, h_strike_end_real = -100000;
    int h_threat = 0, h_kind = -1;
    static pe_addr_t wave_ids[16];
    pe_addr_t aya = PE_LoadU32(0x8009D254u), rec = PE_LoadU32(0x8009D278u), head = 0, tail = 0;
    unsigned mode = PE_LoadU32(0x8009D28Cu), atb, pe, tail_idx = 0xFFu, cmd = PE_LoadU8(0x8009CE3Cu);
    int hp, mxhp, ax, az, hx = 0, hz = 0, tx = -3354, tz = -109, tpx = -3354, tpz = -109, headhp = 0, tailhp = 0, hd, td, nw, best = 8;
    long long bestscore = 0;
    PoWave w[PO_WMAX];
    uint16_t mask = 0xFFFFu;
    const char *state = "free";
    if (!aya || !rec) return base;
    hp = PoHp(rec); mxhp = PE_LoadU16(rec + 28u); atb = PE_LoadU16(rec + 16u); pe = PoPe(rec);
    ax = (int32_t)PE_LoadU32(aya + 40u) >> 16; az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    for (unsigned i = 0; i < 45u; i++) {
        pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
        if (!p) break;
        if (p == aya || !PE_RangeIsRam(p, 640u) || !PE_RangeIsRam(PE_LoadU32(p), 24u)) continue;
        if (PE_LoadU8(p + 12u) == 4u) { tail = p; tail_idx = i; tailhp = (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u); }
        else if (PE_LoadU8(p + 12u) == 3u) { head = p; headhp = (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u); }
    }
    if (head && headhp > 0) { hx = (int32_t)PE_LoadU32(head + 40u) >> 16; hz = (int32_t)PE_LoadU32(head + 48u) >> 16; }
    else head = 0;
    if (tail && tailhp > 0) { tx = (int32_t)PE_LoadU32(tail + 40u) >> 16; tz = (int32_t)PE_LoadU32(tail + 48u) >> 16; }
    else tail = 0;
    hd = head ? PoIsqrt((long long)(hx - ax) * (hx - ax) + (long long)(hz - az) * (hz - az)) : 99999;
    /* the shot rule's distance (func_80030534): the target's point at +0x268/+0x26C against Aya's +0x2A/+0x32 */
    if (tail) { tpx = (int16_t)PE_LoadU16(tail + 0x268u); tpz = (int16_t)PE_LoadU16(tail + 0x26Cu); }
    td = tail ? PoIsqrt((long long)(tpx - ax) * (tpx - ax) + (long long)(tpz - az) * (tpz - az)) : 99999;
    nw = PoM34Waves(w, PO_WMAX);

    if (!first) {   /* one-time read-only dump: known PEs, item slots, weapon and armor words */
        pe_addr_t gun = PE_LoadU32(rec + 0x68u), arm = PE_LoadU32(rec + 0x6Cu);
        int npe = (int32_t)PE_LoadU32(0x8009D040u);
        first = 1;
        fprintf(stderr, "[PADONLY] m34v3 START f=%d hp=%d/%d pe=%u atb=%u lv=%d atk=%u def=%u npe=%d pe_ids=", g_frame, hp, mxhp, pe, atb,
                (int16_t)PE_LoadU16(rec + 4u), PE_LoadU16(rec + 0x1Eu), PE_LoadU16(rec + 0x20u), npe);
        for (int i = 0; i < npe && i < 24; i++) fprintf(stderr, "%s%d", i ? "," : "", (int16_t)PE_LoadU16(0x800A1D9Cu + (unsigned)i * 2u));
        fprintf(stderr, " items=");
        for (unsigned i = 0; i < 20u; i++) fprintf(stderr, "%s%08X", i ? "," : "", PE_LoadU32(0x800C0E48u + i * 4u));
        fprintf(stderr, " slot=%u tail=%d,%d tail_pt=%d,%d\n", PE_LoadU8(0x800C0E20u), tx, tz, tpx, tpz);
        if (gun && PE_RangeIsRam(gun, 24u))
            fprintf(stderr, "[PADONLY] m34v3 weapon=%08X atk=%d range=%d kind=%d shotf=%u ammo=%08X flags=%08X w14=%08X\n", (unsigned)gun,
                    (int16_t)PE_LoadU16(gun), (int16_t)PE_LoadU16(gun + 2u), (int16_t)PE_LoadU16(gun + 6u), PE_LoadU32(gun + 8u),
                    PE_LoadU32(gun + 12u), PE_LoadU32(gun + 16u), PE_LoadU32(gun + 20u));
        if (arm && PE_RangeIsRam(arm, 8u))
            fprintf(stderr, "[PADONLY] m34v3 armor=%08X w0=%08X w4=%08X\n", (unsigned)arm, PE_LoadU32(arm), PE_LoadU32(arm + 4u));
    }
    /* wave bookkeeping + read-only wave trace (spawn, every 2 frames while live) */
    for (int i = 0; i < nw; i++) {
        int known = 0;
        for (int j = 0; j < wave_ids_n; j++) if (wave_ids[j] == w[i].data) known = 1;
        if (!known || w[i].age <= 1) {
            if (!known) {
                if (wave_ids_n < 16) wave_ids[wave_ids_n++] = w[i].data;
                else { memmove(wave_ids, wave_ids + 1, sizeof wave_ids - sizeof wave_ids[0]); wave_ids[15] = w[i].data; }
                waves_seen++;
                last_wave_f = g_frame;
                fprintf(stderr, "[PADONLY] m34w NEW f=%d n=%d data=%08X pos=%d,%d vel=%d,%d size=%d age=%d r=%d,%d,%d,%d,%d,%d,%d,%d,%d "
                        "aya=%d,%d head=%d,%d mode=%u cmd=%u hp=%d\n", g_frame, waves_seen, (unsigned)w[i].data, w[i].x, w[i].z,
                        w[i].vx, w[i].vz, w[i].size, w[i].age, w[i].r[0], w[i].r[1], w[i].r[2], w[i].r[3], w[i].r[4], w[i].r[5],
                        w[i].r[6], w[i].r[7], w[i].r[8], ax, az, hx, hz, mode, cmd, hp);
            }
        }
        if (g_frame % 2 == 0)
            fprintf(stderr, "[PADONLY] m34w f=%d data=%08X pos=%d,%d size=%d age=%d aya=%d,%d hit_now=%d hit1=%d mode=%u\n", g_frame,
                    (unsigned)w[i].data, w[i].x, w[i].z, w[i].size, w[i].age, ax, az, PoWaveHit(&w[i], 0, ax, az),
                    PoWaveHit(&w[i], 1, ax, az), mode);
    }
    if (nw != last_wave_n) { if (!nw) fprintf(stderr, "[PADONLY] m34w GONE f=%d hp=%d aya=%d,%d\n", g_frame, hp, ax, az); last_wave_n = nw; }
    {
        static int parts_all_done;
        if (head && !parts_all_done) { parts_all_done = 1; PoM34Parts("init", 1); }
        PoM34Parts("live", 0);
    }
    /* the alligator's attack sequence (run 2/3 traces): idle 2 / walk 4,17 -> turn 5 -> windup 8 (or 16) -> strike 9,11,12 with
     * its action byte 0 == 1 for 3-8 frames; the tail spin lays parts 48-51 on an arc ~500-680 from the body root */
    {
        pe_addr_t hb = head ? PoKseg0(PE_LoadU32(head)) : 0, ha = hb && PE_RangeIsRam(hb, 32u) ? PoKseg0(PE_LoadU32(hb + 24u)) : 0;
        unsigned an = head ? PE_LoadU8(head + 14u) : 0u;
        int act = ha && PE_RangeIsRam(ha, 16u) ? (int)PE_LoadU8(ha) : -1;
        int kind = hb && PE_RangeIsRam(hb, 32u) ? (int)((PE_LoadU32(hb) >> 21u) & 7u) : -1;
        h_threat = head && (an == 5u || an == 8u || an == 9u || an == 11u || an == 12u || an == 16u || act == 1);
        h_kind = head ? kind : -1;
        if (h_threat && (act == 1 || an == 9u || an == 11u || an == 12u || an == 13u)) h_struck = 1;   /* PE_PADONLY_M34_SAFEFIRE bookkeeping */
        if (head && (int)an != h_anim_last || act != h_act_last) {
            fprintf(stderr, "[PADONLY] m34h f=%d anim=%u act=%d kind=%d rot=%u root=%d,%d aya=%d,%d hd=%d threat=%d\n", g_frame, an, act, kind,
                    PE_LoadU16(head + 0x3Au), hx, hz, ax, az, hd, h_threat);
            if (h_was_threat && !h_threat) {
                h_strike_end = g_frame;
                if (h_struck) h_strike_end_real = g_frame;   /* the sequence really struck (run 2: a 1-frame idle between windup and strike) */
                h_struck = 0;
            }
            h_anim_last = (int)an; h_act_last = act;
        }
        h_was_threat = h_threat;
    }
    if (last_hp >= 0 && hp < last_hp) {
        PoM34Parts("hit", 1);
        hits_seen++;
        fprintf(stderr, "[PADONLY] m34v3 HIT f=%d hp=%d (%+d) waves=%d hd=%d aya=%d,%d head=%d,%d mode=%u cmd=%u\n", g_frame, hp, hp - last_hp,
                nw, hd, ax, az, hx, hz, mode, cmd);
    }
    last_hp = hp;

    if (mode == 1u) {   /* command menu (heal) or target list (fire) */
        static const uint16_t cyc[4] = {0xFFDFu, 0xFF7Fu, 0xFFBFu, 0xFFEFu};   /* Right moved the selection 0->1 in run 4 */
        unsigned sel = PE_LoadU8(0x8009CE44u);
        int threat = 0;
        if (g_po_bh.on) return PoBattleHeal(hp, pe);
        /* the target list assigns one shot per Cross (0x8009CE3C counts them; run 4: 1 after the first, the list closed
         * after the second), so keep committing while firing; without a fire intent never cancel a queued command */
        if (!fire_on && cmd) return 0xFFFFu;
        for (int i = 0; i < nw && !threat; i++)
            for (int k = 0; k <= PO_WH && !threat; k++) threat = PoWaveHit(&w[i], k, ax, az);
        if (fire_on && threat && !cmd) {   /* a wave would hit her while she stands in the shot: back out and dodge */
            fire_on = 0;
            fprintf(stderr, "[PADONLY] m34v3 FIRE_ABORT f=%d waves=%d sel=%u\n", g_frame, nw, sel);
        }
        if (!fire_on || tail_idx == 0xFFu) { if (g_frame % 8 == 3) mask = 0xDFFFu; state = "menu-cancel"; }
        else if ((int)sel != (int)tail_idx) {
            if ((int)sel != last_sel) { last_sel = (int)sel; fire_tries = 0; }
            if (g_frame % 8 == 3) { if (++fire_tries > 3) { fire_tries = 0; fire_dir = (fire_dir + 1) % 4; } mask = cyc[fire_dir]; }
            state = "menu-select";
        } else { if (g_frame % 8 == 3) mask = 0xBFFFu; state = "menu-commit"; }
        if (g_frame - last_log >= 10) {
            last_log = g_frame;
            fprintf(stderr, "[PADONLY] m34v3 f=%d mode=1 sel=%u tidx=%u hp=%d atb=%u pe=%u waves=%d state=%s pad=%04X\n", g_frame, sel,
                    tail_idx, hp, atb, pe, nw, state, (unsigned)mask);
        }
        return mask;
    }
    if (mode != 0u) { if (g_frame % 32 == 3) mask = 0xBFFFu; return mask; }   /* intro / outro text: Cross to advance */
    if (g_po_bh.on) return PoBattleHeal(hp, pe);
    if (cmd) {   /* a committed command is playing (Aya stands still); do not touch the pad */
        fire_on = 0; cmd_wait = g_frame;
        if (g_frame - last_log >= 10) {
            last_log = g_frame;
            fprintf(stderr, "[PADONLY] m34v3 f=%d mode=0 cmd=%u hp=%d atb=%u pe=%u aya=%d,%d hd=%d td=%d tailhp=%d waves=%d state=command\n",
                    g_frame, cmd, hp, atb, pe, ax, az, hd, td, tailhp, nw);
        }
        return 0xFFFFu;
    }
    if (fire_on && (g_frame - fire_start > 60 || nw)) {   /* the list never opened, or a wave came: dodge first, retry later */
        fprintf(stderr, "[PADONLY] m34v3 FIRE_DROP f=%d waves=%d age=%d\n", g_frame, nw, g_frame - fire_start);
        fire_on = 0;
    }

    /* ---- decide heal / fire: only with the ATB full, no wave in flight and no attack sequence running ---- */
    if (atb < 9000u) atb_full_f = 0; else if (!atb_full_f) atb_full_f = g_frame;
    if (atb >= 9000u && !nw && !h_threat && g_frame - cmd_wait > 4) {
        if (hp > 0 && hp <= g_po_m34_heal_hp && pe >= 60u && g_frame - g_po_bh.last > 120) {
            g_po_bh.on = 1; g_po_bh.start = g_frame;
            fprintf(stderr, "[PADONLY] m34v3 HEAL f=%d hp=%d pe=%u hd=%d td=%d aya=%d,%d last_wave=%d\n", g_frame, hp, pe, hd, td, ax, az,
                    g_frame - last_wave_f);
            return 0xEFFFu;
        }
        if (tail && td <= g_po_m34_v3_fire_d && (hp > g_po_m34_heal_hp || pe < 60u) && !fire_on &&
            (g_po_m34_safefire > 0   /* PE_PADONLY_M34_SAFEFIRE: the window follows a real strike, and never with the root this close */
                 ? (g_frame - h_strike_end_real <= 70 || hd >= g_po_m34_fire_root) && hd >= g_po_m34_safefire
                 : (g_frame - h_strike_end <= 70 || hd >= g_po_m34_fire_root))) {   /* fire_root default 1000 */
            fire_on = 1; fire_start = g_frame; fire_tries = 0; last_sel = -1;
            fprintf(stderr, "[PADONLY] m34v3 FIRE f=%d hp=%d td=%d hd=%d aya=%d,%d tailhp=%d last_wave=%d\n", g_frame, hp, td, hd, ax, az,
                    tailhp, g_frame - last_wave_f);
        }
    }
    if (fire_on) {   /* open the target list: Cross (the world keeps running until the list is up) */
        if (g_frame % 4 == 1) mask = 0xBFFFu;
        state = "fire";
    } else {
        /* ---- movement: emulate every live wave's hit test for the eight directions and standing still ---- */
        int dx[8], dz[8], gx = g_po_m34_wall_x, gz;
        const int x_lo = -2800, x_hi = -780, z_lo = -720, z_hi = 720;
        const int wfl = g_po_m34_wflee > 0 && h_threat && (h_kind == 4 || h_kind == 5);   /* 0 unless PE_PADONLY_M34_WFLEE is set */
        PoPadDirs(dx, dz);
        /* racetrack kite: follow the loop away from the body root; flip when the root sits ahead on the loop */
        {
            int ua = PoLoopProject(ax, az), ur = head ? PoLoopProject(hx, hz) : ua + PoLoopLen() / 2, L = PoLoopLen();
            int ahead = ((ur - ua) * loop_dir % L + L) % L;   /* loop distance from Aya to the root in her direction */
            if (g_po_m34_flipfar > 0) {
                /* PE_PADONLY_M34_FLIPFAR=<d>: with the root within d, run the loop in the direction whose point 500 ahead lies
                 * farther from the root (run 2: the head walked in from the loop's interior, which the projection rule below does
                 * not see, and pinned her in the NW corner) */
                int fx, fz, bx, bz, df, db;
                PoLoopPoint(ua + loop_dir * 500, &fx, &fz);
                PoLoopPoint(ua - loop_dir * 500, &bx, &bz);
                df = PoIsqrt((long long)(fx - hx) * (fx - hx) + (long long)(fz - hz) * (fz - hz));
                db = PoIsqrt((long long)(bx - hx) * (bx - hx) + (long long)(bz - hz) * (bz - hz));
                if (head && hd < g_po_m34_flipfar && db > df + 100 && g_frame - loop_flip_f > 45) {
                    loop_dir = -loop_dir; loop_flip_f = g_frame;
                    fprintf(stderr, "[PADONLY] m34v3 loop flipfar f=%d dir=%d aya=%d,%d root=%d,%d hd=%d d_fwd=%d d_back=%d\n", g_frame, loop_dir,
                            ax, az, hx, hz, hd, df, db);
                }
            } else if (head && ahead < 900 && hd < 1200 && g_frame - loop_flip_f > 45) {
                loop_dir = -loop_dir; loop_flip_f = g_frame;
                fprintf(stderr, "[PADONLY] m34v3 loop flip f=%d dir=%d aya=%d,%d root=%d,%d hd=%d ahead=%d\n", g_frame, loop_dir, ax, az, hx, hz,
                        hd, ahead);
            }
            PoLoopPoint(ua + loop_dir * 350, &gx, &gz);
        }
        (void)goal_z; (void)goal_z_f;
        for (int c = 0; c <= 8; c++) {
            int vx = c < 8 ? dx[c] * g_po_m34_speed / 4096 : 0, vz = c < 8 ? dz[c] * g_po_m34_speed / 4096 : 0;
            int hit_k = 0;
            long long s;
            /* the draw of frame j tests the wave state j updates after the one read now (run 4: wave 4 hit at 64223 against
             * the position read at 64222, i.e. draw before update) against Aya after j or j+1 steps: test both */
            for (int k = 0; k <= PO_WH && !hit_k && nw; k++) {
                int px = PoClampI(ax + vx * (k + 1), x_lo - 40, x_hi + 40), pz = PoClampI(az + vz * (k + 1), z_lo - 60, z_hi + 60);
                int qx = PoClampI(ax + vx * k, x_lo - 40, x_hi + 40), qz = PoClampI(az + vz * k, z_lo - 60, z_hi + 60);
                for (int i = 0; i < nw; i++)
                    if (PoWaveHit(&w[i], k, px, pz) || PoWaveHit(&w[i], k, qx, qz)) { hit_k = k + 1; break; }
            }
            {
                int px = ax + vx * 8, pz = az + vz * 8, ox = 0, oz = 0, hd8;
                long long gd = PoIsqrt((long long)(gx - px) * (gx - px) + (long long)(gz - pz) * (gz - pz));
                if (px < x_lo) ox = x_lo - px; else if (px > x_hi) ox = px - x_hi;
                if (pz < z_lo) oz = z_lo - pz; else if (pz > z_hi) oz = pz - z_hi;
                hd8 = head ? PoIsqrt((long long)(hx - px) * (hx - px) + (long long)(hz - pz) * (hz - pz)) : 99999;
                {
                    /* follow the loop (goal point 350 ahead), keep clear of the body root (contact < ~300, stomp at the root,
                     * tail spin inside ~930: costs grow toward the root), walls cost */
                    s = -gd - 12LL * (ox + oz) + (c == last_choice ? 25 : 0)
                        - (hd8 < 500 ? 8LL * (500 - hd8) : 0) - (hd8 < g_po_m34_keep ? 2LL * (g_po_m34_keep - hd8) : 0);
                    if (h_threat && hd8 < g_po_m34_keep) s -= 3LL * (g_po_m34_keep - hd8);   /* an attack is coming: distance first */
                    if (wfl) s += (long long)g_po_m34_wflee * (hd8 < 1800 ? hd8 : 1800);   /* PE_PADONLY_M34_WFLEE: run from a wave windup */
                }
                if (hit_k) s -= 1000000LL - 10000LL * hit_k;
            }
            if (c == 0 || s > bestscore) { bestscore = s; best = c; }
        }
        last_choice = best;
        mask = best < 8 ? kPoPad8[best] : 0xFFFFu;
        state = bestscore < -500000 ? "dodge-fail" : nw ? "dodge" : wfl ? "wflee" : td <= g_po_m34_v3_fire_d ? "band" : "go";
    }
    if (g_frame - last_log >= 10 || (nw && g_frame - last_log >= 2)) {
        int mvx = ax - last_ax, mvz = az - last_az, mf = g_frame - last_pos_f;
        last_log = g_frame;
        pe_addr_t hb = head ? PoKseg0(PE_LoadU32(head)) : 0, ha = hb && PE_RangeIsRam(hb, 32u) ? PoKseg0(PE_LoadU32(hb + 24u)) : 0;
        fprintf(stderr, "[PADONLY] m34v3 f=%d mode=0 hp=%d atb=%u pe=%u aya=%d,%d mv=%d,%d/%d head=%d,%d hd=%d td=%d tp=%d,%d tailhp=%d "
                "waves=%d state=%s pick=%d score=%lld pad=%04X hanim=%u hrot=%u hact=%d,%u\n", g_frame, hp, atb, pe, ax, az, mvx, mvz, mf, hx,
                hz, hd, td, tpx, tpz, tailhp, nw, state, best, bestscore, (unsigned)mask, head ? PE_LoadU8(head + 14u) : 0u,
                head ? PE_LoadU16(head + 0x3Au) : 0u, ha && PE_RangeIsRam(ha, 16u) ? (int)PE_LoadU8(ha) : -1,
                ha && PE_RangeIsRam(ha, 16u) ? PE_LoadU16(ha + 12u) : 0u);
        last_ax = ax; last_az = az; last_pos_f = g_frame;
    }
    return mask;
}

/* M34 phase 2 (sewer tunnel A8064448; the switch skips RouteBattleShim's HP lock, so it is a real fight): the type-2
 * alligator.  Pad only, same pieces as v3: its attack sequence from its anim/action state (traced on change), hold
 * 750-1000 away and back off during an attack, fire with the ATB full only while no attack runs (right after one, or from
 * 700+), heal at <= 34 HP.  Run 7 (485ee828c): RouteBattleShim's 300-600 band took three 23-24 HP hits and Aya died
 * with the enemy 42 HP short.  PE_PADONLY_P2=shim keeps RouteBattleShim (with the PE heal only). */
static uint16_t PoM34Phase2(uint16_t mask)
{
    static int fire_on, fire_start, fire_tries, fire_dir, last_sel = -1, e_anim_last = -9, e_act_last = -9, e_was_threat,
               e_strike_end = -100000, last_log, last_hp = -1, last_choice = 8, cmd_wait, first;
    pe_addr_t rec = PE_LoadU32(0x8009D278u), aya = PE_LoadU32(0x8009D254u), en = 0;
    unsigned mode = PE_LoadU32(0x8009D28Cu), atb, pe, cmd = PE_LoadU8(0x8009CE3Cu), en_idx = 0xFFu;
    int hp, ax, az, ex = 0, ez = 0, epx = 0, epz = 0, ed = 99999, etd = 99999, threat = 0, best = 8;
    long long bd = -1, bestscore = 0;
    const char *state = "free";
    if (!rec || !aya) return mask;
    hp = PoHp(rec); atb = PE_LoadU16(rec + 16u); pe = PoPe(rec);
    ax = (int32_t)PE_LoadU32(aya + 40u) >> 16; az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    for (unsigned i = 0; i < 45u; i++) {   /* the nearest living type-2 enemy (the type-3 actor here is the inert far one, run 8) */
        pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
        int x, z; long long d;
        if (!p) break;
        if (p == aya || !PE_RangeIsRam(p, 640u) || !PE_RangeIsRam(PE_LoadU32(p), 24u) || (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u) <= 0 ||
            PE_LoadU8(p + 12u) != 2u) continue;
        x = (int32_t)PE_LoadU32(p + 40u) >> 16; z = (int32_t)PE_LoadU32(p + 48u) >> 16;
        d = (long long)(x - ax) * (x - ax) + (long long)(z - az) * (z - az);
        if (bd < 0 || d < bd) { bd = d; en = p; en_idx = i; ex = x; ez = z; }
    }
    if (en) {
        pe_addr_t eb = PoKseg0(PE_LoadU32(en)), ea = eb && PE_RangeIsRam(eb, 32u) ? PoKseg0(PE_LoadU32(eb + 24u)) : 0;
        unsigned an = PE_LoadU8(en + 14u);
        int act = ea && PE_RangeIsRam(ea, 16u) ? (int)PE_LoadU8(ea) : -1, kind = eb ? (int)((PE_LoadU32(eb) >> 21u) & 7u) : -1;
        ed = PoIsqrt(bd);
        epx = (int16_t)PE_LoadU16(en + 0x268u); epz = (int16_t)PE_LoadU16(en + 0x26Cu);
        etd = PoIsqrt((long long)(epx - ax) * (epx - ax) + (long long)(epz - az) * (epz - az));
        threat = an == 5u || an == 8u || an == 9u || an == 11u || an == 12u || an == 13u || an == 16u || act == 1;
        if ((int)an != e_anim_last || act != e_act_last) {
            fprintf(stderr, "[PADONLY] p2e f=%d anim=%u act=%d kind=%d pw=%u type=%u pos=%d,%d aya=%d,%d ed=%d threat=%d hp=%d\n", g_frame, an,
                    act, kind, ea && PE_RangeIsRam(ea, 16u) ? PE_LoadU16(ea + 12u) : 0u, PE_LoadU8(en + 12u), ex, ez, ax, az, ed, threat, hp);
            if (e_was_threat && !threat) e_strike_end = g_frame;
            e_anim_last = (int)an; e_act_last = act;
        }
        e_was_threat = threat;
    }
    if (!en) {   /* the type-2 alligator from the D20C actor list (it joins the 0x8009E000 target list only later, run 9) */
        int n = 0;
        for (pe_addr_t a = PoKseg0(PE_LoadU32(0x8009D20Cu)); a && n < 32 && PE_RangeIsRam(a, 0x280u); a = PoKseg0(PE_LoadU32(a + 4u)), n++)
            if (a != aya && PE_LoadU8(a + 12u) == 2u) {
                pe_addr_t eb = PoKseg0(PE_LoadU32(a)), ea = eb && PE_RangeIsRam(eb, 32u) ? PoKseg0(PE_LoadU32(eb + 24u)) : 0;
                unsigned an = PE_LoadU8(a + 14u);
                int act = ea && PE_RangeIsRam(ea, 16u) ? (int)PE_LoadU8(ea) : -1;
                ex = (int32_t)PE_LoadU32(a + 40u) >> 16; ez = (int32_t)PE_LoadU32(a + 48u) >> 16;
                ed = PoIsqrt((long long)(ex - ax) * (ex - ax) + (long long)(ez - az) * (ez - az));
                threat = an == 5u || an == 8u || an == 9u || an == 11u || an == 12u || an == 13u || an == 16u || act == 1;
                if ((int)an != e_anim_last || act != e_act_last) {
                    fprintf(stderr, "[PADONLY] p2e f=%d anim=%u act=%d kind=%d pw=%u type=2 (actor list) pos=%d,%d aya=%d,%d ed=%d threat=%d "
                            "hp=%d\n", g_frame, an, act, eb ? (int)((PE_LoadU32(eb) >> 21u) & 7u) : -1,
                            ea && PE_RangeIsRam(ea, 16u) ? PE_LoadU16(ea + 12u) : 0u, ex, ez, ax, az, ed, threat, hp);
                    if (e_was_threat && !threat) e_strike_end = g_frame;
                    e_anim_last = (int)an; e_act_last = act;
                }
                e_was_threat = threat;
                break;
            }
    }
    if (!first) { first = 1; fprintf(stderr, "[PADONLY] p2 START f=%d hp=%d pe=%u atb=%u aya=%d,%d\n", g_frame, hp, pe, atb, ax, az); }
    if (last_hp >= 0 && hp < last_hp) {
        PoM34Parts("p2hit", 1);
        fprintf(stderr, "[PADONLY] p2 HIT f=%d hp=%d (%+d) ed=%d aya=%d,%d en=%d,%d mode=%u cmd=%u\n", g_frame, hp, hp - last_hp, ed, ax, az, ex,
                ez, mode, cmd);
    }
    last_hp = hp;
    if (g_po_p2_shim) {   /* RouteBattleShim's movement; its Cross held back while the alligator attacks; the PE heal */
        if (g_po_bh.on) { if (mode == 0u || mode == 1u) return PoBattleHeal(hp, pe); g_po_bh.on = 0; }
        if (mode == 0u && hp > 0 && hp <= g_po_p2_heal_hp && atb >= 9000u && pe >= 60u && !cmd && !threat && g_frame - g_po_bh.last > 120) {
            g_po_bh.on = 1; g_po_bh.start = g_frame;
            fprintf(stderr, "[PADONLY] p2 HEAL f=%d hp=%d pe=%u ed=%d\n", g_frame, hp, pe, ed);
            return 0xEFFFu;
        }
        if (mode == 0u && threat && !(mask & 0x4000u)) {   /* no volley (Aya stands still ~50 frames) into its strike */
            static int last_gate;
            if (g_frame - last_gate >= 30) { last_gate = g_frame; fprintf(stderr, "[PADONLY] p2 HOLD_FIRE f=%d ed=%d\n", g_frame, ed); }
            mask |= 0x4000u;
        }
        return mask;
    }
    if (mode == 1u) {
        static const uint16_t cyc[4] = {0xFFDFu, 0xFF7Fu, 0xFFBFu, 0xFFEFu};
        unsigned sel = PE_LoadU8(0x8009CE44u);
        uint16_t m = 0xFFFFu;
        if (g_po_bh.on) return PoBattleHeal(hp, pe);
        if (!fire_on && cmd) return 0xFFFFu;
        if (!fire_on || en_idx == 0xFFu) { if (g_frame % 8 == 3) m = 0xDFFFu; }
        else if (sel != en_idx) {
            if ((int)sel != last_sel) { last_sel = (int)sel; fire_tries = 0; }
            if (g_frame % 8 == 3) { if (++fire_tries > 3) { fire_tries = 0; fire_dir = (fire_dir + 1) % 4; } m = cyc[fire_dir]; }
        } else if (g_frame % 8 == 3) m = 0xBFFFu;
        return m;
    }
    if (mode != 0u) { if (g_frame % 32 == 3) return 0xBFFFu; return 0xFFFFu; }
    if (g_po_bh.on) return PoBattleHeal(hp, pe);
    if (cmd) { fire_on = 0; cmd_wait = g_frame; return 0xFFFFu; }
    if (fire_on && (g_frame - fire_start > 60 || threat)) fire_on = 0;
    if (atb >= 9000u && !threat && en && g_frame - cmd_wait > 4) {
        if (hp > 0 && hp <= 34 && pe >= 60u && g_frame - g_po_bh.last > 120) {
            g_po_bh.on = 1; g_po_bh.start = g_frame;
            fprintf(stderr, "[PADONLY] p2 HEAL f=%d hp=%d pe=%u ed=%d\n", g_frame, hp, pe, ed);
            return 0xEFFFu;
        }
        if (!fire_on && etd <= 1040 && (g_frame - e_strike_end <= 70 || ed >= 700)) {
            fire_on = 1; fire_start = g_frame; fire_tries = 0; last_sel = -1;
            fprintf(stderr, "[PADONLY] p2 FIRE f=%d hp=%d ed=%d etd=%d ehp=%d since_strike=%d\n", g_frame, hp, ed, etd,
                    (int32_t)PE_LoadU32(PE_LoadU32(en) + 16u), g_frame - e_strike_end);
        }
    }
    if (fire_on) { state = "fire"; if (g_frame % 4 == 1) return 0xBFFFu; return 0xFFFFu; }
    if (!en) { state = "wait"; mask = 0xFFFFu; }   /* the alligator has not come out yet: hold the east end */
    else {
        int dx[8], dz[8];
        const int x_lo = 700, x_hi = 2550, z_lo = -640, z_hi = 170;   /* tunnel extents seen in run 7 and the aided route */
        PoPadDirs(dx, dz);
        for (int c = 0; c <= 8; c++) {
            int vx = c < 8 ? dx[c] * g_po_m34_speed / 4096 : 0, vz = c < 8 ? dz[c] * g_po_m34_speed / 4096 : 0;
            int px = ax + vx * 8, pz = az + vz * 8, ox = 0, oz = 0, d8 = 99999;
            long long sc;
            if (px < x_lo) ox = x_lo - px; else if (px > x_hi) ox = px - x_hi;
            if (pz < z_lo) oz = z_lo - pz; else if (pz > z_hi) oz = pz - z_hi;
            if (en) d8 = PoIsqrt((long long)(ex - px) * (ex - px) + (long long)(ez - pz) * (ez - pz));
            if (threat) sc = 3LL * (d8 < 1100 ? d8 : 1100);                    /* attack running: distance first */
            else sc = -2LL * (d8 < 750 ? 750 - d8 : d8 > 1000 ? d8 - 1000 : 0);   /* hold 750-1000 */
            sc += -12LL * (ox + oz) + (c == last_choice ? 25 : 0) - (c == 8 ? 10 : 0);
            if (c == 0 || sc > bestscore) { bestscore = sc; best = c; }
        }
        last_choice = best;
        mask = best < 8 ? kPoPad8[best] : 0xFFFFu;
        state = threat ? "evade" : "hold";
    }
    if (g_frame - last_log >= 10) {
        last_log = g_frame;
        fprintf(stderr, "[PADONLY] p2 f=%d hp=%d atb=%u pe=%u aya=%d,%d en=%d,%d ed=%d etd=%d state=%s pad=%04X\n", g_frame, hp, atb, pe, ax, az,
                ex, ez, ed, etd, state, (unsigned)mask);
    }
    return mask;
}

#include "route_padonly_day2.h"

static uint16_t RoutePadOnlyM34(uint16_t mask)
{
    uint32_t tok = (uint32_t)GA_TOKEN;
    if (g_route_aids_pad_only && getenv("PE_PADONLY_D2WALK") &&
        !strcmp(getenv("PE_PADONLY_D2WALK"), "1")) mask = RoutePadOnlyDay2(mask);
    if (!g_route_aids_pad_only || !g_po_m34) return mask;
    if (!(PE_LoadU32(0x8009D1A0u) & 2u)) return mask;
    if (tok == 0xA8064448u && !g_po_m34_v2) { mask = PoM34Phase2(mask); g_po_last_mask = mask; return mask; }
    if (tok != 0xA8003248u) return mask;
    mask = g_po_m34_v2 ? PoM34(mask) : PoM34v3(mask);
    g_po_last_mask = mask;
    return mask;
}

/* ---- read-only trace, once per presented frame (after g_frame++) ---------------------------------------------
 * room lines (HP/PE/ATB/ammo at every room change), HP-change lines inside battles with the nearest enemy, a
 * 45-frame battle heartbeat with up to four enemies, and one summary line per fight (min/peak HP, HP jumps).
 * Prefix [PADONLY]: neither [ROUTE] nor [ROUTE_AIDS], and no marker text the checkers key on. */
static void PoScanEnemies(pe_addr_t aya, int *n, long long *nd2, int *nhp, int ex[4], int ez[4], int ehp[4], unsigned et[4])
{
    int ax = (int32_t)PE_LoadU32(aya + 40u) >> 16, az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    *n = 0; *nd2 = -1; *nhp = 0;
    for (unsigned i = 0; i < 45u; i++) {
        pe_addr_t p = PE_LoadU32(0x8009E000u + i * 12u);
        int x, z, hp; long long d;
        if (!p) break;
        if (p == aya || !PE_RangeIsRam(p, 640u) || !PE_RangeIsRam(PE_LoadU32(p), 24u)) continue;
        hp = (int32_t)PE_LoadU32(PE_LoadU32(p) + 16u);
        if (hp <= 0) continue;
        x = (int32_t)PE_LoadU32(p + 40u) >> 16; z = (int32_t)PE_LoadU32(p + 48u) >> 16;
        d = (long long)(x - ax) * (x - ax) + (long long)(z - az) * (z - az);
        if (*n < 4) { ex[*n] = x; ez[*n] = z; ehp[*n] = hp; et[*n] = PE_LoadU8(p + 12u); }
        (*n)++;
        if (*nd2 < 0 || d < *nd2) { *nd2 = d; *nhp = hp; }
    }
}

static void RoutePadOnlyTrace(void)
{
    static uint32_t last_tok = 0xFFFFFFFFu;
    static int last_hp = -9999, bon, bstart, bhp0, bmin, bpeak, bjumps, blogged, last_beat;
    pe_addr_t rec = PE_LoadU32(0x8009D278u), aya = PE_LoadU32(0x8009D254u);
    uint32_t tok = (uint32_t)GA_TOKEN;
    unsigned d1a0 = PE_LoadU32(0x8009D1A0u), mode = PE_LoadU32(0x8009D28Cu);
    int hp, ax = 0, az = 0, n = 0, nhp = 0, ex[4] = {0}, ez[4] = {0}, ehp[4] = {0};
    unsigned et[4] = {0}, mx;
    long long nd2 = -1;

    if (!g_route_aids_pad_only || !rec || !aya) return;
    hp = PoHp(rec); mx = PE_LoadU16(rec + 28u);
    ax = (int32_t)PE_LoadU32(aya + 40u) >> 16; az = (int32_t)PE_LoadU32(aya + 48u) >> 16;
    if (tok != last_tok) {
        pe_addr_t gun = PE_LoadU32(rec + 104u);
        last_tok = tok;
        fprintf(stderr, "[PADONLY] room f=%d tok=%08X story=%X mode=%u d1a0=%X hp=%d/%u pe=%u atb=%u loaded=%u reserve=%u "
                "item4=%u slot=%u pos=%d,%d\n", g_frame, (unsigned)tok, PE_LoadU32(0x800A7918u), mode, d1a0, hp, mx,
                PoPe(rec), PE_LoadU16(rec + 16u), gun ? (PE_LoadU32(gun + 12u) & 0x3FFu) : 0u, PE_LoadU16(0x800A1E6Eu),
                PE_LoadU16(0x800C0E50u), PE_LoadU8(0x800C0E20u), ax, az);
    }
    if (d1a0 & 2u) {
        PoScanEnemies(aya, &n, &nd2, &nhp, ex, ez, ehp, et);
        if (!bon) {
            bon = 1; blogged = 0; bstart = g_frame; bhp0 = bmin = bpeak = hp; bjumps = 0; last_hp = hp; last_beat = g_frame;
            fprintf(stderr, "[PADONLY] fight START f=%d tok=%08X hp=%d/%u pe=%u atb=%u n=%d\n", g_frame, (unsigned)tok, hp, mx,
                    PoPe(rec), PE_LoadU16(rec + 16u), n);
        }
        if (hp != last_hp) {
            if (hp > last_hp + 6) bjumps++;
            if (hp < bmin) bmin = hp;
            if (hp > bpeak) bpeak = hp;
            fprintf(stderr, "[PADONLY] hp f=%d tok=%08X mode=%u hp=%d (%+d) atb=%u pe=%u aya=%d,%d n=%d near_d2=%lld near_hp=%d "
                    "pad=%04X cmd=%u\n", g_frame, (unsigned)tok, mode, hp, hp - last_hp, PE_LoadU16(rec + 16u), PoPe(rec), ax, az, n,
                    nd2, nhp, (unsigned)g_po_last_mask, PE_LoadU8(0x8009CE3Cu));
            last_hp = hp;
        }
        if (g_frame - last_beat >= 45) {
            last_beat = g_frame;
            fprintf(stderr, "[PADONLY] btl f=%d tok=%08X mode=%u hp=%d atb=%u pe=%u aya=%d,%d n=%d "
                    "e0=%d,%d,%d,t%u e1=%d,%d,%d,t%u e2=%d,%d,%d,t%u e3=%d,%d,%d,t%u pad=%04X cmd=%u\n", g_frame, (unsigned)tok,
                    mode, hp, PE_LoadU16(rec + 16u), PoPe(rec), ax, az, n, ex[0], ez[0], ehp[0], et[0], ex[1], ez[1], ehp[1], et[1],
                    ex[2], ez[2], ehp[2], et[2], ex[3], ez[3], ehp[3], et[3], (unsigned)g_po_last_mask, PE_LoadU8(0x8009CE3Cu));
        }
        if (hp <= 0 && !blogged) {
            blogged = 1;
            fprintf(stderr, "[PADONLY] fight END f=%d tok=%08X start=%d min=%d peak=%d hp_jumps=%d end=dead\n", g_frame,
                    (unsigned)tok, bhp0, bmin, bpeak, bjumps);
        }
    } else if (bon) {
        bon = 0;
        if (!blogged)
            fprintf(stderr, "[PADONLY] fight END f=%d tok=%08X frames=%d start=%d min=%d peak=%d end_hp=%d hp_jumps=%d end=clear\n",
                    g_frame, (unsigned)tok, g_frame - bstart, bhp0, bmin, bpeak, hp, bjumps);
        last_hp = hp;
    }
}

#endif /* PC_PORT_ROUTE_PADONLY_PILOT_H */
