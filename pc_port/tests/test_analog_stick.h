/* Opt-in ANALOG STICK (pe_plat/enhance.h).
 *   - pad reply plumbing: bootstrap/game_port.c PE_Port_StorePadId (the
 *     func_8003F3C4 field fill) and PE_Port_FillPadBuffer (the 801909B4 /
 *     8019234C / 80192CE8 overlay fills);
 *   - the retail analog branch of func_8003EB04 (generated TU from the
 *     matching src/func_8003EB04.c, oracle-hashed by DAY1_game_input);
 *   - the analog field walk func_800710A4 (src/func_800710A4.c:45-57).
 * Needs test_enhance.h (enh_isolate, enh_fixture, ENH_*) included first. */
#include "host_pad_sdl.h"      /* HostPadSDL_AxisToByte (static inline) */

#define AS_PAD 0x800BE9A0u

static uint16_t as_raw_word;
static uint16_t as_raw_source(void) { return as_raw_word; }
static uint8_t as_sticks[4];
static int as_sticks_ok;
static int as_calls;
static int as_analog_source(uint8_t s[4])
{
    as_calls++;
    if (!as_sticks_ok) return 0;
    memcpy(s, as_sticks, 4);
    return 1;
}

static void as_canary(uint8_t *mirror)
{
    for (uint32_t i = 0; i < 8u; i++) {
        PE_StoreU8(AS_PAD + i, (uint8_t)(0xA0u + i));
        if (mirror) mirror[i] = (uint8_t)(0xA0u + i);
    }
}

static int as_pad_is(const uint8_t want[8])
{
    for (uint32_t i = 0; i < 8u; i++)
        if (PE_LoadU8(AS_PAD + i) != want[i]) return 0;
    return 1;
}

/* The fill-site body before this option existed, verbatim, applied to a
 * host mirror of the same eight bytes: the "today" reference. */
static void as_old_fill(uint8_t mirror[8])
{
    uint16_t raw = PE_Port_ReadPadRaw();
    mirror[0] = 0x00u; mirror[1] = 0x41u;                  /* PE_StoreU16(A0, 0x4100) */
    mirror[2] = (uint8_t)raw; mirror[3] = (uint8_t)(raw >> 8);  /* PE_StoreU16(A2, raw) */
}

static void test_ANALOG_axis_to_byte(void)
{
    TEST("ANALOG_axis_to_byte");
    ASSERT(HostPadSDL_AxisToByte(0) == 0x80u, "centre is 0x80");
    ASSERT(HostPadSDL_AxisToByte(-32768) == 0x00u && HostPadSDL_AxisToByte(32767) == 0xFFu,
           "full up/left 0x00, full down/right 0xFF");
    ASSERT(HostPadSDL_AxisToByte(-16384) == 0x40u && HostPadSDL_AxisToByte(16384) == 0xC0u,
           "half deflection 0x40 / 0xC0");
    /* Against the retail thresholds (src/func_8003EB04.c:155-176): half
     * deflection is outside the 0x5A..0xA0 dead zone but short of the run
     * bytes (< 0x14, >= 0xE7); full deflection reaches them. */
    ASSERT(HostPadSDL_AxisToByte(-16384) < 0x5Au && HostPadSDL_AxisToByte(-16384) >= 0x14u &&
           HostPadSDL_AxisToByte(16384) >= 0xA1u && HostPadSDL_AxisToByte(16384) < 0xE7u,
           "half deflection: direction without run");
    ASSERT(HostPadSDL_AxisToByte(-32768) < 0x14u && HostPadSDL_AxisToByte(32767) >= 0xE7u,
           "full deflection: run bytes");
    ASSERT(HostPadSDL_AxisToByte(-3000) >= 0x5Au && HostPadSDL_AxisToByte(3000) <= 0xA0u,
           "small stick noise stays inside the retail dead zone");
    PASS();
}

/* Option off, or no analog source / controller: the eight pad-buffer bytes
 * are exactly what the pre-option fill wrote. */
static void test_ANALOG_fill_off_is_today(void)
{
    uint8_t want[8];
    TEST("ANALOG_fill_off_is_today");
    enh_isolate();
    ResetTestState();
    as_raw_word = 0xBFEFu;                 /* Cross + d-pad Up held (active low) */
    memcpy(as_sticks, "\x11\x22\x33\x44", 4); as_sticks_ok = 1; as_calls = 0;

    /* no pad source: nothing is written */
    PE_Port_SetPadSource(NULL);
    as_canary(want);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(want), "no pad source: pad buffer untouched");

    /* off, digital source only */
    PE_Port_SetPadSource(as_raw_source);
    as_canary(want); as_old_fill(want);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(want), "off: overlay fill byte-identical to the old fill");
    /* off, with an analog source installed and reporting */
    PE_Port_SetPadAnalogSource(as_analog_source);
    as_canary(want); as_old_fill(want);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(want) && as_calls == 0, "off: the analog source is never asked");
    /* the field (func_8003F3C4) site stores only the ID word */
    as_canary(want); want[0] = 0x00u; want[1] = 0x41u;
    ASSERT(PE_Port_StorePadId() == 0 && as_pad_is(want), "off: field site stores exactly 0x4100");

    /* on, but the source reports no controller */
    pe_enh_set_analog_stick(1);
    as_sticks_ok = 0;
    as_canary(want); as_old_fill(want);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(want) && as_calls == 1, "on without a controller: digital reply");
    /* on, but no analog source (route-pad / PE_PAD_SCRIPT runs) */
    as_sticks_ok = 1;
    PE_Port_SetPadSource(as_raw_source);     /* installing a pad source drops it */
    as_canary(want); as_old_fill(want);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(want) && as_calls == 1, "a pad source swap drops the analog source");

    PE_Port_SetPadSource(NULL);
    pe_enh_set_analog_stick(0);
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

static void test_ANALOG_fill_on(void)
{
    static const uint8_t want[8] = {0x00, 0x73, 0xEF, 0xBF, 0x11, 0x22, 0x33, 0x44};
    uint8_t mirror[8];
    TEST("ANALOG_fill_on");
    enh_isolate();
    ResetTestState();
    pe_enh_set_analog_stick(1);
    as_raw_word = 0xBFEFu;
    memcpy(as_sticks, "\x11\x22\x33\x44", 4); as_sticks_ok = 1; as_calls = 0;
    PE_Port_SetPadSource(as_raw_source);
    PE_Port_SetPadAnalogSource(as_analog_source);
    as_canary(mirror);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(want), "on: ID 73h, the raw buttons, then RX RY LX LY");
    as_canary(mirror);
    ASSERT(PE_Port_StorePadId() == 1, "field site reports the analog reply");
    ASSERT(PE_LoadU16(AS_PAD) == 0x7300u && PE_LoadU16(AS_PAD + 2u) == 0xA3A2u &&
           PE_LoadU8(AS_PAD + 4u) == 0x11u && PE_LoadU8(AS_PAD + 7u) == 0x44u,
           "field site: ID 73h + sticks, button word left to the site");
    /* controller unplugged: back to the digital reply on the next fill */
    as_sticks_ok = 0;
    as_canary(mirror); as_old_fill(mirror);
    PE_Port_FillPadBuffer();
    ASSERT(as_pad_is(mirror), "unplugged: digital 41h again");
    PE_Port_SetPadSource(NULL);
    pe_enh_set_analog_stick(0);
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

/* One retail input frame with the port's real libpad record (func_800844E4:
 * PadStateDiscon, as at runtime) and the retail button table (func_8003E974). */
static uint32_t as_3eb04(uint16_t id, uint16_t raw, uint8_t lx, uint8_t ly, int menu_window)
{
    ResetTestState();
    func_800844E4(AS_PAD, AS_PAD + 0x22u);
    func_8003E974();
    PE_StoreU32(0x800B0CD8u, 0u);
    PE_StoreU32(0x8009D154u, 0u);
    if (menu_window) {          /* a window node keyed (1, 0): func_80062A34(1,0) != 0 */
        PE_StoreU32(0x8009D154u, 0x80150000u);
        PE_StoreU32(0x80150000u, 0u);
        PE_StoreU32(0x80150020u, 1u);
        PE_StoreU32(0x80150024u, 0u);
    }
    PE_StoreU16(AS_PAD, id);
    PE_StoreU16(AS_PAD + 2u, raw);
    PE_StoreU8(AS_PAD + 6u, lx);
    PE_StoreU8(AS_PAD + 7u, ly);
    func_8003EB04();
    return PE_LoadU32(0x8009D26Cu);
}

static void test_ANALOG_retail_branch(void)
{
    uint32_t align, bits;
    TEST("ANALOG_retail_branch");
    align = PE_ControllerAlignCalls;
    bits = as_3eb04(0x7300u, 0xFFFFu, 0x80u, 0x40u, 0);
    ASSERT(bits == 0x8u, "half up: Up (8) only, no run");
    ASSERT(PE_LoadU32(0x8009D1F4u) == 0x8u, "and a pressed edge for menus");
    ASSERT(func_800825C0(0) == 0 && PE_ControllerAlignCalls == align && !PE_Port_ShouldStop(),
           "libpad record stays PadStateDiscon: no PadSetMainMode/ActAlign, no stop");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0x80u, 0x00u, 0) == 0x9u, "full up: Up + run (1)");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0x80u, 0xFFu, 0) == 0x21u, "full down: Down (0x20) + run");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0xC0u, 0x80u, 0) == 0x10u, "half right: Right (0x10)");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0x00u, 0x80u, 0) == 0x41u, "full left: Left (0x40) + run");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0x5Au, 0xA0u, 0) == 0u, "dead zone 0x5A..0xA0: nothing");
    /* Retail analog mode ignores the d-pad and Circle-run (D_8009D26C &= ~0x79). */
    ASSERT(as_3eb04(0x7300u, 0xFFEFu, 0x80u, 0x80u, 0) == 0u, "analog: d-pad Up gives no Up bit");
    ASSERT(as_3eb04(0x4100u, 0xFFEFu, 0x80u, 0x80u, 0) == 0x8u, "digital: d-pad Up gives Up");
    ASSERT(as_3eb04(0x7300u, 0xDFFFu, 0x80u, 0x80u, 0) == 0x40000400u, "analog: Circle without run bit");
    ASSERT(as_3eb04(0x4100u, 0xDFFFu, 0x80u, 0x80u, 0) == 0x40000401u, "digital: Circle sets run bit");
    /* Window 0 open (func_80062A34(1,0)): only full deflection, never run. */
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0x80u, 0x40u, 1) == 0u, "menu window: half deflection ignored");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0x80u, 0x00u, 1) == 0x8u, "menu window: full up, no run");
    ASSERT(as_3eb04(0x7300u, 0xFFFFu, 0xFFu, 0x80u, 1) == 0x10u, "menu window: full right, no run");
    ASSERT(PE_ControllerAlignCalls == align && !PE_Port_ShouldStop() && !g_stub_order_count,
           "no handshake call, stop or stub across every frame");
    PASS();
}

/* The trig table entries these walks read (ResetTestState zero-fills guest
 * RAM).  Values read from the retail SLUS_006.62 data: the rsin/rcos
 * quadrant table D_8009589C (func_80077D30 / func_80077DC4) at 0, 0x200,
 * 0x400 = 0, 2896, 4096 and the ratan2 table D_8009A6EC (func_80079FB4) at
 * 0, 1024 = 0, 512. */
static void as_seed_trig(void)
{
    PE_StoreU16(0x8009589Cu + 2u * 0x000u, 0u);
    PE_StoreU16(0x8009589Cu + 2u * 0x200u, 2896u);
    PE_StoreU16(0x8009589Cu + 2u * 0x400u, 4096u);
    PE_StoreU16(0x8009A6ECu + 2u * 0u, 0u);
    PE_StoreU16(0x8009A6ECu + 2u * 1024u, 512u);
}

static void as_walk_fixture(uint32_t pad_bits, uint8_t lx, uint8_t ly, uint16_t yaw)
{
    enh_fixture(0x16u, pad_bits);
    as_seed_trig();
    PE_StoreU16(0x800BE9A0u, 0x7300u);
    PE_StoreU16(0x800BD022u, yaw);
    PE_StoreU8(0x800BE9A6u, lx);
    PE_StoreU8(0x800BE9A7u, ly);
}

/* func_800710A4 with the analog ID: the walk follows the stick angle and, on
 * the axes, matches the d-pad walk (retail walk speed 0x50000).  Before the
 * analog block was ported this wrote no velocity at all. */
static void test_ANALOG_field_walk(void)
{
    TEST("ANALOG_field_walk");
    enh_isolate();
    as_walk_fixture(8u, 0x80u, 0x00u, 0u);                              /* stick up */
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == 0x50000 && PE_LoadU32(ENH_ACTOR + 0x68u) == 0u &&
           PE_LoadU16(ENH_ACTOR + 0x3Au) == 0x800u, "stick up = d-pad Up walk (z 0x50000, facing 0x800)");
    as_walk_fixture(0x10u, 0xFFu, 0x80u, 0u);                           /* stick right */
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x68u) == 0x50000 && PE_LoadU32(ENH_ACTOR + 0x70u) == 0u &&
           PE_LoadU16(ENH_ACTOR + 0x3Au) == 0xC00u, "stick right = d-pad Right walk (x 0x50000, facing 0xC00)");
    as_walk_fixture(0x18u, 0xC0u, 0x40u, 0u);                           /* half up-right */
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    /* atan2(-64, 64) = -0x200; sin/cos(0xA00) = -2896: 0x50000 * 2896/4096 */
    ASSERT(PE_LoadU16(ENH_ACTOR + 0x3Au) == 0xA00u && PE_LoadU32(ENH_ACTOR + 0x68u) == 0x38900u &&
           PE_LoadU32(ENH_ACTOR + 0x70u) == 0x38900u, "diagonal: facing 0xA00, x = z = 0x38900");
    /* the analog camera yaw D_800BD022 turns the heading */
    as_walk_fixture(8u, 0x80u, 0x00u, 0x400u);
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    ASSERT(PE_LoadU16(ENH_ACTOR + 0x3Au) == 0xC00u, "facing = stick heading + D_800BD022");
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count, "no stop or stub");
    PASS();
}

/* Dev menu: OPTIONS row 4 is CONTROLS; its one row toggles ANALOG STICK. */
static void test_ANALOG_dev_menu(void)
{
    static uint8_t fb[320 * 240 * 3];
    char label[64];
    TEST("ANALOG_dev_menu");
    enh_isolate();
    setenv("PE_DEV_MENU", "1", 1);
    pe_plat_cheats_reset();
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS (row 7) */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    for (int i = 0; i < 3; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* CONTROLS (row 4) */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_plat_cheats_overlay(fb, 320, 240) != fb, "controls page draws");
    pe_enh_option_label(pe_enh_option_index("analog_stick"), label, sizeof label);
    ASSERT(strcmp(label, "ANALOG STICK: OFF") == 0, "label shows the retail OFF");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_analog_stick() == 1, "row 1 turns ANALOG STICK on");
    pe_enh_reset();
    ASSERT(pe_enh_analog_stick() == 1, "the setting persists");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_analog_stick() == 0, "and back off");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1);                              /* -> OPTIONS row 4 */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);                              /* wraps to row 1 */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> ENHANCEMENTS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_move_speed_pct() == 125, "BACK returns to OPTIONS row 4; DOWN wraps to row 1");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    setenv("PE_ANALOG_STICK", "1", 1); pe_enh_reset();
    ASSERT(pe_enh_analog_stick() == 1, "PE_ANALOG_STICK=1 env override");
    unsetenv("PE_ANALOG_STICK"); pe_enh_reset();
    ASSERT(pe_enh_analog_stick() == 0, "env override is not written back");
    unsetenv("PE_DEV_MENU");
    pe_plat_cheats_reset();
    unlink(enh_conf); pe_enh_reset();
    PASS();
}

static void test_ANALOG_all(void)
{
    test_ANALOG_axis_to_byte();
    test_ANALOG_fill_off_is_today();
    test_ANALOG_fill_on();
    test_ANALOG_retail_branch();
    test_ANALOG_field_walk();
    test_ANALOG_dev_menu();
    enh_isolate();
    unsetenv("PE_ENH_CONFIG");
}
