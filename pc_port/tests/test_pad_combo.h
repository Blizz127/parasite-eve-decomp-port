/* Select+Start dev-menu combo (platform/host_pad_combo.c).  A lone Select or
 * Start reaches the game (late if short, live if held past the window); one
 * connection's pair opens the menu and the game never sees it; the open menu
 * swallows input and drains after closing; a drifting stick or a stuck button
 * cannot freeze the pad; a late second button, a pair split across pads, a pad
 * reconnected with buttons held, a spent pair after the drain timeout and a pad
 * that disappears while a press is withheld cannot open the menu or invent a
 * press.  Steps carry explicit millisecond times: the X11 pacer polls several
 * times per frame, so the window and the late tap are timed, not counted in
 * calls.  Each scenario first connects its pads with nothing held, as a pad is
 * plugged in before it is pressed. */
static uint16_t padcombo_k(HostPadCombo *c, uint32_t key, uint16_t held, int open, uint64_t t, HostPadComboEvents *ev)
{
    HostPadOrigin o;
    o.key = key; o.held = held;
    return HostPadCombo_Step(c, &o, 1, open, t, ev);
}
static uint16_t padcombo_1(HostPadCombo *c, uint16_t held, int open, uint64_t t, HostPadComboEvents *ev)
{
    return padcombo_k(c, 1u, held, open, t, ev);
}
static uint16_t padcombo_2(HostPadCombo *c, uint32_t ka, uint16_t a, uint32_t kb, uint16_t b, int open, uint64_t t,
                           HostPadComboEvents *ev)
{
    HostPadOrigin o[2];
    o[0].key = ka; o[0].held = a; o[1].key = kb; o[1].held = b;
    return HostPadCombo_Step(c, o, 2, open, t, ev);
}
#define PC_SS (HOST_PAD_SELECT | HOST_PAD_START)

static void test_PADCOMBO_lone_buttons(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    TEST("pad_combo_lone_buttons");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 900, &ev);
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 1000, &ev) == 0 && !ev.open, "a lone Start is withheld first");
    ASSERT(padcombo_1(&c, HOST_PAD_START | HOST_PAD_CROSS, 0, 1100, &ev) == HOST_PAD_CROSS,
           "other buttons stay live while Start is withheld");
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 1000 + HOST_PAD_COMBO_WINDOW_MS - 1, &ev) == 0,
           "still withheld inside the window");
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 1000 + HOST_PAD_COMBO_WINDOW_MS, &ev) == HOST_PAD_START,
           "held past the window: delivered live");
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 1500, &ev) == HOST_PAD_START, "stays live while held");
    ASSERT(padcombo_1(&c, 0, 0, 1510, &ev) == 0, "released after going live: no extra late press");
    /* a short tap: delivered late, for HOST_PAD_COMBO_TAP_MS of polls */
    ASSERT(padcombo_1(&c, HOST_PAD_SELECT, 0, 2000, &ev) == 0, "short Select withheld");
    ASSERT(padcombo_1(&c, 0, 0, 2050, &ev) == HOST_PAD_SELECT, "short Select delivered on release");
    for (int i = 0; i < 20; i++)
        ASSERT(padcombo_1(&c, 0, 0, 2051, &ev) == HOST_PAD_SELECT, "many polls in one frame keep the tap");
    ASSERT(padcombo_1(&c, 0, 0, 2050 + HOST_PAD_COMBO_TAP_MS - 1, &ev) == HOST_PAD_SELECT, "tap lasts its time");
    ASSERT(padcombo_1(&c, 0, 0, 2050 + HOST_PAD_COMBO_TAP_MS, &ev) == 0, "tap ends");
    PASS();
}

static void test_PADCOMBO_opens_menu(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    uint16_t seen = 0;
    TEST("pad_combo_opens_menu");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 50, &ev);
    seen |= padcombo_1(&c, HOST_PAD_SELECT, 0, 100, &ev);
    ASSERT(!ev.open, "Select alone does not open");
    seen |= padcombo_1(&c, PC_SS, 0, 150, &ev);
    ASSERT(ev.open, "Select then Start inside the window opens the menu");
    seen |= padcombo_1(&c, PC_SS, 0, 200, &ev);
    ASSERT(!ev.open, "held combo does not repeat");
    seen |= padcombo_1(&c, HOST_PAD_START, 0, 250, &ev);
    seen |= padcombo_1(&c, 0, 0, 300, &ev);
    seen |= padcombo_1(&c, 0, 0, 400, &ev);
    ASSERT(!(seen & PC_SS), "the game never sees the combo buttons");
    ASSERT(padcombo_1(&c, HOST_PAD_CROSS, 0, 500, &ev) == HOST_PAD_CROSS, "live again after the release");
    PASS();
}

static void test_PADCOMBO_menu_navigation(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    TEST("pad_combo_menu_navigation");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 0, &ev);
    (void)padcombo_1(&c, PC_SS, 0, 1, &ev);
    ASSERT(ev.open, "combo opens");
    ASSERT(padcombo_1(&c, PC_SS | HOST_PAD_CROSS, 1, 10, &ev) == 0 && !ev.ok,
           "buttons held since the open do not navigate");
    (void)padcombo_1(&c, 0, 1, 20, &ev);
    (void)padcombo_1(&c, HOST_PAD_CROSS, 1, 30, &ev);
    ASSERT(ev.ok, "Cross confirms once armed");
    (void)padcombo_1(&c, HOST_PAD_CROSS, 1, 40, &ev);
    ASSERT(!ev.ok, "a held Cross does not repeat");
    (void)padcombo_1(&c, 0, 1, 50, &ev);
    (void)padcombo_1(&c, HOST_PAD_DOWN, 1, 60, &ev);
    ASSERT(ev.down && !ev.up, "Down moves");
    (void)padcombo_1(&c, HOST_PAD_DOWN | HOST_PAD_UP, 1, 70, &ev);
    ASSERT(!ev.up, "Up with Down held is ignored");
    (void)padcombo_1(&c, 0, 1, 80, &ev);
    (void)padcombo_1(&c, HOST_PAD_CIRCLE, 1, 90, &ev);
    ASSERT(ev.back, "Circle returns");
    ASSERT(padcombo_1(&c, HOST_PAD_CIRCLE, 0, 100, &ev) == 0, "closing Circle drains, not delivered");
    ASSERT(padcombo_1(&c, 0, 0, 110, &ev) == 0 && !c.drain, "drain ends on release");
    ASSERT(padcombo_1(&c, HOST_PAD_CROSS, 0, 120, &ev) == HOST_PAD_CROSS, "live after the drain");
    PASS();
}

static void test_PADCOMBO_drift_and_stuck(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    TEST("pad_combo_drift_and_stuck");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 0, &ev);
    (void)padcombo_1(&c, PC_SS | HOST_PAD_LEFT, 0, 1, &ev);
    ASSERT(ev.open, "combo with the stick off-centre opens");
    ASSERT(padcombo_1(&c, HOST_PAD_LEFT, 0, 10, &ev) == HOST_PAD_LEFT && !c.drain,
           "a drifting stick does not hold the drain");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 0, &ev);
    (void)padcombo_1(&c, PC_SS, 0, 1, &ev);
    (void)padcombo_1(&c, HOST_PAD_RIGHT, 1, 10, &ev);
    (void)padcombo_1(&c, HOST_PAD_RIGHT | HOST_PAD_CROSS, 1, 20, &ev);
    ASSERT(ev.ok, "the menu arms with a drifting stick");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 0, &ev);
    (void)padcombo_1(&c, PC_SS, 0, 1, &ev);
    ASSERT(padcombo_1(&c, HOST_PAD_SQUARE, 0, 10, &ev) == 0, "a held button drains");
    ASSERT(padcombo_1(&c, HOST_PAD_SQUARE, 0, HOST_PAD_COMBO_DRAIN_MS - 1, &ev) == 0, "still draining");
    (void)padcombo_1(&c, HOST_PAD_SQUARE, 0, 1 + HOST_PAD_COMBO_DRAIN_MS, &ev);
    ASSERT(!c.drain, "a stuck button ends the drain after HOST_PAD_COMBO_DRAIN_MS");
    ASSERT(padcombo_1(&c, HOST_PAD_SQUARE, 0, 2 + HOST_PAD_COMBO_DRAIN_MS, &ev) == HOST_PAD_SQUARE,
           "and goes live");
    PASS();
}

static void test_PADCOMBO_window_enforced(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    TEST("pad_combo_window_enforced");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 900, &ev);
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 1000, &ev) == 0, "Start withheld");
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 1300, &ev) == HOST_PAD_START, "Start live at the window");
    ASSERT(padcombo_1(&c, PC_SS, 0, 1400, &ev) == PC_SS && !ev.open,
           "Select after Start went live: no combo, both live");
    ASSERT(padcombo_1(&c, HOST_PAD_SELECT, 0, 1500, &ev) == HOST_PAD_SELECT && !ev.open,
           "same sequence: Select stays live");
    ASSERT(padcombo_1(&c, PC_SS, 0, 1550, &ev) == PC_SS && !ev.open,
           "no combo until both buttons are released");
    ASSERT(padcombo_1(&c, 0, 0, 1600, &ev) == 0, "release: no late press");
    ASSERT(padcombo_1(&c, HOST_PAD_SELECT, 0, 1700, &ev) == 0, "a new sequence");
    (void)padcombo_1(&c, PC_SS, 0, 1750, &ev);
    ASSERT(ev.open, "the new sequence opens");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 1900, &ev);
    ASSERT(padcombo_1(&c, HOST_PAD_SELECT, 0, 2000, &ev) == 0, "Select withheld");
    ASSERT(padcombo_1(&c, PC_SS, 0, 2000 + HOST_PAD_COMBO_WINDOW_MS, &ev) == PC_SS && !ev.open,
           "second button exactly at the window: no combo");
    ASSERT(padcombo_1(&c, 0, 0, 2400, &ev) == 0, "released");
    ASSERT(padcombo_1(&c, HOST_PAD_START, 0, 3000, &ev) == 0, "Start first");
    (void)padcombo_1(&c, PC_SS, 0, 3000 + HOST_PAD_COMBO_WINDOW_MS - 1, &ev);
    ASSERT(ev.open, "Select one ms inside the window opens");
    PASS();
}

static void test_PADCOMBO_spent_pair(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    TEST("pad_combo_spent_pair");
    memset(&c, 0, sizeof c);
    (void)padcombo_1(&c, 0, 0, 0, &ev);
    (void)padcombo_1(&c, PC_SS, 0, 1, &ev);
    ASSERT(ev.open, "opens");
    (void)padcombo_1(&c, PC_SS, 1, 10, &ev);
    (void)padcombo_1(&c, PC_SS | HOST_PAD_LEFT, 1, 20, &ev);
    ASSERT(padcombo_1(&c, PC_SS | HOST_PAD_LEFT, 0, 100, &ev) == 0 && !ev.open, "closed with the pair held: drain");
    ASSERT(padcombo_1(&c, PC_SS | HOST_PAD_LEFT, 0, 100 + HOST_PAD_COMBO_DRAIN_MS - 1, &ev) == 0 && !ev.open,
           "still draining");
    ASSERT(padcombo_1(&c, PC_SS | HOST_PAD_LEFT, 0, 100 + HOST_PAD_COMBO_DRAIN_MS, &ev) == HOST_PAD_LEFT &&
           !ev.open && !c.drain, "drain timeout: directions live, no reopen from the held pair");
    ASSERT(padcombo_1(&c, PC_SS | HOST_PAD_LEFT | HOST_PAD_CROSS, 0, 2200, &ev) == (HOST_PAD_LEFT | HOST_PAD_CROSS) &&
           !ev.open, "ordinary input works, still no reopen");
    ASSERT(padcombo_1(&c, PC_SS | HOST_PAD_LEFT, 0, 3000, &ev) == HOST_PAD_LEFT && !ev.open, "the pair stays spent");
    ASSERT(padcombo_1(&c, 0, 0, 3100, &ev) == 0, "released");
    (void)padcombo_1(&c, PC_SS, 0, 3200, &ev);
    ASSERT(ev.open, "a fresh pair opens");
    PASS();
}

static void test_PADCOMBO_origins(void)
{
    HostPadCombo c;
    HostPadComboEvents ev;
    TEST("pad_combo_origins");
    /* Select on one pad, Start on another: not the combo */
    memset(&c, 0, sizeof c);
    (void)padcombo_2(&c, 1u, 0, 2u, 0, 0, 0, &ev);
    ASSERT(padcombo_2(&c, 1u, HOST_PAD_SELECT, 2u, HOST_PAD_START, 0, 1, &ev) == 0 && !ev.open,
           "a pair split across pads is not the combo");
    (void)padcombo_2(&c, 1u, HOST_PAD_SELECT, 2u, HOST_PAD_START, 0, 100, &ev);
    ASSERT(!ev.open, "still not the combo");
    ASSERT(padcombo_2(&c, 1u, HOST_PAD_SELECT, 2u, HOST_PAD_START, 0, 301, &ev) == PC_SS && !ev.open,
           "the split pair goes live after the window");
    ASSERT(padcombo_2(&c, 1u, 0, 2u, 0, 0, 400, &ev) == 0, "released");
    /* a pad that (re)connects with the pair held */
    memset(&c, 0, sizeof c);
    ASSERT(padcombo_k(&c, 5u, PC_SS, 0, 1000, &ev) == 0 && !ev.open, "reconnect with the pair held: no combo");
    ASSERT(padcombo_k(&c, 5u, PC_SS, 0, 1500, &ev) == 0 && !ev.open, "the held pair stays spent");
    ASSERT(padcombo_k(&c, 5u, 0, 0, 1600, &ev) == 0, "released");
    (void)padcombo_k(&c, 5u, PC_SS, 0, 1700, &ev);
    ASSERT(ev.open, "then the pair opens");
    /* pads that connect holding Cross while the menu is open */
    memset(&c, 0, sizeof c);
    (void)padcombo_k(&c, 7u, HOST_PAD_CROSS, 1, 0, &ev);
    ASSERT(!ev.ok, "a pad connecting with Cross held does not confirm");
    (void)padcombo_k(&c, 7u, 0, 1, 10, &ev);
    (void)padcombo_k(&c, 8u, HOST_PAD_CROSS, 1, 20, &ev);
    ASSERT(!ev.ok, "a reconnect (new key) holding Cross does not confirm");
    (void)padcombo_k(&c, 8u, 0, 1, 30, &ev);
    (void)padcombo_k(&c, 8u, HOST_PAD_CROSS, 1, 40, &ev);
    ASSERT(ev.ok, "a real press confirms");
    /* a pad that disappears while its lone Start is withheld */
    memset(&c, 0, sizeof c);
    (void)padcombo_k(&c, 9u, 0, 0, 0, &ev);
    ASSERT(padcombo_k(&c, 9u, HOST_PAD_START, 0, 1, &ev) == 0, "pending");
    ASSERT(HostPadCombo_Step(&c, NULL, 0, 0, 50, &ev) == 0, "the pad vanished: no late press");
    ASSERT(HostPadCombo_Step(&c, NULL, 0, 0, 60, &ev) == 0 && HostPadCombo_Step(&c, NULL, 0, 0, 120, &ev) == 0,
           "nothing later either");
    /* the same pad seen through SDL and js*, released a few ms apart */
    memset(&c, 0, sizeof c);
    (void)padcombo_2(&c, 0x40000001u, 0, 0x80000004u, 0, 0, 0, &ev);
    ASSERT(padcombo_2(&c, 0x40000001u, HOST_PAD_START, 0x80000004u, HOST_PAD_START, 0, 1, &ev) == 0, "both pending");
    ASSERT(padcombo_2(&c, 0x40000001u, 0, 0x80000004u, HOST_PAD_START, 0, 40, &ev) == HOST_PAD_START,
           "SDL released first: the late press starts");
    ASSERT(padcombo_2(&c, 0x40000001u, 0, 0x80000004u, 0, 0, 48, &ev) == HOST_PAD_START,
           "js released: still one press");
    ASSERT(padcombo_2(&c, 0x40000001u, 0, 0x80000004u, 0, 0, 100, &ev) == HOST_PAD_START, "the same press");
    ASSERT(padcombo_2(&c, 0x40000001u, 0, 0x80000004u, 0, 0, 108, &ev) == 0, "one press, then nothing");
    memset(&c, 0, sizeof c);
    (void)padcombo_2(&c, 0x40000001u, 0, 0x80000004u, 0, 0, 0, &ev);
    (void)padcombo_2(&c, 0x40000001u, PC_SS, 0x80000004u, PC_SS, 0, 1, &ev);
    ASSERT(ev.open, "the duplicated pair opens");
    (void)padcombo_2(&c, 0x40000001u, PC_SS, 0x80000004u, PC_SS, 1, 10, &ev);
    ASSERT(!ev.open, "and only once");
    PASS();
}

static void test_PADCOMBO_all(void)
{
    test_PADCOMBO_lone_buttons();
    test_PADCOMBO_opens_menu();
    test_PADCOMBO_menu_navigation();
    test_PADCOMBO_drift_and_stuck();
    test_PADCOMBO_window_enforced();
    test_PADCOMBO_spent_pair();
    test_PADCOMBO_origins();
}
