/*
 * pe_plat cheats: opt-in player/testing cheat console on the mods layer
 * (docs/ARCHITECTURE-PORT.md "A cheat menu and console").
 *
 * Everything is OFF by default.  While every cheat is off the console never
 * writes guest state and the overlay is not drawn, so game logic is
 * unchanged.  PE_CHEATS=0 hard-disables the console (keys ignored).
 *
 * Keys (host window; F6 stays fast-forward):
 *   F1 help overlay         F2 god mode (HP held at max)
 *   F3 infinite ammo + full Parasite Energy
 *   F4 instant win (enemy HP -> 0; the game's own victory/reward path runs)
 *   F5 area kit (key items given by the loaded room scripts, via the game's
 *      own add-item routine func_80053D2C)
 *   F7 fast-forward speed: max / 2x / 4x / 8x (cycles; F6 toggles FF)
 *   F8 warp menu (Day-1 checkpoints; Up/Down + Enter, F8 closes)
 * "CHEATS" is shown on screen whenever any cheat is active.
 *
 * Scripted keys for headless tests: PE_CHEAT_KEYS="frame:KEY,frame:KEY"
 * (frame = present count; KEY = F1..F8, UP, DOWN, ENTER).
 */
#ifndef PE_PLAT_CHEATS_H
#define PE_PLAT_CHEATS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PE_CHEAT_KEY_NONE = 0,
    PE_CHEAT_KEY_F1, PE_CHEAT_KEY_F2, PE_CHEAT_KEY_F3, PE_CHEAT_KEY_F4,
    PE_CHEAT_KEY_F5, PE_CHEAT_KEY_F7, PE_CHEAT_KEY_F8,
    PE_CHEAT_KEY_UP, PE_CHEAT_KEY_DOWN, PE_CHEAT_KEY_ENTER,
    PE_CHEAT_KEY_COUNT
} PeCheatKey;

/* Reads PE_CHEATS / PE_CHEAT_KEYS and subscribes to the mods FRAME_TICK
 * event.  Idempotent. */
void pe_plat_cheats_init(void);
int  pe_plat_cheats_enabled(void);          /* 0 when PE_CHEATS=0 */
/* A key press (pressed=1) or release.  Returns 1 when the console consumed
 * it (the host must not pass it to the game pad). */
int  pe_plat_cheats_key(PeCheatKey key, int pressed);
int  pe_plat_cheats_menu_open(void);        /* warp menu: arrows/Enter are ours */
/* Once per presented frame (emits the mods FRAME_TICK event). */
void pe_plat_cheats_frame(void);
/* Pixels to present: `rgb` unchanged, or a copy with the overlay drawn when
 * any overlay text is visible.  RGB888, width*height*3. */
const uint8_t *pe_plat_cheats_overlay(const uint8_t *rgb, int width, int height);
/* Fast-forward speed while FF is on: 0 = uncapped, else N emulated frames
 * per wall frame. */
int  pe_plat_cheats_ff_multiplier(void);

/* Test access. */
typedef struct {
    int god, ammo, help, menu, menu_index, ff_multiplier;
    unsigned wins, kits, warps;
} PePlatCheatState;
void pe_plat_cheats_get_state(PePlatCheatState *out);
void pe_plat_cheats_reset(void);            /* all off (tests) */
/* One game-state pass (what FRAME_TICK runs); exposed for tests. */
void pe_plat_cheats_apply(void);
/* F5 core: scan [lo,hi) for field op 0xA7 (give item) with an immediate key
 * item id (0x100..0x17F) and add each id not owned.  Returns ids added. */
int  pe_plat_cheats_area_kit(uint32_t lo, uint32_t hi);
int  pe_plat_cheats_warp_count(void);
const char *pe_plat_cheats_warp_name(int index);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_CHEATS_H */
