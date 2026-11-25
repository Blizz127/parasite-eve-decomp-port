/* Keyboard pad word and F11/F12 focus recovery through the real X11 event loop (host_window.c's
 * HostWindow_Poll) with a fake Xlib, without a display.  Like test_enh_window.c
 * it includes host_window.c itself.  Synthetic keycodes, keysyms and pad bits
 * only: no window, device, game or save.
 *
 * Several keys share one button (Return/space/z/x are all Cross by default).
 * The button must stay held while any of its keys is down, a release that the
 * cheat menu consumes must not leave its contribution stuck, and focus loss
 * clears everything. */
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
#include "../platform/host_window.c"

#define KC_RETURN 36u
#define KC_Z      52u
#define KC_C      54u

static int menu_open;
static int menu_keys;
static int cheat_downs[PE_CHEAT_KEY_COUNT];

/* Every external HostWindow_Poll reaches, as an inert stub. */
void HostPad_Poll(void) {}
void HostDevMenu_Poll(void) {}
int HostDevMenu_CapturesInput(void) { return 0; }
uint8_t HostPad_Hotkeys(void) { return 0; }
int pe_plat_cheats_menu_open(void) { return menu_open; }
int pe_plat_cheats_key(PeCheatKey key, int pressed) { assert(key > PE_CHEAT_KEY_NONE && key < PE_CHEAT_KEY_COUNT); if (pressed) cheat_downs[key]++; menu_keys++; return menu_open; }
int pe_plat_cheats_ff_multiplier(void) { return 0; }
const HostPadRemap *HostPadRemap_Global(void) { return NULL; }
uint16_t HostPadRemap_Key(const HostPadRemap *r, unsigned long ks)
{
    (void)r;
    if (ks == XK_Return || ks == 'z') return HOST_PAD_CROSS;
    if (ks == 'c') return HOST_PAD_CIRCLE;
    return 0;
}
void HostAudio_SetFastForward(int on) { (void)on; }
int HostAudio_QueueFrames(void) { return 0; }
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; }
uint32_t PE_GPU_VSyncQuery(void) { return 0; }
uint32_t HostFB_ScanOutCount(void) { return 0; }
uint64_t HostFB_GetPresentCount(void) { return 0; }
void HostFB_GetState(int *a, int *b, int *c, int *d) { if (a) *a = 0; if (b) *b = 0; if (c) *c = 0; if (d) *d = 0; }
void PE_StrFeed_GetStats(PeStrFeedStats *out) { memset(out, 0, sizeof *out); }
int PE_StrFeed_Active(void) { return 0; }

static XEvent queued[16];
static int queued_n, queued_i;
static int fake_pending(void *d) { (void)d; return queued_i < queued_n; }
static int fake_next(void *d, XEvent *e) { (void)d; *e = queued[queued_i++]; return 0; }
static int fake_peek(void *d, XEvent *e) { (void)d; *e = queued[queued_i]; return 0; }
static KeySym fake_lookup(XKeyEvent *k, int index)
{
    (void)index;
    switch (k->keycode) {
    case KC_RETURN: return XK_Return;
    case KC_Z: return 'z';
    case KC_C: return 'c';
    case 67u: return XK_F1;
    case 74u: return XK_F8;
    case 9u: return XK_Escape;
    case 111u: return XK_Up;
    case 116u: return XK_Down;
    case 95u: return XK_F11;
    case 96u: return XK_F12;
    default: return 0;
    }
}

static unsigned long now_ms = 1000;
static void key(unsigned keycode, int down)
{
    XEvent e;
    memset(&e, 0, sizeof e);
    e.xkey.type = down ? KeyPress : KeyRelease;
    e.xkey.keycode = keycode;
    e.xkey.time = now_ms++;   /* distinct times: never mistaken for auto-repeat */
    assert(queued_n < (int)(sizeof queued / sizeof queued[0]));
    queued[queued_n++] = e;
}
static void focus_out(void)
{
    XEvent e;
    memset(&e, 0, sizeof e);
    e.xfocus.type = FocusOut; e.xfocus.mode = 0; e.xfocus.detail = 3;
    queued[queued_n++] = e;
}
/* Deliver the queued events through the production loop. */
static uint16_t poll_held(void)
{
    assert(HostWindow_Poll() == 0);
    assert(queued_i == queued_n);
    queued_n = queued_i = 0;
    return g_sony_held;
}

static int original_key_merge_controls(void)
{
    g_dpy = (void *)1;
    XPending = fake_pending; XNextEvent = fake_next; XPeekEvent = fake_peek; XLookupKeysym = fake_lookup;

    /* 1. Two keys on one button: releasing one keeps the button held. */
    key(KC_RETURN, 1); key(KC_Z, 1);
    assert(poll_held() == HOST_PAD_CROSS);
    key(KC_RETURN, 0);
    if (poll_held() != HOST_PAD_CROSS) { fprintf(stderr, "case 1: Cross dropped while z is still held\n"); return 1; }
    key(KC_Z, 0);
    assert(poll_held() == 0);

    /* 2. Mixed buttons: each key clears only its own contribution. */
    key(KC_Z, 1); key(KC_C, 1);
    assert(poll_held() == (HOST_PAD_CROSS | HOST_PAD_CIRCLE));
    key(KC_C, 0);
    assert(poll_held() == HOST_PAD_CROSS);
    key(KC_Z, 0);
    assert(poll_held() == 0);

    /* 3. A release consumed by the cheat menu (Return = ENTER while it is open)
     *    must still clear that key's Cross. */
    key(KC_RETURN, 1);
    assert(poll_held() == HOST_PAD_CROSS);
    menu_open = 1; menu_keys = 0;
    key(KC_RETURN, 0);
    {
        uint16_t held = poll_held();
        assert(menu_keys == 1);   /* the menu did consume the release */
        if (held != 0) { fprintf(stderr, "case 3: Cross stuck after a menu-consumed release\n"); return 1; }
    }
    menu_open = 0;

    /* 4. Focus loss clears everything; a later release is harmless and a new
     *    press counts alone. */
    key(KC_C, 1); key(KC_Z, 1);
    assert(poll_held() == (HOST_PAD_CROSS | HOST_PAD_CIRCLE));
    focus_out();
    assert(poll_held() == 0);
    key(KC_C, 0);
    assert(poll_held() == 0);
    key(KC_RETURN, 1);
    assert(poll_held() == HOST_PAD_CROSS);   /* z's pre-loss contribution is gone */
    key(KC_RETURN, 0);
    assert(poll_held() == 0);

    puts("window-key-merge: shared-button, mixed, menu-consumed release and focus-loss controls pass");
    return 0;
}

/* Action-level focus regression. This is a synthetic event stream; no X server,
 * window, real screenshot, game, or owner input is involved. */
static int fullscreen_actions, failures, controls;
static Window fake_root(void *d) { assert(d == g_dpy); return 3; }
static Atom fake_atom(void *d, const char *name, int only_existing)
{
    (void)d; (void)only_existing;
    if (!strcmp(name, "_NET_WM_STATE")) return 10;
    if (!strcmp(name, "_NET_WM_STATE_FULLSCREEN")) return 11;
    if (!strcmp(name, "_MOTIF_WM_HINTS")) return 12;
    assert(0 && "unexpected atom lookup"); return 0;
}
static int fake_property(void *d, Window w, Atom property, Atom type, int format,
                         int mode, const unsigned char *bytes, int count)
{
    (void)d; (void)w; (void)mode;
    assert(property == 12 && type == 12 && format == 32 && count == 5);
    assert(((const long *)bytes)[2] == (g_fs_now ? 0 : 1));
    return 0;
}
static int fake_send(void *d, Window w, int propagate, long mask, XEvent *e)
{
    (void)d; (void)propagate;
    assert(w == 3 && mask == (SubstructureRedirectMask | SubstructureNotifyMask));
    assert(e->type == ClientMessage && e->xclient.message_type == 10);
    assert(e->xclient.data[1] == 11 && e->xclient.data[3] == 1);
    assert(e->xclient.data[0] == (g_fs_now ? NET_WM_STATE_ADD : NET_WM_STATE_REMOVE));
    fullscreen_actions++;
    return 1;
}
static int fake_flush(void *d) { (void)d; return 0; }
static void focus(int in, int mode, int detail)
{
    XEvent e;
    memset(&e, 0, sizeof e);
    e.xfocus.type = in ? FocusIn : FocusOut;
    e.xfocus.mode = mode; e.xfocus.detail = detail;
    assert(queued_n < (int)(sizeof queued / sizeof queued[0]));
    queued[queued_n++] = e;
}
static int count(unsigned kc) { return kc == 95u ? fullscreen_actions : screenshot_actions; }
static void reset_hot_case(void)
{
    g_hot_held = 0; g_fs_now = 0;
    fullscreen_actions = screenshot_actions = 0;
    HostFocus_Reset(&g_focus);
    assert(queued_i == 0 && queued_n == 0);
}
static void one_key(unsigned kc, int down)
{
    key(kc, down);
    assert(poll_held() == 0);
}
static void repeat_pair(unsigned kc)
{
    key(kc, 0); key(kc, 1);
    queued[queued_n-1].xkey.time = queued[queued_n-2].xkey.time;
    assert(poll_held() == 0);
}
static void repeat_controls(unsigned kc, int expected)
{
    one_key(kc, 1); assert(count(kc) == expected);  /* duplicate press */
    repeat_pair(kc); assert(count(kc) == expected); /* X11 repeat release/press */
}
static void ordinary_control(unsigned kc)
{
    reset_hot_case();
    one_key(kc, 1); assert(count(kc) == 1);
    repeat_controls(kc, 1);
    one_key(kc, 0); assert(count(kc) == 1);
    one_key(kc, 1); assert(count(kc) == 2);
    one_key(kc, 0);
    controls++;
}
static void late_release_control(unsigned kc)
{
    reset_hot_case();
    one_key(kc, 1); assert(count(kc) == 1);
    focus(0, 0, 3); poll_held(); assert(g_focus.focused == 0);
    /* The old key-up may arrive during focus recovery; it must do no action. */
    one_key(kc, 0); assert(count(kc) == 1);
    focus(1, 0, 3); poll_held(); assert(g_focus.focused == 1);
    one_key(kc, 1); assert(count(kc) == 2);
    repeat_controls(kc, 2);
    one_key(kc, 0);
    controls++;
}
static void ignored_focus_control(unsigned kc, int mode, int detail)
{
    reset_hot_case();
    assert(HostFocus_X11Event(0, mode, detail) == -1);
    one_key(kc, 1); assert(count(kc) == 1);
    focus(0, mode, detail); poll_held(); assert(g_focus.focused == 1);
    repeat_controls(kc, 1);
    focus(1, mode, detail); poll_held(); assert(g_focus.focused == 1);
    repeat_controls(kc, 1);
    one_key(kc, 0); one_key(kc, 1); assert(count(kc) == 2);
    one_key(kc, 0);
    controls++;
}
static void real_focus_case(unsigned kc, int mode, int detail)
{
    int before, after;
    reset_hot_case();
    assert(HostFocus_X11Event(0, mode, detail) == 0);
    one_key(kc, 1); assert(count(kc) == 1);
    repeat_controls(kc, 1);
    focus(0, mode, detail); poll_held(); assert(g_focus.focused == 0);
    /* The user releases the physical key while another window owns focus.
     * That old key-up is absent from this window's stream. */
    focus(1, mode, detail); poll_held(); assert(g_focus.focused == 1);
    before = count(kc);
    one_key(kc, 1);
    after = count(kc);
    if (after != before + 1) {
        fprintf(stderr, "FAIL X11 F%u: fresh press after real focus loss mode=%d detail=%d: actions before=%d after=%d expected=%d\n",
                kc - 84u, mode, detail, before, after, before + 1);
        failures++;
    } else {
        printf("PASS X11 F%u real focus mode=%d detail=%d actions=%d->%d\n", kc - 84u, mode, detail, before, after);
    }
    /* Check repeat/key-up behavior independently of the missing action above. */
    repeat_controls(kc, after);
    one_key(kc, 0); assert(count(kc) == after);
    one_key(kc, 1); assert(count(kc) == after + 1);
    one_key(kc, 0);
    controls++;
}
static int existing_key_focus_controls(void)
{
    static const int ignored[][2] = {{1,3},{2,3},{0,2},{0,5},{0,6},{0,7}};
    assert(original_key_merge_controls() == 0);
    g_win = 2;
    XDefaultRootWindow = fake_root; XInternAtom = fake_atom;
    XChangeProperty = fake_property; XSendEvent = fake_send; XFlush = fake_flush;
    g_last_rgb = fixture_frame; g_last_w = g_last_h = 1;
    for (unsigned kc = 95u; kc <= 96u; kc++) {
        ordinary_control(kc);
        late_release_control(kc);
        for (unsigned i = 0; i < sizeof ignored / sizeof ignored[0]; i++)
            ignored_focus_control(kc, ignored[i][0], ignored[i][1]);
        real_focus_case(kc, 0, 3); /* NotifyNormal/NotifyNonlinear */
        real_focus_case(kc, 3, 3); /* NotifyWhileGrabbed is real under existing policy */
    }
    printf("X11 hotkey-focus: %d control groups passed, %d fresh-press regressions failed (assertions enabled)\n", controls, failures);
    return failures ? 1 : 0;
}

/* Separate cheat/menu repeat latch: count dispatches to pe_plat_cheats_key.
 * The fake console remains open so Enter/Back cannot alter a real menu/game. */
static int cheat_failures, cheat_controls;
static void cheat_start(unsigned kc)
{
    menu_open = 1;
    one_key(kc, 0); /* Rearm through the real handler, not a test state write. */
    memset(cheat_downs, 0, sizeof cheat_downs);
    HostFocus_Reset(&g_focus);
}
static void cheat_repeats(unsigned kc, PeCheatKey ck, int expected)
{
    one_key(kc, 1); assert(cheat_downs[ck] == expected);
    repeat_pair(kc); assert(cheat_downs[ck] == expected);
}
static void cheat_ordinary(unsigned kc, PeCheatKey ck)
{
    cheat_start(kc);
    one_key(kc, 1); assert(cheat_downs[ck] == 1);
    cheat_repeats(kc, ck, 1);
    one_key(kc, 0); assert(cheat_downs[ck] == 1);
    one_key(kc, 1); assert(cheat_downs[ck] == 2);
    one_key(kc, 0);
    cheat_controls++;
}
static void cheat_late_up(unsigned kc, PeCheatKey ck)
{
    cheat_start(kc);
    one_key(kc, 1); assert(cheat_downs[ck] == 1);
    focus(0, 0, 3); poll_held(); assert(g_focus.focused == 0);
    one_key(kc, 0); assert(cheat_downs[ck] == 1);
    focus(1, 0, 3); poll_held(); assert(g_focus.focused == 1);
    one_key(kc, 1); assert(cheat_downs[ck] == 2);
    cheat_repeats(kc, ck, 2);
    one_key(kc, 0);
    cheat_controls++;
}
static void cheat_ignored(unsigned kc, PeCheatKey ck, int mode, int detail)
{
    cheat_start(kc);
    assert(HostFocus_X11Event(0, mode, detail) == -1);
    one_key(kc, 1); assert(cheat_downs[ck] == 1);
    focus(0, mode, detail); poll_held(); assert(g_focus.focused == 1);
    cheat_repeats(kc, ck, 1);
    focus(1, mode, detail); poll_held(); assert(g_focus.focused == 1);
    cheat_repeats(kc, ck, 1);
    one_key(kc, 0); one_key(kc, 1); assert(cheat_downs[ck] == 2);
    one_key(kc, 0);
    cheat_controls++;
}
static void cheat_real(unsigned kc, PeCheatKey ck, const char *label, int mode)
{
    int before, after;
    cheat_start(kc);
    one_key(kc, 1); assert(cheat_downs[ck] == 1);
    cheat_repeats(kc, ck, 1);
    focus(0, mode, 3); poll_held(); assert(g_focus.focused == 0);
    /* Released in another window: this window receives no old key-up. */
    focus(1, mode, 3); poll_held(); assert(g_focus.focused == 1);
    before = cheat_downs[ck];
    one_key(kc, 1);
    after = cheat_downs[ck];
    if (after != before + 1) {
        fprintf(stderr, "FAIL X11 cheat-focus %s mode=%d: dispatches before=%d after=%d expected=%d\n",
                label, mode, before, after, before + 1);
        cheat_failures++;
    } else printf("PASS X11 cheat-focus %s mode=%d dispatches=%d->%d\n", label, mode, before, after);
    cheat_repeats(kc, ck, after);
    one_key(kc, 0); assert(cheat_downs[ck] == after);
    one_key(kc, 1); assert(cheat_downs[ck] == after + 1);
    one_key(kc, 0);
    cheat_controls++;
}
static void cheat_arrow_repeat(unsigned kc, PeCheatKey ck)
{
    cheat_start(kc);
    one_key(kc, 1); assert(cheat_downs[ck] == 1);
    one_key(kc, 1); assert(cheat_downs[ck] == 2); /* arrows deliberately repeat */
    repeat_pair(kc); assert(cheat_downs[ck] == 3);
    focus(0, 0, 3); poll_held();
    focus(1, 0, 3); poll_held();
    one_key(kc, 1); assert(cheat_downs[ck] == 4);
    one_key(kc, 0); assert(cheat_downs[ck] == 4);
    cheat_controls++;
}
int main(void)
{
    static const struct { unsigned kc; PeCheatKey ck; const char *label; } cases[] = {
        {67u, PE_CHEAT_KEY_F1, "F1"}, {74u, PE_CHEAT_KEY_F8, "F8"},
        {KC_RETURN, PE_CHEAT_KEY_ENTER, "Enter"}, {9u, PE_CHEAT_KEY_BACK, "Back"},
    };
    static const int ignored[][2] = {{1,3},{2,3},{0,2},{0,5},{0,6},{0,7}};
    assert(existing_key_focus_controls() == 0);
    for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; i++) {
        cheat_ordinary(cases[i].kc, cases[i].ck);
        cheat_late_up(cases[i].kc, cases[i].ck);
        for (unsigned j = 0; j < sizeof ignored / sizeof ignored[0]; j++)
            cheat_ignored(cases[i].kc, cases[i].ck, ignored[j][0], ignored[j][1]);
        cheat_real(cases[i].kc, cases[i].ck, cases[i].label, 0);
        cheat_real(cases[i].kc, cases[i].ck, cases[i].label, 3);
    }
    cheat_arrow_repeat(111u, PE_CHEAT_KEY_UP);
    cheat_arrow_repeat(116u, PE_CHEAT_KEY_DOWN);
    printf("X11 cheat-focus: %d control groups passed, %d fresh-press regressions failed (assertions enabled)\n",
           cheat_controls, cheat_failures);
    return cheat_failures ? 1 : 0;
}
