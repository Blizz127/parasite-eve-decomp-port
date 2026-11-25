/* Synthetic effect-stack and RAM lifecycle integration; no game data. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guest_ram.h"
#include "pe_port_compat.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d\n",__LINE__); return 1; } } while(0)
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
/* Invented callback adapter invokes the native stack model in draw context. */
int func_800C9B68(int slot) { PE_EffectStackWeaponCallback(0x800C9EA8u,(pe_addr_t)slot);return 1; }
int func_800CE144(int slot) { (void)slot;abort(); }
int func_800CD8C8(int slot) { (void)slot;abort(); }
int main(void) {
    PeEffectStackCause cause;int16_t xyz[3];
    PE_RamInit();PE_StoreU8(0x80001001u,3);
    PE_EffectStackBegin();
    CHECK(PE_EffectStackWeaponDraw(0x800C9B68u,0x80001000u)==1);
    PE_EffectStackCause(&cause);CHECK(cause.active && cause.known==7);
    PE_EffectStackPosition(xyz);CHECK(xyz[0]==0 && xyz[1]==0 && xyz[2]==192);
    PE_EffectStackEnd();PE_EffectStackBegin();
    PE_EffectStackCause(&cause);CHECK(cause.known==4); /* same image retains Z */
    CHECK(PE_EffectStackWeaponDraw(0x800C9B68u,0x80001000u)==1);
    PE_EffectStackCause(&cause);CHECK(cause.known==7);PE_EffectStackEnd();
    PE_RamDestroy();PE_EffectStackBegin();
    PE_EffectStackCause(&cause);CHECK(cause.known==0 && cause.reset_fn==PE_EFFECT_STACK_RESET_RAM);
    PE_EffectStackEnd();PE_RamInit();PE_EffectStackBegin();
    PE_EffectStackCause(&cause);CHECK(cause.known==0 && cause.reset_fn==PE_EFFECT_STACK_RESET_RAM);
    PE_EffectStackEnd();PE_RamDestroy();
    puts("synthetic native effect-stack RAM lifecycle controls pass");return 0;
}
