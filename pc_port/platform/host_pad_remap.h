/*
 * Controls remapping: opt-in, config-file driven.  Pure: no X11, Win32 or SDL,
 * so every rule is unit-tested without a window (tests/test_host_pad_remap.c).
 *
 * File (one setting per line, '#' starts a comment):
 *   $XDG_CONFIG_HOME/parasite-eve-port/controls.conf   (~/.config/... when unset;
 *   %APPDATA%\parasite-eve-port\controls.conf on Windows), or PE_CONTROLS_CONFIG=<path>
 *
 *   pad.<psx>=<psx>                    a connected controller's PS1 button <psx>
 *                                      is delivered to the game as the other one
 *   key.<psx>=<keyname>[,<keyname>...] keyboard bindings (X11 keysym names, or 0x<hex>);
 *                                      the line replaces that button's default keys
 *   <psx>: cross circle square triangle l1 r1 l2 r2 select start up down left right
 *
 * No file means identity: the pad word and the key table are exactly the
 * pre-remap behaviour.  Nothing here ever creates the file.  Unknown or invalid
 * lines are ignored with one warning each; a repeated pad.<x>/key.<x> line
 * replaces the earlier one (and warns).  A key drives one button: a key.<x>
 * line takes its keys from whatever button (default or earlier line) had them.
 *
 * Esc and F1-F8 (quit, the cheat console, fast-forward) are never bindable,
 * and the dev-menu combo reads the controller's unremapped bits, so no config
 * can lock the player out of the menus.
 */
#ifndef HOST_PAD_REMAP_H
#define HOST_PAD_REMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "host_pad.h"

/* Per-line / per-file limits: a longer line or a larger file is rejected. */
#define HOST_PAD_REMAP_LINE_MAX       512
#define HOST_PAD_REMAP_FILE_MAX       65536
#define HOST_PAD_REMAP_KEYS_PER_LINE  16
/* 14 buttons x 16 keys never exceeds this; the parser still checks. */
#define HOST_PAD_REMAP_MAX_KEYS       256

typedef struct {
    uint16_t pad_dest[16];  /* source bit index -> delivered bit (identity by default) */
    int      pad_active;    /* some pad_dest entry is not identity */
    uint32_t key_sym[HOST_PAD_REMAP_MAX_KEYS];  /* keysym (A-Z folded to a-z) ... */
    uint16_t key_bit[HOST_PAD_REMAP_MAX_KEYS];  /* ... -> HOST_PAD_* bit */
    int      nkey;
    uint16_t pad_set;       /* buttons with a pad.<x> line in effect */
    uint16_t key_set;       /* buttons with a key.<x> line in effect */
    int      loaded;        /* settings in effect: popcount(pad_set) + popcount(key_set) */
    int      warnings;      /* lines ignored or replaced by the last parse/load */
} HostPadRemap;

/* Identity pad map and the built-in keyboard table (the old if-chain). */
void HostPadRemap_Defaults(HostPadRemap *r);

/* Reset r to the defaults, then apply text[0..len).  src labels warnings
 * ("<src>:<line>: ..."); log may be NULL (silent).  Returns r->loaded. */
int  HostPadRemap_Parse(HostPadRemap *r, const char *text, size_t len,
                        const char *src, FILE *log);

/* The config file path from the environment (PE_CONTROLS_CONFIG, then the XDG
 * config dir, then ~/.config; %APPDATA% first on Windows).  0 = no candidate. */
int  HostPadRemap_ConfigPath(char *out, size_t cap);

/* Read and apply one file into r.  A missing file is silent and leaves the
 * defaults (returns 0); any other open/read error warns once on log and also
 * leaves the defaults.  1 = a file was read; "[CONTROLS] loaded N remaps from
 * <path>" is then written to log. */
int  HostPadRemap_LoadFile(HostPadRemap *r, const char *path, FILE *log);

/* Process-wide config.  Identity until HostPadRemap_Init() runs (the window
 * backends call it from HostWindow_Open, so headless and route runs never read
 * the file); HostPadRemap_Reset() returns it to identity (HostWindow_Close). */
const HostPadRemap *HostPadRemap_Global(void);
int  HostPadRemap_Init(void);   /* 1 = a file applied (logged on stderr) */
void HostPadRemap_Reset(void);

/* pad.* applied to a controller word (active-high HOST_PAD_* bits); bits that
 * are not one of the 14 buttons pass through. */
uint16_t HostPadRemap_Pad(const HostPadRemap *r, uint16_t held);

/* The raw (active-low) pad word handed to the game: ~(keys | remapped
 * controller).  controller_filtered is HostDevMenu_FilterPad's output, so the
 * combo has already seen the physical bits; keyboard bits are never touched by
 * pad.*. */
uint16_t HostPadRemap_PadWord(const HostPadRemap *r, uint16_t keys_held,
                              uint16_t controller_filtered);

/* Keyboard: the HOST_PAD_* bit a keysym (XLookupKeysym index 0) drives, 0 if
 * none.  Reserved keys always return 0. */
uint16_t HostPadRemap_Key(const HostPadRemap *r, unsigned long keysym);
/* Esc and F1-F8: consumed upstream, never bindable. */
int  HostPadRemap_KeyReserved(unsigned long keysym);

/* Windows virtual-key code -> X11 keysym (explicit table, 0 = unmapped), so
 * the Win32 backend shares the keysym-keyed table. */
unsigned long HostPadRemap_KeysymFromWin32Vk(unsigned vk);

#endif
