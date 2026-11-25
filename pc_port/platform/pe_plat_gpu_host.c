/* Host-model hooks of the clean-room GPU providers; see pe_plat/gpu_host.h.
 * Written from the host GPU model (pe_gpu.h, host_framebuffer.h, game_port.h)
 * and the GPU/DMA documentation (psx-spx: DMA linked-list nodes are
 * "header word = (word count << 24) | next address, then count GP0 words",
 * list end 0xFFFFFF).  No SDK source was read. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_gpu.h"
#include "game_port.h"
#include "host_framebuffer.h"
#include "pe_plat/gpu_host.h"
#include "pe_plat/gpu_resetgraph.h"
#include "pe_plat/gpu_regs.h"

#define LIST_END      0xFFFFFFu
#define NODE_WORDS_MAX 255u

uint32_t pe_plat_gpu_host_vsync_query(void) { return PE_GPU_VSyncQuery(); }

void pe_plat_gpu_host_drawsync(int32_t mode) { HostFB_DrawSync((int)mode); }

void pe_plat_gpu_host_present_dispenv(uint32_t env) { HostFB_PresentDispEnv(env); }

int pe_plat_gpu_host_wait_tick(void)
{
    if (PE_Port_ShouldStop()) return -1;
    if (PE_GPU_DMA2Pending()) {
        unsigned epoch = PE_Port_StopEpoch();
        PEPortDmaIrqCheckpointResult r = PE_Port_ServiceDmaIrqCheckpoint();
        if (PE_Port_StopEpoch() != epoch || PE_Port_ShouldStop()) return -1;
        if (r == PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY || r == PE_PORT_DMA_IRQ_CHECKPOINT_STALE) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
        /* A transfer that stays unserviceable is left to the retail timeout check (GPU reset, -1). */
        return 0;
    }
    /* Nothing pending: the GPU reports "ready" in this model unless the host (or a test) holds it
     * not-ready, and nothing in the wait changes that, so the wait could never end. */
    return (PE_GPU_ReadStatus() & PE_GPU_STATUS_READY_GP0) ? 0 : 1;
}

/* A node's GP0 words must form whole packets (PE_GPU_GP0_StreamPacketWords).
 * A packet that runs past its node is cut (the retired hand walker's rule,
 * kept by DRW1_* / B54KR_* tests).  Returns 1 when fine; otherwise records the
 * named cut (identity: node address, first word of the bad packet). */
static int node_ok(uint32_t node)
{
    uint32_t words[NODE_WORDS_MAX], header, count, i = 0;
    PeGpuState st;
    if (node == LIST_END) return 1;
    node &= 0xFFFFFCu;
    if (node + 4u > PE_RAM_SIZE) return 1;              /* the model names its own range boundary */
    header = PE_LoadU32(PE_RAM_BASE + node);
    count = header >> 24;
    if ((count + 1u) * 4u > PE_RAM_SIZE - node) return 1;
    PE_GPU_GetState(&st);
    if (st.gp0_state != PE_GPU_GP0_IDLE) return 1;      /* payload of an earlier packet continues */
    for (i = 0; i < count; i++) words[i] = PE_LoadU32(PE_RAM_BASE + node + 4u + i * 4u);
    for (i = 0; i < count;) {
        uint32_t n = PE_GPU_GP0_StreamPacketWords(words + i, count - i);
        if (n == 0u) {
            PE_GuestHook_Refuse("func_80076B98_packet_cut", "func_80076B98", 0x80076B98u, 4,
                                PE_RAM_BASE + node, header, i, words[i], "unsupported GP0 packet in DMA list node");
            return 0;
        }
        i += n;
    }
    return 1;
}

int pe_plat_gpu_host_dma_preflight(uint32_t packet)
{
    if (!node_ok(packet & 0xFFFFFFu)) return 0;
    /* A transfer still in flight in the host model (the guest started the next DMA without waiting; the
     * hardware register write would be undefined): let the host finish it first. */
    if (PE_GPU_DMA2Pending()) {
        unsigned epoch = PE_Port_StopEpoch();
        (void)PE_Port_ServiceDmaIrqCheckpoint();
        if (PE_Port_StopEpoch() != epoch || PE_Port_ShouldStop()) return 0;
        if (PE_GPU_DMA2Pending()) {
            PE_GuestHook_Refuse("func_80076B98_dma_busy", "func_80076B98", 0x80076B98u, 1, packet, 0, 0, 0,
                                "GPU DMA2 still busy at DMA start");
            return 0;
        }
    }
    return 1;
}

void pe_plat_gpu_host_dma_run(void)
{
    uint32_t guard = 0;
    /* ResetGraph (retail boot) enables DMA channel 2 in DPCR before any GPU DMA; a host state that
     * never ran it (unit tests that seed GPU state by hand) gets the same enable here. */
    if (PE_GPU_DMA2Pending() && !(PE_GPU_ReadDPCR() & PE_GPU_DMA2_DPCR_ENABLE)) PE_GPU_EnableDMA2();
    while (PE_GPU_DMA2Pending() && PE_GPU_ReadDMA2CHCR() == PE_GPU_DMA2_CHCR_LIST &&
           !PE_Port_ShouldStop() && guard++ < (1u << 20)) {
        uint64_t token = PE_GPU_DMA2EventToken();
        pe_addr_t node = PE_GPU_ReadDMA2MADR();
        if (node != LIST_END && (node & 0xFFFFFCu) >= PE_RAM_SIZE) {
            /* A link outside RAM ends the walk silently (the contract DRW1_drawotag_empty_chain keeps: the
             * ClearOTagR tail's link 0x4957F8 is no RAM).  Real hardware would fetch mirrored RAM or bus
             * garbage; the transfer is cancelled, not completed. */
            (void)PE_GPU_WriteDMA2Register(8u, PE_GPU_ReadDMA2CHCR() & ~PE_GPU_DMA2_CHCR_BUSY);
            return;
        }
        if (!node_ok(node)) return;
        /* One service = one list node: it returns 0 until the end marker, so progress is judged by the
         * node pointer (or completion), not by the return value. */
        (void)PE_GPU_ServiceDMA2Completion(token);
        if (PE_GPU_DMA2Pending() && PE_GPU_DMA2EventToken() == token && PE_GPU_ReadDMA2MADR() == node)
            return;                                      /* not serviceable now: the wait pumps it later */
    }
}

/* GPU command-word writer (guest 0x80077A54, BIOS A(49h) GPU_cw): one word to the GP0 port.  ResetGraph hands
 * it the low 24 bits of the jump-table pointer, a GP0 opcode-00h NOP; a word the model refuses (state that
 * cannot take it) is ignored, as the NOP carries no data. */
void pe_plat_gpu_cw(uint32_t word) { (void)PE_GPU_WriteGP0(word); }

void pe_plat_gpu_host_reset_prepare(void)
{
    static const uint32_t regs[][2] = {
        {0x80095854u, PE_GPU_REG_GPU1}, {0x80095858u, PE_GPU_REG_MADR2}, {0x8009585Cu, PE_GPU_REG_BCR2},
        {0x80095860u, PE_GPU_REG_CHCR2}, {0x80095870u, 0x1F8010F0u}, {0x80095674u, PE_GPU_REG_IMASK},
        {0x80095850u, 0x1F801810u}};
    unsigned i;
    for (i = 0; i < sizeof regs / sizeof regs[0]; i++)
        if (!pe_gpu_reg_ok(PE_LoadU32(regs[i][0]), 4)) PE_StoreU32(regs[i][0], regs[i][1]);
    if (PE_LoadU16(0x800957CCu) == 0u && PE_LoadU16(0x800957D8u) == 0u) {
        PE_StoreU16(0x800957CCu, 1024u);
        PE_StoreU16(0x800957D8u, 512u);
    }
}
