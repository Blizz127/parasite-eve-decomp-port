/* Native HBlank-clocked timer1. The original VBlank initializer uses 0x107.
 * Hardware contract: https://psx-spx.consoledev.net/timers/
 * All VBlank synchronization modes support overflow/target reset.
 * System clocks and timer IRQ delivery remain boundaries.
 * Physical write/reset hold cycles are not modelled. */
#include "pe_timer1.h"
static uint16_t counter,mode,target;
static int initialized,running;
void PE_Timer1_Reset(void)
{
    counter=mode=target=0;initialized=running=0;
}
void PE_Timer1_InitializeVBlankCounter(void)
{
    (void)PE_Timer1_WriteMode(0x107u);
}
int PE_Timer1_WriteMode(uint16_t value)
{
    uint16_t next=value&0x3ffu;
    /* Bit9 has no prescaler effect for timer1. With sync disabled its
     * selector bits are inert; IRQ style bits are inert without IRQ enables. */
    unsigned sync=(next>>1)&3u;
    if(!(next&0x100u) || (next&0x30u))return 0;
    mode=next|0x400u;counter=0;initialized=1;
    running=!(next&1u) || sync!=3u;
    return 1;
}
uint32_t PE_Timer1_ReadCounter(void) {return counter;}
void PE_Timer1_WriteCounter(uint16_t value) {counter=value;}
uint32_t PE_Timer1_ReadTarget(void) {return target;}
void PE_Timer1_WriteTarget(uint16_t value) {target=value;}
uint32_t PE_Timer1_PeekMode(void) {return mode;}
uint32_t PE_Timer1_ReadMode(void)
{
    uint32_t result=mode;
    mode&=(uint16_t)~0x1800u;
    return result;
}
int PE_Timer1_HBlankEdge(int vblank_level,PeIrqGeneration generation)
{
    if(generation!=PE_IRQ_Generation())return 0;
    if(initialized && running) {
        if(mode&1u) {
            unsigned sync=(mode>>1)&3u;
            if((sync==0u && vblank_level) || (sync==2u && !vblank_level))return 1;
        }
        counter++;
        if(counter==0xFFFFu)mode|=0x1000u;
        if(counter==target) {
            mode|=0x800u;
            if(mode&8u)counter=0;
        }
    }
    return 1;
}
int PE_Timer1_VBlankEdge(PeIrqGeneration generation)
{
    if(generation!=PE_IRQ_Generation())return 0;
    if(initialized) {
        if((mode&7u)==3u || (mode&7u)==5u)counter=0; /* sync1/2: VBlank entry */
        running=1;
    }
    return 1;
}
