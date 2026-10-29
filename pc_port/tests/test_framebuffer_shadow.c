/* Standalone scan-out test with only the DISPENV and present-hook boundary
 * mocked; GPU rasterization and both framebuffer copies are production code. */
#include "pe_gpu.h"
#include "host_framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1); } } while(0)
static uint8_t env[32];
static int presents, hooks;
uint8_t PE_LoadU8(pe_addr_t a) { CHECK(a < sizeof env); return env[a]; }
uint16_t PE_LoadU16(pe_addr_t a) { CHECK(a+1 < sizeof env); return env[a]|((uint16_t)env[a+1]<<8); }
int PE_Port_FramePresentationAllowed(void) { return 1; }
void PE_Port_FramePresented(int n) { presents=n; }
void PE_Port_InvokePresentHook(void) { hooks++; }
static void half(unsigned at,unsigned v) { env[at]=v;env[at+1]=v>>8; }
static void gp(uint32_t w) { CHECK(PE_GPU_WriteGP0(w)); }
static void scan(unsigned x,unsigned y,unsigned width,unsigned top,unsigned height,int rgb24)
{ memset(env,0,sizeof env);half(0,x);half(2,y);half(4,width);half(10,top);half(14,height);env[17]=rgb24;HostFB_PresentDispEnv(0); }
static void pixel(const uint8_t *p, unsigned width,unsigned x,unsigned y,unsigned r,unsigned g,unsigned b)
{ p+=(y*width+x)*3; CHECK(p[0]==r && p[1]==g && p[2]==b); }
int main(void)
{
    uint8_t baseline[320*240*3];
    char path[4096]; const char *dir=getenv("TMPDIR"); if(!dir || !*dir) dir=".";
    CHECK(snprintf(path,sizeof path,"%s/pe-framebuffer-shadow-XXXXXX",dir)<(int)sizeof path);
    int fd=mkstemp(path); CHECK(fd>=0); CHECK(close(fd)==0);
    for (unsigned scale=1;scale<=4;scale++) {
        PE_GPU_SetRenderScale(scale);PE_GPU_Init();HostFB_Init();presents=hooks=0;
        HostFB_SetDispMask(1);
        gp(0xA0000000);gp((256u<<16)|10u);gp((2u<<16)|2u);gp(0x03E0001Fu);gp(0x7FFF7C00u);
        scan(10,256,2,3,2,0);
        CHECK(presents==1 && hooks==1);
        if(scale==1) memcpy(baseline,HostFB_GetPixels(),sizeof baseline);
        else CHECK(!memcmp(baseline,HostFB_GetPixels(),sizeof baseline));
        int w,h;const uint8_t *hi=HostFB_GetPresentationPixels(&w,&h);CHECK(w==320*(int)scale && h==240*(int)scale);
        if(scale==1) CHECK(hi==HostFB_GetPixels()); else CHECK(hi!=HostFB_GetPixels());
        for(unsigned sy=0;sy<scale;sy++)for(unsigned sx=0;sx<scale;sx++) {
            pixel(hi,w,sx,3*scale+sy,255,0,0);pixel(hi,w,scale+sx,3*scale+sy,0,255,0);
            pixel(hi,w,sx,4*scale+sy,0,0,255);pixel(hi,w,scale+sx,4*scale+sy,255,255,255);
        }
        pixel(hi,w,0,0,0,0,0);pixel(hi,w,2*scale,3*scale,0,0,0);
        /* Display mask and geometry changes black out former scanlines. */
        HostFB_SetDispMask(0);scan(10,256,2,3,2,0);hi=HostFB_GetPresentationPixels(&w,&h);pixel(hi,w,0,3*scale,0,0,0);
        HostFB_SetDispMask(1);scan(10,256,1,0,1,0);hi=HostFB_GetPresentationPixels(&w,&h);pixel(hi,w,0,0,255,0,0);pixel(hi,w,0,3*scale,0,0,0);
        /* 24-bit bytes straddle VRAM words and wrap x, retain exact RGB. */
        gp(0xA0000000);gp(1023);gp((1u<<16)|2u);gp(0x00332211u);
        scan(1023,0,1,0,1,1);hi=HostFB_GetPresentationPixels(&w,&h);pixel(HostFB_GetPixels(),320,0,0,0x11,0x22,0x33);
        /* A movie frame presents the native 320x240 frame itself. */
        CHECK(hi==HostFB_GetPixels() && w==320 && h==240);pixel(hi,w,0,0,0x11,0x22,0x33);
        /* Export is presentation-sized and leaves native fb/counters intact. */
        CHECK(HostFB_WritePresentationPPM(path)==0);
        FILE *f=fopen(path,"rb");CHECK(f);int fw,fh;CHECK(fscanf(f,"P6\n%d %d\n255\n",&fw,&fh)==2);CHECK(fw==w && fh==h);CHECK(fclose(f)==0);
        CHECK(presents==4 && hooks==4);
    }
    /* Returning to the same scale without a rescan must reject the stale HD buffer. */
    PE_GPU_SetRenderScale(2); scan(10,256,1,0,1,0);
    PE_GPU_SetRenderScale(3); PE_GPU_SetRenderScale(2);
    int tw,th; CHECK(HostFB_GetPresentationPixels(&tw,&th)==HostFB_GetPixels()); CHECK(tw==320 && th==240);
    PE_GPU_SetRenderScale(1);int w,h;CHECK(HostFB_GetPresentationPixels(&w,&h)==HostFB_GetPixels());CHECK(w==320&&h==240);
    CHECK(remove(path)==0);
    puts("framebuffer shadow: scale1..4 native parity, second buffer, display/movies/export PASS");return 0;
}
