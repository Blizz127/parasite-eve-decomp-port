/* BIOS A(2Fh) rand / A(30h) srand host model.  Contract from psx-spx "BIOS
 * Misc Functions" (https://psx-spx.consoledev.net/kernelbios/#bios-misc-functions):
 * rand advances x = x*41C64E6Dh + 3039h and returns (x/10000h) AND 7FFFh;
 * srand sets x.  The guest addresses are bound in pe_plat/bios_rand.h. */
#include "pe_plat/bios_rand.h"

static uint32_t s_bios_rand_seed = 1u;

void pe_plat_bios_srand(uint32_t seed)
{
    s_bios_rand_seed = seed;
}

unsigned int pe_plat_bios_rand(void)
{
    s_bios_rand_seed = s_bios_rand_seed * 0x41C64E6Du + 0x3039u;
    return (s_bios_rand_seed >> 16) & 0x7FFFu;
}

/* The current seed, so a test can restore it with srand after running an
 * RNG-calling callback. */
uint32_t PE_BiosRandSeed(void)
{
    return s_bios_rand_seed;
}
