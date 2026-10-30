/* Shadow dithering modes: production GPU, no guest/device mocks needed. */
#include "pe_gpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pe_gpu.c raises GP0(1Fh) IRQ1 through the interrupt controller (96c69bd62);
 * this standalone GPU test links no controller. */
void PE_IRQ_AssertSources(uint16_t sources) { (void)sources; }
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
#define XY(x,y) ((uint32_t)(x)|((uint32_t)(y)<<16))
static void gp(uint32_t w){CHECK(PE_GPU_WriteGP0(w));}
#define PACKET(...) do{const uint32_t w[]={__VA_ARGS__};for(size_t i=0;i<sizeof(w)/sizeof(w[0]);i++)gp(w[i]);}while(0)
static uint16_t native(unsigned x,unsigned y){uint16_t p;CHECK(PE_GPU_ReadVRAM(x,y,&p));return p;}
static uint16_t shadow(unsigned x,unsigned y){uint16_t p;CHECK(PE_GPU_ReadRenderPixel(x,y,&p));return p;}
static const int D[4][4]={{-4,0,-3,1},{2,-2,3,-1},{-3,1,-4,0},{3,-1,2,-2}};
static uint16_t grey(int v)
{
    unsigned c=(unsigned)(v<0?0:v>255?255:v)>>3;
    return (uint16_t)(c|(c<<5)|(c<<10));
}
/* Dithering on (E1 bit 9). A gouraud triangle and a gouraud line whose
 * vertices are all 0x40 grey: dithering alternates 7 and 8 per channel. */
static void scene(void)
{
    PE_GPU_Init();
    PACKET(0xE1000200u,0xE3000000u,0xE407FFFFu,0xE5000000u,0xE6000000u);
    PACKET(0x30404040u,XY(10,10),0x00404040u,XY(60,10),0x00404040u,XY(10,60));
    PACKET(0x50404040u,XY(100,20),0x00404040u,XY(140,20));
}
static void snapshot(uint16_t *p){for(unsigned y=0;y<512;y++)for(unsigned x=0;x<1024;x++)p[y*1024+x]=native(x,y);}
int main(void)
{
    uint16_t *base=malloc(PE_GPU_VRAM_PIXELS*sizeof(*base));CHECK(base);
    CHECK(PE_GPU_GetShadowDither()==0);
    PE_GPU_SetShadowDither(7);CHECK(PE_GPU_GetShadowDither()==0);
    for(unsigned scale=2;scale<=4;scale++){
        PE_GPU_SetRenderScale(scale);CHECK(PE_GPU_GetRenderScale()==(int)scale);
        for(int mode=0;mode<3;mode++){
            PE_GPU_SetShadowDither(mode);CHECK(PE_GPU_GetShadowDither()==mode);
            scene();
            if(!mode){
                snapshot(base);
                CHECK(native(12,12)!=native(13,12)); /* the native fixture really dithers */
            }else
                for(unsigned y=0;y<512;y++)for(unsigned x=0;x<1024;x++)CHECK(base[y*1024+x]==native(x,y));
            for(unsigned y=12*scale;y<30*scale;y++)for(unsigned x=12*scale;x<30*scale;x++){
                uint16_t want=mode==2?grey(0x40):mode==1?grey(0x40+D[y&3][x&3]):native(x/scale,y/scale);
                CHECK(shadow(x,y)==want);
            }
            for(unsigned y=20*scale;y<21*scale;y++)for(unsigned x=101*scale;x<139*scale;x++){
                uint16_t want=mode==2?grey(0x40):mode==1?grey(0x40+D[y&3][x&3]):native(x/scale,20);
                CHECK(shadow(x,y)==want);
            }
        }
    }
    PE_GPU_SetShadowDither(0);PE_GPU_SetRenderScale(1);free(base);
    puts("gpu dithering: native VRAM identical in every mode; retail magnified, fine at shadow resolution, off flat PASS");
    return 0;
}
