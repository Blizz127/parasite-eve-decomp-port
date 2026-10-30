/* Shadow polygon filtering: production GPU, no guest/device mocks needed. */
#include "pe_gpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pe_gpu.c raises GP0(1Fh) IRQ1 through the interrupt controller (96c69bd62);
 * this standalone GPU test links no controller. */
void PE_IRQ_AssertSources(uint16_t sources) { (void)sources; }
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
#define XY(x,y) ((uint32_t)(x)|((uint32_t)(y)<<16))
#define UV(u,v) ((uint32_t)(u)|((uint32_t)(v)<<8))
static void gp(uint32_t w){CHECK(PE_GPU_WriteGP0(w));}
#define PACKET(...) do{const uint32_t w[]={__VA_ARGS__};for(size_t i=0;i<sizeof(w)/sizeof(w[0]);i++)gp(w[i]);}while(0)
static uint16_t native(unsigned x,unsigned y){uint16_t p;CHECK(PE_GPU_ReadVRAM(x,y,&p));return p;}
static uint16_t shadow(unsigned x,unsigned y){uint16_t p;CHECK(PE_GPU_ReadRenderPixel(x,y,&p));return p;}
static void upload(unsigned x,unsigned y,uint16_t a,uint16_t b){PACKET(0xA0000000u,XY(x,y),XY(2,1),a|((uint32_t)b<<16));}
static void quad(unsigned x,unsigned y,unsigned depth,unsigned u1,uint32_t command)
{
    uint32_t mode=depth|(depth<<7);
    PACKET(command,XY(x,y),(0x1901u<<16)|UV(0,0),XY(x+12,y),(mode<<16)|UV(u1,0),
           XY(x,y+8),UV(0,1),XY(x+12,y+8),UV(u1,1));
}
static void fixture(unsigned depth,uint16_t c0,uint16_t c1,uint16_t c2,uint16_t c3)
{
    PE_GPU_Init();
    for(unsigned row=0;row<2;row++){
        upload(0,row,0x3210,0x3210);
        upload(64,row,0x0100,0x0302);
        upload(128,row,c0,c1);upload(130,row,c2,c3);
    }
    upload(16,100,c0,c1);upload(18,100,c2,c3);
    PACKET(0xE3000000u,0xE407FFFFu,0xE5000000u,0xE6000000u);
    PACKET(0x60F80000u,XY(100,100),XY(100,24));
    quad(100,100,depth,3,0x2D808080u);
    /* Rectangles remain nearest even with polygon filtering on. */
    PACKET(0xE1000000u|depth|(depth<<7),0x65808080u,XY(150,100),(0x1901u<<16)|UV(0,0),XY(4,2));
}
static void snapshot(uint16_t *p){for(unsigned y=0;y<512;y++)for(unsigned x=0;x<1024;x++)p[y*1024+x]=native(x,y);}
int main(void)
{
    uint16_t *base=malloc(PE_GPU_VRAM_PIXELS*sizeof(*base));CHECK(base);
    for(unsigned scale=1;scale<=4;scale++)for(unsigned depth=0;depth<3;depth++){
        PE_GPU_SetRenderScale(scale);PE_GPU_SetTextureFilter(0);fixture(depth,0x001F,0x03E0,0,0xFC00);
        snapshot(base);PeGpuState before,after;PE_GPU_GetState(&before);
        PE_GPU_SetTextureFilter(1);CHECK(PE_GPU_GetTextureFilter()==1);fixture(depth,0x001F,0x03E0,0,0xFC00);
        PE_GPU_GetState(&after);CHECK(!memcmp(&before,&after,sizeof before));
        for(unsigned y=0;y<512;y++)for(unsigned x=0;x<1024;x++)CHECK(base[y*1024+x]==native(x,y));
        if(scale>1){
            CHECK(shadow(102*scale,101*scale)==0x0210); /* equal opaque red/green weights */
            CHECK(shadow(105*scale,101*scale)==0x03E0); /* transparent tap cannot darken green */
            CHECK(shadow(106*scale,101*scale)==0x7C00); /* nearest transparent retains blue background */
            CHECK(shadow(111*scale,101*scale)==0xFC00); /* excludes zero taps, preserves STP */
            for(unsigned sy=0;sy<scale;sy++)for(unsigned sx=0;sx<scale;sx++)
                CHECK(shadow(150*scale+sx,100*scale+sy)==0x001F); /* sprite nearest */
            /* Opaque red and STP blue must never filter into one another. */
            fixture(depth,0x001F,0xFC00,0xFC00,0xFC00);
            CHECK(shadow(101*scale,101*scale)==0x001F);
            CHECK(shadow(102*scale,101*scale)==0xFC00);
            PACKET(0xE1000000u,0x60F80000u,XY(100,100),XY(12,8));
            quad(100,100,depth,3,0x2F808080u);
            CHECK(shadow(101*scale,101*scale)==0x001F); /* opaque class does not blend */
            CHECK(shadow(102*scale,101*scale)==0xFC00); /* blue+blue ABR0=(31+31)/2=31 (carry kept, 0c6d72e09) */
            /* Fully masked destination is protected before sampling. */
            PACKET(0xE6000001u,0x600000F8u,XY(100,100),XY(12,8),0xE6000002u);
            quad(100,100,depth,3,0x2F808080u);
            for(unsigned y=100*scale;y<108*scale;y++)for(unsigned x=100*scale;x<112*scale;x++)CHECK(shadow(x,y)==0x801F);
        }else CHECK(shadow(102,101)==native(102,101));
    }
    PE_GPU_SetTextureFilter(0);PE_GPU_SetRenderScale(1);free(base);
    puts("gpu texture filter: scales/depths native parity, interpolation, zero/STP coverage, masks, sprite nearest PASS");return 0;
}
