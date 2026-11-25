/* Synthetic input poll sequences; no physical controller, game or saves. */
#include <stdint.h>
#include <stdio.h>
#include "host_pad.h"
#include "host_pad_combo.h"
static int step(HostPadCombo *c, uint16_t held, uint64_t time, uint16_t expected, int open)
{
    HostPadOrigin origin={1,held};HostPadComboEvents events;
    uint16_t actual=HostPadCombo_Step(c,&origin,1,0,time,&events);
    if(actual!=expected || events.open!=open) {
        fprintf(stderr,"t=%llu held=%04x output=%04x expected=%04x open=%d expected=%d\n",
                (unsigned long long)time,held,actual,expected,events.open,open);return 0;
    }
    return 1;
}
static int rollover(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};
    return step(&c,0,0,0,0) && step(&c,first,10,0,0) &&
           step(&c,second,100,first,0) && step(&c,second,159,first,0) &&
           step(&c,second,160,0,0) && step(&c,second,309,0,0) &&
           step(&c,second,399,0,0) && step(&c,second,400,second,0) &&
           step(&c,0,401,0,0);
}
static int short_rollover(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};
    return step(&c,0,0,0,0) && step(&c,first,10,0,0) &&
           step(&c,second,100,first,0) && step(&c,0,120,first|second,0) &&
           step(&c,0,160,second,0) && step(&c,0,180,0,0);
}
static int distinct_controllers(void)
{
    HostPadCombo c={0};HostPadOrigin pads[2]={{1,0},{2,0}};HostPadComboEvents events;
    if(HostPadCombo_Step(&c,pads,2,0,0,&events) || events.open) return 0;
    pads[0].held=HOST_PAD_SELECT;pads[1].held=HOST_PAD_START;
    if(HostPadCombo_Step(&c,pads,2,0,10,&events) || events.open) return 0;
    uint16_t actual=HostPadCombo_Step(&c,pads,2,0,310,&events);
    return !events.open && actual==(HOST_PAD_SELECT|HOST_PAD_START);
}
static int long_gap(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};
    return step(&c,0,0,0,0) && step(&c,first,10,0,0) &&
           step(&c,second,400,first,0) && step(&c,second,699,0,0) &&
           step(&c,second,700,second,0);
}
static int repeated_rollover(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};
    return step(&c,0,0,0,0) && step(&c,first,10,0,0) &&
           step(&c,second,100,first,0) && step(&c,first,120,first|second,0) &&
           step(&c,0,130,first|second,0) && step(&c,0,180,first,0) &&
           step(&c,0,190,0,0);
}
static int disconnect_after_rollover(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};HostPadComboEvents e;
    if(!step(&c,0,0,0,0) || !step(&c,first,10,0,0) ||
       !step(&c,second,100,first,0)) return 0;
    uint16_t out=HostPadCombo_Step(&c,NULL,0,0,110,&e);
    if(out!=first || e.open) return 0;
    out=HostPadCombo_Step(&c,NULL,0,0,160,&e);
    return out==0 && !e.open;
}
static int overlap_after_rollover(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};
    return step(&c,0,0,0,0) && step(&c,first,10,0,0) &&
           step(&c,second,100,first,0) && step(&c,first|second,120,0,1) &&
           step(&c,first|second,130,0,0) && step(&c,0,140,0,0);
}
static int clock_wrap(uint16_t first,uint16_t second)
{
    HostPadCombo c={0};uint64_t base=UINT64_MAX-250u;
    return step(&c,0,base,0,0) && step(&c,first,base+10u,0,0) &&
           step(&c,second,base+100u,first,0) && step(&c,second,base+160u,0,0) &&
           step(&c,second,base+399u,0,0) && step(&c,second,base+400u,second,0);
}
int main(void)
{
    HostPadCombo combo={0};uint16_t select=HOST_PAD_SELECT,start=HOST_PAD_START;
    if(!rollover(select,start) || !rollover(start,select) ||
       !short_rollover(select,start) || !short_rollover(start,select)) return 1;
    if(!step(&combo,0,0,0,0) || !step(&combo,select,10,0,0) ||
       !step(&combo,select|start,100,0,1)) return 2;
    if(!distinct_controllers()) return 3;
    if(!long_gap(select,start) || !long_gap(start,select) ||
       !repeated_rollover(select,start) || !repeated_rollover(start,select) ||
       !disconnect_after_rollover(select,start) || !disconnect_after_rollover(start,select) ||
       !overlap_after_rollover(select,start) || !overlap_after_rollover(start,select) ||
       !clock_wrap(select,start) || !clock_wrap(start,select)) return 4;
    puts("synthetic rollover, timing, disconnect, overlap and clock-wrap controls pass");return 0;
}
