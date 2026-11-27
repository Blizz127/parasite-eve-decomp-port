/* Host-only menu controls. No SDL video/audio initialization or guest writes.
 * The menu reads every controller connection (HostPad_Origins: SDL
 * GameController and Linux joystick devices), so every controller the game can
 * use opens and drives it; the combo must come from one connection.  This file
 * loads no SDL of its own: host_pad_sdl.c is the only SDL user, so the menu and
 * the game always see the same devices.
 *
 * Select+Start (Back/View + Start/Menu) opens the menu; Up/Down move, Cross
 * (A) confirms, Circle (B) returns.  A lone Select or Start still reaches the
 * game: host_pad_combo.c withholds it only for the combo window.
 */
#include "host_dev_menu.h"
#include "host_pad.h"
#include "host_pad_combo.h"
#include "pe_plat/cheats.h"
#include <string.h>
#include <time.h>

static HostPadCombo g_combo;
static uint16_t g_deliver;
static int g_capture;

static uint64_t now_ms(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return 0;
    return (uint64_t)t.tv_sec * 1000u + (uint64_t)t.tv_nsec / 1000000u;
}

void HostDevMenu_Step(const HostPadOrigin *in, int n, uint64_t now)
{
    HostPadComboEvents ev;
    int open;
    pe_plat_cheats_init();
    if (!pe_plat_cheats_dev_menu_enabled()) {
        memset(&g_combo, 0, sizeof g_combo);
        g_deliver = 0;
        for (int k = 0; k < n; k++) g_deliver |= in[k].held;
        g_capture = 0;
        return;
    }
    open = pe_plat_cheats_menu_open();
    g_deliver = HostPadCombo_Step(&g_combo, in, n, open, now, &ev);
    if (ev.open) pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    else if (open) {
        if (ev.back) pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1);
        else if (ev.ok) pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
        else if (ev.up) pe_plat_cheats_key(PE_CHEAT_KEY_UP, 1);
        else if (ev.down) pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);
    }
    /* A key press can close the menu (Circle on the root page, a warp pick):
     * the combo machine sees that on its next step and drains from there. */
    g_capture = open || pe_plat_cheats_menu_open() || g_combo.drain;
}

void HostDevMenu_Poll(void)
{
    HostPadOrigin in[HOST_PAD_COMBO_ORIGINS];
    int n = HostPad_Origins(in, HOST_PAD_COMBO_ORIGINS);
    HostDevMenu_Step(in, n, now_ms());
}

int HostDevMenu_CapturesInput(void) { return g_capture; }

uint16_t HostDevMenu_FilterPad(uint16_t held)
{
    return pe_plat_cheats_dev_menu_enabled() ? g_deliver : held;
}

void HostDevMenu_Close(void)
{
    memset(&g_combo, 0, sizeof g_combo);
    g_deliver = 0;
    g_capture = 0;
}
