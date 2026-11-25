/* Synthetic native guest-RAM lifecycle; no game data or saves. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guest_ram.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d\n",__LINE__); return 1; } } while(0)
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
int main(void) {
    uint64_t initial=PE_RamGeneration();PE_RamDestroy();CHECK(PE_RamGeneration()==initial);
    PE_RamInit();uint64_t first=PE_RamGeneration();CHECK(first!=initial);
    PE_RamInit();CHECK(PE_RamGeneration()==first);
    PE_StoreU32(0x80001000u,0x12345678u);
    PE_RamReset();uint64_t reset=PE_RamGeneration();CHECK(reset!=first);
    CHECK(PE_LoadU32(0x80001000u)==0);
    /* A generation-keyed cache becomes stale as soon as its image is freed. */
    uint64_t cached=reset;PE_RamDestroy();uint64_t destroyed=PE_RamGeneration();
    CHECK(destroyed!=cached);
    PE_RamDestroy();CHECK(PE_RamGeneration()==destroyed);
    PE_RamInit();CHECK(PE_RamGeneration()!=destroyed);
    CHECK(PE_LoadU32(0x80001000u)==0);
    PE_RamDestroy();puts("synthetic RAM generation lifecycle controls pass");return 0;
}
