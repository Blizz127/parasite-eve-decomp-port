/* Synthetic host RGB clipping checks. No VRAM, disc, device or game. */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "host_framebuffer.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"check failed at line %d\n",__LINE__); return 1; } } while(0)
static int rgb_is(const uint8_t *p,unsigned x,unsigned y,uint8_t r,uint8_t g,uint8_t b) {
    const uint8_t *px=p+(y*PE_PORT_FB_WIDTH+x)*3u;return px[0]==r && px[1]==g && px[2]==b;
}
int main(int argc,char **argv) {
    HostFB_Init();
    if(argc==2 && strcmp(argv[1],"positive")==0) {
        HostFB_ClearImage(INT_MAX-1,0,INT_MAX,1,1,2,3);return 0;
    }
    if(argc==2 && strcmp(argv[1],"negative")==0) {
        HostFB_ClearImage(INT_MIN,0,-1,1,1,2,3);return 0;
    }
    const uint8_t *fb=HostFB_GetPixels();
    HostFB_ClearImage(INT_MAX-1,0,INT_MAX,1,1,2,3);
    HostFB_ClearImage(INT_MIN,0,-1,1,1,2,3);
    HostFB_ClearImage(0,INT_MAX-1,1,INT_MAX,1,2,3);
    HostFB_ClearImage(0,INT_MIN,1,-1,1,2,3);
    HostFB_ClearImage(INT_MIN,0,INT_MAX,1,1,2,3);
    HostFB_ClearImage(0,INT_MIN,1,INT_MAX,1,2,3);
    for(unsigned y=0;y<PE_PORT_FB_HEIGHT;y++) for(unsigned x=0;x<PE_PORT_FB_WIDTH;x++)
        CHECK(rgb_is(fb,x,y,0,0,0));
    HostFB_ClearImage(-2,10,3,1,1,2,3);
    CHECK(rgb_is(fb,0,10,1,2,3) && rgb_is(fb,1,10,0,0,0) && rgb_is(fb,0,9,0,0,0));
    HostFB_ClearImage(0,0,INT_MAX,1,4,5,6);
    for(unsigned x=0;x<PE_PORT_FB_WIDTH;x++) CHECK(rgb_is(fb,x,0,4,5,6));
    CHECK(rgb_is(fb,0,1,0,0,0));
    HostFB_ClearImage(5,-2,1,3,7,8,9);
    CHECK(rgb_is(fb,5,0,7,8,9) && rgb_is(fb,5,1,0,0,0));
    HostFB_ClearImage(2,0,1,INT_MAX,10,11,12);
    for(unsigned y=0;y<PE_PORT_FB_HEIGHT;y++) CHECK(rgb_is(fb,2,y,10,11,12));
    HostFB_ClearImage(2,0,-1,1,30,30,30);
    HostFB_ClearImage(2,0,1,-1,30,30,30);
    CHECK(rgb_is(fb,2,0,10,11,12));
    puts("synthetic host framebuffer extreme clipping controls pass");return 0;
}
