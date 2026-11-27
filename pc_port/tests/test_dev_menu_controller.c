/* Hardware-free dev-menu controller regression.  The menu reads every
 * controller connection (HostPad_Origins: SDL GameController and Linux
 * joystick pads), so the test steps HostDevMenu_Step with synthetic
 * connections and a millisecond clock.  The fixed bug: with the menu on, a
 * lone Start or Select used to be reserved for the combo and never reached the
 * game.  Kept from the SDL version: a combo cannot be synthesized across two
 * devices, and a pad reconnecting with the combo held must first release. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host_dev_menu.h"
#include "host_pad.h"
#include "pe_plat/cheats.h"
#include "pe_guest_ram.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); exit(1); } checks++; } while (0)
static int checks;
static uint64_t now_ms = 1000;
/* HostDevMenu_Poll's source in the game; the test steps directly. */
int HostPad_Origins(HostPadOrigin *out, int max) { (void)out; (void)max; return 0; }
static PePlatCheatState state(void) { PePlatCheatState s; pe_plat_cheats_get_state(&s); return s; }
/* One host poll 16 ms after the last, with the given connections: the
 * controller bits the game would read. */
static uint16_t stepn(const HostPadOrigin *in, int n)
{
    uint16_t merged = 0;
    for (int k = 0; k < n; k++) merged |= in[k].held;
    now_ms += 16;
    HostDevMenu_Step(in, n, now_ms);
    return HostDevMenu_FilterPad(merged);
}
static uint16_t stepk(uint32_t key, uint16_t held) { HostPadOrigin o; o.key = key; o.held = held; return stepn(&o, 1); }
static uint16_t step(uint16_t held) { return stepk(1u, held); }
static uint16_t step2(uint32_t ka, uint16_t a, uint32_t kb, uint16_t b)
{
    HostPadOrigin o[2];
    o[0].key = ka; o[0].held = a; o[1].key = kb; o[1].held = b;
    return stepn(o, 2);
}
static void tap(uint16_t b) { step(b); step(0); }
static void combo(void) { step(HOST_PAD_SELECT); step(HOST_PAD_SELECT | HOST_PAD_START); step(HOST_PAD_START); step(0); }
int main(int argc, char **argv)
{
    unsetenv("PE_DEV_MENU");
    if (argc > 1 && strcmp(argv[1], "--disabled") == 0) {
        setenv("PE_CHEATS", "0", 1); setenv("PE_DEV_MENU", "1", 1);
        step(0); combo(); CHECK(!state().menu);
        CHECK(pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1) == 0);
        CHECK(!HostDevMenu_CapturesInput());
        CHECK(step(HOST_PAD_START) == HOST_PAD_START); /* hard-disabled: nothing withheld */
        printf("DEV_MENU hard-disable PASS checks=%d\n", checks); return 0;
    }
    unsetenv("PE_CHEATS");
    PE_RamInit();
    unsigned char *before = malloc(PE_RAM_SIZE); CHECK(before != NULL);
    PE_StoreU32(0x80140000u, 0x12345678u);
    memcpy(before, PE_TranslateConst(PE_RAM_BASE, PE_RAM_SIZE), PE_RAM_SIZE);
    /* Menu off (PE_DEV_MENU=0): the combo is inert and Start/Select pass
     * straight through. */
    setenv("PE_DEV_MENU", "0", 1);
    CHECK(step(0) == 0);                                              /* pad 1 connected */
    CHECK(step(HOST_PAD_START) == HOST_PAD_START); step(0);
    combo(); CHECK(!state().menu); CHECK(!HostDevMenu_CapturesInput());
    /* Player mode (PE_DEV_MENU unset, the packaged default): the same combo
     * machine runs, so Select+Start opens the menu and Circle closes it. */
    unsetenv("PE_DEV_MENU");
    step(0);                                                          /* the menu machine sees pad 1 idle */
    combo(); CHECK(state().menu);
    (void)step(HOST_PAD_CIRCLE); CHECK(!state().menu);
    (void)step(0); (void)step(0); CHECK(!HostDevMenu_CapturesInput());
    for (int i = 0; i < 10; i++) (void)step(0);                       /* late taps end */
    setenv("PE_DEV_MENU", "1", 1);
    step(0);                                                          /* the menu machine sees pad 1 idle */
    /* A lone Start reaches the game: withheld for the combo window, then live. */
    CHECK(step(HOST_PAD_START) == 0);
    for (int i = 0; i < 30; i++) (void)step(HOST_PAD_START);       /* held ~0.5 s */
    CHECK(step(HOST_PAD_START) == HOST_PAD_START);
    CHECK(!HostDevMenu_CapturesInput()); CHECK(!state().menu);
    CHECK(step(0) == 0);
    /* A short Select tap is delivered late, for several polls. */
    CHECK(step(HOST_PAD_SELECT) == 0);
    CHECK(step(0) == HOST_PAD_SELECT);
    CHECK(step(0) == HOST_PAD_SELECT);
    for (int i = 0; i < 4; i++) (void)step(0);
    CHECK(step(0) == 0);
    /* Select+Start opens the menu; neither button reaches the game. */
    CHECK(step(HOST_PAD_SELECT) == 0);
    CHECK(step(HOST_PAD_SELECT | HOST_PAD_START) == 0); CHECK(state().menu); CHECK(HostDevMenu_CapturesInput());
    pe_plat_cheats_frame(); pe_plat_cheats_frame();
    CHECK(memcmp(before, PE_TranslateConst(PE_RAM_BASE, PE_RAM_SIZE), PE_RAM_SIZE) == 0);
    free(before); /* opening the menu writes no guest RAM */
    (void)step(HOST_PAD_SELECT | HOST_PAD_START); CHECK(state().menu); /* no held repeat */
    (void)step(HOST_PAD_START); (void)step(0);
    (void)step(HOST_PAD_CROSS); CHECK(state().god);                    /* row 0: GOD MODE */
    (void)step(HOST_PAD_CROSS); (void)step(HOST_PAD_CROSS); CHECK(state().god); /* held: no repeat */
    (void)step(0);
    tap(HOST_PAD_DOWN); tap(HOST_PAD_CROSS); CHECK(state().ammo);        /* row 1 */
    tap(HOST_PAD_UP); tap(HOST_PAD_CROSS); CHECK(!state().god);           /* back on row 0 */
    (void)step(HOST_PAD_CIRCLE); CHECK(!state().menu); CHECK(HostDevMenu_CapturesInput()); /* root: closes */
    CHECK(step(HOST_PAD_CIRCLE) == 0);                                  /* the closing B drains */
    (void)step(0); CHECK(!HostDevMenu_CapturesInput());
    CHECK(step(HOST_PAD_CROSS) == HOST_PAD_CROSS); (void)step(0);       /* live again */
    /* WARP BY DAY: Day 5 lists nothing to pick, Day 2 lists its checkpoints. */
    combo(); CHECK(state().menu);
    for (int i = 0; i < 5; i++) tap(HOST_PAD_DOWN);
    tap(HOST_PAD_CROSS);                                                /* WARP BY DAY */
    for (int i = 0; i < 4; i++) tap(HOST_PAD_DOWN);
    tap(HOST_PAD_CROSS);                                                /* DAY 5 */
    tap(HOST_PAD_CROSS); CHECK(state().menu);                           /* empty: nothing picked */
    tap(HOST_PAD_CIRCLE);                                               /* back to the days */
    for (int i = 0; i < 3; i++) tap(HOST_PAD_UP);
    tap(HOST_PAD_CROSS);                                                /* DAY 2 */
    tap(HOST_PAD_CROSS); CHECK(!state().menu);                          /* DAY 2 START picked */
    (void)step(0); CHECK(!HostDevMenu_CapturesInput());
    /* A drifting stick does not hold the drain after the menu closes. */
    combo(); CHECK(state().menu);
    (void)step(HOST_PAD_CIRCLE); CHECK(!state().menu);
    CHECK(step(HOST_PAD_LEFT) == HOST_PAD_LEFT); CHECK(!HostDevMenu_CapturesInput());
    (void)step(0);
    /* A combo cannot be synthesized across two devices. */
    (void)step2(1u, 0, 2u, 0);                                          /* pad 2 connects */
    (void)step2(1u, HOST_PAD_SELECT, 2u, HOST_PAD_START);
    (void)step2(1u, HOST_PAD_SELECT, 2u, HOST_PAD_START);
    CHECK(!state().menu);
    (void)step2(1u, 0, 2u, 0);
    for (int i = 0; i < 5; i++) (void)step(0);                          /* their late taps end; pad 2 gone */
    /* Disconnect while a lone Select is withheld: no late press; a reconnect
     * (new key) with the combo held must release before it can open. */
    CHECK(step(HOST_PAD_SELECT) == 0); CHECK(!HostDevMenu_CapturesInput());
    CHECK(stepn(NULL, 0) == 0); CHECK(stepn(NULL, 0) == 0);           /* pad 1 gone */
    CHECK(stepk(3u, HOST_PAD_SELECT | HOST_PAD_START) == 0); CHECK(!state().menu);
    CHECK(stepk(3u, HOST_PAD_SELECT | HOST_PAD_START) == 0); CHECK(!state().menu);
    (void)stepk(3u, 0);
    (void)stepk(3u, HOST_PAD_SELECT); (void)stepk(3u, HOST_PAD_SELECT | HOST_PAD_START); CHECK(state().menu);
    (void)stepk(3u, 0);
    /* Keyboard Escape still closes the menu with no controller connected. */
    (void)stepn(NULL, 0); CHECK(state().menu);
    pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1); (void)stepn(NULL, 0); (void)stepn(NULL, 0);
    CHECK(!HostDevMenu_CapturesInput());
    HostDevMenu_Close(); PE_RamDestroy();
    printf("DEV_MENU controller combo/navigation PASS checks=%d\n", checks);
    return 0;
}
