/* Host joystick hot-plug tests against a mock /dev/input/js* layer.
 *
 * host_pad.c is compiled into this file with open/read/ioctl/close and
 * clock_gettime redirected to the mock below, so the real HostPad_Poll()
 * path (rescan, non-blocking read, ENODEV close, reopen) runs against
 * devices that disappear mid-read and come back at the same js path with a
 * different name, button/axis count and code maps.  The mock hands out the
 * lowest free fd number, as the kernel does, so a reconnect reuses the fd of
 * the device that just vanished.  Replays the owner's Legion Go sequence
 * (build/lanes/legion/run-20260929T151702Z.log): js0 permission denied,
 * Legion-Controller on js0 + js1, both "No such device", Generic X-Box pad
 * on js0.  No hardware or privileges needed. */
#define _GNU_SOURCE
#include "host_pad.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <linux/joystick.h>

#define MOCK_PATHS 8
#define MOCK_FDS   16
#define MOCK_FD0   100          /* never collides with a real descriptor */
#define MOCK_QUEUE 256

typedef struct {
    int present, eacces, generation;
    char name[64];
    uint8_t nbtn, nax;
    uint16_t btnmap[512];
    uint8_t axmap[64];
    struct js_event q[MOCK_QUEUE];
    int qhead, qlen;
    int enodev_after;           /* >0: that many events, then ENODEV */
    int eintr_once;
} MockDev;

static MockDev g_mock[MOCK_PATHS];
static struct { int used, dev, generation; } g_fdtab[MOCK_FDS];
static uint64_t g_mock_ns = 1000000000ull;
static int g_mock_open_fds;

static int mock_path_index(const char *path)
{
    int n;
    if (sscanf(path, "/dev/input/js%d", &n) == 1 && n >= 0 && n < MOCK_PATHS) return n;
    return -1;
}

static int mock_open(const char *path, int flags, ...)
{
    int d = mock_path_index(path);
    (void)flags;
    if (d < 0 || !g_mock[d].present) { errno = ENOENT; return -1; }
    if (g_mock[d].eacces) { errno = EACCES; return -1; }
    for (int i = 0; i < MOCK_FDS; i++) {
        if (g_fdtab[i].used) continue;
        g_fdtab[i].used = 1;
        g_fdtab[i].dev = d;
        g_fdtab[i].generation = g_mock[d].generation;
        g_mock_open_fds++;
        return MOCK_FD0 + i;
    }
    errno = EMFILE;
    return -1;
}

/* NULL for a closed descriptor (EBADF); *gone for a removed device. */
static MockDev *mock_fd(int fd, int *gone)
{
    int i = fd - MOCK_FD0;
    MockDev *m;
    *gone = 0;
    if (i < 0 || i >= MOCK_FDS || !g_fdtab[i].used) return NULL;
    m = &g_mock[g_fdtab[i].dev];
    *gone = !m->present || m->generation != g_fdtab[i].generation;
    return m;
}

static int g_ebadf;

static int mock_close(int fd)
{
    int i = fd - MOCK_FD0;
    if (i < 0 || i >= MOCK_FDS || !g_fdtab[i].used) { g_ebadf++; errno = EBADF; return -1; }
    g_fdtab[i].used = 0;
    g_mock_open_fds--;
    return 0;
}

static ssize_t mock_read(int fd, void *buf, size_t len)
{
    int gone;
    MockDev *m = mock_fd(fd, &gone);
    size_t max = len / sizeof(struct js_event), n = 0;
    if (!m) { g_ebadf++; errno = EBADF; return -1; }
    if (m->eintr_once) { m->eintr_once = 0; errno = EINTR; return -1; }
    if (gone) { errno = ENODEV; return -1; }
    while (n < max && m->qlen > 0) {
        if (m->enodev_after > 0 && --m->enodev_after == 0) {
            /* the device vanishes in the middle of this burst */
            m->present = 0;
            m->qlen = 0;
            break;
        }
        memcpy((struct js_event *)buf + n, &m->q[m->qhead], sizeof(struct js_event));
        m->qhead = (m->qhead + 1) % MOCK_QUEUE;
        m->qlen--;
        n++;
    }
    if (n == 0) { errno = m->present ? EAGAIN : ENODEV; return -1; }
    return (ssize_t)(n * sizeof(struct js_event));
}

static int mock_ioctl(int fd, unsigned long req, ...)
{
    int gone;
    MockDev *m = mock_fd(fd, &gone);
    va_list ap;
    void *arg;
    va_start(ap, req);
    arg = va_arg(ap, void *);
    va_end(ap);
    if (!m) { g_ebadf++; errno = EBADF; return -1; }
    if (gone) { errno = ENODEV; return -1; }
    if (req == JSIOCGBUTTONS) { *(uint8_t *)arg = m->nbtn; return 0; }
    if (req == JSIOCGAXES)    { *(uint8_t *)arg = m->nax; return 0; }
    if (req == JSIOCGBTNMAP)  { memcpy(arg, m->btnmap, _IOC_SIZE(req)); return 0; }
    if (req == JSIOCGAXMAP)   { memcpy(arg, m->axmap, _IOC_SIZE(req)); return 0; }
    if (_IOC_TYPE(req) == 'j' && _IOC_NR(req) == 0x13) {       /* JSIOCGNAME(len) */
        size_t l = _IOC_SIZE(req);
        snprintf((char *)arg, l, "%s", m->name);
        return (int)strlen(m->name);
    }
    errno = EINVAL;
    return -1;
}

static int mock_clock_gettime(clockid_t id, struct timespec *t)
{
    (void)id;
    t->tv_sec = (time_t)(g_mock_ns / 1000000000ull);
    t->tv_nsec = (long)(g_mock_ns % 1000000000ull);
    return 0;
}

#define open          mock_open
#define read          mock_read
#define ioctl         mock_ioctl
#define close         mock_close
#define clock_gettime mock_clock_gettime
#include "host_pad.c"
#undef open
#undef read
#undef ioctl
#undef close
#undef clock_gettime

static int g_fail;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); g_fail = 1; } } while (0)

static void mock_push(int d, uint8_t type, uint8_t number, int16_t value)
{
    MockDev *m = &g_mock[d];
    struct js_event *e;
    if (m->qlen >= MOCK_QUEUE) return;
    e = &m->q[(m->qhead + m->qlen) % MOCK_QUEUE];
    e->time = 0;
    e->type = type;
    e->number = number;
    e->value = value;
    m->qlen++;
}

/* Plug a device in at js<d>: new generation, empty queue, JS_EVENT_INIT
 * burst for every button and axis (as joydev sends on open). */
static void mock_plug(int d, const char *name, uint8_t nbtn, const uint16_t *btn,
                      uint8_t nax, const uint8_t *ax, const int16_t *ax_rest)
{
    MockDev *m = &g_mock[d];
    int gen = m->generation + 1;
    memset(m, 0, sizeof(*m));
    m->generation = gen;
    m->present = 1;
    snprintf(m->name, sizeof(m->name), "%s", name);
    m->nbtn = nbtn;
    m->nax = nax;
    memcpy(m->btnmap, btn, (size_t)nbtn * sizeof(btn[0]));
    memcpy(m->axmap, ax, nax < 64 ? nax : 64);
    for (int b = 0; b < nbtn; b++) mock_push(d, JS_EVENT_BUTTON | JS_EVENT_INIT, (uint8_t)b, 0);
    for (int a = 0; a < nax && a < 64; a++)
        mock_push(d, JS_EVENT_AXIS | JS_EVENT_INIT, (uint8_t)a, ax_rest ? ax_rest[a] : 0);
}

static void mock_unplug(int d)
{
    g_mock[d].present = 0;
    g_mock[d].qlen = 0;
}

/* Advance past the rescan interval, then poll. */
static void poll_rescan(void)
{
    g_mock_ns += HOST_PAD_SCAN_NS;
    HostPad_Poll();
}

static int slots_open(void)
{
    int n = 0;
    for (int i = 0; i < HOST_PAD_MAX_DEVICES; i++) n += g_slots[i].fd >= 0;
    return n;
}

/* xpad ("Generic X-Box pad"): 11 buttons, 8 axes, triggers rest at -32767. */
static const uint16_t XPAD_BTN[11] = {0x130, 0x131, 0x133, 0x134, 0x136, 0x137,
                                      0x13a, 0x13b, 0x13c, 0x13d, 0x13e};
static const uint8_t XPAD_AX[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x10, 0x11};
static const int16_t XPAD_REST[8] = {0, 0, -32767, 0, 0, -32767, 0, 0};

/* A wider HID layout at the same path: more buttons and axes, D-pad as
 * buttons, Start at a high index, extra trigger-happy codes. */
static uint16_t WIDE_BTN[40];
static uint8_t WIDE_AX[20];

static void init_wide(void)
{
    for (int i = 0; i < 40; i++) WIDE_BTN[i] = (uint16_t)(0x2c0 + i);   /* BTN_TRIGGER_HAPPY* */
    WIDE_BTN[0] = 0x130;      /* A */
    WIDE_BTN[1] = 0x131;      /* B */
    WIDE_BTN[20] = 0x220;     /* D-pad up */
    WIDE_BTN[33] = 0x13b;     /* Start at index 33 */
    for (int i = 0; i < 20; i++) WIDE_AX[i] = (uint8_t)(0x20 + i);      /* unmapped ABS */
    WIDE_AX[0] = 0x00;
    WIDE_AX[1] = 0x01;
    WIDE_AX[12] = 0x0a;       /* brake (L2) at index 12 */
}

static void reset_all(void)
{
    HostPad_Close();
    memset(g_mock, 0, sizeof(g_mock));
    memset(g_fdtab, 0, sizeof(g_fdtab));
    g_mock_open_fds = 0;
    g_ebadf = 0;
    unsetenv("PE_JOYSTICK");
}

/* The exact device sequence from the owner's Legion Go log. */
static void test_legion_sequence(void)
{
    int js0_fd;
    reset_all();

    /* Steam Input holds the raw controller: js0 is EACCES for a while. */
    g_mock[0].present = 1;
    g_mock[0].eacces = 1;
    HostPad_Poll();
    for (int i = 0; i < 5; i++) poll_rescan();
    CHECK(slots_open() == 0);
    CHECK(HostPad_Held() == 0);

    /* Legion-Controller appears on js0 and js1 (wide layout). */
    mock_plug(0, "Legion-Controller 1-E3", 40, WIDE_BTN, 20, WIDE_AX, NULL);
    mock_plug(1, "Legion-Controller 1-E3", 40, WIDE_BTN, 20, WIDE_AX, NULL);
    poll_rescan();
    CHECK(slots_open() == 2);
    CHECK(HostPad_Held() == 0);
    js0_fd = g_slots[0].fd;

    /* Hold Start (index 33) on js0 and brake (L2, axis 12) on js1. */
    mock_push(0, JS_EVENT_BUTTON, 33, 1);
    mock_push(1, JS_EVENT_AXIS, 12, 30000);
    HostPad_Poll();
    CHECK(HostPad_Held() == (HOST_PAD_START | HOST_PAD_L2));

    /* Both vanish while held, js0 in the middle of an event burst. */
    for (int k = 0; k < 20; k++) mock_push(0, JS_EVENT_AXIS, 0, (int16_t)(k * 100));
    g_mock[0].enodev_after = 7;
    mock_unplug(1);
    HostPad_Poll();                      /* js1 gone; js0 returns its last 6 events */
    CHECK(slots_open() == 1);
    CHECK(HostPad_Held() == HOST_PAD_START);
    HostPad_Poll();                      /* js0's next read is ENODEV */
    CHECK(slots_open() == 0);
    CHECK(g_mock_open_fds == 0);
    CHECK(HostPad_Held() == 0);          /* nothing stays held after a vanish */
    HostPad_Poll();                      /* no read of a closed/stale fd */
    CHECK(g_ebadf == 0);

    /* Steam's virtual pad arrives on js0: fewer buttons/axes, same fd. */
    mock_plug(0, "Generic X-Box pad", 11, XPAD_BTN, 8, XPAD_AX, XPAD_REST);
    poll_rescan();
    CHECK(slots_open() == 1);
    CHECK(g_slots[0].fd == js0_fd);      /* fd number reused */
    CHECK(HostPad_Held() == 0);          /* INIT triggers at rest are not L2/R2 */

    /* The old device's map must not survive: index 33 no longer exists,
     * index 7 is Start and axis 12 is past this device's axes. */
    mock_push(0, JS_EVENT_BUTTON, 33, 1);
    mock_push(0, JS_EVENT_AXIS, 12, 30000);
    HostPad_Poll();
    CHECK(HostPad_Held() == 0);
    mock_push(0, JS_EVENT_BUTTON, 7, 1);
    mock_push(0, JS_EVENT_AXIS, 5, 32767);
    HostPad_Poll();
    CHECK(HostPad_Held() == (HOST_PAD_START | HOST_PAD_R2));
    mock_push(0, JS_EVENT_BUTTON, 7, 0);
    mock_push(0, JS_EVENT_AXIS, 5, -32767);
    HostPad_Poll();
    CHECK(HostPad_Held() == 0);
    CHECK(g_ebadf == 0);
}

/* Renumbering: js1 vanishes, js0 then vanishes, a device returns on js1
 * only; slots are reused without stale paths or out-of-range writes. */
static void test_renumber_and_limits(void)
{
    uint16_t max_btn[512];
    uint8_t max_ax[64];
    int16_t rest[64];
    reset_all();

    mock_plug(0, "pad A", 11, XPAD_BTN, 8, XPAD_AX, XPAD_REST);
    mock_plug(1, "pad B", 11, XPAD_BTN, 8, XPAD_AX, XPAD_REST);
    HostPad_Poll();
    CHECK(slots_open() == 2);
    mock_unplug(1);
    HostPad_Poll();
    mock_unplug(0);
    HostPad_Poll();
    CHECK(slots_open() == 0);

    /* Largest counts the js API can report: 255 buttons (u8), 64 axes;
     * every event number 0..255, including indices past the device's
     * counts, must stay inside the decoder's tables. */
    for (int i = 0; i < 512; i++) max_btn[i] = (uint16_t)(0x100 + i);
    for (int i = 0; i < 64; i++) { max_ax[i] = (uint8_t)i; rest[i] = -32767; }
    mock_plug(1, "max pad", 255, max_btn, 64, max_ax, rest);
    poll_rescan();
    CHECK(slots_open() == 1);
    CHECK(!strcmp(g_slots[0].path, "/dev/input/js1"));
    for (int n = 0; n < 256; n++) {
        mock_push(1, JS_EVENT_BUTTON, (uint8_t)n, 1);
        mock_push(1, JS_EVENT_AXIS, (uint8_t)n, 32767);
        if (g_mock[1].qlen > MOCK_QUEUE - 2) HostPad_Poll();
    }
    g_mock[1].eintr_once = 1;
    HostPad_Poll();
    CHECK(slots_open() == 1);
    mock_unplug(1);
    HostPad_Poll();
    CHECK(slots_open() == 0);
    CHECK(HostPad_Held() == 0);
    CHECK(HostPad_Hotkeys() == 0);

    /* More devices than slots: the extras are left for a later rescan. */
    for (int d = 0; d < MOCK_PATHS; d++) mock_plug(d, "pad", 11, XPAD_BTN, 8, XPAD_AX, XPAD_REST);
    poll_rescan();
    CHECK(slots_open() == HOST_PAD_MAX_DEVICES);
    for (int d = 0; d < MOCK_PATHS; d++) mock_unplug(d);
    HostPad_Poll();
    CHECK(slots_open() == 0);
    CHECK(g_mock_open_fds == 0);
    CHECK(g_ebadf == 0);
}

/* Hotkeys held on a device that vanishes are released. */
static void test_hotkey_release_on_vanish(void)
{
    reset_all();
    mock_plug(0, "Generic X-Box pad", 11, XPAD_BTN, 8, XPAD_AX, XPAD_REST);
    HostPad_Poll();
    mock_push(0, JS_EVENT_BUTTON, 10, 1);          /* R3: fast-forward hold */
    HostPad_Poll();
    CHECK(HostPad_Hotkeys() == HOST_PAD_HOTKEY_FF_HOLD);
    mock_unplug(0);
    HostPad_Poll();
    CHECK(HostPad_Hotkeys() == 0);
}

int main(void)
{
    init_wide();
    test_legion_sequence();
    test_renumber_and_limits();
    test_hotkey_release_on_vanish();
    reset_all();
    if (g_fail) return 1;
    printf("host-pad hotplug tests: OK\n");
    return 0;
}
