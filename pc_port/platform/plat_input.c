/*
 * pe_plat input: backend dispatch, rebinding and injection (step P0).
 * The host_pad binding lives in plat_input_host_pad.c.
 */
#include "pe_plat/input.h"

#include <stddef.h>

static const PePlatInputBackend *plat_in_backend;
static uint16_t plat_in_map[16];      /* physical bit index -> logical bits */
static uint16_t plat_in_injected[PE_PLAT_INPUT_PORTS];

void pe_plat_input_reset_bindings(void)
{
    int i;
    for (i = 0; i < 16; i++) plat_in_map[i] = (uint16_t)(1u << i);
}

void pe_plat_input_set_backend(const PePlatInputBackend *backend)
{
    plat_in_backend = backend;
}

void pe_plat_input_init(void)
{
    int p;
    pe_plat_input_reset_bindings();
    for (p = 0; p < PE_PLAT_INPUT_PORTS; p++) plat_in_injected[p] = 0;
}

void pe_plat_input_shutdown(void)
{
    const PePlatInputBackend *b = plat_in_backend;
    plat_in_backend = NULL;
    if (b && b->close) b->close(b->ctx);
}

void pe_plat_input_poll(void)
{
    if (plat_in_backend && plat_in_backend->poll)
        plat_in_backend->poll(plat_in_backend->ctx);
}

static int plat_single_bit(uint16_t v)
{
    int i;
    if (!v || (v & (v - 1u))) return -1;
    for (i = 0; i < 16; i++) if (v == (uint16_t)(1u << i)) return i;
    return -1;
}

int pe_plat_input_bind(uint16_t physical, uint16_t logical)
{
    int i = plat_single_bit(physical);
    if (i < 0 || (logical && plat_single_bit(logical) < 0)) return 0;
    plat_in_map[i] = logical;
    return 1;
}

void pe_plat_input_inject(int port, uint16_t buttons)
{
    if (port >= 0 && port < PE_PLAT_INPUT_PORTS) plat_in_injected[port] = buttons;
}

void pe_plat_input_state(int port, PePlatInputState *out)
{
    uint16_t phys = 0, logical = 0;
    int i;
    if (!out) return;
    out->buttons = 0; out->hotkeys = 0; out->connected = 0;
    if (port < 0 || port >= PE_PLAT_INPUT_PORTS) return;
    if (plat_in_backend) {
        out->connected = 1;
        if (plat_in_backend->held) phys = plat_in_backend->held(plat_in_backend->ctx, port);
        if (port == 0 && plat_in_backend->hotkeys)
            out->hotkeys = plat_in_backend->hotkeys(plat_in_backend->ctx);
    }
    for (i = 0; i < 16; i++) if (phys & (1u << i)) logical |= plat_in_map[i];
    out->buttons = (uint16_t)(logical | plat_in_injected[port]);
}

uint16_t pe_plat_input_pad_word(uint16_t buttons)
{
    return (uint16_t)~buttons;
}
