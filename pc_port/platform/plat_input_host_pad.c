/*
 * pe_plat input backend -> host_pad (Linux joystick) (step P0 adapter).
 *
 * host_pad reports the console's button layout active-high, identical to
 * PE_PLAT_BTN_*; the static asserts pin that so a remap is never silent.
 * Links only where host_pad.c is linked (the POSIX windowed executable).
 */
#include "pe_plat/input.h"

#include <stddef.h>

#include "host_pad.h"

_Static_assert(HOST_PAD_SELECT == PE_PLAT_BTN_SELECT && HOST_PAD_START == PE_PLAT_BTN_START &&
               HOST_PAD_UP == PE_PLAT_BTN_UP && HOST_PAD_RIGHT == PE_PLAT_BTN_RIGHT &&
               HOST_PAD_DOWN == PE_PLAT_BTN_DOWN && HOST_PAD_LEFT == PE_PLAT_BTN_LEFT &&
               HOST_PAD_L2 == PE_PLAT_BTN_L2 && HOST_PAD_R2 == PE_PLAT_BTN_R2 &&
               HOST_PAD_L1 == PE_PLAT_BTN_L1 && HOST_PAD_R1 == PE_PLAT_BTN_R1 &&
               HOST_PAD_TRIANGLE == PE_PLAT_BTN_TRIANGLE && HOST_PAD_CIRCLE == PE_PLAT_BTN_CIRCLE &&
               HOST_PAD_CROSS == PE_PLAT_BTN_CROSS && HOST_PAD_SQUARE == PE_PLAT_BTN_SQUARE,
               "host_pad button layout must equal PE_PLAT_BTN_*");
_Static_assert(HOST_PAD_HOTKEY_FF_HOLD == PE_PLAT_HOTKEY_FAST_FORWARD_HOLD &&
               HOST_PAD_HOTKEY_FF_TOGGLE == PE_PLAT_HOTKEY_FAST_FORWARD_TOGGLE,
               "host_pad hotkeys must equal PE_PLAT_HOTKEY_*");

static void plat_hp_poll(void *ctx) { (void)ctx; HostPad_Poll(); }
static uint16_t plat_hp_held(void *ctx, int port)
{
    (void)ctx;
    return port == 0 ? HostPad_Held() : 0u;   /* host_pad ORs all devices into port 0 */
}
static uint8_t plat_hp_hotkeys(void *ctx) { (void)ctx; return HostPad_Hotkeys(); }
static void plat_hp_close(void *ctx) { (void)ctx; HostPad_Close(); }

static const PePlatInputBackend plat_host_pad_backend = {
    NULL, plat_hp_poll, plat_hp_held, plat_hp_hotkeys, plat_hp_close, "host_pad"
};

const PePlatInputBackend *pe_plat_input_host_pad_backend(void)
{
    return &plat_host_pad_backend;
}
