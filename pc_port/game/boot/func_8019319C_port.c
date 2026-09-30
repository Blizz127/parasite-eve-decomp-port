/*
 * Title object +0x14 callback func_8019319C — allocates type-2 via FBC0.
 *
 * Retail overlay: [0x8019319C, 0x801931BC), 8 words / 0x20.
 */
#include "psx_compat.h"
#include "game_port.h"

extern pe_addr_t func_8018FBC0(int type);

void func_8019319C(pe_addr_t node)
{
    (void)node;
    (void)func_8018FBC0(2);
}
