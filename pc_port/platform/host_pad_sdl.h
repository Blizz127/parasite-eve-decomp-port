/*
 * Host gamepad input — SDL2 GameController backend (dlopen, no link dep).
 *
 * Why: under Steam / Gamescope (Bazzite, SteamOS, Steam Deck, Legion Go)
 * the kernel joystick nodes (/dev/input/js*) are often absent (no joydev)
 * or hidden behind Steam Input, which presents its own virtual pad. SDL2
 * reads evdev/hidraw directly, honours Steam's SDL_GAMECONTROLLERCONFIG and
 * SDL_GAMECONTROLLER_IGNORE_DEVICES, and ships the SDL_GameControllerDB
 * mappings, so the controller works however Steam exposes it.
 *
 * libSDL2-2.0.so.0 is dlopened at the first poll. If it is missing or fails
 * to initialise, host_pad.c keeps the /dev/input/js* path (unchanged).
 *
 * Devices:
 *   - SDL_IsGameController -> GameController API (Xbox positional layout):
 *       A=Cross  B=Circle  X=Square  Y=Triangle  LB/RB=L1/R1  LT/RT=L2/R2
 *       Back=Select  Start=Start  D-pad + left stick = directions
 *       R3 = fast-forward while held, L3 = toggle fast-forward (port hotkeys)
 *     Nintendo-labelled pads stay positional (SDL_GAMECONTROLLER_USE_BUTTON_LABELS=0).
 *   - anything else -> raw SDL_Joystick fallback: buttons by index in the
 *     xpad order (A B X Y LB RB Back Start Guide LS RS), hat 0 and axes 0/1
 *     as directions; PE_PAD_RAW_MAP overrides the button indices.
 * Hot-plug: devices are rescanned about once a second.
 *
 * Mappings: SDL's built-in database, then (if present) the first readable of
 *   $PE_GAMECONTROLLERDB, <exe dir>/gamecontrollerdb.txt,
 *   $XDG_CONFIG_HOME/parasite-eve-port/gamecontrollerdb.txt
 *   (~/.config/... when XDG_CONFIG_HOME is unset); Steam's
 *   SDL_GAMECONTROLLERCONFIG environment is applied by SDL itself.
 *
 * Environment:
 *   PE_PAD_SDL=off          disable this backend (use /dev/input/js* only)
 *   PE_SDL2_LIB=path        dlopen this library instead of libSDL2-2.0.so.0
 *   PE_PAD_RAW_MAP=c,o,s,t,l1,r1,l2,r2,select,start
 *                           raw-joystick button indices (-1 = unused) for
 *                           Cross,Circle,Square,Triangle,L1,R1,L2,R2,Select,Start
 *   PE_PAD_SWAP_XY=1        swap Square/Triangle (as for the js path)
 *   PE_PAD_DEBUG=1          log devices and every held-mask change
 */
#ifndef HOST_PAD_SDL_H
#define HOST_PAD_SDL_H

#include <stdint.h>
#include "host_pad_combo.h"

/* SDL_GameControllerButton / SDL_GameControllerAxis values (SDL2 ABI). */
enum {
    HPS_BTN_A, HPS_BTN_B, HPS_BTN_X, HPS_BTN_Y, HPS_BTN_BACK, HPS_BTN_GUIDE,
    HPS_BTN_START, HPS_BTN_LSTICK, HPS_BTN_RSTICK, HPS_BTN_LSHOULDER,
    HPS_BTN_RSHOULDER, HPS_BTN_DPAD_UP, HPS_BTN_DPAD_DOWN, HPS_BTN_DPAD_LEFT,
    HPS_BTN_DPAD_RIGHT, HPS_BTN_COUNT = 21
};
enum { HPS_AXIS_LX, HPS_AXIS_LY, HPS_AXIS_RX, HPS_AXIS_RY, HPS_AXIS_TL, HPS_AXIS_TR, HPS_AXIS_COUNT };
/* SDL_HAT_* bits */
#define HPS_HAT_UP    0x01u
#define HPS_HAT_RIGHT 0x02u
#define HPS_HAT_DOWN  0x04u
#define HPS_HAT_LEFT  0x08u

/* SDL triggers rest at 0 and reach 32767. */
#define HOST_PAD_SDL_TRIGGER_THRESHOLD 16000

/* Raw-joystick button map: index per PSX button, -1 = unused. */
typedef struct { int cross, circle, square, triangle, l1, r1, l2, r2, select, start, l3, r3; } HostPadRawMap;

/* Pure mappings (exposed for tests). */
uint16_t HostPadSDL_MapController(const uint8_t btn[HPS_BTN_COUNT], const int16_t axis[HPS_AXIS_COUNT],
                                  int swap_xy, uint8_t *hotkeys);
/* stick_dpad = 0 drops the left-stick -> d-pad bits (opt-in ANALOG STICK:
 * the stick then reaches the game as analog bytes, as on a DualShock). */
uint16_t HostPadSDL_MapControllerEx(const uint8_t btn[HPS_BTN_COUNT], const int16_t axis[HPS_AXIS_COUNT],
                                    int swap_xy, int stick_dpad, uint8_t *hotkeys);
/* SDL axis (-32768..32767, 0 = centre; up/left negative) to a DualShock
 * stick byte (0x00..0xFF, 0x80 = centre; up/left 0x00): linear, exact at
 * both ends and at the centre. */
static inline uint8_t HostPadSDL_AxisToByte(int16_t v)
{
    return (uint8_t)(((int32_t)v + 32768) >> 8);
}
void     HostPadSDL_RawMapDefaults(HostPadRawMap *m);
/* Parse "c,o,s,t,l1,r1,l2,r2,select,start"; returns 0 on success (m unchanged on error). */
int      HostPadSDL_RawMapParse(HostPadRawMap *m, const char *spec);
uint16_t HostPadSDL_MapJoystick(const HostPadRawMap *m, const uint8_t *btn, int nbtn,
                                const int16_t *axis, int naxis, unsigned hat, int swap_xy,
                                uint8_t *hotkeys);

/* Runtime (no-ops returning 0 when SDL2 is unavailable). */
void     HostPadSDL_Poll(void);
int      HostPadSDL_Count(void);      /* open SDL devices */
uint16_t HostPadSDL_Held(void);
uint8_t  HostPadSDL_Hotkeys(void);
/* Opt-in ANALOG STICK.  HostPadSDL_HeldAnalog: HostPadSDL_Held without the
 * GameControllers' left-stick -> d-pad bits.  HostPadSDL_AnalogSticks: 1 when
 * at least one GameController is open, with out[] = RX, RY, LX, LY bytes
 * (per axis the most-deflected open GameController); 0 otherwise. */
uint16_t HostPadSDL_HeldAnalog(void);
int      HostPadSDL_AnalogSticks(uint8_t out[4]);
/* Open SDL devices as HostPadOrigin (key 0x40000000 | SDL instance id; SDL
 * never reuses an instance id, so a reconnect is a new key). */
int      HostPadSDL_Origins(HostPadOrigin *out, int max);
void     HostPadSDL_Close(void);

#endif
