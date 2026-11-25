/* Native CD DMA issuer; retain host stop boundaries and guest alias behavior. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "game_port.h"
extern void pe_plat_cd_stream_stop(const char *);
void pe_plat_cd_dma_issue_host(uint32_t channel, uint32_t address, uint32_t blocks,
                          uint32_t words, uint32_t control, uint32_t interrupt,
                          uint32_t unused)
{
    (void)unused;
    if(channel!=3u){pe_plat_cd_stream_stop("func_8007CEAC_channel");return;}
    if(PE_CdLoadU32(PE_CDREG_DMA3+8u)&0x01000000u){if(PE_Port_ShouldStop())return;pe_plat_cd_stream_stop("func_8007CEAC_busy");return;}
    uint32_t dpcr_ptr=PE_LoadU32(0x8009b344u);
    uint32_t ptr=PE_LoadU32(0x8009b348u);
    uint8_t enable=ptr==0x1f8010f4u ? (uint8_t)(PE_GPU_ReadDICR()>>16) : PE_CdLoadU8(ptr+2u);
    enable=(interrupt&255u)==1u ? (uint8_t)(enable|8u) : (uint8_t)(enable&~8u);
    if(ptr==0x1f8010f4u) PE_GPU_WriteDICR((PE_GPU_ReadDICR()&0x0000ffffu)|((uint32_t)enable<<16));
    else PE_CdStoreU8(ptr+2u,enable);
    ptr=PE_LoadU32(0x8009b348u);
    if(ptr==0x1f8010f4u) (void)PE_GPU_ReadDICR(); else (void)PE_CdLoadU32(ptr);
    ptr=dpcr_ptr;
    if(ptr==0x1f8010f0u) PE_GPU_WriteDPCR(PE_GPU_ReadDPCR()|0x8000u);
    else { uint32_t dpcr=PE_CdLoadU32(ptr); PE_CdStoreU32(ptr,dpcr|0x8000u); }
    PE_CdStoreU32(PE_CDREG_DMA3,address);
    PE_CdStoreU32(PE_CDREG_DMA3+4u,(blocks<<16)|words);
    ptr=PE_LoadU32(0x8009b32cu);
    if(!(PE_CdLoadU8(ptr)&0x40u)){pe_plat_cd_stream_stop("func_8007CEAC_data_wait");return;}
    PE_CdStoreU32(PE_CDREG_DMA3+8u,control);
    (void)PE_CdLoadU32(PE_CDREG_DMA3+8u);
}
