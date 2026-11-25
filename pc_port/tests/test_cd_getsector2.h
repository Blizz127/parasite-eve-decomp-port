/* CD_getsector2 0x8007C044 (pe_plat_cd_getsector2, pe_plat_cd_gen29.c,
 * cdgen29): production-path checks of the host integration the black-box
 * vectors cannot see. The vector checker treats PE_CdStore* as plain value
 * stores; here the real pe_cdreg / pe_mmio owners receive them.
 *
 * Device detached (the suite default): the CD status data-request bit never
 * sets, so the wait ends on its poll budget with an unresolved-boundary
 * stop request (port contract), and the function still finishes its
 * register writes and returns 0. The pointer words carry their retail EXE
 * values (read from SLUS_006.62 .data: 0x8009B27C.. -> 0x1F801800 /
 * 0x1F801803 / 0x1F801020 / 0x1F801018 / DPCR / DMA3 MADR, BCR, CHCR). */
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "pe_plat/cd.h"

static void CDGS2_PlantPointers(void)
{
    PE_StoreU32(0x8009B27Cu, 0x1F801800u);
    PE_StoreU32(0x8009B288u, 0x1F801803u);
    PE_StoreU32(0x8009B28Cu, 0x1F801020u);
    PE_StoreU32(0x8009B2B0u, 0x1F801018u);
    PE_StoreU32(0x8009B2B4u, 0x1F8010F0u);
    PE_StoreU32(0x8009B2B8u, 0x1F8010B0u);
    PE_StoreU32(0x8009B2BCu, 0x1F8010B4u);
    PE_StoreU32(0x8009B2C0u, 0x1F8010B8u);
}

static void test_CDGS2_detached_registers(void)
{
    int r;
    TEST("CDGS2_detached_registers");
    ResetTestState();
    PE_CdReg_Reset();
    ASSERT(!PE_CdReg_DeviceEnabled(), "CD device must be detached");
    CDGS2_PlantPointers();
    PE_StoreU32(0x1F8010F0u, 0x00000800u);       /* DPCR: GPU channel only */
    r = pe_plat_cd_getsector2(0x80100000u, 0x200u);
    ASSERT(r == 0, "returns 0");
    ASSERT(PE_CdReg_ReadU32(0x1F801020u) == 0x1325u, "0x1F801020 word");
    ASSERT(PE_CdReg_ReadU32(0x1F801018u) == 0x21020843u, "0x1F801018 word");
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_DMA3) == 0x80100000u, "DMA3 MADR");
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_DMA3 + 4u) == 0x10200u, "DMA3 BCR");
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_DMA3 + 8u) == 0x11400100u, "DMA3 CHCR");
    ASSERT(PE_GPU_ReadDPCR() == 0x00008800u, "DPCR: CD channel enabled, GPU kept");
    ASSERT((PE_CdReg_ReadU8(PE_CDREG_BASE) & 3u) == 0u, "CD index 0");
    ASSERT(PE_Port_ShouldStop(), "budget exit requests a stop");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY,
           "stop reason is unresolved boundary");
    PE_Port_RunControlReset();
    PASS();
}

static void test_CDGS2_stop_requested(void)
{
    int r;
    TEST("CDGS2_stop_requested");
    ResetTestState();
    PE_CdReg_Reset();
    CDGS2_PlantPointers();
    PE_StoreU32(0x1F8010F0u, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_FRAME_LIMIT);
    r = pe_plat_cd_getsector2(0x80101000u, 1u);
    ASSERT(r == 0, "returns 0");
    ASSERT(PE_Port_GetStopReason() == PE_PORT_STOP_FRAME_LIMIT,
           "an earlier stop is kept (no budget stop)");
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_DMA3 + 4u) == 0x10001u, "DMA3 BCR");
    ASSERT(PE_CdReg_ReadU32(PE_CDREG_DMA3 + 8u) == 0x11400100u,
           "the function finishes after a stop exit");
    ASSERT(PE_GPU_ReadDPCR() == 0x8000u, "DPCR bit 15");
    PE_Port_RunControlReset();
    PASS();
}
