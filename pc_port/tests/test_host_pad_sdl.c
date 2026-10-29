/* SDL2 pad backend tests: the pure GameController / raw-joystick mappings,
 * then the runtime against tests/stub_sdl2.c (argv[1]) via PE_SDL2_LIB:
 * controller and raw-joystick hot-plug, detach, mapping-db load, and the
 * js* fallback when SDL2 is missing or fails to initialise. No hardware. */
#define _GNU_SOURCE
#include "host_pad.h"
#include "host_pad_sdl.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_fail;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); g_fail = 1; } } while (0)

static void test_controller_map(void)
{
    uint8_t b[HPS_BTN_COUNT] = {0}, hk = 0xFF;
    int16_t a[HPS_AXIS_COUNT] = {0};
    static const struct { int btn; uint16_t want; } one[] = {
        {HPS_BTN_A, HOST_PAD_CROSS}, {HPS_BTN_B, HOST_PAD_CIRCLE}, {HPS_BTN_X, HOST_PAD_SQUARE},
        {HPS_BTN_Y, HOST_PAD_TRIANGLE}, {HPS_BTN_BACK, HOST_PAD_SELECT}, {HPS_BTN_START, HOST_PAD_START},
        {HPS_BTN_LSHOULDER, HOST_PAD_L1}, {HPS_BTN_RSHOULDER, HOST_PAD_R1},
        {HPS_BTN_DPAD_UP, HOST_PAD_UP}, {HPS_BTN_DPAD_DOWN, HOST_PAD_DOWN},
        {HPS_BTN_DPAD_LEFT, HOST_PAD_LEFT}, {HPS_BTN_DPAD_RIGHT, HOST_PAD_RIGHT},
        {HPS_BTN_GUIDE, 0},
    };
    CHECK(HostPadSDL_MapController(b, a, 0, &hk) == 0 && hk == 0);
    for (unsigned i = 0; i < sizeof(one) / sizeof(one[0]); i++) {
        memset(b, 0, sizeof(b));
        b[one[i].btn] = 1;
        CHECK(HostPadSDL_MapController(b, a, 0, &hk) == one[i].want);
        CHECK(hk == 0);
    }
    memset(b, 0, sizeof(b));
    b[HPS_BTN_X] = 1;
    CHECK(HostPadSDL_MapController(b, a, 1, NULL) == HOST_PAD_TRIANGLE);   /* PE_PAD_SWAP_XY */
    memset(b, 0, sizeof(b));
    b[HPS_BTN_RSTICK] = 1; b[HPS_BTN_LSTICK] = 1;
    CHECK(HostPadSDL_MapController(b, a, 0, &hk) == 0);
    CHECK(hk == (HOST_PAD_HOTKEY_FF_HOLD | HOST_PAD_HOTKEY_FF_TOGGLE));
    memset(b, 0, sizeof(b));
    a[HPS_AXIS_TL] = HOST_PAD_SDL_TRIGGER_THRESHOLD - 1; a[HPS_AXIS_TR] = 32767;
    CHECK(HostPadSDL_MapController(b, a, 0, NULL) == HOST_PAD_R2);
    a[HPS_AXIS_TL] = HOST_PAD_SDL_TRIGGER_THRESHOLD; a[HPS_AXIS_TR] = 0;
    CHECK(HostPadSDL_MapController(b, a, 0, NULL) == HOST_PAD_L2);
    memset(a, 0, sizeof(a));
    a[HPS_AXIS_LX] = -32768; a[HPS_AXIS_LY] = 32767;
    CHECK(HostPadSDL_MapController(b, a, 0, NULL) == (HOST_PAD_LEFT | HOST_PAD_DOWN));
    a[HPS_AXIS_LX] = HOST_PAD_STICK_DEADZONE - 1; a[HPS_AXIS_LY] = -(HOST_PAD_STICK_DEADZONE - 1);
    CHECK(HostPadSDL_MapController(b, a, 0, NULL) == 0);                  /* inside the deadzone */
    a[HPS_AXIS_RX] = 32767;                                               /* right stick unused */
    CHECK(HostPadSDL_MapController(b, a, 0, NULL) == 0);

    /* Opt-in ANALOG STICK: stick_dpad = 0 drops only the left-stick bits. */
    memset(a, 0, sizeof(a));
    a[HPS_AXIS_LX] = -32768; a[HPS_AXIS_LY] = 32767;
    CHECK(HostPadSDL_MapControllerEx(b, a, 0, 1, NULL) == (HOST_PAD_LEFT | HOST_PAD_DOWN));
    CHECK(HostPadSDL_MapControllerEx(b, a, 0, 0, NULL) == 0);
    b[HPS_BTN_DPAD_UP] = 1; b[HPS_BTN_A] = 1;
    CHECK(HostPadSDL_MapControllerEx(b, a, 0, 0, NULL) == (HOST_PAD_UP | HOST_PAD_CROSS));
    CHECK(HostPadSDL_MapController(b, a, 0, NULL) == HostPadSDL_MapControllerEx(b, a, 0, 1, NULL));
    memset(b, 0, sizeof(b));

    /* SDL axis -> DualShock stick byte: 0x80 centre, up/left 0x00, down/right 0xFF. */
    CHECK(HostPadSDL_AxisToByte(-32768) == 0x00 && HostPadSDL_AxisToByte(0) == 0x80 &&
          HostPadSDL_AxisToByte(32767) == 0xFF);
    CHECK(HostPadSDL_AxisToByte(-16384) == 0x40 && HostPadSDL_AxisToByte(16384) == 0xC0);
    CHECK(HostPadSDL_AxisToByte(-1) == 0x7F && HostPadSDL_AxisToByte(255) == 0x80 &&
          HostPadSDL_AxisToByte(256) == 0x81);
    for (int v = -32768; v < 32767; v++)
        if (HostPadSDL_AxisToByte((int16_t)(v + 1)) < HostPadSDL_AxisToByte((int16_t)v)) { CHECK(!"monotonic"); break; }
}

static void test_raw_map(void)
{
    HostPadRawMap m;
    uint8_t b[16] = {0}, hk = 0;
    int16_t a[2] = {0, 0};
    HostPadSDL_RawMapDefaults(&m);
    b[0] = 1; CHECK(HostPadSDL_MapJoystick(&m, b, 16, a, 2, 0, 0, &hk) == HOST_PAD_CROSS);
    b[0] = 0; b[7] = 1; CHECK(HostPadSDL_MapJoystick(&m, b, 16, a, 2, 0, 0, &hk) == HOST_PAD_START);
    b[7] = 0; b[10] = 1; CHECK(HostPadSDL_MapJoystick(&m, b, 16, a, 2, 0, 0, &hk) == 0 && hk == HOST_PAD_HOTKEY_FF_HOLD);
    b[10] = 0;
    CHECK(HostPadSDL_MapJoystick(&m, b, 16, a, 2, HPS_HAT_UP | HPS_HAT_RIGHT, 0, &hk) == (HOST_PAD_UP | HOST_PAD_RIGHT));
    a[1] = 32767; CHECK(HostPadSDL_MapJoystick(&m, b, 16, a, 2, 0, 0, &hk) == HOST_PAD_DOWN);
    a[1] = 0;
    b[3] = 1; CHECK(HostPadSDL_MapJoystick(&m, b, 3, a, 2, 0, 0, &hk) == 0);  /* index beyond nbtn */
    /* a DirectInput PS1 adapter: Triangle Circle Cross Square L2 R2 L1 R1 Select Start */
    CHECK(HostPadSDL_RawMapParse(&m, "2,1,3,0,6,7,4,5,8,9") == 0);
    memset(b, 0, sizeof(b));
    b[2] = 1; b[4] = 1;
    CHECK(HostPadSDL_MapJoystick(&m, b, 16, a, 2, 0, 0, &hk) == (HOST_PAD_CROSS | HOST_PAD_L2));
    CHECK(HostPadSDL_RawMapParse(&m, "1,2,3") != 0);
    CHECK(HostPadSDL_RawMapParse(&m, "1,2,3,4,5,6,7,8,9,x") != 0);
    CHECK(m.cross == 2);                                                  /* unchanged on error */
    CHECK(HostPadSDL_RawMapParse(&m, "0,1,2,3,4,5,-1,-1,6,7") == 0 && m.l2 == -1);
}

static void sleep_scan(void)
{   /* the backend rescans about once a second */
    struct timespec ts = {1, 50000000};
    nanosleep(&ts, NULL);
}

static void test_runtime(const char *stub)
{
    void *h = dlopen(stub, RTLD_NOW | RTLD_GLOBAL);
    void (*reset)(void), (*plug)(int, int32_t, int, int, int, int), (*unplug)(int);
    void (*button)(int, int, int), (*axis)(int, int, int), (*hat)(int, int), (*init_fail)(int);
    int (*inited)(void), (*hints)(void), (*mappings)(void);
    FILE *f;
    CHECK(h != NULL);
    if (!h) { fprintf(stderr, "%s\n", dlerror()); return; }
    *(void **)&reset = dlsym(h, "stub_reset");   *(void **)&plug = dlsym(h, "stub_plug");
    *(void **)&unplug = dlsym(h, "stub_unplug"); *(void **)&button = dlsym(h, "stub_button");
    *(void **)&axis = dlsym(h, "stub_axis");     *(void **)&hat = dlsym(h, "stub_hat");
    *(void **)&init_fail = dlsym(h, "stub_set_init_fail");
    *(void **)&inited = dlsym(h, "stub_inited"); *(void **)&hints = dlsym(h, "stub_hints");
    *(void **)&mappings = dlsym(h, "stub_mappings_added");
    setenv("PE_SDL2_LIB", stub, 1);
    setenv("PE_JOYSTICK", "off", 1);

    /* mapping db from PE_GAMECONTROLLERDB */
    f = fopen("pe_test_gamecontrollerdb.txt", "w");
    if (f) { fputs("# test\n", f); fclose(f); }
    setenv("PE_GAMECONTROLLERDB", "pe_test_gamecontrollerdb.txt", 1);

    reset();
    plug(0, 7, 1, 0, 0, 0);                                    /* Steam virtual pad */
    HostPad_Poll();
    CHECK(inited() == 1);
    CHECK(hints() >= 2);
    CHECK(mappings() == 1);
    CHECK(HostPadSDL_Count() == 1);
    button(0, HPS_BTN_A, 1); axis(0, HPS_AXIS_TR, 32767);
    HostPad_Poll();
    CHECK(HostPad_Held() == (HOST_PAD_CROSS | HOST_PAD_R2));
    button(0, HPS_BTN_A, 0); axis(0, HPS_AXIS_TR, 0); button(0, HPS_BTN_RSTICK, 1);
    HostPad_Poll();
    CHECK(HostPad_Held() == 0 && HostPad_Hotkeys() == HOST_PAD_HOTKEY_FF_HOLD);
    button(0, HPS_BTN_RSTICK, 0);

    /* Opt-in ANALOG STICK: a GameController reports RX, RY, LX, LY bytes and
     * the analog held mask leaves its left stick out of the d-pad bits. */
    {
        uint8_t st[4] = {0, 0, 0, 0};
        axis(0, HPS_AXIS_LX, -32768); axis(0, HPS_AXIS_LY, 16384);
        axis(0, HPS_AXIS_RX, 32767);  axis(0, HPS_AXIS_RY, 0);
        button(0, HPS_BTN_DPAD_RIGHT, 1);
        HostPad_Poll();
        CHECK(HostPadSDL_AnalogSticks(st) == 1);
        CHECK(st[0] == 0xFF && st[1] == 0x80 && st[2] == 0x00 && st[3] == 0xC0);
        CHECK(HostPad_Held() == (HOST_PAD_LEFT | HOST_PAD_DOWN | HOST_PAD_RIGHT));
        CHECK(HostPad_HeldAnalog() == HOST_PAD_RIGHT);
        CHECK(HostPadSDL_HeldAnalog() == HOST_PAD_RIGHT);
        axis(0, HPS_AXIS_LX, 0); axis(0, HPS_AXIS_LY, 0); axis(0, HPS_AXIS_RX, 0);
        button(0, HPS_BTN_DPAD_RIGHT, 0);
        HostPad_Poll();
        CHECK(HostPadSDL_AnalogSticks(st) == 1 && st[0] == 0x80 && st[1] == 0x80 &&
              st[2] == 0x80 && st[3] == 0x80);
    }

    /* hot-plug a raw (unmapped) joystick: hat + button 1 = Circle */
    plug(1, 9, 0, 12, 2, 1);
    sleep_scan();
    HostPad_Poll();
    CHECK(HostPadSDL_Count() == 2);
    hat(1, HPS_HAT_LEFT); button(1, 1, 1);
    HostPad_Poll();
    CHECK(HostPad_Held() == (HOST_PAD_LEFT | HOST_PAD_CIRCLE));

    /* unplug the controller: its state drops, the joystick stays */
    button(0, HPS_BTN_START, 1);
    unplug(0);
    HostPad_Poll();
    CHECK(HostPadSDL_Count() == 1);
    CHECK(HostPad_Held() == (HOST_PAD_LEFT | HOST_PAD_CIRCLE));
    {   /* a raw joystick alone is no analog source; its mask is unchanged */
        uint8_t st[4] = {1, 2, 3, 4};
        CHECK(HostPadSDL_AnalogSticks(st) == 0 && st[0] == 1 && st[3] == 4);
        CHECK(HostPad_HeldAnalog() == (HOST_PAD_LEFT | HOST_PAD_CIRCLE));
    }
    unplug(1);
    HostPad_Poll();
    CHECK(HostPadSDL_Count() == 0 && HostPad_Held() == 0);

    /* re-plug after a detach with a new instance id */
    plug(0, 11, 1, 0, 0, 0);
    sleep_scan();
    HostPad_Poll();
    CHECK(HostPadSDL_Count() == 1);
    HostPad_Close();
    CHECK(inited() == 0);

    /* init failure -> backend unavailable, js path remains (no crash) */
    reset();
    init_fail(1);
    plug(0, 3, 1, 0, 0, 0);
    HostPad_Poll();
    CHECK(HostPadSDL_Count() == 0 && HostPad_Held() == 0);
    HostPad_Close();

    /* missing library -> fallback, no crash */
    setenv("PE_SDL2_LIB", "/nonexistent/libSDL2-2.0.so.0", 1);
    HostPad_Poll();
    CHECK(HostPadSDL_Count() == 0);
    HostPad_Close();

    /* PE_PAD_SDL=off disables it */
    setenv("PE_SDL2_LIB", stub, 1);
    setenv("PE_PAD_SDL", "off", 1);
    reset();
    plug(0, 5, 1, 0, 0, 0);
    HostPad_Poll();
    CHECK(inited() == 0 && HostPadSDL_Count() == 0);
    HostPad_Close();
    unsetenv("PE_PAD_SDL");
    remove("pe_test_gamecontrollerdb.txt");
}

int main(int argc, char **argv)
{
    test_controller_map();
    test_raw_map();
    if (argc > 1) test_runtime(argv[1]);
    else { fprintf(stderr, "FAIL: stub library path not given\n"); g_fail = 1; }
    if (g_fail) return 1;
    printf("host-pad-sdl: OK\n");
    return 0;
}
