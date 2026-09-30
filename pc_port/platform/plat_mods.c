/* pe_plat mods: event hook table and manifest parser (step P0). */
#include "pe_plat/mods.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int handle;                  /* 0 = free slot */
    PePlatModEvent event;
    PePlatModHook hook;
    void *user;
} PlatModSlot;

static PlatModSlot plat_mod_slots[PE_PLAT_MODS_MAX_HOOKS];
static int plat_mod_next_handle = 1;
static uint64_t plat_mod_sequence;

static const char *const plat_mod_names[PE_PLAT_EVENT_COUNT] = {
    "boot", "frame_tick", "room_enter", "battle_start", "battle_end",
    "item_get", "before_render", "after_render"
};

void pe_plat_mods_reset(void)
{
    memset(plat_mod_slots, 0, sizeof(plat_mod_slots));
    plat_mod_next_handle = 1;
    plat_mod_sequence = 0;
}

int pe_plat_mods_subscribe(PePlatModEvent event, PePlatModHook hook, void *user)
{
    int i;
    if ((int)event < 0 || event >= PE_PLAT_EVENT_COUNT || !hook) return 0;
    for (i = 0; i < PE_PLAT_MODS_MAX_HOOKS; i++) {
        if (!plat_mod_slots[i].handle) {
            plat_mod_slots[i].handle = plat_mod_next_handle++;
            plat_mod_slots[i].event = event;
            plat_mod_slots[i].hook = hook;
            plat_mod_slots[i].user = user;
            return plat_mod_slots[i].handle;
        }
    }
    return 0;
}

int pe_plat_mods_unsubscribe(int handle)
{
    int i;
    if (handle <= 0) return 0;
    for (i = 0; i < PE_PLAT_MODS_MAX_HOOKS; i++)
        if (plat_mod_slots[i].handle == handle) {
            memset(&plat_mod_slots[i], 0, sizeof(plat_mod_slots[i]));
            return 1;
        }
    return 0;
}

int pe_plat_mods_emit(PePlatModEvent event, uint32_t arg0, uint32_t arg1)
{
    PePlatModEventData data;
    int order[PE_PLAT_MODS_MAX_HOOKS], n = 0, i, j, ran = 0;
    if ((int)event < 0 || event >= PE_PLAT_EVENT_COUNT) return 0;
    data.event = event; data.arg0 = arg0; data.arg1 = arg1;
    data.sequence = plat_mod_sequence++;
    /* Snapshot matching slots in subscription (handle) order so hooks may
     * subscribe/unsubscribe while running. */
    for (i = 0; i < PE_PLAT_MODS_MAX_HOOKS; i++)
        if (plat_mod_slots[i].handle && plat_mod_slots[i].event == event) {
            for (j = n; j > 0 && plat_mod_slots[order[j - 1]].handle > plat_mod_slots[i].handle; j--)
                order[j] = order[j - 1];
            order[j] = i;
            n++;
        }
    {
        int handles[PE_PLAT_MODS_MAX_HOOKS];
        for (i = 0; i < n; i++) handles[i] = plat_mod_slots[order[i]].handle;
        for (i = 0; i < n; i++) {
            PlatModSlot *s = &plat_mod_slots[order[i]];
            if (s->handle != handles[i]) continue;   /* removed meanwhile */
            s->hook(&data, s->user);
            ran++;
        }
    }
    return ran;
}

int pe_plat_mods_hook_count(PePlatModEvent event)
{
    int i, n = 0;
    for (i = 0; i < PE_PLAT_MODS_MAX_HOOKS; i++)
        if (plat_mod_slots[i].handle && plat_mod_slots[i].event == event) n++;
    return n;
}

const char *pe_plat_mods_event_name(PePlatModEvent event)
{
    if ((int)event < 0 || event >= PE_PLAT_EVENT_COUNT) return "unknown";
    return plat_mod_names[event];
}

static void plat_mod_copy(char *dst, size_t cap, const char *s, size_t len)
{
    if (len >= cap) len = cap - 1;
    memcpy(dst, s, len);
    dst[len] = '\0';
}

int pe_plat_mods_parse_manifest(const char *text, PePlatModManifest *out)
{
    const char *p = text;
    if (!text || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->abi = PE_PLAT_MODS_ABI_VERSION;
    while (*p) {
        const char *eol = strchr(p, '\n'), *eq, *ks, *ke, *vs, *ve;
        if (!eol) eol = p + strlen(p);
        ks = p;
        while (ks < eol && isspace((unsigned char)*ks)) ks++;
        eq = memchr(ks, '=', (size_t)(eol - ks));
        if (ks < eol && *ks != '#' && eq) {
            ke = eq;
            while (ke > ks && isspace((unsigned char)ke[-1])) ke--;
            vs = eq + 1;
            while (vs < eol && isspace((unsigned char)*vs)) vs++;
            ve = eol;
            while (ve > vs && isspace((unsigned char)ve[-1])) ve--;
            if ((size_t)(ke - ks) == 4 && !memcmp(ks, "name", 4))
                plat_mod_copy(out->name, sizeof(out->name), vs, (size_t)(ve - vs));
            else if ((size_t)(ke - ks) == 7 && !memcmp(ks, "version", 7))
                plat_mod_copy(out->version, sizeof(out->version), vs, (size_t)(ve - vs));
            else if ((size_t)(ke - ks) == 10 && !memcmp(ks, "load_order", 10))
                out->load_order = atoi(vs);
            else if ((size_t)(ke - ks) == 3 && !memcmp(ks, "abi", 3))
                out->abi = atoi(vs);
        }
        p = *eol ? eol + 1 : eol;
    }
    return out->name[0] != '\0' && out->abi == PE_PLAT_MODS_ABI_VERSION;
}
