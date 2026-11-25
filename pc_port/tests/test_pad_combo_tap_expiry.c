/* Synthetic polling sequences, no device, window, game, or saves. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "host_pad.h"
#include "host_pad_combo.h"

static uint16_t step(HostPadCombo *c, uint16_t held, uint64_t t, int *opened)
{
    HostPadOrigin p = {1, held}; HostPadComboEvents e;
    uint16_t out = HostPadCombo_Step(c, &p, 1, 0, t, &e);
    *opened |= e.open;
    return out;
}
int main(void)
{
    HostPadCombo c = {0}; int opened = 0;
    if (step(&c, 0, 0, &opened) || step(&c, HOST_PAD_SELECT, 10, &opened) ||
        step(&c, 0, 20, &opened) != HOST_PAD_SELECT ||
        step(&c, HOST_PAD_START, 30, &opened) != HOST_PAD_SELECT) abort();
    uint16_t out = step(&c, 0, 90, &opened);
    if (out != HOST_PAD_START || opened) {
        fprintf(stderr,"stale tap: got 0x%04x expected Start only\n", out);
        return 1;
    }
    if (step(&c, 0, 150, &opened)) abort();
    c = (HostPadCombo){0}; opened = 0;
    if (step(&c, 0, 200, &opened) || step(&c, HOST_PAD_SELECT, 210, &opened) ||
        step(&c, 0, 220, &opened) != HOST_PAD_SELECT ||
        step(&c, HOST_PAD_START, 230, &opened) != HOST_PAD_SELECT ||
        step(&c, 0, 240, &opened) != (HOST_PAD_SELECT|HOST_PAD_START) || opened) abort();
    c = (HostPadCombo){0}; opened = 0;
    (void)step(&c, 0, 300, &opened);
    (void)step(&c, HOST_PAD_SELECT, 310, &opened);
    if (step(&c, HOST_PAD_SELECT|HOST_PAD_START, 320, &opened) || !opened) abort();
    puts("expired tap, overlapping taps and menu-combo controls pass");
    return 0;
}
