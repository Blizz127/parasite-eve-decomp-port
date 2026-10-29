/*
 * Windows controller input: host_pad.h over XInput (see host_pad_win32.h).
 * cmake/windows.cmake builds this file instead of host_pad.c/host_pad_sdl.c,
 * which read Linux joystick nodes and dlopen libSDL2.
 *
 * XInput is loaded on the first poll.  Connected slots are read every poll;
 * an empty slot is probed about once a second (XInputGetState on an empty
 * slot is slow), so a controller may be plugged in or removed at any time.
 * Each connection gets a new HostPadOrigin key, so the dev-menu combo machine
 * (host_pad_combo.c) sees a reconnect as a new controller.
 */
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <xinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "host_pad.h"
#include "host_pad_sdl.h"     /* HOST_PAD_SDL_TRIGGER_THRESHOLD, HostPadSDL_AxisToByte */
#include "host_pad_win32.h"

#define XI_SLOTS 4u           /* XUSER_MAX_COUNT */
#define XI_PROBE_MS 1000u

typedef DWORD (WINAPI *xi_get_state_fn)(DWORD, XINPUT_STATE *);

static struct {
    int tried, disabled, debug, swap_xy;
    HMODULE lib;
    xi_get_state_fn get_state;
    const char *lib_name;
    ULONGLONG last_probe;
    uint32_t next_key;
    uint16_t last_held;
    struct {
        int connected, input_seen;
        uint32_t key;
        XINPUT_GAMEPAD pad;
    } slot[XI_SLOTS];
} X;

/* SDL's convention for the shared thresholds: Y grows downward. */
static int16_t flip_y(int16_t v) { return v == INT16_MIN ? INT16_MAX : (int16_t)-v; }

uint16_t HostPadXInput_Map(uint16_t b, uint8_t lt, uint8_t rt, int16_t lx, int16_t ly,
                           int swap_xy, int stick_dpad, uint8_t *hotkeys)
{
    uint16_t h = 0;
    uint8_t hk = 0;
    int16_t sy = flip_y(ly);
    if (b & XINPUT_GAMEPAD_A) h |= HOST_PAD_CROSS;
    if (b & XINPUT_GAMEPAD_B) h |= HOST_PAD_CIRCLE;
    if (b & XINPUT_GAMEPAD_X) h |= swap_xy ? HOST_PAD_TRIANGLE : HOST_PAD_SQUARE;
    if (b & XINPUT_GAMEPAD_Y) h |= swap_xy ? HOST_PAD_SQUARE : HOST_PAD_TRIANGLE;
    if (b & XINPUT_GAMEPAD_BACK) h |= HOST_PAD_SELECT;
    if (b & XINPUT_GAMEPAD_START) h |= HOST_PAD_START;
    if (b & XINPUT_GAMEPAD_LEFT_SHOULDER) h |= HOST_PAD_L1;
    if (b & XINPUT_GAMEPAD_RIGHT_SHOULDER) h |= HOST_PAD_R1;
    if (b & XINPUT_GAMEPAD_DPAD_UP) h |= HOST_PAD_UP;
    if (b & XINPUT_GAMEPAD_DPAD_DOWN) h |= HOST_PAD_DOWN;
    if (b & XINPUT_GAMEPAD_DPAD_LEFT) h |= HOST_PAD_LEFT;
    if (b & XINPUT_GAMEPAD_DPAD_RIGHT) h |= HOST_PAD_RIGHT;
    if (b & XINPUT_GAMEPAD_RIGHT_THUMB) hk |= HOST_PAD_HOTKEY_FF_HOLD;
    if (b & XINPUT_GAMEPAD_LEFT_THUMB) hk |= HOST_PAD_HOTKEY_FF_TOGGLE;
    /* Triggers on SDL's 0..32767 scale, so the threshold is the SDL one. */
    if ((int)lt * 32767 / 255 >= HOST_PAD_SDL_TRIGGER_THRESHOLD) h |= HOST_PAD_L2;
    if ((int)rt * 32767 / 255 >= HOST_PAD_SDL_TRIGGER_THRESHOLD) h |= HOST_PAD_R2;
    if (stick_dpad) {
        if (lx <= -HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_LEFT;
        if (lx >=  HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_RIGHT;
        if (sy <= -HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_UP;
        if (sy >=  HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_DOWN;
    }
    if (hotkeys) *hotkeys = hk;
    return h;
}

static void xi_load(void)
{
    static const char *const names[] = {"xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll"};
    const char *env = getenv("PE_JOYSTICK");
    const char *s;
    X.tried = 1;
    s = getenv("PE_PAD_DEBUG");
    X.debug = s && *s && *s != '0';
    s = getenv("PE_PAD_SWAP_XY");
    X.swap_xy = s && *s && *s != '0';
    if (env && (!strcmp(env, "off") || !strcmp(env, "0") || !strcmp(env, "none"))) {
        X.disabled = 1;
        return;
    }
    for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]) && !X.get_state; i++) {
        X.lib = LoadLibraryA(names[i]);
        if (!X.lib) continue;
        X.get_state = (xi_get_state_fn)(void (*)(void))GetProcAddress(X.lib, "XInputGetState");
        if (X.get_state) X.lib_name = names[i];
        else { FreeLibrary(X.lib); X.lib = NULL; }
    }
    if (!X.get_state)
        fprintf(stderr, "[PAD] XInput unavailable: keyboard only\n");
    else if (X.debug)
        fprintf(stderr, "[PAD] XInput loaded (%s)\n", X.lib_name);
}

void HostPad_Poll(void)
{
    ULONGLONG now;
    int probe;
    if (!X.tried) xi_load();
    if (X.disabled || !X.get_state) return;
    now = GetTickCount64();
    probe = X.last_probe == 0 || now - X.last_probe >= XI_PROBE_MS;
    if (probe) X.last_probe = now ? now : 1;
    for (DWORD i = 0; i < XI_SLOTS; i++) {
        XINPUT_STATE st;
        if (!X.slot[i].connected && !probe) continue;
        memset(&st, 0, sizeof(st));
        if (X.get_state(i, &st) == ERROR_SUCCESS) {
            if (!X.slot[i].connected) {
                X.slot[i].connected = 1;
                X.slot[i].input_seen = 0;
                X.slot[i].key = 0x20000000u | ((++X.next_key & 0x3FFFFFFu) << 2) | i;
                fprintf(stderr, "[PAD] XInput controller %lu connected\n", (unsigned long)i);
            }
            X.slot[i].pad = st.Gamepad;
            if (!X.slot[i].input_seen &&
                HostPadXInput_Map(st.Gamepad.wButtons, st.Gamepad.bLeftTrigger,
                                  st.Gamepad.bRightTrigger, st.Gamepad.sThumbLX,
                                  st.Gamepad.sThumbLY, 0, 1, NULL)) {
                X.slot[i].input_seen = 1;
                fprintf(stderr, "[PAD] first input: XInput controller %lu\n", (unsigned long)i);
            }
        } else if (X.slot[i].connected) {
            memset(&X.slot[i], 0, sizeof(X.slot[i]));
            fprintf(stderr, "[PAD] XInput controller %lu removed\n", (unsigned long)i);
        }
    }
    if (X.debug) {
        uint16_t held = HostPad_Held();
        if (held != X.last_held)
            fprintf(stderr, "[PAD] held=0x%04X (PSX pad word 0x%04X)\n", held, (uint16_t)~held);
        X.last_held = held;
    }
}

static uint16_t slot_held(unsigned i, int stick_dpad, uint8_t *hk)
{
    const XINPUT_GAMEPAD *p = &X.slot[i].pad;
    return HostPadXInput_Map(p->wButtons, p->bLeftTrigger, p->bRightTrigger,
                             p->sThumbLX, p->sThumbLY, X.swap_xy, stick_dpad, hk);
}

uint16_t HostPad_Held(void)
{
    uint16_t held = 0;
    for (unsigned i = 0; i < XI_SLOTS; i++)
        if (X.slot[i].connected) held |= slot_held(i, 1, NULL);
    return held;
}

uint16_t HostPad_HeldAnalog(void)
{
    uint16_t held = 0;
    for (unsigned i = 0; i < XI_SLOTS; i++)
        if (X.slot[i].connected) held |= slot_held(i, 0, NULL);
    return held;
}

uint8_t HostPad_Hotkeys(void)
{
    uint8_t all = 0;
    for (unsigned i = 0; i < XI_SLOTS; i++)
        if (X.slot[i].connected) {
            uint8_t hk = 0;
            (void)slot_held(i, 1, &hk);
            all |= hk;
        }
    return all;
}

int HostPad_Origins(HostPadOrigin *out, int max)
{
    int n = 0;
    for (unsigned i = 0; i < XI_SLOTS && n < max; i++)
        if (X.slot[i].connected) {
            out[n].key = X.slot[i].key;
            out[n].held = slot_held(i, 1, NULL);
            n++;
        }
    return n;
}

/* The most-deflected value per axis over the connected controllers. */
static void deflect(int16_t *best, int16_t v)
{
    int a = v < 0 ? -(int)v : v, b = *best < 0 ? -(int)*best : *best;
    if (a > b) *best = v;
}

int HostPadXInput_AnalogSticks(uint8_t out[4])
{
    int16_t rx = 0, ry = 0, lx = 0, ly = 0;
    int any = 0;
    for (unsigned i = 0; i < XI_SLOTS; i++) {
        const XINPUT_GAMEPAD *p = &X.slot[i].pad;
        if (!X.slot[i].connected) continue;
        any = 1;
        deflect(&rx, p->sThumbRX); deflect(&ry, flip_y(p->sThumbRY));
        deflect(&lx, p->sThumbLX); deflect(&ly, flip_y(p->sThumbLY));
    }
    if (!any) return 0;
    out[0] = HostPadSDL_AxisToByte(rx);
    out[1] = HostPadSDL_AxisToByte(ry);
    out[2] = HostPadSDL_AxisToByte(lx);
    out[3] = HostPadSDL_AxisToByte(ly);
    return 1;
}

void HostPad_Close(void)
{
    for (unsigned i = 0; i < XI_SLOTS; i++) memset(&X.slot[i], 0, sizeof(X.slot[i]));
    X.last_probe = 0;
    X.last_held = 0;
}
#endif /* _WIN32 */
