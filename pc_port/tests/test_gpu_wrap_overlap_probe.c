/* Isolated hypothesis probe: compares wrap/nonwrap overlap in the native
 * internal GPU copy. This is NOT an original-hardware oracle. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_gpu.c"

static void probe(unsigned x, uint16_t out[4])
{
    memset(g_gpu.vram,0,sizeof g_gpu.vram);
    g_gpu.state.mask_setting=0;
    for (unsigned i=0;i<4;i++) g_gpu.vram[(x+i)&1023u]=(uint16_t)(i+1);
    if (!GpuVramCopy(x,(x+1)&1023u,(1u<<16)|4u)) abort();
    for (unsigned i=0;i<4;i++) out[i]=g_gpu.vram[(x+1+i)&1023u];
}
int main(void)
{
    uint16_t a[4],b[4];
    static const uint16_t wanted[4]={1,2,3,4};
    probe(10,a); probe(1022,b);
    if(memcmp(a,wanted,sizeof a)) { fputs("nonwrapped control failed\n",stderr); return 2; }
    printf("nonwrap=%u,%u,%u,%u wrap=%u,%u,%u,%u\n",a[0],a[1],a[2],a[3],b[0],b[1],b[2],b[3]);
    if(memcmp(a,b,sizeof a)) { fputs("wrap overlap differs; hardware semantics unresolved\n",stderr); return 1; }
    return 0;
}
