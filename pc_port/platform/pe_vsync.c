/* Original 80073A44..80073C54,132 words (VSync and its wait helper).
 * SHA256 e356692b0a159f0f9e07da321a2ea515c4789379094ff39d6d77f42858169af4.
 * Device access order is retained, including stable timer reads, signed
 * vblank comparisons, entry-time return value and both post-wait baselines. */
#include "pe_vsync.h"
#include "game_port.h"
#define READ(a) clock->read_word(clock->context,(a))
#define WRITE(a,v) clock->write_word(clock->context,(a),(v))
#define CHECK() do {if(PE_Port_StopEpoch()!=epoch)return 0;} while(0)
static int vsync_wait(uint32_t target,uint32_t frames,const PeVSyncClock *clock)
{
    uint32_t epoch=PE_Port_StopEpoch(),budget=frames<<15;
    uint32_t current=READ(0x800956ACu);CHECK();
    if((int32_t)current<(int32_t)target) {
        for(;;) {
            if(--budget==UINT32_MAX) {
                clock->bios_call(clock->context,0xB0u,0x3Fu,0x800116FCu,0);CHECK();
                clock->bios_call(clock->context,0xB0u,0x5Bu,0,0);CHECK();
                clock->bios_call(clock->context,0xC0u,0xAu,3,0);CHECK();
                break;
            }
            current=READ(0x800956ACu);CHECK();
            if((int32_t)current>=(int32_t)target)break;
        }
    }
    return 1;
}
uint32_t PE_RetailVSync(int32_t mode,const PeVSyncClock *clock)
{
    uint32_t epoch=PE_Port_StopEpoch();
    pe_addr_t gpu=READ(0x80094574u);CHECK();
    pe_addr_t timer=READ(0x80094578u);CHECK();
    uint32_t status=READ(gpu);CHECK();
    uint32_t ticks,again;
    do {ticks=READ(timer);CHECK();again=READ(timer);CHECK();} while(ticks!=again);
    uint32_t baseline=READ(0x8009457Cu);CHECK();
    uint32_t elapsed=(ticks-baseline)&65535u;
    if(mode<0) {uint32_t count=READ(0x800956ACu);CHECK();return count;}
    if(mode==1)return elapsed;
    uint32_t target=READ(0x80094580u);CHECK();
    if(mode>0)target+=(uint32_t)mode-1u;
    if(!vsync_wait(target,mode>0?(uint32_t)mode-1u:0u,clock))return 0;
    gpu=READ(0x80094574u);CHECK();status=READ(gpu);CHECK();
    target=READ(0x800956ACu);CHECK();
    if(!vsync_wait(target+1u,1u,clock))return 0;
    if(status&0x400000u) {
        gpu=READ(0x80094574u);CHECK();
        uint32_t field=READ(gpu);CHECK();
        if(!((field^status)&0x80000000u)) {
            do {field=READ(gpu);CHECK();} while(!((field^status)&0x80000000u));
        }
    }
    uint32_t count=READ(0x800956ACu);CHECK();timer=READ(0x80094578u);CHECK();
    WRITE(0x80094580u,count);CHECK();
    do {
        ticks=READ(timer);CHECK();WRITE(0x8009457Cu,ticks);CHECK();
        baseline=READ(0x8009457Cu);CHECK();again=READ(timer);CHECK();
    } while(baseline!=again);
    return elapsed;
}
#undef READ
#undef WRITE
#undef CHECK
