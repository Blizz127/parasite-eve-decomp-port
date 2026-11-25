/* Pure synthetic controller mapping parser; no SDL initialization or device. */
#include <stdio.h>
#include <string.h>
#include "host_pad_sdl.h"
#include "host_pad.h"
int main(void)
{
    HostPadRawMap map, old;
    uint8_t btn[16] = {0}, hotkeys = 0;
    int16_t axis[2] = {0};
    HostPadSDL_RawMapDefaults(&map);
    old = map;
    if (HostPadSDL_RawMapParse(&map,"2,1,3,0,6,7,4,5,8,9,10") != -1 ||
        memcmp(&map,&old,sizeof map)) {
        fprintf(stderr,"extra field accepted or altered map\n"); return 1;
    }
    if (HostPadSDL_RawMapParse(&map,"2,1,3,0,6,7,4,5,8,9,") != -1 ||
        memcmp(&map,&old,sizeof map)) return 2;
    if (HostPadSDL_RawMapParse(&map,"2,1,3,0,6,7,4,5,8,9garbage") != -1 ||
        memcmp(&map,&old,sizeof map)) return 3;
    if (HostPadSDL_RawMapParse(&map,"2,1,3,0,6,7,4,5,8,9") != 0) return 4;
    btn[2] = 1; btn[4] = 1;
    if (HostPadSDL_MapJoystick(&map,btn,16,axis,2,0,0,&hotkeys) !=
        (HOST_PAD_CROSS|HOST_PAD_L2) || hotkeys) return 5;
    puts("malformed tails rejected atomically; valid joystick mapping passes");
    return 0;
}
