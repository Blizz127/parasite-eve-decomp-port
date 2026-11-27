/* Loader device window: the CD drive for a synchronous libds read.
 *
 * Host-model contract, not a retail function.  Retail has the drive attached at all times, so a read started with the
 * DsRead command sequence (0x8007F0C8 queues Pause / Setmode / Setloc / ReadN) completes on its own: each command's
 * completion interrupt starts the next, the sectors arrive through the data callback and the sector DMA, and the
 * read-busy word 0x8009B6CC drops when the sector counter reaches zero.  The host attaches the controller model only
 * for the windows that need it (the first mount, the credits, the movies), so a read issued with the model detached
 * queued its commands and nothing ever advanced them: the boot spun on the status getter with the fourth command
 * (ReadN) pending and no sector arriving (smoke trace: device not enabled, qlen 1, [0x8009B6B4] = 0xF forever).
 *
 * The window is the one the first mount already uses: DsRead opens it before it starts the command sequence
 * (PE_Cd_AttachDevice, which needs an idle lane and an empty queue, then HostFB_DeviceQuietBegin: the drive runs on its
 * own clock and the emulated VBlank / counters do not move, as the pre-native synchronous read took no emulated
 * time).  The game was ported against reads that are finished when the issue returns (the pre-native read function copied
 * the sectors on the spot; the room-load state machines test the destination right after the issue), so DsRead calls
 * PE_CdHost_LoaderDrain as its last step: the real command chain runs (Pause, Setmode, Setloc, ReadN, each completion
 * starting the next; sector interrupt, data callback, sector DMA, counter decrement) until the read has finished, then the
 * window closes.  A read started any other way is advanced by HostFB_DevicePoll (every guest status spin) through
 * PE_CdHost_LoaderService, which closes the window the same way.  Finished = the busy word was seen set and is clear again,
 * queue empty, lane idle.
 * Clock: inside the window VSync(-1) stands still at the call-count value the window opened with (HostFB_VSync), the window
 * is quiet (no VBlank edge, counters restored at close), and the GPU DMA checkpoint is serviced on every pass (a LoadImage the
 * game started before the read completes while the read runs, as it does on hardware).
 * Host wait contract: a stop request ends the drain and gives the host its state back; a read still unfinished after
 * CDH_LOADER_BUDGET drive cycles (1200 retraces = the read poll's deadline) prints a diagnostic and is aborted through the
 * abort entry like the poll's expiry (the caller's poll loop then runs out its retrace deadline and takes the error path);
 * CDH_LOADER_MAX_EXPIRED such windows in a row request a stop.  A read of no sectors has nothing to wait for and is
 * aborted at once.  PE_CD_LOADER_TRACE=1 prints one line per open / drain / close.  The window is a no-op where it cannot
 * open (no disc, device already attached by another owner, drive not idle or queue not empty): the read then queues its
 * commands as before and the host that owns the device advances them. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/cdhost.h"
#include "pe_plat/framebuffer_api.h"
#include "pe_plat/gpu_hw.h"
#include "pe_plat/cd.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/irq_hw.h"

#define CDH_LOADER_BUDGET 677376000u /* drive cycles (20 s of console time) before the window is declared stuck */
#define CDH_LOADER_MAX_EXPIRED 3u    /* windows in a row ended by the budget before the host stops */
#define CDH_SPIN_CYCLES   64u        /* one guest status spin (HostFB_DevicePoll) */
#define CDH_DRAIN_CYCLES  4096u      /* one drain pass (HostFB_StreamTick) */
#define W_BUSY       0x8009B6CCu      /* 1 while a DsRead read runs */
#define W_COUNT      0x8009B6B4u      /* sectors still to arrive */
#define W_QUEUE_LEN  0x800A3608u
#define W_LANE       0x8009B574u      /* 1 = drive command lane idle */
#define W_IN_IRQ     0x800945E6u      /* nonzero while an interrupt handler runs */

static int loader_trace(void)
{
    static int on = -1;
    if (on < 0) on = getenv("PE_CD_LOADER_TRACE") != NULL;   /* debug aid: one line per window open / drain / close */
    return on;
}

static PeCdAttach g_attach;
static int g_open;
static int g_seen_busy;
static uint64_t g_cycles;
static unsigned g_expired;   /* consecutive windows ended by the budget (a drive that keeps failing) */

int PE_CdHost_LoaderIsOpen(void) { return g_open; }

int PE_CdHost_LoaderOpen(void)
{
    if (g_open) {
        if (PE_CdReg_DeviceEnabled()) return 1;
        /* stale: the device model was reset or detached under an open window (a test reset, a stop): the window is gone */
        g_open = 0;
        HostFB_DeviceQuietEnd();
    }
    if (!PE_Cd_AttachDevice(&g_attach)) {   /* no disc / device already owned / drive not idle: unchanged */
        if (loader_trace())
            fprintf(stderr, "[cd-loader] attach refused (device=%d booted=%u lane=%u queue=%u)\n", PE_CdReg_DeviceEnabled(),
                    PE_LoadU32(0x8009B554u), PE_LoadU32(W_LANE), PE_LoadU32(W_QUEUE_LEN));
        return 0;
    }
    HostFB_DeviceQuietBegin();
    if (loader_trace()) fprintf(stderr, "[cd-loader] open (sectors=0x%X dest=0x%08X)\n", PE_LoadU32(W_COUNT), PE_LoadU32(0x8009B6B0u));
    g_open = 1;
    g_seen_busy = 0;
    g_cycles = 0u;
    return 1;
}

void PE_CdHost_LoaderClose(void)
{
    if (!g_open) return;
    g_open = 0;
    if (loader_trace()) fprintf(stderr, "[cd-loader] close (sectors left=0x%X stop=%d)\n", PE_LoadU32(W_COUNT), PE_Port_ShouldStop());
    HostFB_DeviceQuietEnd();
    PE_Cd_DetachDevice(&g_attach);
}

/* A real read takes console time during which a GPU DMA the game started before it (a LoadImage whose source buffer the
 * read is about to overwrite) completes through its DMA interrupt.  The window runs the drive on its own clock, so give
 * that checkpoint the same service HostFB_VSync gives it while a movie decodes. */
static void service_gpu_dma(void)
{
    if (PE_GPU_DMA2Pending()) {
        PEPortDmaIrqCheckpointResult r = PE_Port_ServiceDmaIrqCheckpoint();
        if ((r == PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY || r == PE_PORT_DMA_IRQ_CHECKPOINT_STALE) && !PE_Port_ShouldStop())
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    }
}

static void loader_service(uint32_t cycles)
{
    if (!g_open || PE_Port_ShouldStop()) return;
    service_gpu_dma();
    if (PE_Port_ShouldStop()) return;
    if (PE_LoadU32(W_BUSY) == 1u) g_seen_busy = 1;
    g_cycles += cycles;
    if (g_cycles > CDH_LOADER_BUDGET) {
        /* 1200 retraces of drive time: the retail read poll would have declared this read expired.  The window clock stands
         * still, so do what the expiry does: abort the read (the abort entry clears the busy word and drops the queue),
         * close the window, and let the caller's own poll loop run out its retrace deadline and take the error path.  A
         * drive that keeps failing is a stop, not an endless retry. */
        PeCdDeviceState st;
        PE_CdReg_GetDeviceState(&st);
        fprintf(stderr, "[cd-loader] the read did not finish in %u drive cycles (busy=%u sectors left=0x%X queue=%u lane=%u; drive: commands=%u responses=%u reading=%u next_lba=%u sectors=%u; I_STAT=%04X I_MASK=%04X); aborting it like the read poll's expiry (%u in a row)\n",
                CDH_LOADER_BUDGET, PE_LoadU32(W_BUSY), PE_LoadU32(W_COUNT), PE_LoadU32(W_QUEUE_LEN), PE_LoadU32(W_LANE),
                st.commands, st.responses, st.reading, st.next_lba, st.sectors, PE_IRQ_ReadStatus(), PE_IRQ_GetMask(), g_expired + 1u);
        fprintf(stderr, "[cd-loader] first drive commands:");
        for (unsigned i = 0; i < 16u && i < st.commands; i++) fprintf(stderr, " %02X", st.command_log[i]);
        fprintf(stderr, "; first responses:");
        for (unsigned i = 0; i < 16u && i < st.responses; i++) fprintf(stderr, " %02X", st.response_log[i]);
        fprintf(stderr, "\n");
        func_80081268();
        if (++g_expired >= CDH_LOADER_MAX_EXPIRED) {
            fprintf(stderr, "[cd-loader] %u reads in a row ended by the budget; requesting stop\n", g_expired);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        }
        PE_CdHost_LoaderClose();
        return;
    }
    if (g_seen_busy && PE_LoadU32(W_BUSY) == 0u && PE_LoadU32(W_QUEUE_LEN) == 0u && PE_LoadU32(W_LANE) == 1u &&
        PE_LoadU16(W_IN_IRQ) == 0u)
    {   g_expired = 0u;
        PE_CdHost_LoaderClose();
    }
}

void PE_CdHost_LoaderService(void) { loader_service(CDH_SPIN_CYCLES); }

void PE_CdHost_LoaderDrain(void)
{
    if (loader_trace()) fprintf(stderr, "[cd-loader] drain (open=%d busy=%u sectors=0x%X)\n", g_open, PE_LoadU32(W_BUSY), PE_LoadU32(W_COUNT));
    if (g_open && PE_LoadU32(W_BUSY) != 1u) {   /* DsRead did not start a read (the sequence was refused): nothing to wait for */
        if (loader_trace())
            fprintf(stderr, "[cd-loader] drain: busy word is %u at entry (read refused, or finished before DsRead's own store of 1): closing\n",
                    PE_LoadU32(W_BUSY));
        PE_CdHost_LoaderClose();
        return;
    }
    g_seen_busy = 1;   /* DsRead stored the busy word before it called this */
    if (PE_LoadU32(W_COUNT) == 0u) {
        /* A read of no sectors has nothing to wait for (retail would let ReadN run on until something stops it): stop it the
         * way the read poll's expiry does, through the abort entry, which clears the busy word and drops the queue.  The
         * drive finishes its command in flight on the passes below and the window closes once the lane is idle. */
        func_80081268();
    }
    while (g_open && !PE_Port_ShouldStop()) {
        HostFB_StreamTick();             /* VSync(-1) + drive time: the next command, sector interrupt, DMA ... */
        loader_service(CDH_DRAIN_CYCLES);
    }
    if (g_open) PE_CdHost_LoaderClose();   /* a stop (a device boundary, the host quitting) ended the drain: give the host its state back */
}
