#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "pe_bootstrap.h"
#include "game_port.h"
int pe_plat_cd_sector_dma_host(uint32_t dst,uint32_t count)
{
 uint32_t p=PE_LoadU32(0x8009b27cu);PE_CdStoreU8(p,0);
 p=PE_LoadU32(0x8009b288u);PE_CdStoreU8(p,0x80);
 p=PE_LoadU32(0x8009b2b0u);PE_CdStoreU32(p,0x20943u);
 p=PE_LoadU32(0x8009b28cu);PE_CdStoreU32(p,0x1323u);
 p=PE_LoadU32(0x8009b2b4u);
 if(p==0x1f8010f0u){uint32_t v=PE_GPU_ReadDPCR();PE_GPU_WriteDPCR(v|0x8000u);}
 else {uint32_t v=PE_LoadU32(p);PE_StoreU32(p,v|0x8000u);}
 p=PE_LoadU32(0x8009b2b8u);PE_CdStoreU32(p,dst);
 p=PE_LoadU32(0x8009b2bcu);PE_CdStoreU32(p,count|0x10000u);
 p=PE_LoadU32(0x8009b27cu);
 if(!(PE_CdLoadU8(p)&0x40u)){Bootstrap_ReturnVoid("func_8007BF44_data_wait","func_8007C564");PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;}
 p=PE_LoadU32(0x8009b2c0u);PE_CdStoreU32(p,0x11000000u);PE_CdReg_ServiceDMA3();
 if(PE_Port_ShouldStop())return 0;
 p=PE_LoadU32(0x8009b2c0u);
 if(PE_CdLoadU32(p)&0x1000000u){Bootstrap_ReturnVoid("func_8007BF44_dma_busy","func_8007C564");PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;}
 p=PE_LoadU32(0x8009b28cu);PE_CdStoreU32(p,0x1325u);return 0;
}
