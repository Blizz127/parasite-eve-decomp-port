/* Opt-in gameplay enhancements (Options > Enhancements in the dev menu).
 *
 * Movement speed scales ONLY Aya's player walk/run velocity in field scenes
 * (not battle, NPCs, scripted moves, cutscenes, timers or RNG).  "Always run"
 * makes the field walk handler take the run path.  Both default off: at the
 * default (100 %, always-run off) the hooks return their input untouched and
 * write no guest state, so the game runs the retail code path unchanged.
 *
 * Settings persist in $XDG_CONFIG_HOME/parasite-eve-port/enhancements.conf
 * (~/.config/... when XDG_CONFIG_HOME is unset) as `move_speed=<pct>` and
 * `always_run=<0|1>` lines.  Overrides:
 *   PE_ENH_CONFIG=<path>  config file to read and write instead (tests use it)
 *   PE_MOVE_SPEED=<pct>   100, 125, 150, 175 or 200 (not written back)
 *   PE_ALWAYS_RUN=<0|1>   (not written back)
 *
 * Assists (dev menu: ASSISTS) live in the same file and default off:
 *   one_hit_kills=<0|1>  PE_ONE_HIT_KILLS=<0|1>   every hit Aya lands is lethal
 *   god_mode=<0|1>       PE_GOD_MODE=<0|1>        Aya's HP never goes down
 *   exp_mult=<1|2|3>     PE_EXP_MULT=<1|2|3>      battle EXP and BP times N
 * At the defaults every assist hook returns its input and writes nothing.
 *
 * Controls (dev menu: OPTIONS > CONTROLS), same file, default off:
 *   analog_stick=<0|1>   PE_ANALOG_STICK=<0|1>    DualShock analog mode
 */
#ifndef PE_PLAT_ENHANCE_H
#define PE_PLAT_ENHANCE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_ENH_MOVE_SPEED_COUNT 5
extern const int pe_enh_move_speed_steps[PE_ENH_MOVE_SPEED_COUNT]; /* 100..200 */

/* ── Option registry ─────────────────────────────────────────────────────
 * Every player-facing option is registered exactly once here (id = config
 * key, category, label, values in cycle order with values[0] = retail, env
 * override, credit).  The dev menu lists the registry per category today; a
 * future standalone Options/Enhancements menu lists the same registry. */
typedef enum {
    PE_ENH_CAT_GRAPHICS = 0,     /* rendering/presentation options */
    PE_ENH_CAT_GAMEPLAY,         /* quality-of-life gameplay options */
    PE_ENH_CAT_ASSIST,           /* cheat-style assists (god mode, EXP, 1-hit) */
    PE_ENH_CAT_AUDIO,            /* output levels (master, movie/CD audio, music, SFX) */
    PE_ENH_CAT_CONTROLS,         /* controller options (DualShock analog mode) */
    PE_ENH_CAT_COUNT
} PeEnhCategory;

typedef enum {
    PE_ENH_FMT_TOGGLE = 0,       /* OFF / ON */
    PE_ENH_FMT_PERCENT,          /* 150% */
    PE_ENH_FMT_MULT,             /* 2X */
    PE_ENH_FMT_NAMED             /* names[k] for values[k], e.g. "FSR 1" */
} PeEnhFormat;

typedef struct {
    const char   *id;            /* config key, e.g. "move_speed" */
    PeEnhCategory category;
    const char   *label;         /* menu label, e.g. "MOVEMENT SPEED" */
    PeEnhFormat   format;
    const int    *values;        /* cycle order; values[0] is the retail default */
    int           nvalues;
    const char   *env;           /* env override, e.g. "PE_MOVE_SPEED" (not written back) */
    const char   *credit;        /* "suggested by", e.g. "u/stellarola", or NULL */
    const char *const *names;    /* PE_ENH_FMT_NAMED only: one name per value; else NULL */
} PeEnhOption;

int  pe_enh_option_count(void);
const PeEnhOption *pe_enh_option(int i);              /* NULL when out of range */
int  pe_enh_option_index(const char *id);             /* -1 when unknown */
int  pe_enh_category_count(PeEnhCategory cat);        /* options in a category */
int  pe_enh_category_option(PeEnhCategory cat, int n);/* n-th option of a category, or -1 */
int  pe_enh_option_value(int i);
int  pe_enh_option_is_retail(int i);                  /* value == values[0] */
void pe_enh_option_set(int i, int value);             /* snapped to a listed value; persists */
void pe_enh_option_cycle(int i);                      /* next listed value, wraps */
/* "MOVEMENT SPEED: 150%" / "ALWAYS RUN: OFF" / "EXP/BP MULTIPLIER: 1X (RETAIL)" */
void pe_enh_option_label(int i, char *buf, size_t n);

/* Loads the config once (idempotent). */
void pe_enh_init(void);
/* Test hook: forget loaded state so the next query re-reads config/env. */
void pe_enh_reset(void);

int  pe_enh_move_speed_pct(void);          /* 100 (default) .. 200 */
int  pe_enh_always_run(void);              /* 0 (default) or 1 */
/* Menu setters: clamp to a listed step and persist to the config file. */
void pe_enh_set_move_speed_pct(int pct);
void pe_enh_cycle_move_speed(void);        /* 100 -> 125 -> ... -> 200 -> 100 */
void pe_enh_set_always_run(int on);

/* Game-side hooks (called from the field walk/run handlers).  `actor` is the
 * guest actor address.  They only act when the actor is Aya (*0x8009D254) and
 * the game is not in battle (D_8009D1A0 & 2 clear). */
int32_t pe_enh_field_speed(uint32_t actor, int32_t speed);  /* speed * pct/100 */
int  pe_enh_field_always_run(uint32_t actor);

/* ── Graphics ────────────────────────────────────────────────────────── */
/* Display scaler: how the finished 320x240 frame is fitted into the window.
 * It runs after HostFB_GetPixels, so VRAM, the present buffer, screenshots,
 * frame counters and guest RAM are identical whatever the setting. */
enum { PE_ENH_SCALER_NEAREST = 0, PE_ENH_SCALER_SHARP_BILINEAR = 1, PE_ENH_SCALER_FSR1 = 2 };
int  pe_enh_display_scaler(void);       /* PE_ENH_SCALER_NEAREST (default) .. FSR1 */
void pe_enh_set_display_scaler(int mode);
int  pe_enh_internal_scale(void);        /* 1 (native default), 2, 3, 4 */
void pe_enh_set_internal_scale(int scale);

/* ── Audio ───────────────────────────────────────────────────────────── */
/* Levels in percent, 100 = retail, listed 100, 90, ..., 0.
 * MASTER VOLUME scales the device feed only (host_audio.c); the mixer, the
 * AKAO driver and the WAV dump are untouched.  MOVIE AUDIO VOLUME scales
 * the SPU's CD-input contribution (movies, CD-XA streams such as the
 * credits); at 100 the mixer skips the multiply. */
int  pe_enh_master_volume(void);
int  pe_enh_movie_volume(void);
void pe_enh_set_master_volume(int percent);
void pe_enh_set_movie_volume(int percent);
/* MUSIC VOLUME and SFX VOLUME scale the SPU voices by class (pe_spu.c): a
 * voice keyed on while the AKAO driver's sound-effect channels own it
 * (D_800BCD50 | D_800BCD60) is SFX, every other voice is music; the class
 * holds until the voice's next key-on.  A class at 100 skips the multiply. */
int  pe_enh_music_volume(void);
int  pe_enh_sfx_volume(void);
void pe_enh_set_music_volume(int percent);
void pe_enh_set_sfx_volume(int percent);

/* ── Controls ────────────────────────────────────────────────────────── */
/* ANALOG STICK.  OFF (retail default): the pad replies as today's digital
 * pad, ID 41h, and the left stick maps to the d-pad.  ON with an SDL
 * GameController connected: the pad replies as a DualShock in analog mode,
 * ID 73h, with the stick bytes at 0x800BE9A4..A7 (RX, RY, LX, LY; 0x80 =
 * centre), and the left stick no longer maps to the d-pad.  Retail's own
 * analog code then runs (func_8003EB04, func_800710A4, func_8007136C,
 * func_80071754, func_80066CE8).  Like retail on a real DualShock, analog
 * mode takes direction and run from the left stick only: func_8003EB04
 * clears the d-pad direction bits and the Circle run bit (D_8009D26C &=
 * ~0x79), so the d-pad does not move Aya or menu cursors.  Without a
 * GameController (or with the raw/js backends only) the reply stays
 * digital.  Only the windowed host installs the analog source; route-pad
 * and PE_PAD_SCRIPT runs stay digital whatever the setting. */
int  pe_enh_analog_stick(void);          /* 0 (default) or 1 */
void pe_enh_set_analog_stick(int on);

/* ── Assists ─────────────────────────────────────────────────────────── */
#define PE_ENH_EXP_MULT_MAX 3
int  pe_enh_one_hit_kills(void);       /* 0 (default) or 1 */
int  pe_enh_god_mode(void);            /* 0 (default) or 1 */
int  pe_enh_exp_mult(void);            /* 1 (default), 2 or 3 */
void pe_enh_set_one_hit_kills(int on);
void pe_enh_set_god_mode(int on);
void pe_enh_set_exp_mult(int mult);    /* clamped to 1..PE_ENH_EXP_MULT_MAX */
void pe_enh_cycle_exp_mult(void);      /* 1 -> 2 -> 3 -> 1 */

/* Game-side assist hooks.
 * pe_enh_enemy_damage: the battle damage routine (func_80028574) passes the
 *   final retail damage just before subtracting it from the enemy's HP
 *   (body+0x10).  With 1-hit kills on it returns max(damage, HP) so the HP
 *   reaches exactly 0 and retail's own death, EXP, BP and item-drop path runs.
 *   Inactive (returns damage) in story-scripted battles, D_8009D1A0 & 0x800.
 * pe_enh_player_shielded: 1 while god mode is on.  The player damage paths
 *   (func_8001F4D4 enemy hits, the contact-damage tick in func_8001D340, the
 *   HP-halving / HP-to-1 status effects in func_80020288) skip only their HP
 *   store; hit reactions, flashes, damage numbers and every other effect run.
 * pe_enh_exp_award / pe_enh_bp_award: the per-kill EXP and BP that
 *   func_80028E94 adds to the battle totals (D_8009D304 / D_8009D21C), times
 *   the multiplier; the level-up logic that consumes the totals is untouched. */
int32_t  pe_enh_enemy_damage(uint32_t body, int32_t damage);
int      pe_enh_player_shielded(void);
uint32_t pe_enh_exp_award(uint32_t exp);
uint32_t pe_enh_bp_award(uint32_t bp);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_ENHANCE_H */
