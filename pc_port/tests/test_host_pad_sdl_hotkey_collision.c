/* Synthetic SDL raw-joystick mapping; no controller, game or save. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "host_pad.h"
#include "host_pad_sdl.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"failed line %d\n",__LINE__); return 1; } } while(0)
static uint16_t map(const HostPadRawMap *m,int button,uint8_t *hotkeys)
{
    uint8_t buttons[16]={0};int16_t axes[2]={0};
    buttons[button]=1;
    return HostPadSDL_MapJoystick(m,buttons,16,axes,2,0,0,hotkeys);
}
int main(void)
{
    HostPadRawMap m;uint8_t hk;
    HostPadSDL_RawMapDefaults(&m);
    CHECK(map(&m,9,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_TOGGLE);
    CHECK(map(&m,10,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_HOLD);
    /* Existing DirectInput example assigns Start to the default L3 index. */
    CHECK(HostPadSDL_RawMapParse(&m,"2,1,3,0,6,7,4,5,8,9")==0);
    CHECK(map(&m,9,&hk)==HOST_PAD_START && hk==0);
    CHECK(map(&m,10,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_HOLD);
    CHECK(m.l3==9 && m.r3==10);
    CHECK(HostPadSDL_RawMapParse(&m,"10,1,2,3,4,5,-1,-1,6,7")==0);
    CHECK(map(&m,10,&hk)==HOST_PAD_CROSS && hk==0);
    CHECK(map(&m,9,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_TOGGLE);
    CHECK(HostPadSDL_RawMapParse(&m,"9,9,2,3,4,5,-1,-1,6,7")==0);
    CHECK(map(&m,9,&hk)==(HOST_PAD_CROSS|HOST_PAD_CIRCLE) && hk==0);
    CHECK(HostPadSDL_RawMapParse(&m,"0,1,2,3,4,5,-1,-1,6,7")==0);
    CHECK(map(&m,9,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_TOGGLE);
    CHECK(map(&m,10,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_HOLD);
    CHECK(HostPadSDL_RawMapParse(&m,"invalid")!=0);
    CHECK(map(&m,9,&hk)==0 && hk==HOST_PAD_HOTKEY_FF_TOGGLE);
    puts("synthetic SDL raw-map hotkey collision controls pass");return 0;
}
