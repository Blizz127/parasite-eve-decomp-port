/*
 * Phase 6D-S — Boot memory-region layout initializer.
 *
 * Produces pe_addr_t guest addresses instead of host pointers.
 * All 19 values match the retail arithmetic of the exact-matching source
 * (src/func_8006A8D4.c), computed from known PS1 guest addresses
 * 0x800F34F8, 0x8010BD00, 0x80120D08, 0x801ED800 and D_80011614.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* Known guest addresses */
#define GA_D_800F34F8  0x800F34F8u
#define GA_D_8010BD00  0x8010BD00u
#define GA_D_80120D08  0x80120D08u
#define GA_D_801ED800  0x801ED800u

void func_8006A8D4(void)
{
    pe_addr_t cursor;
    pe_addr_t next;
    uint32_t  step_8000;
    uint32_t  step_48000;

    step_48000 = 0x48000;
    cursor = GA_D_800F34F8;
    next = cursor + 0x1800;
    D_800B0E24 = cursor;
    cursor += 0x6000;
    D_800B0E28 = next;
    next = 0xE000;  /* this is a delta, not a guest address */
    D_800B0E2C = cursor;
    cursor += next;
    D_800B0E30 = cursor;

    cursor = GA_D_8010BD00;
    next   = GA_D_80120D08;
    D_800B0E40 = cursor;
    cursor = next + 0x1C98;
    D_800B0E34 = next;
    next += 0x5C98;
    step_8000 = 0x8000;
    D_800B0E38 = cursor;
    cursor += step_8000;
    D_800B0E3C = next;
    next = cursor + 0x2400;
    D_800B0E44 = cursor;
    cursor += 0x4800;
    D_800B0E4C = cursor;
    cursor += step_48000;
    D_800B0E48 = next;
    next = cursor + 0x4000;
    D_800B0E50 = cursor;
    cursor += step_8000;
    D_800B0E54 = next;
    next = cursor + 0x3800;
    D_800B0E5C = next;
    /* Retail reads the D_80011614 global here. Bootstrap fixtures use their
     * policy address; real-disc startup adopts the authenticated rodata
     * value before this function runs. */
    next = D_80011614;
    D_800B0E58 = cursor;
    cursor += 0x7000;
    D_800B0E60 = cursor;

    cursor = GA_D_801ED800;
    D_800B0E6C = cursor;
    cursor = next - 8;
    D_800B0E64 = cursor;
    D_800B0E68 = next;
}
