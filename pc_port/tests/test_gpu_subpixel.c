/* Sub-pixel geometry on the shadow surface: production GPU with a fake GTE
 * sidecar, no guest/device mocks needed.
 *  - native VRAM is identical with the lookup on and off;
 *  - a polygon whose every vertex word has a fraction is drawn at that
 *    fraction on the shadow (here +0.5 px in x, +0.25 px in y);
 *  - a polygon with one vertex missing keeps the integer shadow;
 *  - the drawing offset still applies, and scale 1 never consults it. */
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

/* Fraction 0x80 in x (half a pixel) and 0x40 in y (a quarter) for the
 * listed vertex words; every other word is unknown. */
static const uint32_t k_known[] = {XY(10,10),XY(40,12),XY(14,50),XY(200,20),XY(230,22),XY(204,60)};
static unsigned lookups;
static int fake_lookup(uint32_t xy, uint32_t *frac)
{
    lookups++;
    for (size_t i = 0; i < sizeof k_known / sizeof k_known[0]; i++)
        if (k_known[i] == xy) { *frac = 0x80u | (0x40u << 8); return 1; }
    return 0;
}

/* Flat red triangle (all vertices known), flat green triangle with one
 * vertex unknown (XY(208,58)), and a gouraud quad drawn through the
 * drawing offset whose vertices are known. */
static void scene(void)
{
    PE_GPU_Init();
    PACKET(0xE1000000u,0xE3000000u,0xE407FFFFu,0xE5000000u,0xE6000000u);
    PACKET(0x200000F8u,XY(10,10),XY(40,12),XY(14,50));
    PACKET(0x2000F800u,XY(200,20),XY(230,22),XY(208,58));
    PACKET(0xE5000000u|(300u<<11)|100u);
    PACKET(0x38F80000u,XY(10,10),0x0000F800u,XY(40,12),0x000000F8u,XY(14,50),0x00F8F8F8u,XY(200,20));
}
static uint16_t *grab(unsigned w,unsigned h,int render)
{
    uint16_t *p=malloc((size_t)w*h*sizeof(*p));CHECK(p);
    for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++)p[(size_t)y*w+x]=render?shadow(x,y):native(x,y);
    return p;
}
int main(void)
{
    /* Scale 1: the lookup is never consulted and nothing changes. */
    PE_GPU_SetRenderScale(1);
    PE_GPU_SetSubpixelLookup(fake_lookup);
    scene();
    CHECK(lookups == 0);
    PE_GPU_SetSubpixelLookup(NULL);
    for (unsigned scale = 2; scale <= 4; scale++) {
        const unsigned W = 1024u * scale, H = 512u * scale;
        const unsigned dx = (0x80u * scale + 128u) >> 8, dy = (0x40u * scale + 128u) >> 8;
        PE_GPU_SetRenderScale(scale); CHECK(PE_GPU_GetRenderScale() == (int)scale);
        PE_GPU_SetSubpixelLookup(NULL); scene();
        uint16_t *vram_off = grab(1024, 512, 0), *off = grab(W, H, 1);
        PE_GPU_SetSubpixelLookup(fake_lookup); lookups = 0; scene();
        CHECK(lookups > 0);
        uint16_t *vram_on = grab(1024, 512, 0), *on = grab(W, H, 1);
        CHECK(memcmp(vram_off, vram_on, 1024u * 512u * sizeof(*vram_on)) == 0);   /* native VRAM untouched */
        CHECK(dx >= 1 && dx < scale && dy < scale);
        /* Red triangle and the offset quad: the whole shape moves by
         * (dx, dy) shadow pixels; compare a box inside each. */
        for (unsigned y = 12 * scale; y < 40 * scale; y++)
            for (unsigned x = 12 * scale; x < 30 * scale; x++)
                CHECK(on[(size_t)(y + dy) * W + x + dx] == off[(size_t)y * W + x]);
        for (unsigned y = 112 * scale; y < 140 * scale; y++)
            for (unsigned x = 312 * scale; x < 330 * scale; x++)
                CHECK(on[(size_t)(y + dy) * W + x + dx] == off[(size_t)y * W + x]);
        {   /* ...and really moved: the left edge column of the red triangle. */
            int moved = 0;
            for (unsigned y = 20 * scale; y < 30 * scale && !moved; y++)
                for (unsigned x = 10 * scale; x < 20 * scale; x++)
                    if (on[(size_t)y * W + x] != off[(size_t)y * W + x]) { moved = 1; break; }
            CHECK(moved);
        }
        /* Green triangle has one unknown vertex: identical integer shadow. */
        for (unsigned y = 18 * scale; y < 62 * scale; y++)
            for (unsigned x = 198 * scale; x < 234 * scale; x++)
                CHECK(on[(size_t)y * W + x] == off[(size_t)y * W + x]);
        free(vram_off); free(vram_on); free(off); free(on);
    }
    PE_GPU_SetSubpixelLookup(NULL); PE_GPU_SetRenderScale(1);
    puts("gpu sub-pixel: native VRAM identical; fully known polygons move by their fraction on the shadow, partly known ones keep the integer shadow PASS");
    return 0;
}
