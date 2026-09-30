/*
 * pe_route_pad.h — canonical deterministic route pad for the boot -> Day-2
 * route proof.
 *
 * Single source of truth shared by the two consumers of the scripted input:
 *
 *   * pc_port/tests/test_route_boot_day2.c — the milestone harness that
 *     asserts the address-exact persist[]/token chain, and
 *   * pc_port/src/port_main.c — the interactive `--route-pad` autopilot.
 *
 * Keeping the stage table here is what makes the windowed run and the harness
 * walk the same route: both feed PeRoutePad_Mask() the identical masks at the
 * identical frame thresholds.
 *
 * STAGE TABLE (bits are active-low Sony pad bits, 0 = pressed):
 *
 *   Stage 1 (frame < 5400): hold 0xFFEF (DOWN) + periodic Cross. Carries
 *     m0002i -> m0003i -> m0091i -> m0004i, the documented first-play prefix.
 *   Stage 2 (5400 <= frame < 6040): hold 0xFFBF (UP) + periodic Cross.
 *     Releases the m0004i walk-to-bench gate, transfers to m0378i at
 *     0x801B69D0.
 *   Stage 3 (6040 <= frame < 6989): hold 0xFF9F (DOWN|UP, held bits 0x30) +
 *     periodic Cross. Walks Aya into m0378i module-4 rectangle #1
 *     (x in (-1067, 533), z in (-560, -300)) at 0x801957A8, sets local[4]=1
 *     and fires the m0378i -> m0377i room_transfer at 0x80195728
 *     (token 0xA80673C8).
 *   Stage 4 (frame >= 6989, the frame m0377i is entered): hold 0xFFAF
 *     (0xFF9F with the 0x10 bit also pressed) + periodic Cross. m0377i module
 *     1's op-77 rectangle at 0x801953C4 (x in (-211, 241), z in
 *     (-4607, -4114)) is an interior trigger the 0xFF9F hold never enters.
 *     With 0xFFAF the op-77 test returns a hit, local[4]=1, and the guard at
 *     0x80195410 falls through instead of looping at 0x8019544C. m0377i
 *     module 5 then writes persist[1]=0x179 and transfers back to m0378i at
 *     0x80195768; m0378i module 0 bounces to m0004i, whose module-4 type-4
 *     task parks at 0x801B6CC8 (the current frontier).
 *
 * Probe sweep: PE_ROUTE_PAD4=0xFFBF/0xFFDF/0xFF9F/0xFF7F leaves the m0377i
 * gate looping exactly as the old three-stage default (0xFF9F hold) did; only
 * masks that add the 0x10 bit (0xFFAF, 0xFF2F, 0xFF8F, 0xFFCF) make the gate
 * report a hit. Setting PE_ROUTE_PAD4=0xFF9F reproduces the old three-stage
 * behaviour exactly (stage 4 continues to hold the stage-3 mask).
 *
 * 0xFF9F = 0xFFBF & 0xFFDF, 0xFFAF = 0xFF9F & 0xFFEF.
 *
 * The masks / switch frames / pulse period are overridable from the
 * environment so the route can be probed without a rebuild; the defaults are
 * the documented first-play sequence and are what the assertions run with.
 */
#ifndef PC_PORT_PE_ROUTE_PAD_H
#define PC_PORT_PE_ROUTE_PAD_H

#include <stdint.h>
#include <stdlib.h>

#define PE_ROUTE_PAD_CROSS         0xBFFFu
#define PE_ROUTE_PAD_DOWN          0xFFEFu
#define PE_ROUTE_PAD_UP            0xFFBFu
#define PE_ROUTE_PAD_DN_LEFT       0xFF9Fu
#define PE_ROUTE_PAD_DN_LEFT_MASK2 0xFFAFu
#define PE_ROUTE_PAD_CROSS_PERIOD  8

#define PE_ROUTE_PAD_SWITCH_FRAME   5400
#define PE_ROUTE_PAD_SWITCH2_FRAME  6040
#define PE_ROUTE_PAD_SWITCH3_FRAME  6989
/* Stage 5 is disabled by default (switch4_frame 0) so the four-stage route is
 * unchanged unless PE_ROUTE_SWITCH4 is set; see the stage-5 note below. */
#define PE_ROUTE_PAD_SWITCH4_FRAME  0

typedef struct PeRoutePadConfig {
    uint16_t stage1;
    uint16_t stage2;
    uint16_t stage3;
    uint16_t stage4;
    uint16_t stage5;
    uint16_t pulse;
    int      switch_frame;
    int      switch2_frame;
    int      switch3_frame;
    int      switch4_frame;
    int      period;
} PeRoutePadConfig;

static inline void PeRoutePad_Defaults(PeRoutePadConfig *c)
{
    c->stage1         = PE_ROUTE_PAD_DOWN;
    c->stage2         = PE_ROUTE_PAD_UP;
    c->stage3         = PE_ROUTE_PAD_DN_LEFT;
    c->stage4         = PE_ROUTE_PAD_DN_LEFT_MASK2;
    c->stage5         = PE_ROUTE_PAD_DN_LEFT_MASK2;
    c->pulse          = PE_ROUTE_PAD_CROSS;
    c->switch_frame   = PE_ROUTE_PAD_SWITCH_FRAME;
    c->switch2_frame  = PE_ROUTE_PAD_SWITCH2_FRAME;
    c->switch3_frame  = PE_ROUTE_PAD_SWITCH3_FRAME;
    c->switch4_frame  = PE_ROUTE_PAD_SWITCH4_FRAME;
    c->period         = PE_ROUTE_PAD_CROSS_PERIOD;
}

static inline uint16_t PeRoutePad_EnvHex16(const char *name, uint16_t dflt)
{
    const char *s = getenv(name);
    return (s && s[0]) ? (uint16_t)strtoul(s, NULL, 16) : dflt;
}

static inline void PeRoutePad_ConfigFromEnv(PeRoutePadConfig *c)
{
    const char *s;

    PeRoutePad_Defaults(c);
    c->stage1 = PeRoutePad_EnvHex16("PE_ROUTE_PAD1", c->stage1);
    c->stage2 = PeRoutePad_EnvHex16("PE_ROUTE_PAD2", c->stage2);
    c->stage3 = PeRoutePad_EnvHex16("PE_ROUTE_PAD3", c->stage3);
    c->stage4 = PeRoutePad_EnvHex16("PE_ROUTE_PAD4", c->stage4);
    c->stage5 = PeRoutePad_EnvHex16("PE_ROUTE_PAD5", c->stage5);
    c->pulse  = PeRoutePad_EnvHex16("PE_ROUTE_PULSE", c->pulse);
    s = getenv("PE_ROUTE_SWITCH");
    if (s && s[0]) c->switch_frame = atoi(s);
    s = getenv("PE_ROUTE_SWITCH2");
    if (s && s[0]) c->switch2_frame = atoi(s);
    s = getenv("PE_ROUTE_SWITCH3");
    if (s && s[0]) c->switch3_frame = atoi(s);
    s = getenv("PE_ROUTE_SWITCH4");
    if (s && s[0]) c->switch4_frame = atoi(s);
    s = getenv("PE_ROUTE_PERIOD");
    if (s && s[0]) c->period = atoi(s);
    if (c->period < 1) c->period = 1;
}

/*
 * The pad mask for a given route frame.  `frame` is supplied by the caller
 * (the harness counts present-hook frames; the interactive port counts pad
 * reads) so the two keep their own clock while sharing this exact table.
 */
static inline uint16_t PeRoutePad_Mask(const PeRoutePadConfig *c, int frame)
{
    uint16_t m;

    if (c->switch4_frame > 0 && frame >= c->switch4_frame)
        m = c->stage5;
    else if (c->switch3_frame > 0 && frame >= c->switch3_frame)
        m = c->stage4;
    else if (c->switch2_frame > 0 && frame >= c->switch2_frame)
        m = c->stage3;
    else if (frame >= c->switch_frame)
        m = c->stage2;
    else
        m = c->stage1;
    if ((frame % c->period) == 3)
        m &= c->pulse;
    return m;
}

#endif /* PC_PORT_PE_ROUTE_PAD_H */
