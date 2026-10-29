/*
 * Host gamepad input — SDL2 GameController backend.  See host_pad_sdl.h.
 */
#include "host_pad_sdl.h"
#include "host_pad.h"

#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define HPS_MAX_DEVICES 8
#define HPS_SCAN_NS     1000000000ull
#define HPS_INIT_JOYSTICK       0x00000200u
#define HPS_INIT_GAMECONTROLLER 0x00002000u

/* ── pure mappings ─────────────────────────────────────────────────── */

uint16_t HostPadSDL_MapController(const uint8_t btn[HPS_BTN_COUNT], const int16_t axis[HPS_AXIS_COUNT],
                                  int swap_xy, uint8_t *hotkeys)
{
    return HostPadSDL_MapControllerEx(btn, axis, swap_xy, 1, hotkeys);
}

uint16_t HostPadSDL_MapControllerEx(const uint8_t btn[HPS_BTN_COUNT], const int16_t axis[HPS_AXIS_COUNT],
                                    int swap_xy, int stick_dpad, uint8_t *hotkeys)
{
    uint16_t h = 0;
    uint8_t hk = 0;
    if (btn[HPS_BTN_A]) h |= HOST_PAD_CROSS;
    if (btn[HPS_BTN_B]) h |= HOST_PAD_CIRCLE;
    if (btn[HPS_BTN_X]) h |= swap_xy ? HOST_PAD_TRIANGLE : HOST_PAD_SQUARE;
    if (btn[HPS_BTN_Y]) h |= swap_xy ? HOST_PAD_SQUARE : HOST_PAD_TRIANGLE;
    if (btn[HPS_BTN_BACK]) h |= HOST_PAD_SELECT;
    if (btn[HPS_BTN_START]) h |= HOST_PAD_START;
    if (btn[HPS_BTN_LSHOULDER]) h |= HOST_PAD_L1;
    if (btn[HPS_BTN_RSHOULDER]) h |= HOST_PAD_R1;
    if (btn[HPS_BTN_DPAD_UP]) h |= HOST_PAD_UP;
    if (btn[HPS_BTN_DPAD_DOWN]) h |= HOST_PAD_DOWN;
    if (btn[HPS_BTN_DPAD_LEFT]) h |= HOST_PAD_LEFT;
    if (btn[HPS_BTN_DPAD_RIGHT]) h |= HOST_PAD_RIGHT;
    if (btn[HPS_BTN_RSTICK]) hk |= HOST_PAD_HOTKEY_FF_HOLD;
    if (btn[HPS_BTN_LSTICK]) hk |= HOST_PAD_HOTKEY_FF_TOGGLE;
    if (axis[HPS_AXIS_TL] >= HOST_PAD_SDL_TRIGGER_THRESHOLD) h |= HOST_PAD_L2;
    if (axis[HPS_AXIS_TR] >= HOST_PAD_SDL_TRIGGER_THRESHOLD) h |= HOST_PAD_R2;
    if (stick_dpad) {
        if (axis[HPS_AXIS_LX] <= -HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_LEFT;
        if (axis[HPS_AXIS_LX] >=  HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_RIGHT;
        if (axis[HPS_AXIS_LY] <= -HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_UP;
        if (axis[HPS_AXIS_LY] >=  HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_DOWN;
    }
    if (hotkeys) *hotkeys = hk;
    return h;
}

void HostPadSDL_RawMapDefaults(HostPadRawMap *m)
{
    /* xpad index order: A B X Y LB RB Back Start Guide LS RS (no L2/R2 buttons). */
    m->cross = 0; m->circle = 1; m->square = 2; m->triangle = 3;
    m->l1 = 4; m->r1 = 5; m->l2 = -1; m->r2 = -1; m->select = 6; m->start = 7;
    m->l3 = 9; m->r3 = 10;
}

int HostPadSDL_RawMapParse(HostPadRawMap *m, const char *spec)
{
    int v[10], n = 0;
    const char *s = spec;
    if (!s || !*s) return -1;
    while (n < 10) {
        char *e;
        long x = strtol(s, &e, 10);
        if (e == s || x < -1 || x > 255) return -1;
        v[n++] = (int)x;
        if (*e == 0) break;
        if (*e != ',') return -1;
        s = e + 1;
    }
    if (n != 10) return -1;
    m->cross = v[0]; m->circle = v[1]; m->square = v[2]; m->triangle = v[3];
    m->l1 = v[4]; m->r1 = v[5]; m->l2 = v[6]; m->r2 = v[7]; m->select = v[8]; m->start = v[9];
    return 0;
}

static int raw_on(const uint8_t *btn, int nbtn, int i) { return i >= 0 && i < nbtn && btn[i]; }

uint16_t HostPadSDL_MapJoystick(const HostPadRawMap *m, const uint8_t *btn, int nbtn,
                                const int16_t *axis, int naxis, unsigned hat, int swap_xy,
                                uint8_t *hotkeys)
{
    uint16_t h = 0;
    uint8_t hk = 0;
    if (raw_on(btn, nbtn, m->cross)) h |= HOST_PAD_CROSS;
    if (raw_on(btn, nbtn, m->circle)) h |= HOST_PAD_CIRCLE;
    if (raw_on(btn, nbtn, m->square)) h |= swap_xy ? HOST_PAD_TRIANGLE : HOST_PAD_SQUARE;
    if (raw_on(btn, nbtn, m->triangle)) h |= swap_xy ? HOST_PAD_SQUARE : HOST_PAD_TRIANGLE;
    if (raw_on(btn, nbtn, m->l1)) h |= HOST_PAD_L1;
    if (raw_on(btn, nbtn, m->r1)) h |= HOST_PAD_R1;
    if (raw_on(btn, nbtn, m->l2)) h |= HOST_PAD_L2;
    if (raw_on(btn, nbtn, m->r2)) h |= HOST_PAD_R2;
    if (raw_on(btn, nbtn, m->select)) h |= HOST_PAD_SELECT;
    if (raw_on(btn, nbtn, m->start)) h |= HOST_PAD_START;
    if (raw_on(btn, nbtn, m->r3)) hk |= HOST_PAD_HOTKEY_FF_HOLD;
    if (raw_on(btn, nbtn, m->l3)) hk |= HOST_PAD_HOTKEY_FF_TOGGLE;
    if (hat & HPS_HAT_UP) h |= HOST_PAD_UP;
    if (hat & HPS_HAT_DOWN) h |= HOST_PAD_DOWN;
    if (hat & HPS_HAT_LEFT) h |= HOST_PAD_LEFT;
    if (hat & HPS_HAT_RIGHT) h |= HOST_PAD_RIGHT;
    if (naxis > 0 && axis[0] <= -HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_LEFT;
    if (naxis > 0 && axis[0] >=  HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_RIGHT;
    if (naxis > 1 && axis[1] <= -HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_UP;
    if (naxis > 1 && axis[1] >=  HOST_PAD_STICK_DEADZONE) h |= HOST_PAD_DOWN;
    if (hotkeys) *hotkeys = hk;
    return h;
}

/* ── runtime (dlopen SDL2) ─────────────────────────────────────────── */

typedef struct {
    void *gc, *js; int32_t id; uint16_t held, held_analog; uint8_t hk, input_seen; char name[96];
    int16_t axis[HPS_AXIS_COUNT];   /* GameController axes from the last poll */
} HpsDev;

static struct {
    int state;             /* 0 untried, 1 ready, -1 unavailable/disabled */
    void *lib;
    int debug, swap_xy;
    uint64_t last_scan;
    HostPadRawMap raw;
    HpsDev dev[HPS_MAX_DEVICES];
    uint16_t last_held;
    int (*Init)(uint32_t);
    void (*Quit)(uint32_t);
    int (*SetHint)(const char *, const char *);
    const char *(*GetError)(void);
    int (*NumJoysticks)(void);
    int (*IsGC)(int);
    int32_t (*DevInstanceID)(int);
    void *(*GCOpen)(int);
    void (*GCClose)(void *);
    int (*GCAttached)(void *);
    uint8_t (*GCButton)(void *, int);
    int16_t (*GCAxis)(void *, int);
    const char *(*GCName)(void *);
    void (*GCUpdate)(void);
    void *(*RWFromFile)(const char *, const char *);
    int (*GCAddMappingsRW)(void *, int);
    void *(*JSOpen)(int);
    void (*JSClose)(void *);
    int (*JSAttached)(void *);
    int (*JSNumButtons)(void *);
    int (*JSNumAxes)(void *);
    int (*JSNumHats)(void *);
    uint8_t (*JSButton)(void *, int);
    int16_t (*JSAxis)(void *, int);
    uint8_t (*JSHat)(void *, int);
    const char *(*JSName)(void *);
    void (*JSUpdate)(void);
} S;

static uint64_t hps_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static int env_on(const char *name)
{
    const char *s = getenv(name);
    return s && *s && *s != '0';
}

static void load_db_file(void)
{
    char path[PATH_MAX + 64], exe[PATH_MAX];
    const char *cands[3] = {0};
    char xdg[PATH_MAX + 64];
    const char *env = getenv("PE_GAMECONTROLLERDB");
    ssize_t n;
    cands[0] = env && *env ? env : NULL;
    n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        char *slash;
        exe[n] = 0;
        slash = strrchr(exe, '/');
        if (slash) { *slash = 0; snprintf(path, sizeof(path), "%s/gamecontrollerdb.txt", exe); cands[1] = path; }
    }
    if (getenv("XDG_CONFIG_HOME") && *getenv("XDG_CONFIG_HOME"))
        snprintf(xdg, sizeof(xdg), "%s/parasite-eve-port/gamecontrollerdb.txt", getenv("XDG_CONFIG_HOME"));
    else if (getenv("HOME"))
        snprintf(xdg, sizeof(xdg), "%s/.config/parasite-eve-port/gamecontrollerdb.txt", getenv("HOME"));
    else xdg[0] = 0;
    cands[2] = xdg[0] ? xdg : NULL;
    for (int i = 0; i < 3; i++) {
        void *rw;
        int added;
        if (!cands[i] || access(cands[i], R_OK) != 0) continue;
        rw = S.RWFromFile(cands[i], "rb");
        if (!rw) continue;
        added = S.GCAddMappingsRW(rw, 1);
        fprintf(stderr, "[PAD] SDL mappings: %s (%d added)\n", cands[i], added);
        return;   /* first readable wins */
    }
}

#define HPS_SYM(field, name) do { *(void **)&S.field = dlsym(S.lib, name); if (!S.field) { miss = name; goto fail; } } while (0)

static void hps_init(void)
{
    const char *miss = NULL;
    const char *libname = getenv("PE_SDL2_LIB");
    const char *off = getenv("PE_PAD_SDL");
    S.state = -1;
    if (off && (!strcmp(off, "off") || !strcmp(off, "0") || !strcmp(off, "none"))) return;
    S.debug = env_on("PE_PAD_DEBUG");
    S.swap_xy = env_on("PE_PAD_SWAP_XY");
    HostPadSDL_RawMapDefaults(&S.raw);
    if (getenv("PE_PAD_RAW_MAP") && HostPadSDL_RawMapParse(&S.raw, getenv("PE_PAD_RAW_MAP")) != 0)
        fprintf(stderr, "[PAD] PE_PAD_RAW_MAP ignored (want 10 comma-separated indices)\n");
    S.lib = dlopen(libname && *libname ? libname : "libSDL2-2.0.so.0", RTLD_NOW | RTLD_LOCAL);
    if (!S.lib) {
        fprintf(stderr, "[PAD] SDL2 not available (%s); using /dev/input/js*\n", dlerror());
        return;
    }
    HPS_SYM(Init, "SDL_InitSubSystem");        HPS_SYM(Quit, "SDL_QuitSubSystem");
    HPS_SYM(SetHint, "SDL_SetHint");           HPS_SYM(GetError, "SDL_GetError");
    HPS_SYM(NumJoysticks, "SDL_NumJoysticks"); HPS_SYM(IsGC, "SDL_IsGameController");
    HPS_SYM(DevInstanceID, "SDL_JoystickGetDeviceInstanceID");
    HPS_SYM(GCOpen, "SDL_GameControllerOpen"); HPS_SYM(GCClose, "SDL_GameControllerClose");
    HPS_SYM(GCAttached, "SDL_GameControllerGetAttached");
    HPS_SYM(GCButton, "SDL_GameControllerGetButton"); HPS_SYM(GCAxis, "SDL_GameControllerGetAxis");
    HPS_SYM(GCName, "SDL_GameControllerName"); HPS_SYM(GCUpdate, "SDL_GameControllerUpdate");
    HPS_SYM(RWFromFile, "SDL_RWFromFile");     HPS_SYM(GCAddMappingsRW, "SDL_GameControllerAddMappingsFromRW");
    HPS_SYM(JSOpen, "SDL_JoystickOpen");       HPS_SYM(JSClose, "SDL_JoystickClose");
    HPS_SYM(JSAttached, "SDL_JoystickGetAttached");
    HPS_SYM(JSNumButtons, "SDL_JoystickNumButtons"); HPS_SYM(JSNumAxes, "SDL_JoystickNumAxes");
    HPS_SYM(JSNumHats, "SDL_JoystickNumHats"); HPS_SYM(JSButton, "SDL_JoystickGetButton");
    HPS_SYM(JSAxis, "SDL_JoystickGetAxis");    HPS_SYM(JSHat, "SDL_JoystickGetHat");
    HPS_SYM(JSName, "SDL_JoystickName");       HPS_SYM(JSUpdate, "SDL_JoystickUpdate");
    /* The port has no SDL window: keep reading pads regardless of focus, and
     * keep the positional (Xbox) layout on Nintendo-labelled pads. Steam's
     * SDL_GAMECONTROLLERCONFIG / IGNORE_DEVICES env is read by SDL itself. */
    S.SetHint("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "1");
    S.SetHint("SDL_GAMECONTROLLER_USE_BUTTON_LABELS", "0");
    /* SDL >= 2.28 hides Steam's virtual gamepad unless this is set.  Steam sets
     * it only for the process it launches, so a port started by a launcher
     * under Game Mode could see no controller at all when Steam also hides the
     * physical pad.  A hint never overrides the same variable in the
     * environment. */
    S.SetHint("SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD", "1");
    if (S.Init(HPS_INIT_JOYSTICK | HPS_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "[PAD] SDL2 init failed (%s); using /dev/input/js*\n", S.GetError());
        dlclose(S.lib); S.lib = NULL;
        return;
    }
    for (int i = 0; i < HPS_MAX_DEVICES; i++) S.dev[i].id = -1;
    load_db_file();
    S.state = 1;
    fprintf(stderr, "[PAD] SDL2 GameController backend ready (SteamAppId %s, SDL_GAMECONTROLLER_IGNORE_DEVICES %s)\n",
            getenv("SteamAppId") ? "set" : "unset",
            getenv("SDL_GAMECONTROLLER_IGNORE_DEVICES") ? "set" : "unset");
    return;
fail:
    fprintf(stderr, "[PAD] SDL2 missing symbol %s; using /dev/input/js*\n", miss);
    dlclose(S.lib); S.lib = NULL;
}

static void close_dev(HpsDev *d, const char *why)
{
    if (d->id < 0) return;
    fprintf(stderr, "[PAD] SDL controller disconnected: %s (%s)\n", d->name, why);
    if (d->gc) S.GCClose(d->gc); else if (d->js) S.JSClose(d->js);
    memset(d, 0, sizeof(*d));
    d->id = -1;
}

static void hps_scan(void)
{
    int n = S.NumJoysticks();
    for (int i = 0; i < n; i++) {
        int32_t id = S.DevInstanceID(i);
        HpsDev *free_slot = NULL;
        int have = 0;
        for (int k = 0; k < HPS_MAX_DEVICES; k++) {
            if (S.dev[k].id == id) have = 1;
            if (S.dev[k].id < 0 && !free_slot) free_slot = &S.dev[k];
        }
        if (have || !free_slot) continue;
        if (S.IsGC(i)) {
            void *gc = S.GCOpen(i);
            if (!gc) continue;
            free_slot->gc = gc;
            snprintf(free_slot->name, sizeof(free_slot->name), "%s", S.GCName(gc) ? S.GCName(gc) : "controller");
        } else {
            void *js = S.JSOpen(i);
            if (!js) continue;
            free_slot->js = js;
            snprintf(free_slot->name, sizeof(free_slot->name), "%s", S.JSName(js) ? S.JSName(js) : "joystick");
        }
        free_slot->id = id;
        fprintf(stderr, "[PAD] SDL controller connected: %s (%s)\n", free_slot->name,
                free_slot->gc ? "GameController" : "raw joystick fallback");
    }
}

void HostPadSDL_Poll(void)
{
    uint64_t t;
    if (S.state == 0) hps_init();
    if (S.state != 1) return;
    S.GCUpdate();
    S.JSUpdate();
    t = hps_now_ns();
    if (!S.last_scan || t - S.last_scan >= HPS_SCAN_NS) { S.last_scan = t; hps_scan(); }
    for (int k = 0; k < HPS_MAX_DEVICES; k++) {
        HpsDev *d = &S.dev[k];
        if (d->id < 0) continue;
        if (d->gc) {
            uint8_t b[HPS_BTN_COUNT];
            int16_t a[HPS_AXIS_COUNT];
            if (!S.GCAttached(d->gc)) { close_dev(d, "detached"); continue; }
            for (int i = 0; i < HPS_BTN_COUNT; i++) b[i] = S.GCButton(d->gc, i);
            for (int i = 0; i < HPS_AXIS_COUNT; i++) a[i] = S.GCAxis(d->gc, i);
            d->held = HostPadSDL_MapController(b, a, S.swap_xy, &d->hk);
            d->held_analog = HostPadSDL_MapControllerEx(b, a, S.swap_xy, 0, NULL);
            memcpy(d->axis, a, sizeof d->axis);
        } else {
            uint8_t b[32] = {0};
            int16_t a[2] = {0, 0};
            int nb, na;
            unsigned hat = 0;
            if (!S.JSAttached(d->js)) { close_dev(d, "detached"); continue; }
            nb = S.JSNumButtons(d->js); if (nb > 32) nb = 32; if (nb < 0) nb = 0;
            na = S.JSNumAxes(d->js); if (na > 2) na = 2; if (na < 0) na = 0;
            for (int i = 0; i < nb; i++) b[i] = S.JSButton(d->js, i);
            for (int i = 0; i < na; i++) a[i] = S.JSAxis(d->js, i);
            if (S.JSNumHats(d->js) > 0) hat = S.JSHat(d->js, 0);
            d->held = HostPadSDL_MapJoystick(&S.raw, b, nb, a, na, hat, S.swap_xy, &d->hk);
            d->held_analog = d->held;   /* no analog reply from a raw joystick */
        }
        if (d->held && !d->input_seen) {
            /* One line per device: which pad and backend actually fed input. */
            d->input_seen = 1;
            fprintf(stderr, "[PAD] first input: SDL %s (%s)\n", d->name, d->gc ? "GameController" : "raw joystick");
        }
    }
    if (S.debug) {
        uint16_t h = HostPadSDL_Held();
        if (h != S.last_held) fprintf(stderr, "[PAD] SDL held=0x%04X\n", h);
        S.last_held = h;
    }
}

int HostPadSDL_Count(void)
{
    int n = 0;
    if (S.state != 1) return 0;
    for (int k = 0; k < HPS_MAX_DEVICES; k++) if (S.dev[k].id >= 0) n++;
    return n;
}

uint16_t HostPadSDL_Held(void)
{
    uint16_t h = 0;
    if (S.state != 1) return 0;
    for (int k = 0; k < HPS_MAX_DEVICES; k++) if (S.dev[k].id >= 0) h |= S.dev[k].held;
    return h;
}

uint16_t HostPadSDL_HeldAnalog(void)
{
    uint16_t h = 0;
    if (S.state != 1) return 0;
    for (int k = 0; k < HPS_MAX_DEVICES; k++) if (S.dev[k].id >= 0) h |= S.dev[k].held_analog;
    return h;
}

int HostPadSDL_AnalogSticks(uint8_t out[4])
{
    /* SDL axis order LX, LY, RX, RY; DualShock reply order RX, RY, LX, LY. */
    static const int order[4] = {HPS_AXIS_RX, HPS_AXIS_RY, HPS_AXIS_LX, HPS_AXIS_LY};
    int16_t best[4] = {0, 0, 0, 0};
    int any = 0;
    if (S.state != 1) return 0;
    for (int k = 0; k < HPS_MAX_DEVICES; k++) {
        const HpsDev *d = &S.dev[k];
        if (d->id < 0 || !d->gc) continue;
        any = 1;
        for (int i = 0; i < 4; i++) {
            int v = d->axis[order[i]], b = best[i];
            if ((v < 0 ? -v : v) > (b < 0 ? -b : b)) best[i] = (int16_t)v;
        }
    }
    if (!any) return 0;
    for (int i = 0; i < 4; i++) out[i] = HostPadSDL_AxisToByte(best[i]);
    return 1;
}

int HostPadSDL_Origins(HostPadOrigin *out, int max)
{
    int n = 0;
    if (S.state != 1) return 0;
    for (int k = 0; k < HPS_MAX_DEVICES && n < max; k++)
        if (S.dev[k].id >= 0) {
            out[n].key = 0x40000000u | ((uint32_t)S.dev[k].id & 0x3FFFFFFFu);
            out[n].held = S.dev[k].held;
            n++;
        }
    return n;
}

uint8_t HostPadSDL_Hotkeys(void)
{
    uint8_t h = 0;
    if (S.state != 1) return 0;
    for (int k = 0; k < HPS_MAX_DEVICES; k++) if (S.dev[k].id >= 0) h |= S.dev[k].hk;
    return h;
}

void HostPadSDL_Close(void)
{
    if (S.state == 1) {
        for (int k = 0; k < HPS_MAX_DEVICES; k++) close_dev(&S.dev[k], "shutdown");
        S.Quit(HPS_INIT_JOYSTICK | HPS_INIT_GAMECONTROLLER);
    }
    if (S.lib) dlclose(S.lib);
    memset(&S, 0, sizeof(S));
}
