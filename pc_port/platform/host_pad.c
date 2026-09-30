/*
 * Host gamepad input via the Linux kernel joystick API.  See host_pad.h.
 */
#include "host_pad.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <linux/joystick.h>

/* evdev codes (linux/input-event-codes.h values, spelled out so the mapping
 * is explicit about the xpad convention: Xbox X -> 0x133, Y -> 0x134). */
#define EV_BTN_A        0x130
#define EV_BTN_B        0x131
#define EV_BTN_C        0x132
#define EV_BTN_X        0x133
#define EV_BTN_Y        0x134
#define EV_BTN_Z        0x135
#define EV_BTN_TL       0x136
#define EV_BTN_TR       0x137
#define EV_BTN_TL2      0x138
#define EV_BTN_TR2      0x139
#define EV_BTN_SELECT   0x13a
#define EV_BTN_START    0x13b
#define EV_BTN_THUMBL   0x13d
#define EV_BTN_THUMBR   0x13e
#define EV_BTN_DPAD_UP    0x220
#define EV_BTN_DPAD_DOWN  0x221
#define EV_BTN_DPAD_LEFT  0x222
#define EV_BTN_DPAD_RIGHT 0x223

#define EV_ABS_X      0x00
#define EV_ABS_Y      0x01
#define EV_ABS_Z      0x02
#define EV_ABS_RZ     0x05
#define EV_ABS_GAS    0x09
#define EV_ABS_BRAKE  0x0a
#define EV_ABS_HAT0X  0x10
#define EV_ABS_HAT0Y  0x11

#define HOST_PAD_MAX_DEVICES 4
#define HOST_PAD_SCAN_NS     1000000000ull

static int g_swap_xy = -1;

static uint16_t button_bit(uint16_t code)
{
    if (g_swap_xy < 0) {
        const char *s = getenv("PE_PAD_SWAP_XY");
        g_swap_xy = (s && *s && *s != '0') ? 1 : 0;
    }
    switch (code) {
    case EV_BTN_A:          return HOST_PAD_CROSS;
    case EV_BTN_B:          return HOST_PAD_CIRCLE;
    case EV_BTN_X:          return g_swap_xy ? HOST_PAD_TRIANGLE : HOST_PAD_SQUARE;
    case EV_BTN_Y:          return g_swap_xy ? HOST_PAD_SQUARE : HOST_PAD_TRIANGLE;
    case EV_BTN_TL:         return HOST_PAD_L1;
    case EV_BTN_TR:         return HOST_PAD_R1;
    case EV_BTN_TL2:        return HOST_PAD_L2;
    case EV_BTN_TR2:        return HOST_PAD_R2;
    case EV_BTN_SELECT:     return HOST_PAD_SELECT;
    case EV_BTN_START:      return HOST_PAD_START;
    case EV_BTN_DPAD_UP:    return HOST_PAD_UP;
    case EV_BTN_DPAD_DOWN:  return HOST_PAD_DOWN;
    case EV_BTN_DPAD_LEFT:  return HOST_PAD_LEFT;
    case EV_BTN_DPAD_RIGHT: return HOST_PAD_RIGHT;
    default:                return 0;
    }
}

void HostPad_DeviceDefaults(HostPadDevice *dev)
{
    /* xpad index layout: buttons A B X Y LB RB Back Start Guide LS RS;
     * axes LX LY LT RX RY RT HatX HatY. */
    static const uint16_t btn[] = {EV_BTN_A, EV_BTN_B, EV_BTN_X, EV_BTN_Y,
                                   EV_BTN_TL, EV_BTN_TR, EV_BTN_SELECT,
                                   EV_BTN_START, 0x13c, EV_BTN_THUMBL,
                                   EV_BTN_THUMBR};
    static const uint8_t ax[] = {EV_ABS_X, EV_ABS_Y, EV_ABS_Z, 0x03, 0x04,
                                 EV_ABS_RZ, EV_ABS_HAT0X, EV_ABS_HAT0Y};
    memset(dev, 0, sizeof(*dev));
    for (unsigned i = 0; i < sizeof(dev->axis_code); i++) dev->axis_code[i] = 0x3f;
    memcpy(dev->btn_code, btn, sizeof(btn));
    memcpy(dev->axis_code, ax, sizeof(ax));
}

void HostPad_DeviceEvent(HostPadDevice *dev, uint8_t type, uint8_t number, int16_t value)
{
    type &= (uint8_t)~JS_EVENT_INIT;
    if (type == JS_EVENT_BUTTON) {
        uint16_t code = dev->btn_code[number];
        uint8_t hk = code == EV_BTN_THUMBR ? HOST_PAD_HOTKEY_FF_HOLD :
                     code == EV_BTN_THUMBL ? HOST_PAD_HOTKEY_FF_TOGGLE : 0u;
        uint16_t bit = button_bit(code);
        if (hk) {
            if (value) dev->hotkeys |= hk;
            else       dev->hotkeys &= (uint8_t)~hk;
            return;
        }
        if (!bit) return;
        if (value) dev->buttons |= bit;
        else       dev->buttons &= (uint16_t)~bit;
    } else if (type == JS_EVENT_AXIS && number < 64) {
        dev->axis[number] = value;
        dev->trigger_seen[number] = 1;
    }
}

uint16_t HostPad_DeviceHeld(const HostPadDevice *dev)
{
    uint16_t held = dev->buttons;
    for (unsigned i = 0; i < 64; i++) {
        int v = dev->axis[i];
        switch (dev->axis_code[i]) {
        case EV_ABS_X:
            if (v <= -HOST_PAD_STICK_DEADZONE) held |= HOST_PAD_LEFT;
            if (v >=  HOST_PAD_STICK_DEADZONE) held |= HOST_PAD_RIGHT;
            break;
        case EV_ABS_Y:
            if (v <= -HOST_PAD_STICK_DEADZONE) held |= HOST_PAD_UP;
            if (v >=  HOST_PAD_STICK_DEADZONE) held |= HOST_PAD_DOWN;
            break;
        case EV_ABS_HAT0X:
            if (v < 0) held |= HOST_PAD_LEFT;
            if (v > 0) held |= HOST_PAD_RIGHT;
            break;
        case EV_ABS_HAT0Y:
            if (v < 0) held |= HOST_PAD_UP;
            if (v > 0) held |= HOST_PAD_DOWN;
            break;
        case EV_ABS_Z: case EV_ABS_BRAKE:
            /* Only once reported: an unseen trigger reads 0, not "rest". */
            if (dev->trigger_seen[i] && v > HOST_PAD_TRIGGER_THRESHOLD) held |= HOST_PAD_L2;
            break;
        case EV_ABS_RZ: case EV_ABS_GAS:
            if (dev->trigger_seen[i] && v > HOST_PAD_TRIGGER_THRESHOLD) held |= HOST_PAD_R2;
            break;
        default:
            break;
        }
    }
    return held;
}

uint8_t HostPad_DeviceHotkeys(const HostPadDevice *dev)
{
    return dev->hotkeys;
}

/* ── Device management ─────────────────────────────────────────────── */
typedef struct {
    int fd;
    char path[64];
    HostPadDevice dev;
} HostPadSlot;

static HostPadSlot g_slots[HOST_PAD_MAX_DEVICES] = {
    {-1, "", {{0}}}, {-1, "", {{0}}}, {-1, "", {{0}}}, {-1, "", {{0}}}};
static uint64_t g_last_scan;
static int g_scanned_once;
static uint16_t g_last_held;
static int g_debug = -1;

static uint64_t now_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return 0;
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}

static int path_open(const char *path)
{
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++)
        if (g_slots[i].fd >= 0 && !strcmp(g_slots[i].path, path)) return 1;
    return 0;
}

static void try_open(const char *path)
{
    HostPadSlot *slot = NULL;
    char name[128] = "unknown";
    int fd;

    if (path_open(path)) return;
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++)
        if (g_slots[i].fd < 0) { slot = &g_slots[i]; break; }
    if (!slot) return;
    fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        if (errno == EACCES) {
            /* The device list is rescanned periodically; say it once per
             * device path, not on every rescan. */
            static char denied[HOST_PAD_MAX_DEVICES * 2][64];
            static int ndenied;
            int seen = 0;
            for (int i = 0; i < ndenied; i++)
                if (strncmp(denied[i], path, sizeof(denied[i])) == 0) { seen = 1; break; }
            if (!seen) {
                fprintf(stderr, "[PAD] %s: permission denied (controller ignored)\n", path);
                if (ndenied < (int)(sizeof(denied) / sizeof(denied[0])))
                    snprintf(denied[ndenied++], sizeof(denied[0]), "%s", path);
            }
        }
        return;
    }
    HostPad_DeviceDefaults(&slot->dev);
    {
        /* Code maps from the driver; the fallback layout stays if absent. */
        uint16_t btn[512];
        uint8_t ax[64];
        uint8_t nbtn = 0, nax = 0;
        if (ioctl(fd, JSIOCGBUTTONS, &nbtn) == 0 && ioctl(fd, JSIOCGAXES, &nax) == 0 &&
            ioctl(fd, JSIOCGBTNMAP, btn) >= 0 && ioctl(fd, JSIOCGAXMAP, ax) >= 0) {
            memset(slot->dev.btn_code, 0, sizeof(slot->dev.btn_code));
            memset(slot->dev.axis_code, 0x3f, sizeof(slot->dev.axis_code));
            memcpy(slot->dev.btn_code, btn, (size_t)nbtn * sizeof(btn[0]));
            memcpy(slot->dev.axis_code, ax, nax < 64 ? nax : 64);
        }
        if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0) snprintf(name, sizeof(name), "unknown");
        name[sizeof(name) - 1] = 0;
    }
    slot->fd = fd;
    snprintf(slot->path, sizeof(slot->path), "%s", path);
    fprintf(stderr, "[PAD] controller connected: %s (%s)\n", path, name);
}

static void close_slot(HostPadSlot *slot, const char *why)
{
    if (slot->fd < 0) return;
    close(slot->fd);
    fprintf(stderr, "[PAD] controller disconnected: %s (%s)\n", slot->path, why);
    slot->fd = -1;
    slot->path[0] = 0;
    memset(&slot->dev, 0, sizeof(slot->dev));
}

static void scan(void)
{
    const char *env = getenv("PE_JOYSTICK");
    if (env && (!strcmp(env, "off") || !strcmp(env, "0") || !strcmp(env, "none"))) return;
    if (env && *env) { try_open(env); return; }
    for (int n = 0; n < 8; n++) {
        char path[32];
        snprintf(path, sizeof(path), "/dev/input/js%d", n);
        try_open(path);
    }
}

void HostPad_Poll(void)
{
    uint64_t t = now_ns();

    if (g_debug < 0) {
        const char *s = getenv("PE_PAD_DEBUG");
        g_debug = (s && *s && *s != '0') ? 1 : 0;
    }
    if (!g_scanned_once || t - g_last_scan >= HOST_PAD_SCAN_NS) {
        g_scanned_once = 1;
        g_last_scan = t;
        scan();
    }
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++) {
        HostPadSlot *slot = &g_slots[i];
        struct js_event ev[32];
        if (slot->fd < 0) continue;
        for (;;) {
            ssize_t n = read(slot->fd, ev, sizeof(ev));
            if (n < 0) {
                if (errno == EINTR) continue;
                if (errno != EAGAIN && errno != EWOULDBLOCK) close_slot(slot, strerror(errno));
                break;
            }
            if (n == 0) { close_slot(slot, "end of stream"); break; }
            for (size_t k = 0; k < (size_t)n / sizeof(ev[0]); k++) {
                if (g_debug)
                    fprintf(stderr, "[PAD] %s event type=0x%02X number=%u value=%d\n",
                            slot->path, ev[k].type, ev[k].number, ev[k].value);
                HostPad_DeviceEvent(&slot->dev, ev[k].type, ev[k].number, ev[k].value);
            }
            if ((size_t)n < sizeof(ev)) break;
        }
    }
    if (g_debug) {
        uint16_t held = HostPad_Held();
        if (held != g_last_held)
            fprintf(stderr, "[PAD] held=0x%04X (PSX pad word 0x%04X)\n", held, (uint16_t)~held);
        g_last_held = held;
    }
}

uint16_t HostPad_Held(void)
{
    uint16_t held = 0;
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++)
        if (g_slots[i].fd >= 0) held |= HostPad_DeviceHeld(&g_slots[i].dev);
    return held;
}

uint8_t HostPad_Hotkeys(void)
{
    uint8_t hk = 0;
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++)
        if (g_slots[i].fd >= 0) hk |= HostPad_DeviceHotkeys(&g_slots[i].dev);
    return hk;
}

void HostPad_Close(void)
{
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++) {
        if (g_slots[i].fd >= 0) close(g_slots[i].fd);
        g_slots[i].fd = -1;
        g_slots[i].path[0] = 0;
    }
    g_scanned_once = 0;
    g_last_held = 0;
}
