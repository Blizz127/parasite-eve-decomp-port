/* Cross-check of the in-place clamp rule used by PE_DispatchClearRect (platform/pe_plat2_gpuprim_clearrect.c)
 * against the retail clear-worker oracle cases (tests/retail_clear_image_cases.h: wh, limit word -> rect_out).
 * Build: cc -std=gnu11 -O1 -Wall -Wextra -Werror -o clearrect_clamp_check pc_port/tools/pe_clearrect_clamp_check.c
 * Run:   ./clearrect_clamp_check   (exit 0 and "0 mismatches" when every case agrees) */
#include <stdint.h>
#include <stdio.h>
#include "../tests/retail_clear_image_cases.h"

static int16_t clamp_extent(int16_t value, int16_t limit)   /* identical to the provider's */
{
    if (value < 0) return 0;
    if (value >= limit) return (int16_t)(limit - 1);
    return value;
}

int main(void)
{
    unsigned i, bad = 0, n = (unsigned)(sizeof DAY1_clear_cases / sizeof DAY1_clear_cases[0]);
    for (i = 0; i < n; i++) {
        uint32_t wh = DAY1_clear_cases[i].wh, lim = DAY1_clear_cases[i].limit;
        int16_t w = clamp_extent((int16_t)(wh & 0xFFFFu), (int16_t)(lim & 0xFFFFu));
        int16_t h = clamp_extent((int16_t)(wh >> 16), (int16_t)(lim >> 16));
        uint32_t got = (uint32_t)(uint16_t)w | ((uint32_t)(uint16_t)h << 16);
        if (got != DAY1_clear_cases[i].rect_out) {
            bad++;
            fprintf(stderr, "case %u: wh %08X limit %08X -> %08X, oracle %08X\n", i, (unsigned)wh, (unsigned)lim,
                    (unsigned)got, (unsigned)DAY1_clear_cases[i].rect_out);
        }
    }
    printf("clearrect clamp: %u cases, %u mismatches\n", n, bad);
    return bad != 0;
}
