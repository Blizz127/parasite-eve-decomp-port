/* Pure held-key merger shared by the X11, SDL and Win32 window backends. */
#ifndef HOST_KEY_MERGE_H
#define HOST_KEY_MERGE_H
#include <stdint.h>

#define HOST_KEY_MERGE_KEYS 512u

typedef struct {
    uint16_t key_bits[HOST_KEY_MERGE_KEYS];
    uint16_t held;
} HostKeyMerge;

/* Clear all keys, including on focus loss or a deliberate input reset. */
void HostKeyMerge_Reset(HostKeyMerge *state);
/* A stable host key ID is in [0,512). On key-down, record its current mapped
 * bitmask; repeated key-down is idempotent and may update a changed mapping.
 * Key-up clears that key regardless of mapped_bits. Returns 0 for an invalid
 * ID or NULL state without changing anything; otherwise returns 1. Callers
 * decide which events are consumed by menus/hotkeys and how key IDs are made. */
int HostKeyMerge_Event(HostKeyMerge *state, uint32_t key, uint16_t mapped_bits, int down);
/* OR of all current key contributions; NULL returns zero. */
uint16_t HostKeyMerge_Held(const HostKeyMerge *state);

/* Backend glue: the keyboard's contribution to the pad word.  Several keys may
 * map to one button (Return/space/z/x are all Cross by default), so a button is
 * held while ANY of its keys is.  Protocol, per key event with a host key ID:
 *   - on every key-up, call HostKeyInput_Release FIRST, before menus, hotkeys
 *     or Esc can consume the event, so a consumed release never leaves an
 *     in-range key's contribution stuck (fallback IDs, below, keep the old
 *     per-bit behaviour and still need the Mapped key-up);
 *   - when the event reaches the pad table, call HostKeyInput_Mapped with the
 *     looked-up bit (key-down records it; key-up only matters for IDs outside
 *     [0,512), which fall back to the old per-bit set/clear so a binding
 *     never silently stops working);
 *   - on focus loss, window open/close or input reset, HostKeyInput_Reset.
 * Each call returns the new held word (HostKeyInput_Held). */
typedef struct {
    HostKeyMerge merge;
    uint16_t direct;   /* bits from keys whose ID is outside the merger range */
} HostKeyInput;

void HostKeyInput_Reset(HostKeyInput *in);
uint16_t HostKeyInput_Release(HostKeyInput *in, uint32_t key);
uint16_t HostKeyInput_Mapped(HostKeyInput *in, uint32_t key, uint16_t mapped_bits, int down);
uint16_t HostKeyInput_Held(const HostKeyInput *in);
#endif
