/* Select+Start dev-menu combo state machine.  See host_pad_combo.h. */
#include "host_pad_combo.h"
#include "host_pad.h"
#include <string.h>

#define SS   (HOST_PAD_SELECT | HOST_PAD_START)
#define DIRS (HOST_PAD_UP | HOST_PAD_DOWN | HOST_PAD_LEFT | HOST_PAD_RIGHT)

/* A connection's Select/Start sequence: idle (none held), pending (withheld,
 * may still become the combo), live (outlived the window: delivered), spent
 * (the combo, the open menu, or held since it connected: not delivered, and no
 * new combo until it has released both buttons). */
enum { HPC_IDLE, HPC_PENDING, HPC_LIVE, HPC_SPENT };

uint16_t HostPadCombo_Step(HostPadCombo *c, const HostPadOrigin *in, int n, int menu_open,
                           uint64_t now_ms, HostPadComboEvents *ev)
{
    HostPadComboOrigin next[HOST_PAD_COMBO_ORIGINS];
    uint16_t merged = 0, presses = 0, out;
    int k, open = 0;
    memset(ev, 0, sizeof *ev);
    memset(next, 0, sizeof next);
    if (n > HOST_PAD_COMBO_ORIGINS) n = HOST_PAD_COMBO_ORIGINS;
    if (n < 0) n = 0;
    /* Match this poll's connections to their state; a key no longer present
     * is dropped with whatever it was withholding. */
    for (k = 0; k < n; k++) {
        const HostPadComboOrigin *old = NULL;
        for (int j = 0; j < HOST_PAD_COMBO_ORIGINS && in[k].key; j++)
            if (c->o[j].key == in[k].key) { old = &c->o[j]; break; }
        if (old) next[k] = *old;
        else {
            /* New connection: what it already holds is not a press, and a held
             * Select/Start counts only after a release. */
            next[k].key = in[k].key;
            next[k].prev = in[k].held;
            next[k].mode = (in[k].held & SS) ? HPC_SPENT : HPC_IDLE;
        }
        presses |= in[k].held & (uint16_t)~next[k].prev;
        merged |= in[k].held;
    }
    memcpy(c->o, next, sizeof next);

    if (menu_open) {
        if (!c->menu_prev) c->armed = 0;            /* just opened: wait for a release */
        if (!(merged & (uint16_t)~DIRS)) c->armed = 1;
        if (c->armed) {
            ev->up   = (presses & HOST_PAD_UP)   && !(merged & HOST_PAD_DOWN);
            ev->down = (presses & HOST_PAD_DOWN) && !(merged & HOST_PAD_UP);
            ev->ok   = (presses & HOST_PAD_CROSS) != 0;
            ev->back = (presses & HOST_PAD_CIRCLE) != 0;
        }
        for (k = 0; k < n; k++) {
            c->o[k].mode = (in[k].held & SS) ? HPC_SPENT : HPC_IDLE;   /* a held pair never reopens */
            c->o[k].prev = in[k].held;
        }
        c->drain = 1; c->drain_since = now_ms; c->tap = 0;
        c->menu_prev = 1;
        return 0;                                    /* the game sees nothing while open */
    }
    if (c->menu_prev) { c->menu_prev = 0; c->drain = 1; c->drain_since = now_ms; }
    if (c->drain) {                                  /* after the combo / the menu: release first */
        if (!(merged & (uint16_t)~DIRS) || now_ms - c->drain_since >= HOST_PAD_COMBO_DRAIN_MS) c->drain = 0;
        else {
            for (k = 0; k < n; k++) {                /* Select/Start pressed meanwhile is spent */
                c->o[k].mode = (in[k].held & SS) ? HPC_SPENT : HPC_IDLE;
                c->o[k].prev = in[k].held;
            }
            return 0;
        }
    }
    out = merged & (uint16_t)~SS;                    /* everything but Select/Start is live */
    for (k = 0; k < n; k++) {
        HostPadComboOrigin *o = &c->o[k];
        uint16_t ss = in[k].held & SS;
        if (!ss) {
            if (o->mode == HPC_PENDING) {            /* short lone press: deliver it late */
                c->tap |= o->prev & SS;
                c->tap_until = now_ms + HOST_PAD_COMBO_TAP_MS;
            }
            o->mode = HPC_IDLE;
        } else {
            if (o->mode == HPC_IDLE) { o->mode = HPC_PENDING; o->since = now_ms; }
            if (o->mode == HPC_PENDING) {
                if (now_ms - o->since >= HOST_PAD_COMBO_WINDOW_MS) o->mode = HPC_LIVE;
                else if (ss == SS) open = 1;
            }
            if (o->mode == HPC_LIVE) out |= ss;
        }
        o->prev = in[k].held;
    }
    if (open) {                                      /* one connection's pair: open, swallow, drain */
        ev->open = 1;
        for (k = 0; k < n; k++)
            if (in[k].held & SS) c->o[k].mode = HPC_SPENT;
        c->drain = 1; c->drain_since = now_ms; c->tap = 0;
        return 0;
    }
    if (c->tap) {
        if (now_ms < c->tap_until) out |= c->tap;
        else c->tap = 0;
    }
    return out;
}
