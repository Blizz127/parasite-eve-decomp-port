/* Production scan-out with a mocked DISPENV and present notification boundary. */
#include "pe_gpu.h"
#include "host_framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do{if(!(e)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#e);exit(1);}}while(0)
static uint8_t env[32];static int presents,hooks;
uint8_t PE_LoadU8(pe_addr_t a){CHECK(a<sizeof env);return env[a];}
uint16_t PE_LoadU16(pe_addr_t a){CHECK(a+1<sizeof env);return env[a]|((uint16_t)env[a+1]<<8);}
int PE_Port_FramePresentationAllowed(void){return 1;}
void PE_Port_FramePresented(int n){presents=n;}
void PE_Port_InvokePresentHook(void){hooks++;}
static void half(unsigned i,unsigned v){env[i]=v;env[i+1]=v>>8;}
static void scan(int rgb24){memset(env,0,sizeof env);half(4,320);half(14,240);env[17]=rgb24;HostFB_PresentDispEnv(0);}
static void gp(uint32_t w){CHECK(PE_GPU_WriteGP0(w));}
int main(void)
{
    uint8_t native[320*240*3];
    for(unsigned scale=1;scale<=4;scale++){
        PE_GPU_SetRenderScale(scale);PE_GPU_SetTextureFilter(0);PE_GPU_Init();HostFB_Init();HostFB_SetSSAA(0);
        HostFB_SetDispMask(1);presents=hooks=0;
        gp(0x200000F8u);gp((100u<<16)|100u);gp((110u<<16)|110u);gp((110u<<16)|100u);
        PeGpuState before,after;PE_GPU_GetState(&before);scan(0);memcpy(native,HostFB_GetPixels(),sizeof native);
        int w,h;const uint8_t *hi=HostFB_GetPresentationPixels(&w,&h);CHECK(w==320*(int)scale && h==240*(int)scale);
        size_t bytes=(size_t)w*h*3;uint8_t *reference=malloc(bytes);CHECK(reference);memcpy(reference,hi,bytes);
        HostFB_SetSSAA(1);CHECK(HostFB_GetSSAA()==1);CHECK(HostFB_GetPresentationPixels(&w,&h)==HostFB_GetPixels());
        scan(0);const uint8_t *aa=HostFB_GetPresentationPixels(&w,&h);CHECK(w==320 && h==240);
        CHECK(!memcmp(native,HostFB_GetPixels(),sizeof native));PE_GPU_GetState(&after);CHECK(!memcmp(&before,&after,sizeof before));
        CHECK(presents==2 && hooks==2);
        if(scale>1){
            CHECK(aa!=HostFB_GetPixels());
            for(unsigned y=0;y<240;y++)for(unsigned x=0;x<320;x++)for(unsigned c=0;c<3;c++){
                unsigned sum=0;
                for(unsigned sy=0;sy<scale;sy++)for(unsigned sx=0;sx<scale;sx++)
                    sum+=reference[((size_t)(y*scale+sy)*320*scale+x*scale+sx)*3+c];
                CHECK(aa[((size_t)y*320+x)*3+c]==(sum+scale*scale/2)/(scale*scale));
            }
            /* Native excluded diagonal pixel gains fractional red coverage. */
            unsigned at=(101*320+101)*3;CHECK(native[at]==0);
            CHECK(aa[at]==(255*scale*(scale-1)/2+scale*scale/2)/(scale*scale));CHECK(aa[at]>0 && aa[at]<255);
        }else CHECK(aa==HostFB_GetPixels());
        free(reference);
        /* Packed movie RGB remains byte-exact with SSAA on. */
        gp(0xA0000000u);gp(0);gp((1u<<16)|2u);gp(0x00332211u);scan(1);
        aa=HostFB_GetPresentationPixels(&w,&h);CHECK(!memcmp(aa,HostFB_GetPixels(),sizeof native));
        CHECK(aa[0]==0x11 && aa[1]==0x22 && aa[2]==0x33);
        HostFB_SetDispMask(0);scan(0);aa=HostFB_GetPresentationPixels(&w,&h);
        for(unsigned i=0;i<sizeof native;i++)CHECK(aa[i]==0);CHECK(presents==4 && hooks==4);
    }
    HostFB_SetSSAA(0);PE_GPU_SetRenderScale(1);
    puts("framebuffer SSAA: scales1..4 box averages, fractional edge, native/movie parity, blanking and counters PASS");return 0;
}
