/* Host joystick mapping tests: synthetic js_events through the decoder, then
 * a FIFO standing in for /dev/input/jsN (PE_JOYSTICK) to exercise the
 * non-blocking read path and disconnect handling.  No hardware needed. */
#define _GNU_SOURCE
#include "host_pad.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/joystick.h>

static int g_fail;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); g_fail = 1; } } while (0)

static void test_decoder(void)
{
    HostPadDevice d;
    HostPad_DeviceDefaults(&d);
    CHECK(HostPad_DeviceHeld(&d) == 0);             /* unseen triggers are not held */
    /* xpad: buttons 0..7 = A B X Y LB RB Back Start */
    static const uint16_t want[8] = {HOST_PAD_CROSS, HOST_PAD_CIRCLE, HOST_PAD_SQUARE,
        HOST_PAD_TRIANGLE, HOST_PAD_L1, HOST_PAD_R1, HOST_PAD_SELECT, HOST_PAD_START};
    for (int b = 0; b < 8; b++) {
        HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, (uint8_t)b, 1);
        CHECK(HostPad_DeviceHeld(&d) == want[b]);
        HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, (uint8_t)b, 0);
        CHECK(HostPad_DeviceHeld(&d) == 0);
    }
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 8, 1);  /* Guide: unmapped */
    CHECK(HostPad_DeviceHeld(&d) == 0);
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 8, 0);
    /* Triggers rest at -32767 (INIT events), press past midpoint. */
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS | JS_EVENT_INIT, 2, -32767);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS | JS_EVENT_INIT, 5, -32767);
    CHECK(HostPad_DeviceHeld(&d) == 0);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 2, 20000);
    CHECK(HostPad_DeviceHeld(&d) == HOST_PAD_L2);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 5, 32767);
    CHECK(HostPad_DeviceHeld(&d) == (HOST_PAD_L2 | HOST_PAD_R2));
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 2, -32767);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 5, -32767);
    /* Left stick with deadzone. */
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 0, 5000);
    CHECK(HostPad_DeviceHeld(&d) == 0);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 0, -30000);
    CHECK(HostPad_DeviceHeld(&d) == HOST_PAD_LEFT);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 1, 30000);
    CHECK(HostPad_DeviceHeld(&d) == (HOST_PAD_LEFT | HOST_PAD_DOWN));
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 0, 0);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 1, 0);
    /* D-pad hat. */
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 6, 32767);
    CHECK(HostPad_DeviceHeld(&d) == HOST_PAD_RIGHT);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 7, -32767);
    CHECK(HostPad_DeviceHeld(&d) == (HOST_PAD_RIGHT | HOST_PAD_UP));
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 6, 0);
    HostPad_DeviceEvent(&d, JS_EVENT_AXIS, 7, 0);
    CHECK(HostPad_DeviceHeld(&d) == 0);
    /* Code-mapped D-pad buttons (drivers that report BTN_DPAD_*). */
    d.btn_code[20] = 0x220;
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 20, 1);
    CHECK(HostPad_DeviceHeld(&d) == HOST_PAD_UP);
}

static void test_hotkeys(void)
{
    /* xpad: button 9 = LS click (L3), 10 = RS click (R3).  They are port
     * hotkeys (fast-forward toggle / hold), never PSX pad bits. */
    HostPadDevice d;
    HostPad_DeviceDefaults(&d);
    CHECK(HostPad_DeviceHotkeys(&d) == 0);
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 10, 1);
    CHECK(HostPad_DeviceHotkeys(&d) == HOST_PAD_HOTKEY_FF_HOLD);
    CHECK(HostPad_DeviceHeld(&d) == 0);
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 9, 1);
    CHECK(HostPad_DeviceHotkeys(&d) == (HOST_PAD_HOTKEY_FF_HOLD | HOST_PAD_HOTKEY_FF_TOGGLE));
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 10, 0);
    CHECK(HostPad_DeviceHotkeys(&d) == HOST_PAD_HOTKEY_FF_TOGGLE);
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 9, 0);
    CHECK(HostPad_DeviceHotkeys(&d) == 0);
    CHECK(HostPad_DeviceHeld(&d) == 0);
    /* Code-mapped (JSIOCGBTNMAP) BTN_THUMBR at another index. */
    d.btn_code[13] = 0x13e;
    HostPad_DeviceEvent(&d, JS_EVENT_BUTTON, 13, 1);
    CHECK(HostPad_DeviceHotkeys(&d) == HOST_PAD_HOTKEY_FF_HOLD);
}

static void send(int fd, uint8_t type, uint8_t number, int16_t value)
{
    struct js_event e = {0, value, type, number};
    CHECK(write(fd, &e, sizeof(e)) == (ssize_t)sizeof(e));
}

static void test_fifo_device(void)
{
    char dir[] = "pe-host-pad-XXXXXX", path[64];
    CHECK(mkdtemp(dir) != NULL);
    snprintf(path, sizeof(path), "%s/js0", dir);
    CHECK(mkfifo(path, 0600) == 0);
    int rd_hold = open(path, O_RDONLY | O_NONBLOCK);   /* lets the writer open */
    int wr = open(path, O_WRONLY | O_NONBLOCK);
    CHECK(rd_hold >= 0 && wr >= 0);
    setenv("PE_JOYSTICK", path, 1);

    HostPad_Poll();                                   /* opens + drains nothing */
    CHECK(HostPad_Held() == 0);
    send(wr, JS_EVENT_BUTTON | JS_EVENT_INIT, 0, 0);
    send(wr, JS_EVENT_BUTTON, 0, 1);                 /* A */
    send(wr, JS_EVENT_AXIS, 7, 32767);               /* hat down */
    HostPad_Poll();
    CHECK(HostPad_Held() == (HOST_PAD_CROSS | HOST_PAD_DOWN));
    send(wr, JS_EVENT_BUTTON, 0, 0);
    HostPad_Poll();
    CHECK(HostPad_Held() == HOST_PAD_DOWN);
    HostPad_Poll();                                   /* no data: EAGAIN keeps device */
    CHECK(HostPad_Held() == HOST_PAD_DOWN);

    /* Unplug: last writer gone -> read returns 0 -> device dropped, state cleared. */
    close(wr);
    close(rd_hold);
    HostPad_Poll();
    CHECK(HostPad_Held() == 0);

    setenv("PE_JOYSTICK", "off", 1);
    HostPad_Close();
    HostPad_Poll();
    CHECK(HostPad_Held() == 0);
    unlink(path);
    rmdir(dir);
}

int main(void)
{
    test_decoder();
    test_hotkeys();
    test_fifo_device();
    if (g_fail) return 1;
    printf("host-pad tests: OK\n");
    return 0;
}
