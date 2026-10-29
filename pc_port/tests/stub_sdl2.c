/* Test-only stand-in for libSDL2-2.0.so.0: just the joystick/controller
 * symbols host_pad_sdl.c dlsyms, driven by stub_* control functions so the
 * runtime (hot-plug, controller vs raw-joystick, fallback) runs without
 * hardware or a real SDL. Never shipped. */
#include <stdint.h>
#include <string.h>

typedef struct { int present, is_gc; int32_t id; uint8_t btn[32]; int16_t axis[8]; uint8_t hat; int nbtn, naxis, nhat; int open; } StubDev;
static StubDev g_dev[4];
static int g_inited, g_init_fail, g_mappings_added;
static int g_hints;

void stub_reset(void) { memset(g_dev, 0, sizeof(g_dev)); g_inited = g_init_fail = g_mappings_added = g_hints = 0; }
void stub_set_init_fail(int f) { g_init_fail = f; }
void stub_plug(int slot, int32_t id, int is_gc, int nbtn, int naxis, int nhat)
{ memset(&g_dev[slot], 0, sizeof(g_dev[slot])); g_dev[slot].present = 1; g_dev[slot].id = id; g_dev[slot].is_gc = is_gc;
  g_dev[slot].nbtn = nbtn; g_dev[slot].naxis = naxis; g_dev[slot].nhat = nhat; }
void stub_unplug(int slot) { g_dev[slot].present = 0; }
void stub_button(int slot, int b, int v) { g_dev[slot].btn[b] = (uint8_t)v; }
void stub_axis(int slot, int a, int v) { g_dev[slot].axis[a] = (int16_t)v; }
void stub_hat(int slot, int v) { g_dev[slot].hat = (uint8_t)v; }
int  stub_inited(void) { return g_inited; }
int  stub_hints(void) { return g_hints; }
int  stub_mappings_added(void) { return g_mappings_added; }

/* device index i = i-th present slot */
static StubDev *by_index(int i) { for (int k = 0; k < 4; k++) if (g_dev[k].present && i-- == 0) return &g_dev[k]; return 0; }

int SDL_InitSubSystem(uint32_t f) { (void)f; if (g_init_fail) return -1; g_inited = 1; return 0; }
void SDL_QuitSubSystem(uint32_t f) { (void)f; g_inited = 0; }
int SDL_SetHint(const char *n, const char *v) { (void)n; (void)v; g_hints++; return 1; }
const char *SDL_GetError(void) { return "stub init failure"; }
int SDL_NumJoysticks(void) { int n = 0; for (int k = 0; k < 4; k++) n += g_dev[k].present; return n; }
int SDL_IsGameController(int i) { StubDev *d = by_index(i); return d && d->is_gc; }
int32_t SDL_JoystickGetDeviceInstanceID(int i) { StubDev *d = by_index(i); return d ? d->id : -1; }
static void *open_i(int i) { StubDev *d = by_index(i); if (!d) return 0; d->open = 1; return d; }
void *SDL_GameControllerOpen(int i) { return open_i(i); }
void SDL_GameControllerClose(void *p) { ((StubDev *)p)->open = 0; }
int SDL_GameControllerGetAttached(void *p) { return ((StubDev *)p)->present; }
uint8_t SDL_GameControllerGetButton(void *p, int b) { return b < 32 ? ((StubDev *)p)->btn[b] : 0; }
int16_t SDL_GameControllerGetAxis(void *p, int a) { return a < 8 ? ((StubDev *)p)->axis[a] : 0; }
const char *SDL_GameControllerName(void *p) { (void)p; return "Stub Steam Virtual Gamepad"; }
void SDL_GameControllerUpdate(void) {}
void *SDL_RWFromFile(const char *f, const char *m) { (void)f; (void)m; return (void *)1; }
int SDL_GameControllerAddMappingsFromRW(void *rw, int fr) { (void)rw; (void)fr; return ++g_mappings_added; }
void *SDL_JoystickOpen(int i) { return open_i(i); }
void SDL_JoystickClose(void *p) { ((StubDev *)p)->open = 0; }
int SDL_JoystickGetAttached(void *p) { return ((StubDev *)p)->present; }
int SDL_JoystickNumButtons(void *p) { return ((StubDev *)p)->nbtn; }
int SDL_JoystickNumAxes(void *p) { return ((StubDev *)p)->naxis; }
int SDL_JoystickNumHats(void *p) { return ((StubDev *)p)->nhat; }
uint8_t SDL_JoystickGetButton(void *p, int b) { return b < 32 ? ((StubDev *)p)->btn[b] : 0; }
int16_t SDL_JoystickGetAxis(void *p, int a) { return a < 8 ? ((StubDev *)p)->axis[a] : 0; }
uint8_t SDL_JoystickGetHat(void *p, int h) { (void)h; return ((StubDev *)p)->hat; }
const char *SDL_JoystickName(void *p) { (void)p; return "Stub USB PS1 adapter"; }
void SDL_JoystickUpdate(void) {}
