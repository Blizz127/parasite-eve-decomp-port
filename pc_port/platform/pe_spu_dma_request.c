/*
 * Host request API for the SPU DMA model: PE_SpuDma_Begin / PE_SpuDma_BeginRead.
 *
 * Provenance: this is a host test/tool request API.  It is NOT linked into the game binary (parasite-eve-port); only
 * pe-native-tests and the standalone checkers under pc_port/tools link it.  The three guest-visible words a request
 * leaves are derived ONLY from:
 *   (a) the black-box vector report docs/evidence/native-cdx-20261126/cdgen51/REPORT.md (per-chunk description: clear
 *       0x8009B44C for the write direction, 0x8009B450 = the source buffer, 0x8009B454 = the number of 64-byte blocks);
 *   (b) the host-test contract in pc_port/tests/test_native.c (the write-direction checks near lines 27225-27229 and
 *       40861-40879) and pc_port/tests/test_spu_dma_event.h, test_spu_dma_callbacks.h, test_spu_mode.h (the read
 *       direction leaves 0x8009B44C = 1).
 * The request itself (validation and transfer record) is the own-model entry PE_SpuDma_RecordRequest in pe_spu_dma.c.
 */
#include "pe_spu_dma.h"
#include "pe_plat_spu_dma_addrs.h"
#include "psx_compat.h"
#include "pe_sdk.h"

#define GA_D_8009B44C 0x8009B44Cu   /* direction word: 0 write, 1 read */
#define GA_D_8009B450 0x8009B450u   /* source buffer */
#define GA_D_8009B454 0x8009B454u   /* number of 64-byte blocks */

static void LeaveRequestWords(pe_addr_t source, uint32_t requested_size, int read_direction)
{
    uint64_t rounded = ((uint64_t)requested_size + (PE_SPU_DMA_BLOCK_SIZE - 1u)) &
                       ~(uint64_t)(PE_SPU_DMA_BLOCK_SIZE - 1u);

    PE_StoreU32(GA_D_8009B44C, read_direction ? 1u : 0u);
    PE_StoreU32(GA_D_8009B450, source);
    PE_StoreU32(GA_D_8009B454, (uint32_t)(rounded / PE_SPU_DMA_BLOCK_SIZE));
}

int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination,
                    uint32_t requested_size, pe_addr_t callback)
{
    int ok = PE_SpuDma_RecordRequest(source, destination, requested_size, callback, 0);
    if (ok) LeaveRequestWords(source, requested_size, 0);
    return ok;
}

int PE_SpuDma_BeginRead(pe_addr_t guest_destination, uint32_t spu_source,
                        uint32_t requested_size, pe_addr_t callback)
{
    int ok;
    if (requested_size == 0u)
        return 0;
    ok = PE_SpuDma_RecordRequest(guest_destination, spu_source, requested_size, callback, 1);
    if (ok) LeaveRequestWords(guest_destination, requested_size, 1);
    return ok;
}
