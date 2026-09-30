/*
 * pe_plat input interface (docs/ARCHITECTURE-PORT.md, step P0).
 *
 * Meaning-level: which logical controller buttons are held, plus port
 * hotkeys (fast-forward) that never reach the game.  Buttons are
 * active-high PE_PLAT_BTN_* bits; pe_plat_input_pad_word() gives the
 * console's active-low digital-pad word for the game's pad buffers.
 *
 * A backend supplies the physical state (default: host_pad, platform/
 * plat_input_host_pad.c).  On top of it this layer adds rebinding (one
 * physical button -> one logical button) and injected input for the
 * route autopilot / test harness, which OR with the physical state.
 *
 * Game code must include only pe_plat headers (never host_pad.h).
 */
#ifndef PE_PLAT_INPUT_H
#define PE_PLAT_INPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_PLAT_BTN_SELECT   0x0001u
#define PE_PLAT_BTN_START    0x0008u
#define PE_PLAT_BTN_UP       0x0010u
#define PE_PLAT_BTN_RIGHT    0x0020u
#define PE_PLAT_BTN_DOWN     0x0040u
#define PE_PLAT_BTN_LEFT     0x0080u
#define PE_PLAT_BTN_L2       0x0100u
#define PE_PLAT_BTN_R2       0x0200u
#define PE_PLAT_BTN_L1       0x0400u
#define PE_PLAT_BTN_R1       0x0800u
#define PE_PLAT_BTN_TRIANGLE 0x1000u
#define PE_PLAT_BTN_CIRCLE   0x2000u
#define PE_PLAT_BTN_CROSS    0x4000u
#define PE_PLAT_BTN_SQUARE   0x8000u

#define PE_PLAT_HOTKEY_FAST_FORWARD_HOLD   0x01u
#define PE_PLAT_HOTKEY_FAST_FORWARD_TOGGLE 0x02u

#define PE_PLAT_INPUT_PORTS 2

typedef struct {
    uint16_t buttons;   /* logical PE_PLAT_BTN_* held (after rebinding) */
    uint8_t  hotkeys;   /* PE_PLAT_HOTKEY_* held */
    uint8_t  connected; /* a physical backend is installed */
} PePlatInputState;

/* Physical source.  Every hook is optional. */
typedef struct {
    void *ctx;
    void     (*poll)(void *ctx);
    uint16_t (*held)(void *ctx, int port);     /* physical PE_PLAT_BTN_* bits */
    uint8_t  (*hotkeys)(void *ctx);
    void     (*close)(void *ctx);
    const char *name;
} PePlatInputBackend;

void pe_plat_input_set_backend(const PePlatInputBackend *backend); /* NULL = none */
/* host_pad backend (defined in plat_input_host_pad.c). */
const PePlatInputBackend *pe_plat_input_host_pad_backend(void);

void pe_plat_input_init(void);          /* identity bindings, no injection */
void pe_plat_input_shutdown(void);
void pe_plat_input_poll(void);
void pe_plat_input_state(int port, PePlatInputState *out);

/* Rebinding: physical button bit -> logical button bit (0 = unbound). */
int  pe_plat_input_bind(uint16_t physical, uint16_t logical);
void pe_plat_input_reset_bindings(void);

/* Injected logical buttons (autopilot / tests), OR-ed into the state. */
void pe_plat_input_inject(int port, uint16_t buttons);

/* Console pad word: active-low button bits. */
uint16_t pe_plat_input_pad_word(uint16_t buttons);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_INPUT_H */
