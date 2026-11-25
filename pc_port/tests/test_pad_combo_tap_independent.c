/* Synthetic input polling: no controller, game, device or save. */
#include <stdint.h>
#include <stdio.h>
#include "host_pad.h"
#include "host_pad_combo.h"
static uint16_t step(HostPadCombo *c, uint16_t held, uint64_t t)
{
    HostPadOrigin o = {1, held}; HostPadComboEvents e;
    uint16_t out = HostPadCombo_Step(c,&o,1,0,t,&e);
    if (e.open) return 0xffff;
    return out;
}
int main(void)
{
    HostPadCombo c = {0};
    if (step(&c,0,0) || step(&c,HOST_PAD_SELECT,10) ||
        step(&c,0,20)!=HOST_PAD_SELECT ||
        step(&c,HOST_PAD_START,30)!=HOST_PAD_SELECT ||
        step(&c,0,70)!=(HOST_PAD_SELECT|HOST_PAD_START)) return 1;
    uint16_t out=step(&c,0,80);
    if (out!=HOST_PAD_START) { fprintf(stderr,"at 80ms got 0x%04x, expected Start only\n",out); return 2; }
    if (step(&c,0,129)!=HOST_PAD_START || step(&c,0,130)) return 3;
    c=(HostPadCombo){0};
    if (step(&c,0,200) || step(&c,HOST_PAD_SELECT,210) ||
        step(&c,0,220)!=HOST_PAD_SELECT ||
        step(&c,HOST_PAD_SELECT,230)!=HOST_PAD_SELECT ||
        step(&c,0,240)!=HOST_PAD_SELECT ||
        step(&c,0,280)!=HOST_PAD_SELECT || step(&c,0,300)) return 4;
    puts("independent tap expiries and same-button refresh pass");
    return 0;
}
