/* Opt-in PAUSE ON FOCUS LOSS (pe_plat/enhance_host.h, platform/plat_enhance.c,
 * platform/host_filter_pause.h, wired in platform/host_window.c): a quality-of-
 * life row, default OFF.  The presenter wiring (the wait, the mute, the pacer
 * restart, the close request) is exercised against a fake X sink by the ctest
 * `enh-window`; this file checks the registry row, the focus policy and the
 * dev menu. */
#include "pe_plat/enhance_host.h"
#include "host_filter_pause.h"

static void test_ENHX_pause_registry(void)
{
    char label[64];
    int o;
    TEST("ENHX_pause_registry");
    enh_isolate();
    unsetenv("PE_PAUSE_ON_FOCUS_LOSS"); pe_enh_reset();
    o = pe_enh_option_index("pause_on_focus_loss");
    ASSERT(o >= 0, "pause_on_focus_loss is registered");
    {
        const PeEnhOption *op = pe_enh_option(o);
        ASSERT(op->category == PE_ENH_CAT_GAMEPLAY && op->format == PE_ENH_FMT_TOGGLE && op->nvalues == 2 &&
               op->values[0] == 0 && op->values[1] == 1 && strcmp(op->env, "PE_PAUSE_ON_FOCUS_LOSS") == 0 &&
               strcmp(op->label, "PAUSE ON FOCUS LOSS") == 0 && op->credit && strcmp(op->credit, "the owner") == 0,
               "a gameplay toggle, OFF first, env override, credit");
    }
    ASSERT(pe_enh_category_option(PE_ENH_CAT_GAMEPLAY, 0) == pe_enh_option_index("move_speed") &&
           pe_enh_category_option(PE_ENH_CAT_GAMEPLAY, 1) == pe_enh_option_index("always_run") &&
           pe_enh_category_option(PE_ENH_CAT_GAMEPLAY, 2) == o && pe_enh_category_option(PE_ENH_CAT_GAMEPLAY, 3) == -1,
           "row 3 of the Enhancements page, after Movement Speed and Always Run");
    ASSERT(pe_enh_pause_on_focus_loss() == 0 && pe_enh_option_is_retail(o), "defaults OFF (retail)");
    ASSERT(access(enh_conf, F_OK) != 0, "queries create no config file");
    pe_enh_option_label(o, label, sizeof label);
    ASSERT(strcmp(label, "PAUSE ON FOCUS LOSS: OFF") == 0, "label");
    pe_enh_option_cycle(o);
    pe_enh_option_label(o, label, sizeof label);
    ASSERT(pe_enh_pause_on_focus_loss() == 1 && strcmp(label, "PAUSE ON FOCUS LOSS: ON") == 0, "A toggles it on");
    pe_enh_option_cycle(o);
    ASSERT(pe_enh_pause_on_focus_loss() == 0, "and off");
    pe_enh_set_pause_on_focus_loss(5);
    ASSERT(pe_enh_pause_on_focus_loss() == 1, "a toggle takes any nonzero as on");
    {
        char text[1024] = {0};
        FILE *fp = fopen(enh_conf, "r");
        size_t n_;
        ASSERT(fp != NULL, "the menu change wrote the config");
        n_ = fread(text, 1, sizeof text - 1, fp);
        text[n_] = 0; fclose(fp);
        ASSERT(strstr(text, "pause_on_focus_loss=1\n") != NULL, "config line pause_on_focus_loss=1");
        ASSERT(pe_enh_move_speed_pct() == 100 && !pe_enh_always_run(), "the movement settings are untouched");
    }
    pe_enh_reset();
    ASSERT(pe_enh_pause_on_focus_loss() == 1, "persists across a reload");
    setenv("PE_PAUSE_ON_FOCUS_LOSS", "0", 1); pe_enh_reset();
    ASSERT(pe_enh_pause_on_focus_loss() == 0, "the env override applies");
    unsetenv("PE_PAUSE_ON_FOCUS_LOSS"); pe_enh_reset();
    ASSERT(pe_enh_pause_on_focus_loss() == 1, "the env override is not written back");
    setenv("PE_PAUSE_ON_FOCUS_LOSS", "yes", 1); pe_enh_reset();
    ASSERT(pe_enh_pause_on_focus_loss() == 0, "an unparsable override means retail");
    unsetenv("PE_PAUSE_ON_FOCUS_LOSS");
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

/* The focus policy: a truth table over every Xlib mode and detail. */
static void test_ENHX_pause_policy(void)
{
    HostFocus f;
    TEST("ENHX_pause_policy");
    HostFocus_Reset(&f);
    ASSERT(f.focused == 1, "a new window has the focus");
    ASSERT(!HostFocus_ShouldPause(&f, 0) && !HostFocus_ShouldPause(&f, 1), "focused: never waits");
    f.focused = 0;
    ASSERT(!HostFocus_ShouldPause(&f, 0), "option OFF: never waits, focus or not");
    ASSERT(HostFocus_ShouldPause(&f, 1), "option ON and no focus: waits");
    /* X11 FocusIn / FocusOut: mode 0..3, detail 0..7 */
    for (int in = 0; in <= 1; in++) for (int mode = 0; mode <= 3; mode++) for (int detail = 0; detail <= 7; detail++) {
        int want, got = HostFocus_X11Event(in, mode, detail);
        if (mode == 1 || mode == 2) want = -1;                                  /* a grab and its release */
        else if (detail == 2 || detail == 5 || detail == 6 || detail == 7) want = -1;   /* inferior, pointer, none */
        else want = in;
        ASSERT(got == want, "X11 focus event filter: grabs, children and the pointer are ignored");
    }
    ASSERT(HostFocus_X11Event(0, 0, 3) == 0 && HostFocus_X11Event(1, 0, 3) == 1 &&
           HostFocus_X11Event(0, 3, 0) == 0 && HostFocus_X11Event(0, 0, 0) == 0, "real losses and gains map to 0 / 1");
    PASS();
}

/* OPTIONS > ENHANCEMENTS row 3 in the dev menu. */
static void test_ENHX_pause_dev_menu(void)
{
    static uint8_t fb[320 * 240 * 3];
    char label[96];
    TEST("ENHX_pause_dev_menu");
    enh_isolate();
    unsetenv("PE_PAUSE_ON_FOCUS_LOSS"); pe_enh_reset();
    setenv("PE_DEV_MENU", "1", 1);
    pe_plat_cheats_reset();
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS (row 7) */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> ENHANCEMENTS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* row 3: PAUSE ON FOCUS LOSS */
    ASSERT(pe_plat_cheats_overlay(fb, 320, 240) != fb, "enhancements page draws");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_pause_on_focus_loss() == 1 && pe_enh_move_speed_pct() == 100 && !pe_enh_always_run(),
           "row 3 toggles the pause and nothing else");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_pause_on_focus_loss() == 0, "and back off");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* wraps to row 1: MOVEMENT SPEED */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_move_speed_pct() == 125, "the page wraps from row 3 to row 1");
    pe_enh_set_move_speed_pct(100);
    for (int n = 0; n < pe_enh_category_count(PE_ENH_CAT_GAMEPLAY); n++) {
        pe_enh_option_label(pe_enh_category_option(PE_ENH_CAT_GAMEPLAY, n), label, sizeof label);
        ASSERT(strlen(label) + 2 <= 50, "every enhancement row fits the 320-pixel overlay (6 px per character)");
    }
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    unsetenv("PE_DEV_MENU");
    pe_plat_cheats_reset();
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

static void test_ENHX_qol_all(void)
{
    test_ENHX_pause_registry();
    test_ENHX_pause_policy();
    test_ENHX_pause_dev_menu();
}
