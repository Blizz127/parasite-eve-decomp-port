#include "host_key_merge.h"
#include <string.h>

void HostKeyMerge_Reset(HostKeyMerge *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

int HostKeyMerge_Event(HostKeyMerge *state, uint32_t key, uint16_t mapped_bits, int down)
{
    if (!state || key >= HOST_KEY_MERGE_KEYS) return 0;
    state->key_bits[key] = down ? mapped_bits : 0u;
    uint16_t held = 0;
    for (unsigned i = 0; i < HOST_KEY_MERGE_KEYS; ++i)
        held = (uint16_t)(held | state->key_bits[i]);
    state->held = held;
    return 1;
}

uint16_t HostKeyMerge_Held(const HostKeyMerge *state)
{
    return state ? state->held : 0u;
}

void HostKeyInput_Reset(HostKeyInput *in)
{
    if (!in) return;
    HostKeyMerge_Reset(&in->merge);
    in->direct = 0;
}

uint16_t HostKeyInput_Held(const HostKeyInput *in)
{
    return in ? (uint16_t)(HostKeyMerge_Held(&in->merge) | in->direct) : 0u;
}

uint16_t HostKeyInput_Release(HostKeyInput *in, uint32_t key)
{
    if (in) (void)HostKeyMerge_Event(&in->merge, key, 0u, 0);
    return HostKeyInput_Held(in);
}

uint16_t HostKeyInput_Mapped(HostKeyInput *in, uint32_t key, uint16_t mapped_bits, int down)
{
    if (!in) return 0u;
    if (key < HOST_KEY_MERGE_KEYS) {
        if (down) (void)HostKeyMerge_Event(&in->merge, key, mapped_bits, 1);
        else (void)HostKeyMerge_Event(&in->merge, key, 0u, 0);
    } else if (down) {
        in->direct = (uint16_t)(in->direct | mapped_bits);
    } else {
        in->direct = (uint16_t)(in->direct & (uint16_t)~mapped_bits);
    }
    return HostKeyInput_Held(in);
}
