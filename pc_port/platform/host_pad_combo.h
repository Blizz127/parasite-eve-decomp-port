/* Select+Start dev-menu combo on the controller input (any controller, any backend).
 *
 * Fed with every controller connection's held HOST_PAD_* bits and a monotonic
 * millisecond clock, as often as the host polls (the X11 pacer polls several
 * times per frame, so the timing is in milliseconds, not calls).  It returns
 * the bits the game may see (the connections ORed, with Select/Start filtered
 * as below) and reports menu events:
 *   - Select and Start held together on ONE connection, the second within
 *     HOST_PAD_COMBO_WINDOW_MS of the first, open the menu once; the game sees
 *     neither, and nothing until the buttons are released (drain).  A pair
 *     split across two pads is not the combo.
 *   - A lone Select or Start is withheld for up to HOST_PAD_COMBO_WINDOW_MS.
 *     Released before that, it is delivered late as a HOST_PAD_COMBO_TAP_MS
 *     press, long enough for the game's once-per-frame pad read; held longer,
 *     it passes through live, and that connection cannot start the combo
 *     until it has released both buttons.
 *   - A connection that appears (connect, reconnect) with buttons held: those
 *     buttons are not presses, and its Select/Start count only after release.
 *     A connection that disappears drops its withheld button (no late press).
 *   - While the menu is open the game sees nothing; Up/Down/Cross/Circle
 *     presses become menu events once every button has been released after
 *     the open (a held button cannot leak a confirm).  A pair still held when
 *     the menu closes stays spent until released: it cannot reopen the menu.
 *   - Directions never hold a drain or the menu's arming: a drifting stick
 *     must not freeze the pad.  A drain also ends after
 *     HOST_PAD_COMBO_DRAIN_MS, so a stuck button cannot freeze it either.
 * Pure: no I/O, no globals, so it is unit-tested step by step. */
#ifndef HOST_PAD_COMBO_H
#define HOST_PAD_COMBO_H
#include <stdint.h>

#define HOST_PAD_COMBO_WINDOW_MS 300u
#define HOST_PAD_COMBO_TAP_MS     60u
#define HOST_PAD_COMBO_DRAIN_MS 2000u
#define HOST_PAD_COMBO_ORIGINS    16

/* One controller connection.  key is non-zero and unique per connection (a
 * reconnect gets a new key); held is its HOST_PAD_* bits. */
typedef struct { uint32_t key; uint16_t held; } HostPadOrigin;

typedef struct {
    uint32_t key;         /* 0 = free */
    uint16_t prev;        /* its held bits at the last step */
    uint8_t  mode;        /* idle, pending, live, spent (host_pad_combo.c) */
    uint64_t since;       /* ms: when its pending Select/Start began */
} HostPadComboOrigin;

typedef struct {
    HostPadComboOrigin o[HOST_PAD_COMBO_ORIGINS];
    uint16_t tap;         /* a short lone press being delivered late */
    uint64_t tap_until;   /* ms */
    int      drain;       /* deliver nothing until the buttons are released */
    uint64_t drain_since; /* ms */
    int      armed;       /* menu navigation presses count (buttons released once) */
    int      menu_prev;   /* the menu was open at the last step */
} HostPadCombo;

typedef struct { int open, up, down, ok, back; } HostPadComboEvents;

/* in[0..n): this poll's connections.  menu_open: the dev menu is open.
 * now_ms: a monotonic clock.  Returns the HOST_PAD_* bits to deliver to the
 * game until the next step. */
uint16_t HostPadCombo_Step(HostPadCombo *c, const HostPadOrigin *in, int n, int menu_open,
                           uint64_t now_ms, HostPadComboEvents *ev);
#endif
