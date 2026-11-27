/* Opt-in movement-speed / always-run enhancement (pe_plat/enhance.h,
 * platform/plat_enhance.c, hooks in game/boot/func_80035558_port.c). */
#include "pe_plat/enhance.h"
#include <unistd.h>

#define ENH_ACTOR  0x80120000u
#define ENH_NPC    0x80120400u
#define ENH_CODEP  0x80123000u

static char enh_conf[256];

/* The suite's own enhancements.conf and no option overrides: main() calls
 * this first, and suites end with it instead of unsetting PE_ENH_CONFIG, so
 * no enhancement hook ever loads the user's ~/.config file. */
static char g_test_enh_conf[256];
static void enh_hermetic(void)
{
    if (!g_test_enh_conf[0])
        snprintf(g_test_enh_conf, sizeof g_test_enh_conf, "%s/pe_native_tests_%d.conf",
                 getenv("TMPDIR") && *getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());
    unlink(g_test_enh_conf);
    setenv("PE_ENH_CONFIG", g_test_enh_conf, 1);
    for (int i = 0; i < pe_enh_option_count(); i++) unsetenv(pe_enh_option(i)->env);
    pe_enh_reset();
}

/* A config path nobody else uses; the file is removed before each test. */
static void enh_isolate(void)
{
    snprintf(enh_conf, sizeof enh_conf, "%s/pe_enh_test_%d.conf",
             getenv("TMPDIR") && *getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp", (int)getpid());
    unlink(enh_conf);
    setenv("PE_ENH_CONFIG", enh_conf, 1);
    unsetenv("PE_MOVE_SPEED");
    unsetenv("PE_ALWAYS_RUN");
    unsetenv("PE_DISPLAY_SCALER");
    unsetenv("PE_INTERNAL_SCALE");
    unsetenv("PE_TEXTURE_FILTER");
    unsetenv("PE_SSAA");
    unsetenv("PE_DITHERING");
    unsetenv("PE_SUBPIXEL_GEOMETRY");
    unsetenv("PE_MASTER_VOLUME");
    unsetenv("PE_MOVIE_VOLUME");
    unsetenv("PE_ANALOG_STICK");
    unsetenv("PE_MUSIC_VOLUME");
    unsetenv("PE_SFX_VOLUME");
    pe_enh_reset();
}

/* Field, digital pad, identity camera matrix, yaw 0: the rotated velocity at
 * actor+0x68/+0x70 is the unrotated walk vector, so it reads back directly. */
static void enh_fixture(uint32_t codep, uint32_t pad_bits)
{
    ResetTestState();
    D_8009D1A0 = 0u;
    PE_StoreU32(0x8009D254u, ENH_ACTOR);
    PE_StoreU32(0x8009D2E8u, 0u);
    PE_StoreU16(0x800BE9A0u, 0x4000u);            /* digital pad type */
    PE_StoreU16(0x800BD020u, 0u);
    for (int i = 0; i < 9; i++) PE_StoreU16(0x800BD000u + 2u * i, (i % 4) ? 0u : 0x1000u);
    for (int i = 0; i < 3; i++) PE_StoreU32(0x800BD014u + 4u * i, 0u);
    PE_StoreU32(ENH_CODEP, codep);
    PE_StoreU32(ENH_ACTOR + 0x20u, 0x10000u);
    PE_StoreU16(ENH_ACTOR + 0x26u, 0x1000u);
    PE_StoreU32(0x8009D26Cu, pad_bits);
}

static void test_ENH_default_is_retail(void)
{
    static uint8_t before[0x200000];
    TEST("ENH_default_is_retail");
    enh_isolate();
    ASSERT(pe_enh_move_speed_pct() == 100 && pe_enh_always_run() == 0,
           "defaults: 100 % and always-run off");
    enh_fixture(0x16u, 8u);
    memcpy(before, PE_TranslateConst(0x80000000u, 0x200000u), sizeof(before));
    for (int32_t v = -0x300000; v <= 0x300000; v += 0x12345)
        ASSERT(pe_enh_field_speed(ENH_ACTOR, v) == v, "100 %: speed passes through unchanged");
    ASSERT(!pe_enh_field_always_run(ENH_ACTOR), "always-run off: walk stays walk");
    ASSERT(memcmp(before, PE_TranslateConst(0x80000000u, 0x200000u), sizeof(before)) == 0,
           "hooks at the defaults write no guest state");
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == 0x50000 && PE_LoadU32(ENH_ACTOR + 0x68u) == 0u,
           "retail walk speed 0x50000 (Up)");
    enh_fixture(0x17u, 8u);
    func_8007136C(ENH_ACTOR, ENH_CODEP);
    ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == 0x168000, "retail run speed 0x168000 (Up)");
    ASSERT(access(enh_conf, F_OK) != 0, "queries never create the config file");
    PASS();
}

static void test_ENH_field_speed_scales(void)
{
    static const struct { int pct; int32_t walk, run; } k[] = {
        {125, 0x64000, 0x1C2000}, {150, 0x78000, 0x21C000},
        {175, 0x8C000, 0x276000}, {200, 0xA0000, 0x2D0000}};
    TEST("ENH_field_speed_scales");
    enh_isolate();
    for (size_t i = 0; i < sizeof k / sizeof k[0]; i++) {
        pe_enh_set_move_speed_pct(k[i].pct);
        enh_fixture(0x16u, 8u);
        func_800710A4(ENH_ACTOR, ENH_CODEP);
        ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == k[i].walk, "field walk scaled");
        enh_fixture(0x17u, 0x10u);                 /* Right: the x axis */
        func_8007136C(ENH_ACTOR, ENH_CODEP);
        ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x68u) == k[i].run, "field run scaled");
    }
    pe_enh_set_move_speed_pct(137);
    ASSERT(pe_enh_move_speed_pct() == 125, "off-step values clamp down to a listed step");
    unlink(enh_conf);
    PASS();
}

static void test_ENH_only_aya_in_field(void)
{
    TEST("ENH_only_aya_in_field");
    enh_isolate();
    pe_enh_set_move_speed_pct(200);
    pe_enh_set_always_run(1);
    enh_fixture(0x16u, 8u);
    ASSERT(pe_enh_field_speed(ENH_NPC, 0x50000) == 0x50000, "another actor: unscaled");
    ASSERT(!pe_enh_field_always_run(ENH_NPC), "another actor: no always-run");
    D_8009D1A0 = 2u;                                /* battle */
    ASSERT(pe_enh_field_speed(ENH_ACTOR, 0xB4000) == 0xB4000, "battle: Aya unscaled");
    ASSERT(!pe_enh_field_always_run(ENH_ACTOR), "battle: no always-run");
    PE_StoreU32(0x8009D254u, 0u);
    D_8009D1A0 = 0u;
    ASSERT(pe_enh_field_speed(0u, 0x50000) == 0x50000, "no Aya actor: unscaled");
    unlink(enh_conf);
    PASS();
}

/* Battle through the real handlers: at 200 % + always-run Aya's battle run
 * (func_8007136C, retail 0x168000 for this equipment record) and the d-pad
 * handler's battle redirect move exactly as retail. */
static void test_ENH_battle_handlers_unscaled(void)
{
    const uint32_t rec = 0x80120800u;
    TEST("ENH_battle_handlers_unscaled");
    enh_isolate();
    pe_enh_set_move_speed_pct(200);
    pe_enh_set_always_run(1);
    for (int h = 0; h < 2; h++) {
        enh_fixture(0x30u, 8u);
        D_8009D1A0 = 2u;
        PE_StoreU32(ENH_ACTOR, rec);
        PE_StoreU32(rec + 0x4Cu, 0u);
        PE_StoreU8(rec + 0x13u, 0x30u);             /* battle run clip == codep */
        if (h) func_800710A4(ENH_ACTOR, ENH_CODEP); else func_8007136C(ENH_ACTOR, ENH_CODEP);
        ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == 0x168000,
               "battle: Aya's run is retail speed at 200 % + always-run");
        ASSERT(PE_LoadU32(ENH_CODEP) == 0x30u, "battle: clip untouched");
    }
    unlink(enh_conf);
    PASS();
}

static void test_ENH_always_run(void)
{
    TEST("ENH_always_run");
    enh_isolate();
    pe_enh_set_always_run(1);
    enh_fixture(0x17u, 8u);                         /* run anim already set */
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == 0x168000,
           "always-run: d-pad alone takes the run path");
    pe_enh_set_move_speed_pct(150);
    enh_fixture(0x17u, 8u);
    func_800710A4(ENH_ACTOR, ENH_CODEP);
    ASSERT((int32_t)PE_LoadU32(ENH_ACTOR + 0x70u) == 0x21C000, "always-run composes with 150 %");
    unlink(enh_conf);
    PASS();
}

static void test_ENH_config_persists(void)
{
    FILE *f;
    char line[64] = {0};
    TEST("ENH_config_persists");
    enh_isolate();
    pe_enh_set_move_speed_pct(175);
    pe_enh_set_always_run(1);
    f = fopen(enh_conf, "r");
    ASSERT(f != NULL, "setter writes the config file");
    if (f) { while (fgets(line, sizeof line, f) && strncmp(line, "move_speed=", 11)) {} fclose(f); }
    ASSERT(strncmp(line, "move_speed=175", 14) == 0, "config records move_speed=175");
    pe_enh_reset();
    ASSERT(pe_enh_move_speed_pct() == 175 && pe_enh_always_run() == 1, "settings reload from config");
    pe_enh_reset();
    setenv("PE_MOVE_SPEED", "150", 1);
    setenv("PE_ALWAYS_RUN", "0", 1);
    ASSERT(pe_enh_move_speed_pct() == 150 && pe_enh_always_run() == 0, "env overrides the config");
    unsetenv("PE_MOVE_SPEED");
    unsetenv("PE_ALWAYS_RUN");
    unlink(enh_conf);
    pe_enh_reset();
    ASSERT(pe_enh_move_speed_pct() == 100, "no config: back to 100 %");
    PASS();
}

static void test_ENH_dev_menu_rows(void)
{
    static uint8_t fb[320 * 240 * 3];
    TEST("ENH_dev_menu_rows");
    enh_isolate();
    setenv("PE_DEV_MENU", "1", 1);
    pe_plat_cheats_reset();
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DEV_MENU, 1);
    for (int i = 0; i < 6; i++) (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);   /* OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> OPTIONS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* -> ENHANCEMENTS */
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);                             /* speed 125 */
    ASSERT(pe_enh_move_speed_pct() == 125, "Options > Enhancements: A cycles movement speed");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_move_speed_pct() == 100, "cycle wraps 200 -> 100");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_DOWN, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_ENTER, 1);
    ASSERT(pe_enh_always_run() == 1, "second row toggles always-run");
    ASSERT(pe_plat_cheats_overlay(fb, 320, 240) != fb, "enhancements page draws");
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1);
    (void)pe_plat_cheats_key(PE_CHEAT_KEY_BACK, 1);
    ASSERT(!pe_plat_cheats_menu_open(), "B backs out to closed");
    unsetenv("PE_DEV_MENU");
    pe_plat_cheats_reset();
    unlink(enh_conf);
    pe_enh_reset();
    PASS();
}

static void test_ENH_all(void)
{
    test_ENH_default_is_retail();
    test_ENH_field_speed_scales();
    test_ENH_only_aya_in_field();
    test_ENH_battle_handlers_unscaled();
    test_ENH_always_run();
    test_ENH_config_persists();
    test_ENH_dev_menu_rows();
    enh_isolate();                                  /* leave defaults for later tests */
    enh_hermetic();
}
