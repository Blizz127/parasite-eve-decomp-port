/* Original743B4 writes0x107 to1F801114 before clearing VBlank callbacks.
 * Hardware contract: https://psx-spx.consoledev.net/timers/
 * This lane implements that configuration only: HBlank clock, synchronize
 * mode3 (wait for first VBlank then free-run), no timer IRQ or target reset.
 * A general timer register interface and GPU edge scheduler remain separate. */
#include "pe_timer1.h"
static uint16_t counter,mode;
static int initialized,running;
void PE_Timer1_Reset(void)
{
    counter=mode=0;initialized=running=0;
}
void PE_Timer1_InitializeVBlankCounter(void)
{
    counter=0;mode=0x507;initialized=1;running=0;
}
uint32_t PE_Timer1_ReadCounter(void) {return counter;}
uint32_t PE_Timer1_PeekMode(void) {return mode;}
uint32_t PE_Timer1_ReadMode(void)
{
    uint32_t result=mode;
    mode&=(uint16_t)~0x1800u;
    return result;
}
int PE_Timer1_HBlankEdge(PeIrqGeneration generation)
{
    if(generation!=PE_IRQ_Generation())return 0;
    if(initialized && running) {
        counter++;
        if(counter==0xFFFFu)mode|=0x1000u;
        if(counter==0)mode|=0x800u; /* reset-state target is zero */
    }
    return 1;
}
int PE_Timer1_VBlankEdge(PeIrqGeneration generation)
{
    if(generation!=PE_IRQ_Generation())return 0;
    if(initialized)running=1;
    return 1;
}
