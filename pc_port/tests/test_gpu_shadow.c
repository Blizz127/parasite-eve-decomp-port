/* Focused presentation surface test. Can link pe_gpu.c alone with function
 * sections / linker garbage collection; uses no guest, audio or device calls. */
#include "pe_gpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* pe_gpu.c raises GP0(1Fh) IRQ1 through the interrupt controller (96c69bd62);
 * this standalone GPU test links no controller. */
void PE_IRQ_AssertSources(uint16_t sources) { (void)sources; }

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); } } while (0)
static void words(const uint32_t *w, size_t n)
{ for (size_t i = 0; i < n; i++) CHECK(PE_GPU_WriteGP0(w[i])); }
#define PACKET(...) do { const uint32_t w[] = {__VA_ARGS__}; words(w, sizeof(w)/sizeof(w[0])); } while (0)
#define UV(u,v) ((uint32_t)(u) | ((uint32_t)(v) << 8))
#define XY(x,y) ((uint32_t)(x) | ((uint32_t)(y) << 16))
static uint16_t native(unsigned x, unsigned y)
{ uint16_t p; CHECK(PE_GPU_ReadVRAM(x,y,&p)); return p; }
static uint16_t shadow(unsigned x, unsigned y)
{ uint16_t p; CHECK(PE_GPU_ReadRenderPixel(x,y,&p)); return p; }
static void cell(unsigned x, unsigned y, unsigned scale, uint16_t value)
{ for (unsigned sy=0;sy<scale;sy++) for (unsigned sx=0;sx<scale;sx++) CHECK(shadow(x*scale+sx,y*scale+sy)==value); }
static void upload(unsigned x, unsigned y, uint16_t a, uint16_t b)
{ PACKET(0xA0000000u, XY(x,y), XY(2,1), a | ((uint32_t)b<<16)); }
static void fixture(void)
{
    /* Whole VRAM fill includes both game draw/display buffers and textures. */
    PACKET(0x02080808u, XY(0,0), XY(1023,511));
    upload(0,400,0x1111,0x2222); /* 4-bit */
    upload(64,400,0x0101,0x0202); /* 8-bit */
    upload(128,400,0x801F,0x03E0); /* direct, STP then opaque */
    upload(16,450,0,0x801F); upload(18,450,0x03E0,0x7C00);
    PACKET(0xE3000000u, 0xE407FFFFu, 0xE5000000u, 0xE6000000u);
    for (unsigned depth=0;depth<3;depth++) {
        uint32_t mode=0x10u|depth*0x80u|depth;
        PACKET(0xE1000000u|mode);
        /* 4-/8-/15-bit rectangles, raw/modulated, STP and texture flips. */
        PACKET(0x65808080u, XY(20+depth*20,20), (0x7081u<<16)|UV(0,144), XY(8,8));
        PACKET(0x67808080u, XY(20+depth*20,24), (0x7081u<<16)|UV(0,144), XY(8,8));
        PACKET(0xE1000000u|mode|0x3000u);
        PACKET(0x64808080u, XY(20+depth*20,32), (0x7081u<<16)|UV(1,144), XY(2,2));
        /* Textured triangle and quad with embedded tpage. */
        PACKET(0x24808080u, XY(80+depth*20,20), (0x7081u<<16)|UV(0,144),
               XY(90+depth*20,20), (mode<<16)|UV(1,144), XY(80+depth*20,30), UV(0,144));
        PACKET(0x2F808080u, XY(80+depth*20,32), (0x7081u<<16)|UV(0,144),
               XY(88+depth*20,32), (mode<<16)|UV(1,144), XY(80+depth*20,40), UV(0,144), XY(88+depth*20,40), UV(1,144));
    }
    for (unsigned abr=0;abr<4;abr++) {
        PACKET(0xE1000200u|(abr<<5));
        PACKET(0x60A08040u, XY(150,20+abr*20), XY(16,16));
        PACKET(0x62808080u, XY(150,20+abr*20), XY(16,16));
        PACKET(0x32808080u, XY(180,20+abr*20), 0x0000F800u, XY(195,20+abr*20), 0x00F80000u, XY(180,35+abr*20));
        PACKET(0x52808080u, XY(210,20+abr*20), 0x00F80000u, XY(225,35+abr*20));
    }
    PACKET(0xE6000001u, 0x780000F8u, XY(240,30));
    PACKET(0xE6000002u, 0x7800F800u, XY(240,30));
    PACKET(0xE6000000u, 0x48F80000u, XY(240,60), XY(250,65), XY(260,60), 0x50005000u);
    /* Offset, inclusive clip, fixed rectangles, lower double buffer. */
    PACKET(0xE3000000u| (280u<<10)|280u, 0xE4000000u|(285u<<10)|285u,
           0xE5000000u|(280u<<11)|280u, 0x680000F8u, XY(0,0), 0x70808080u, XY(2,2));
    PACKET(0xE3000000u, 0xE407FFFFu, 0xE5000000u);
    PACKET(0x280000F8u, XY(100,256), XY(110,256), XY(100,266), XY(110,266));
    CHECK(PE_GPU_WriteGP1(0x04000002u));
    CHECK(PE_GPU_MoveImage(XY(100,256),XY(1020,510),XY(8,4)));
    CHECK(PE_GPU_MoveImage(XY(1020,510),XY(500,300),XY(8,4)));
    /* Native overlapping-copy semantics preserved, shadow invalidated. */
    CHECK(PE_GPU_MoveImage(XY(500,300),XY(501,300),XY(7,4)));
    CHECK(PE_GPU_WriteGP1(0x04000000u));
    PACKET(0xC0000000u, XY(500,300), XY(2,1));
    uint32_t out; CHECK(PE_GPU_ReadGP0(&out));
    CHECK(out == ((uint32_t)native(500,300)|((uint32_t)native(501,300)<<16)));
}
/* 2D sprites must remain exact nearest-expanded native cells, including
 * transparent index zero, post-CLUT STP, masks, flips and texture window. */
static void rectangles(unsigned scale)
{
    PE_GPU_Init();
    upload(0,400,0x0210,0x0012);
    upload(64,400,0x0100,0x0002);
    upload(128,400,0,0x801F); upload(130,400,0x03E0,0x7FFF);
    upload(16,450,0,0x801F); upload(18,450,0x03E0,0x7C00);
    PACKET(0xE3000000u,0xE407FFFFu,0xE5000000u,0xE6000000u,0xE2000000u);
    for(unsigned depth=0;depth<3;depth++) {
        unsigned x=20+depth*20;
        uint32_t mode=0x10u|(depth<<7)|depth;
        PACKET(0xE1000000u|mode,0x60F80000u,XY(x,20),XY(8,8));
        PACKET(0x67808080u,XY(x,20),(0x7081u<<16)|UV(0,144),XY(4,1));
        CHECK(native(x,20)==0x7C00u); /* transparent source keeps blue */
        CHECK(native(x+1,20)==0xBC0Fu); /* STP blend, ABR0, texel mask retained */
        PACKET(0xE6000002u,0x64804080u,XY(x,20),(0x7081u<<16)|UV(0,144),XY(4,1));
        CHECK(native(x+1,20)==0xBC0Fu); /* mask-check blocks masked sprite cell */
        PACKET(0xE6000001u,0x74808080u,XY(x,32),(0x7081u<<16)|UV(0,144));
        PACKET(0xE6000000u,0xE1000000u|mode|0x3000u,0x64808080u,XY(x,44),(0x7081u<<16)|UV(3,144),XY(4,2));
        PACKET(0xE2000021u,0x65808080u,XY(x,48),(0x7081u<<16)|UV(8,144),XY(4,2));
        PACKET(0xE2000000u);
    }
    /* Every subpixel in the entire display area checks sprite sampling and
     * blending, not merely a few selected pixels. */
    for(unsigned y=0;y<240;y++) for(unsigned x=0;x<320;x++) cell(x,y,scale,native(x,y));
}

int main(void)
{
    uint16_t *reference=malloc(PE_GPU_VRAM_PIXELS*sizeof(*reference)); CHECK(reference);
    PeGpuState baseline, actual;
    for (unsigned scale=1;scale<=4;scale++) {
        PE_GPU_SetRenderScale(scale); PE_GPU_Init(); CHECK(PE_GPU_GetRenderScale()==(int)scale);
        fixture(); PE_GPU_GetState(&actual);
        if (scale==1) baseline=actual; else CHECK(memcmp(&baseline,&actual,sizeof actual)==0);
        for (unsigned y=0;y<512;y++) for (unsigned x=0;x<1024;x++) {
            uint16_t value=native(x,y);
            if (scale==1) reference[y*1024+x]=value; else CHECK(value==reference[y*1024+x]);
        }
        cell(240,30,scale,0x801F); /* set/check mask */
        cell(500,300,scale,native(500,300)); /* copy resync */
        upload(1023,511,0x1234,0x5678); /* wrapped upload */
        cell(1023,511,scale,0x1234); cell(0,511,scale,0x5678);
        PACKET(0x02000000u,XY(96,96),XY(32,32));
        PACKET(0x200000F8u,XY(100,100),XY(110,110),XY(100,110));
        CHECK(native(101,101)==0);
        if (scale>1) CHECK(shadow(101*scale,101*scale+1)==0x1F); /* subpixel edge coverage */
        CHECK(native(101,101)==0); /* shadow never writes back */
        PE_GPU_Reset(); CHECK(shadow(1023*scale,511*scale)==0x1234);
        uint16_t p; CHECK(!PE_GPU_ReadRenderPixel(1024*scale,0,&p)); CHECK(!PE_GPU_ReadRenderPixel(0,512*scale,&p));
        PE_GPU_Init(); cell(1023,511,scale,0);
        rectangles(scale);
    }
    upload(10,10,0x7C00,0x03E0); PE_GPU_SetRenderScale(3); cell(10,10,3,0x7C00);
    PE_GPU_SetRenderScale(0); CHECK(PE_GPU_GetRenderScale()==1); CHECK(shadow(10,10)==0x7C00);
    free(reference); puts("gpu shadow: scales 1..4 native VRAM/state parity, coverage, transfers, masks, lifecycle PASS");
    return 0;
}
