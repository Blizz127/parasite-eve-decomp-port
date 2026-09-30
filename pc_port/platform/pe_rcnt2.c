/* Root counter 2 register model; contract in pe_rcnt2.h. */
#include "pe_rcnt2.h"

static uint16_t counter, mode = 0x0400u, target;
static uint32_t prescale;   /* CPU cycles not yet worth one /8 tick */

void PE_Rcnt2_Reset(void)
{
    counter = 0; mode = 0x0400u; target = 0; prescale = 0;
}

void PE_Rcnt2_WriteCounter(uint16_t value) { counter = value; }

void PE_Rcnt2_WriteMode(uint16_t value)
{
    /* bits 0..9 writable; the write sets bit 10 (no IRQ request), clears
     * the reached flags and resets the counter. */
    mode = (uint16_t)((value & 0x03FFu) | 0x0400u);
    counter = 0;
    prescale = 0;
}

void PE_Rcnt2_WriteTarget(uint16_t value) { target = value; }
uint16_t PE_Rcnt2_ReadCounter(void) { return counter; }
uint16_t PE_Rcnt2_ReadTarget(void) { return target; }
uint16_t PE_Rcnt2_PeekMode(void) { return mode; }

uint16_t PE_Rcnt2_ReadMode(void)
{
    uint16_t v = mode;
    mode &= (uint16_t)~0x1800u;
    return v;
}

static int stopped(void)
{
    unsigned sync = (mode >> 1) & 3u;
    return (mode & 1u) && (sync == 0u || sync == 3u);
}

void PE_Rcnt2_Advance(uint32_t cycles)
{
    uint64_t ticks;

    if (stopped())
        return;
    if (mode & 0x0200u) {
        uint64_t total = (uint64_t)prescale + cycles;
        ticks = total / 8u;
        prescale = (uint32_t)(total % 8u);
    } else {
        ticks = cycles;
    }
    while (ticks) {
        if ((mode & 0x0008u) && target != 0 && counter < target) {
            /* reset-at-target: the counter runs 0..target-1 */
            uint64_t to_target = (uint64_t)(target - counter);
            if (ticks >= to_target) {
                mode |= 0x0800u;
                ticks -= to_target;
                counter = (uint16_t)(ticks % target);
            } else {
                counter = (uint16_t)(counter + ticks);
            }
            return;
        }
        if (!(mode & 0x0008u)) {
            /* free running: 16-bit wrap; flag target / 0xFFFF crossings */
            uint64_t d_t = (uint16_t)(target - counter - 1u) + 1u;   /* 1..65536 */
            uint64_t d_f = (uint16_t)(0xFFFFu - counter - 1u) + 1u;
            if (ticks >= d_t) mode |= 0x0800u;
            if (ticks >= d_f) mode |= 0x1000u;
            counter = (uint16_t)(counter + ticks);
            return;
        }
        /* reset mode with counter >= target (or target 0): step once */
        counter++;
        ticks--;
        if (counter == target) {
            mode |= 0x0800u;
            counter = 0;
        }
        if (counter == 0xFFFFu)
            mode |= 0x1000u;
    }
}
