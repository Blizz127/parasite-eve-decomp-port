/*
 * pe_plat mods interface (docs/ARCHITECTURE-PORT.md, step P0).
 *
 * Named game events that plugins subscribe to, plus the mod manifest
 * format.  This is the hook table only: folder scanning, shared-object
 * plugins, asset replacement and the cheat console build on it later.
 * With no subscriber every emit is a no-op, so default play is unchanged.
 *
 * Manifest (`mod.txt`, one `key = value` per line, `#` comments):
 *   name = Example Mod
 *   version = 1.0
 *   load_order = 10
 *   abi = 1
 *
 * In-house implementation: platform/plat_mods.c.
 */
#ifndef PE_PLAT_MODS_H
#define PE_PLAT_MODS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_PLAT_MODS_ABI_VERSION 1
#define PE_PLAT_MODS_MAX_HOOKS   64

typedef enum {
    PE_PLAT_EVENT_BOOT = 0,
    PE_PLAT_EVENT_FRAME_TICK,
    PE_PLAT_EVENT_ROOM_ENTER,      /* arg0 = room id */
    PE_PLAT_EVENT_BATTLE_START,
    PE_PLAT_EVENT_BATTLE_END,
    PE_PLAT_EVENT_ITEM_GET,        /* arg0 = item id, arg1 = count */
    PE_PLAT_EVENT_BEFORE_RENDER,
    PE_PLAT_EVENT_AFTER_RENDER,
    PE_PLAT_EVENT_COUNT
} PePlatModEvent;

typedef struct {
    PePlatModEvent event;
    uint32_t arg0, arg1;
    uint64_t sequence;             /* emits so far, all events */
} PePlatModEventData;

typedef void (*PePlatModHook)(const PePlatModEventData *event, void *user);

typedef struct {
    char name[64];
    char version[32];
    int  load_order;
    int  abi;
} PePlatModManifest;

void pe_plat_mods_reset(void);                      /* drop every hook */
/* Returns a handle > 0, or 0 when the event is invalid or the table full. */
int  pe_plat_mods_subscribe(PePlatModEvent event, PePlatModHook hook, void *user);
int  pe_plat_mods_unsubscribe(int handle);
/* Calls the hooks in subscription order; returns how many ran. */
int  pe_plat_mods_emit(PePlatModEvent event, uint32_t arg0, uint32_t arg1);
int  pe_plat_mods_hook_count(PePlatModEvent event);
const char *pe_plat_mods_event_name(PePlatModEvent event);

/* Parse manifest text.  1 when it has a name and a compatible (or absent,
 * meaning current) ABI. */
int  pe_plat_mods_parse_manifest(const char *text, PePlatModManifest *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_MODS_H */
