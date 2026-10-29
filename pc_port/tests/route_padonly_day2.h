/* Opt-in Day-2 controller continuation. Source candidate, NOT RUN.
 * This is a finite supported prefix, not a completed 0x90 -> 0x128 walk.
 * Office capture: 6d952618b, route_day2_station_pads.h, entry-relative.
 * Hall prefix: retained counter-accepted-3a088.sh controller schedule;
 * its time origin was the m0042 return, NOT the current run's absolute clock.
 * These old captures have no timing/parity guarantee on this new trajectory.
 * Unknown paths fail explicitly; no guessed steering, guest store or warp.
 */
#ifndef PC_PORT_ROUTE_PADONLY_DAY2_H
#define PC_PORT_ROUTE_PADONLY_DAY2_H

static uint16_t RoutePadOnlyDay2(uint16_t incoming)
{
    static int armed, done, failed, office_start, hall_start = -1;
    static unsigned peak = 0x90u;
    static uint32_t last_token;
    unsigned story = PE_LoadU32(0x800A7918u);
    uint32_t token = (uint32_t)GA_TOKEN;
    uint16_t mask = 0xFFFFu;
    int elapsed;

    if (!g_route_aids_pad_only || done) return incoming;
    if (!armed) {
        if (token != 0xA80040C8u || story != 0x90u) return incoming;
        armed = 1; office_start = g_frame; last_token = token;
        fprintf(stderr, "[D2WALK] ARM frame=%d token=%08X story=%X\n", g_frame, (unsigned)token, story);
    }
    if (story < peak || story > 0x128u) {
        if (!failed) fprintf(stderr, "[D2WALK] FAIL story-order frame=%d token=%08X story=%X peak=%X\n",
                             g_frame, (unsigned)token, story, peak);
        failed = 1;
    }
    if (story > peak) peak = story;
    if (story == 0x128u && !failed) {
        done = 1;
        fprintf(stderr, "[D2WALK] ENDPOINT frame=%d token=%08X story=128\n", g_frame, (unsigned)token);
        return 0xFFFFu;
    }
    if (failed) return 0xFFFFu;
    if (token != last_token) {
        fprintf(stderr, "[D2WALK] ROOM frame=%d token=%08X story=%X\n", g_frame, (unsigned)token, story);
        last_token = token;
    }
    if (hall_start < 0 && token == 0xA80040C8u && story == 0x90u) {
        elapsed = g_frame - office_start;
        if (elapsed >= 2400) goto unsupported;
        if (elapsed >= 912 && elapsed < 1912) mask = 0xFFCFu;
        if ((elapsed % 8) == 3) mask &= 0xBFFFu;
        return mask;
    }
    if (hall_start < 0) {
        if (token != 0xA8004148u || story != 0x90u) goto unsupported;
        hall_start = g_frame;
    }
    elapsed = g_frame - hall_start;
    /* Retained prefix expected m0042 -> m0043 -> m0081 -> m0082.
     * The office return alone does not prove this path on the candidate. */
    if (story != 0x90u || (token != 0xA8004148u && token != 0xA80041C8u &&
                          token != 0xA80050C8u && token != 0xA8005148u &&
                          token != 0xA8066148u)) goto unsupported;
    if (elapsed >= 8400) goto unsupported;
    if      (elapsed <   80) mask = 0xFFCFu;
    else if (elapsed <  495) mask = 0xFFEFu;
    else if (elapsed <  805) mask = 0xFF7Fu;
    else if (elapsed <  995) mask = 0xFFBFu;
    else if (elapsed < 1100) mask = 0xFF7Fu;
    else if (elapsed < 1400) mask = 0xFFFFu;
    else if (elapsed < 1700) mask = 0xFFEFu;
    else if (elapsed < 1740) mask = 0xFF7Fu;
    else if (elapsed < 1800) mask = 0xFFEFu;
    if ((elapsed % 20) >= 3 && (elapsed % 20) < 7)
        mask &= elapsed < 1400 || (elapsed / 20) % 2 ? 0xBFFFu : 0xDFFFu;
    return mask;

unsupported:
    failed = 1;
    fprintf(stderr, "[D2WALK] UNSUPPORTED frame=%d token=%08X story=%X; no qualified continuation\n",
            g_frame, (unsigned)token, story);
    return 0xFFFFu;
}

#endif
