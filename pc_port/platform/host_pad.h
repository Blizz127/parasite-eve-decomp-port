/*
 * Host gamepad input — Linux kernel joystick API (/dev/input/js*).
 *
 * Zero new link dependencies: uses only <linux/joystick.h> and plain
 * open/read/ioctl.  Devices are opened non-blocking and rescanned about once
 * per second, so a controller may be plugged in or removed at any time.
 *
 * Mapping is by evdev code (JSIOCGBTNMAP / JSIOCGAXMAP), falling back to the
 * xpad index layout when the ioctls are unavailable.  Default layout (Xbox /
 * Steam Input / Steam Deck / Legion Go):
 *   A=Cross  B=Circle  X=Square  Y=Triangle  LB/RB=L1/R1  LT/RT=L2/R2
 *   Back/View=Select  Start/Menu=Start  D-pad + left stick = directions
 *   Port hotkeys (not PSX buttons): R3 (right stick click) = fast-forward
 *   while held; L3 (left stick click) = toggle fast-forward (like F6)
 *
 * Environment:
 *   PE_JOYSTICK=/dev/input/jsN   use only this device (default: js0..js7)
 *   PE_JOYSTICK=off              disable controller input
 *   PE_PAD_DEBUG=1               print every change of the held mask
 */
#ifndef HOST_PAD_H
#define HOST_PAD_H

#include <stdint.h>

#define HOST_PAD_SELECT   0x0001u
#define HOST_PAD_START    0x0008u
#define HOST_PAD_UP       0x0010u
#define HOST_PAD_RIGHT    0x0020u
#define HOST_PAD_DOWN     0x0040u
#define HOST_PAD_LEFT     0x0080u
#define HOST_PAD_L2       0x0100u
#define HOST_PAD_R2       0x0200u
#define HOST_PAD_L1       0x0400u
#define HOST_PAD_R1       0x0800u
#define HOST_PAD_TRIANGLE 0x1000u
#define HOST_PAD_CIRCLE   0x2000u
#define HOST_PAD_CROSS    0x4000u
#define HOST_PAD_SQUARE   0x8000u

/* Host hotkeys from stick clicks (never reach the game's pad word). */
#define HOST_PAD_HOTKEY_FF_HOLD   0x01u  /* R3 held */
#define HOST_PAD_HOTKEY_FF_TOGGLE 0x02u  /* L3 held (toggle on press) */

/* Left-stick deadzone and trigger threshold (js range is -32767..32767). */
#define HOST_PAD_STICK_DEADZONE   12000
#define HOST_PAD_TRIGGER_THRESHOLD 0   /* triggers rest at -32767 */

/* Per-device decoded state (exposed for tests). */
typedef struct {
    uint16_t btn_code[512];  /* js button index -> evdev code */
    uint8_t  axis_code[64];  /* js axis index   -> evdev ABS code */
    uint16_t buttons;        /* held PSX bits from buttons */
    int16_t  axis[64];       /* last value per js axis index */
    int      trigger_seen[64]; /* trigger axis moved/initialised */
    uint8_t  hotkeys;        /* held HOST_PAD_HOTKEY_* bits */
} HostPadDevice;

/* Fill the fallback (xpad) index layout. */
void     HostPad_DeviceDefaults(HostPadDevice *dev);
/* Apply one js_event (type/number/value); returns nothing, updates dev. */
void     HostPad_DeviceEvent(HostPadDevice *dev, uint8_t type, uint8_t number, int16_t value);
/* Held PSX bits (active-high) for one device. */
uint16_t HostPad_DeviceHeld(const HostPadDevice *dev);
/* Held HOST_PAD_HOTKEY_* bits for one device. */
uint8_t  HostPad_DeviceHotkeys(const HostPadDevice *dev);

/* Poll all devices (non-blocking, hot-plug rescan). Safe to call every frame. */
void     HostPad_Poll(void);
/* Held PSX bits (active-high) OR-ed over all connected controllers. */
uint16_t HostPad_Held(void);
/* Held HOST_PAD_HOTKEY_* bits OR-ed over all connected controllers. */
uint8_t  HostPad_Hotkeys(void);
/* Close all devices. */
void     HostPad_Close(void);

#endif
