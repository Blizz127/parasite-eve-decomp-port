/* Keyboard pad word and F11/F12 focus recovery through the SDL backend's real key handler (handle_key in
 * host_window_sdl.c, included here), without SDL or a window.  Synthetic SDL
 * key events only: no device, game or save.  Same contract as
 * test_window_key_merge.c, plus the scancode-0 fallback. */
#define _GNU_SOURCE 1
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
/* Keep the exact real header, renaming only its callable screenshot entry
 * while it is parsed. The backend itself calls the action-count shim below. */
#define HostShot_Take retained_HostShot_Take
#include "../platform/host_png.h"
#undef HostShot_Take
static int screenshot_actions;
static uint8_t fixture_frame[3] = {0x22, 0x44, 0x66};
static int HostShot_Take(const uint8_t *rgb, int w, int h)
{
    assert(rgb == fixture_frame && w == 1 && h == 1);
    screenshot_actions++;
    return 0;
}
#include "../platform/host_window_sdl.c"

static int menu_open, menu_keys;
int pe_plat_cheats_menu_open(void) { return menu_open; }
int pe_plat_cheats_key(PeCheatKey key, int pressed) { (void)key; (void)pressed; menu_keys++; return menu_open; }
int pe_plat_cheats_ff_multiplier(void) { return 0; }
int HostDevMenu_CapturesInput(void) { return 0; }
const HostPadRemap *HostPadRemap_Global(void) { return NULL; }
uint16_t HostPadRemap_Key(const HostPadRemap *r, unsigned long ks)
{
    (void)r;
    if (ks == 0xFF0Dul || ks == 'z') return HOST_PAD_CROSS;
    if (ks == 'c') return HOST_PAD_CIRCLE;
    return 0;
}
void HostAudio_SetFastForward(int on) { (void)on; }
/* HostWindow_Poll's other externals, inert (case 6 drives the real poll loop). */
void HostPad_Poll(void) {}
uint8_t HostPad_Hotkeys(void) { return 0; }
void HostDevMenu_Poll(void) {}

static HsEvent queued[8];
static int queued_n, queued_i;
static int fake_poll_event(HsEvent *e)
{
    if (queued_i >= queued_n) return 0;
    *e = queued[queued_i++];
    return 1;
}

static uint16_t ev(int32_t scancode, int32_t sym, int down, int repeat)
{
    HsKeyboardEvent k;
    memset(&k, 0, sizeof k);
    k.type = down ? HS_KEYDOWN : HS_KEYUP;
    k.repeat = (uint8_t)repeat;
    k.keysym.scancode = scancode; k.keysym.sym = sym;
    assert(handle_key(&k) == 0);
    return g_sony_held;
}
#define RET 40, 13
#define Z   29, 'z'
#define C   6, 'c'

static int original_key_merge_controls(void)
{
    /* 1. shared button */
    ev(RET, 1, 0); assert(ev(Z, 1, 0) == HOST_PAD_CROSS);
    if (ev(RET, 0, 0) != HOST_PAD_CROSS) { fprintf(stderr, "case 1: Cross dropped while z is still held\n"); return 1; }
    assert(ev(Z, 0, 0) == 0);
    /* 2. repeats are idempotent; one release clears */
    ev(Z, 1, 0); ev(Z, 1, 1); assert(ev(Z, 1, 1) == HOST_PAD_CROSS);
    assert(ev(Z, 0, 0) == 0);
    /* 3. menu-consumed release (Return = ENTER while the menu is open) */
    assert(ev(RET, 1, 0) == HOST_PAD_CROSS);
    menu_open = 1; menu_keys = 0;
    { uint16_t h = ev(RET, 0, 0); assert(menu_keys == 1);
      if (h != 0) { fprintf(stderr, "case 3: Cross stuck after a menu-consumed release\n"); return 1; } }
    menu_open = 0;
    /* 4. scancode 0 (UNKNOWN) still works through the per-bit fallback */
    assert(ev(0, 'c', 1, 0) == HOST_PAD_CIRCLE);
    assert(ev(Z, 1, 0) == (HOST_PAD_CIRCLE | HOST_PAD_CROSS));
    assert(ev(0, 'c', 0, 0) == HOST_PAD_CROSS);
    assert(ev(Z, 0, 0) == 0);
    /* 5. focus loss resets (the backend's FOCUS_LOST arm) */
    ev(C, 1, 0); ev(Z, 1, 0);
    HostKeyInput_Reset(&g_keys); g_sony_held = 0;   /* what HS_WE_FOCUS_LOST does */
    assert(ev(C, 0, 0) == 0);
    assert(ev(RET, 1, 0) == HOST_PAD_CROSS);
    assert(ev(RET, 0, 0) == 0);
    /* 6. the real HS_WE_FOCUS_LOST arm, through HostWindow_Poll's event loop */
    ev(C, 1, 0); assert(ev(Z, 1, 0) == (HOST_PAD_CIRCLE | HOST_PAD_CROSS));
    g_win = (void *)1; SDL.PollEvent = fake_poll_event;
    memset(queued, 0, sizeof queued);
    queued[0].type = HS_WINDOWEVENT; queued[0].window.type = HS_WINDOWEVENT;
    queued[0].window.event = HS_WE_FOCUS_LOST;
    queued_n = 1; queued_i = 0;
    assert(HostWindow_Poll() == 0 && queued_i == 1);
    g_win = NULL;
    if (g_sony_held != 0) { fprintf(stderr, "case 6: focus loss kept pad bits\n"); return 1; }
    assert(ev(C, 0, 0) == 0);            /* a release after the loss is harmless */
    if (ev(RET, 1, 0) != HOST_PAD_CROSS) { fprintf(stderr, "case 6: z's pre-loss contribution survived focus loss\n"); return 1; }
    assert(ev(RET, 0, 0) == 0);
    assert(ev(Z, 0, 0) == 0);
    puts("window-sdl-key-merge: shared-button, repeat, menu-consumed release, scancode-0 fallback, reset and real focus-loss controls pass");
    return 0;
}

/* All new key/focus events traverse the real SDL poll loop. */
static int fullscreen_actions, failures, controls;
static int fake_fullscreen(void *window, uint32_t flags)
{
    assert(window == g_win);
    assert(flags == (g_fs_now ? HS_WINDOW_FULLSCREEN_DESKTOP : 0u));
    fullscreen_actions++;
    return 0;
}
static void fake_window_size(void *window, int *w, int *h)
{
    assert(window == g_win); *w = 640; *h = 480;
}
static void poll_all(void)
{
    assert(HostWindow_Poll() == 0);
    assert(queued_i == queued_n);
    queued_i = queued_n = 0;
    assert(g_sony_held == 0);
}
static void one_key(int sc, int down, int repeat)
{
    HsEvent e;
    memset(&e, 0, sizeof e);
    e.type = down ? HS_KEYDOWN : HS_KEYUP;
    e.key.type = e.type; e.key.repeat = (uint8_t)repeat;
    e.key.keysym.scancode = sc;
    e.key.keysym.sym = HS_K_SCANCODE_MASK | sc;
    queued[queued_n++] = e;
    poll_all();
}
static void focus(int gained)
{
    HsEvent e;
    memset(&e, 0, sizeof e);
    e.type = e.window.type = HS_WINDOWEVENT;
    /* SDL_WINDOWEVENT_FOCUS_GAINED=12, intentionally ignored by this backend. */
    e.window.event = gained ? 12 : HS_WE_FOCUS_LOST;
    queued[queued_n++] = e;
    poll_all();
}
static int count(int sc) { return sc == 68 ? fullscreen_actions : screenshot_actions; }
static void reset_hot_case(void)
{
    g_hot_held = 0; g_fs_now = 0;
    fullscreen_actions = screenshot_actions = 0;
    assert(queued_n == 0 && queued_i == 0);
}
static void repeat_controls(int sc, int expected)
{
    one_key(sc, 1, 1); assert(count(sc) == expected); /* SDL repeat flag */
    one_key(sc, 1, 0); assert(count(sc) == expected); /* duplicate unmarked press */
}
static void ordinary_control(int sc)
{
    reset_hot_case();
    one_key(sc, 1, 0); assert(count(sc) == 1);
    repeat_controls(sc, 1);
    one_key(sc, 0, 0); assert(count(sc) == 1);
    one_key(sc, 1, 0); assert(count(sc) == 2);
    one_key(sc, 0, 0);
    controls++;
}
static void late_release_control(int sc)
{
    reset_hot_case();
    one_key(sc, 1, 0); assert(count(sc) == 1);
    focus(0);
    one_key(sc, 0, 0); assert(count(sc) == 1);
    focus(1);
    one_key(sc, 1, 0); assert(count(sc) == 2);
    repeat_controls(sc, 2);
    one_key(sc, 0, 0);
    controls++;
}
static void real_focus_case(int sc)
{
    int before, after;
    reset_hot_case();
    one_key(sc, 1, 0); assert(count(sc) == 1);
    repeat_controls(sc, 1);
    focus(0);
    /* Physical key released elsewhere: no key-up arrives here. */
    focus(1);
    /* Repeat-marked events are never a new physical press, even after loss. */
    one_key(sc, 1, 1); assert(count(sc) == 1);
    before = count(sc);
    one_key(sc, 1, 0);
    after = count(sc);
    if (after != before + 1) {
        fprintf(stderr, "FAIL SDL F%d: fresh press after focus loss: actions before=%d after=%d expected=%d\n",
                sc - 57, before, after, before + 1);
        failures++;
    } else {
        printf("PASS SDL F%d real focus actions=%d->%d\n", sc - 57, before, after);
    }
    repeat_controls(sc, after);
    one_key(sc, 0, 0); assert(count(sc) == after);
    one_key(sc, 1, 0); assert(count(sc) == after + 1);
    one_key(sc, 0, 0);
    controls++;
}
int main(void)
{
    assert(original_key_merge_controls() == 0);
    g_win = (void *)1;
    SDL.PollEvent = fake_poll_event;
    SDL.SetWindowFullscreen = fake_fullscreen;
    SDL.GetWindowSize = fake_window_size;
    queued_n = queued_i = 0;
    g_last_rgb = fixture_frame; g_last_w = g_last_h = 1;
    for (int sc = 68; sc <= 69; sc++) {
        ordinary_control(sc);
        late_release_control(sc);
        real_focus_case(sc);
    }
    printf("SDL hotkey-focus: %d control groups passed, %d fresh-press regressions failed (assertions enabled)\n", controls, failures);
    return failures ? 1 : 0;
}
