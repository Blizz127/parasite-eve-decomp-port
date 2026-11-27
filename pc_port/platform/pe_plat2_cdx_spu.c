/* SPU installer / init chain, stream feed leaves and SPU DMA adapters of batch cdgen44
 * (7 retail routines), written clean-room from the black-box vector sets cdgen44
 * (docs/evidence/native-cdx-20261126), the recorded seam contracts and psx-spx; no SDK
 * source, decompiled source, tables or disassembly were read.  The behaviour below is exactly
 * what the vectors show; no claim is made about the original's source.  The derivation table
 * with vector evidence for every line is docs/evidence/native-cdx-20261126/cdx-spu/AUTHOR-REPORT.md.
 *
 * Routines (retail address -> symbol, see pe_plat/cdx_spu.h):
 *   80085644 stream_bringup      stream (SPU) bring-up: state reset, 72 records, reverb, counter + event
 *   8007D15C install_dma_event   one-shot guard, DMA channel 4 callback + BIOS event registration
 *   8007D054 init_default        init_wrapper(0)
 *   8007D074 init_wrapper(mode)  ResetCallback, SPU initialize, state reset, installer
 *   8008CB54 reverb_transition   reverb mode change through the reverb setters
 *   8008CBA8 stream_dispatch     command record enqueue, three immediate commands
 *   80085E54 spu_write           SPU RAM write: programmed I/O or DMA channel 4
 *   80085DF4 spu_read            SPU RAM read through DMA channel 4
 *
 * Guest words used (little-endian guest RAM, read at every use; all located from the vectors):
 *   0x8009B3EC  installer one-shot guard                   0x8009B384  BIOS event handle
 *   0x8009B390/B394/B398/B3A0/B3A8/B3AC  reverb state words, B3A4/B3A6 halfwords  (init_wrapper)
 *   0x8009B3B8..B3E6  24 halfwords reset to 0xC000 (init_wrapper(0))
 *   0x8009B3B0/B3B4/B3E8/B388/B38C/B418/B45C/B460/B464  words cleared by init_wrapper
 *   0x8009B46C  initial value of the word 0x8009B398 (loaded by init_wrapper)
 *   0x8009B3A0  current reverb mode word (reverb_transition)
 *   0x800BCD80..BCD90  pending command word + 4 arguments    0x8009D268  re-entry marker
 *   0x8009D2F4  command queue write index                    0x800B8628  queue base, 36-byte records
 *   0x8009CDF0  command 0x24 sequence counter
 *   0x8009B3FC  pointer to the SPU register block (1F801C00)
 *   0x8009B400/B404/B408  pointers to DMA4 MADR / BCR / CHCR (1F8010C0/C4/C8)
 *   0x8009B410  pointer to SPU_DELAY (handled by the shared leaves 8007DC5C / 8007DC84)
 *   0x8009B414  halfword: SPU transfer address register value (address / 8)
 *   0x8009B418  transfer mode word (0: DMA, nonzero: programmed I/O)   0x8009B424  address shift
 *   0x8009B430/B434  SPU transfer state word / callback word
 *   0x8009B44C/B450/B454  last DMA bookkeeping: direction, source, block count
 *
 * Seam contract (stated in pe_plat/cdx_spu.h): PE_SpuDma_InstallIrq returns nonzero on success; the
 * installer registers the DMA callback and the BIOS event only then.
 *
 * Undefined edges (no vector shows them; listed in the AUTHOR-REPORT): init_wrapper with mode != 0
 * resets no halfword table; stream_bringup does not model the zero-divisor break of the original's
 * address-unit code (dropped from the vectors).  stream_bringup sets the transfer start by calling the
 * transfer-address leaf func_80085EB4 with the BYTE address 0x1010 (docs/evidence/native-cdx-20261126/cdgen44/
 * SPU-ORACLE-CHAIN.md: the original's value, 0x202 units for the shift word 3, replaces the entry value of
 * 0x8009B414); the earlier entry-value store-back came from a generator artefact (own-written readback seeded
 * the word, docs/evidence/native-cdx-20261126/cdx-spu2/REPORT.md).  Host contracts of the DMA
 * adapters: a pending host DMA completes before a new transfer is issued (PE_SpuDma_DeliverPending, the
 * issue is dropped when that ends in a stop). */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_guest_decomp.h"
#include "pe_plat/cdx_spu.h"
#include "pe_plat/spu_a.h"
#include "pe_plat/spu_b.h"
#include "pe_plat/audio.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/spu_dma_hw.h"
#include "pe_plat/ctl.h"
#include "pe_plat/state.h"
#include "pe_plat/bios_file.h"

#define W_INSTALLED   0x8009B3ECu
#define W_EVENT       0x8009B384u
#define W_REVERB      0x8009B390u
#define W_REVERB_P1   0x8009B394u
#define W_REVERB_P2   0x8009B398u
#define W_REVERB_MODE 0x8009B3A0u
#define H_B3A4        0x8009B3A4u
#define H_B3A6        0x8009B3A6u
#define W_B3A8        0x8009B3A8u
#define W_B3AC        0x8009B3ACu
#define W_B388        0x8009B388u
#define W_B38C        0x8009B38Cu
#define W_B3B0        0x8009B3B0u
#define W_B3B4        0x8009B3B4u
#define W_B3E8        0x8009B3E8u
#define W_B418        0x8009B418u
#define W_B438        0x8009B438u
#define W_HEAP_COUNT  0x8009B45Cu
#define W_HEAP_ZERO   0x8009B460u
#define W_HEAP_TABLE  0x8009B464u
#define W_REVERB_INIT 0x8009B46Cu
#define H_TABLE_C000  0x8009B3B8u      /* 24 halfwords up to 0x8009B3E6 */
#define TABLE_ENTRIES 24u

#define DMA_IRQ_HANDLER 0x8007D614u    /* the DMA channel 4 completion handler the host model recognises */
#define EVENT_CLASS     0xF0000009u    /* BIOS event class of the SPU DMA event */
#define EVENT_SPEC      0x00000020u
#define EVENT_MODE      0x00002000u    /* BIOS EvMdNOINTR (psx-spx, BIOS Event Functions) */

#define W_SPU_BASE    0x8009B3FCu
#define W_MADR_PTR    0x8009B400u
#define W_BCR_PTR     0x8009B404u
#define W_CHCR_PTR    0x8009B408u
#define H_XFER_ADDR   0x8009B414u
#define W_SHIFT       0x8009B424u
#define W_STATE_B430  0x8009B430u
#define W_CALLBACK    0x8009B434u
#define W_DMA_DIR     0x8009B44Cu
#define W_DMA_SRC     0x8009B450u
#define W_DMA_BLOCKS  0x8009B454u

#define R_XFER_ADDR   0x1A6u           /* SPU register block offsets (psx-spx "SPU Control Registers") */
#define R_SPUCNT      0x1AAu
#define R_XFER_FIFO   0x1A8u
#define R_SPUSTAT     0x1AEu
#define SPUCNT_MWRITE 0x0010u          /* ... 1 = manual write */
#define SPUSTAT_MASK  0x07FFu          /* SPUSTAT bits 0-10 (psx-spx) */
#define SPUSTAT_BUSY  0x0400u          /* bit 10: data transfer busy */
#define WAIT_POLLS    3841u            /* polls of every bounded SPUSTAT wait (vectors: 3841 reads, then on) */
#define SPUCNT_XFER   0x0030u          /* SPUCNT bits 5-4: sound RAM transfer mode */
#define SPUCNT_DMAW   0x0020u          /* ... 2 = DMA write */
#define SPUCNT_DMAR   0x0030u          /* ... 3 = DMA read */
#define DMA_MAX_BYTES 0x0007EFF0u      /* size clamp of one transfer (vectors: RET 0x7EFF0) */
#define DMA_BLOCK     0x40u            /* bytes per DMA block; BCR low half = words per block (0x10) */
#define CHCR_WRITE    0x01000201u      /* start, block mode, RAM -> SPU */
#define CHCR_READ     0x01000200u      /* start, block mode, SPU -> RAM */

#define W_QUEUE_INDEX 0x8009D2F4u
#define W_QUEUE_BUSY  0x8009D268u
#define W_QUEUE_BASE  0x800B8628u
#define QUEUE_STRIDE  36u
#define W_CMD         0x800BCD80u
#define W_ARG0        0x800BCD84u
#define W_ARG1        0x800BCD88u
#define W_ARG2        0x800BCD8Cu
#define W_ARG3        0x800BCD90u
#define W_SEQUENCE    0x8009CDF0u
#define W_STATE_BLOCK 0x8009D2C8u      /* pointer to the stream state block (halfword +0x54 read by payload commands) */

/* -------------------------------------------------------------- installer */

/* noinline: the cdgen46 checker replaces this symbol (weakened in its build line) to record the installer as the
 * stopped callee the stream bring-up vectors show.
 * One-shot: with the guard word zero, set it, then inside a critical section register the DMA
 * channel 4 callback and open + enable the BIOS event, storing its handle.  Registrations only when the
 * host seam reports the completion handler installed (nonzero). */
void pe_plat_cdxspu_install_dma_event(void) __attribute__((noinline));
void pe_plat_cdxspu_install_dma_event(void)
{
    if (PE_LoadU32(W_INSTALLED) != 0u) return;
    PE_StoreU32(W_INSTALLED, 1u);
    (void)func_80072714();                        /* EnterCriticalSection */
    if (PE_SpuDma_InstallIrq(DMA_IRQ_HANDLER)) {
        int handle;
        func_8007DD14(DMA_IRQ_HANDLER);           /* DMA channel 4 callback slot */
        handle = PE_Event_Open(EVENT_CLASS, EVENT_SPEC, EVENT_MODE, 0u);
        PE_StoreU32(W_EVENT, (uint32_t)handle);
        (void)PE_Event_Enable(handle);
    }
    func_80072724();                              /* ExitCriticalSection */
}

/* ------------------------------------------------------------------- init */

void pe_plat_cdxspu_init_wrapper(int mode)
{
    uint32_t i, w;

    (void)func_80073C94();                        /* ResetCallback */
    func_8007D1D4(mode);                          /* SPU initialize */
    if (mode == 0)
        for (i = TABLE_ENTRIES; i-- > 0u; )
            PE_StoreU16(H_TABLE_C000 + 2u * i, 0xC000u);
    pe_plat_cdxspu_install_dma_event();
    PE_StoreU32(W_REVERB, 0u);
    PE_StoreU32(W_REVERB_P1, 0u);
    PE_StoreU32(W_REVERB_MODE, 0u);
    PE_StoreU16(H_B3A4, 0u);
    PE_StoreU16(H_B3A6, 0u);
    PE_StoreU32(W_B3A8, 0u);
    PE_StoreU32(W_B3AC, 0u);
    w = PE_LoadU32(W_REVERB_INIT);
    PE_StoreU32(W_REVERB_P2, w);
    func_8007DAE0(0xD1, w, 0);
    PE_StoreU32(W_HEAP_COUNT, 0u);
    PE_StoreU32(W_HEAP_ZERO, 0u);
    PE_StoreU32(W_HEAP_TABLE, 0u);
    PE_StoreU32(W_B38C, 0u);
    PE_StoreU32(W_B418, 0u);
    PE_StoreU32(W_B388, 0u);
    PE_StoreU32(W_B3B4, 0u);
    PE_StoreU32(W_B3B0, 0u);
    PE_StoreU32(W_B3E8, 0u);
}

void pe_plat_cdxspu_init_default(void)
{
    pe_plat_cdxspu_init_wrapper(0);
}

/* ----------------------------------------------------------------- reverb */

/* Reverb mode transition: nothing happens when the requested mode equals the current mode word;
 * otherwise the reverb is switched off, the mode set with bit 8 (0x100) added, and the reverb switched
 * on again. */
void pe_plat_cdxspu_reverb_transition(uint32_t mode)
{
    if (mode == PE_LoadU32(W_REVERB_MODE)) return;
    (void)func_80085A64(0);
    (void)func_8008CF70((int)(mode | 0x100u));
    if (PE_Port_ShouldStop()) return;     /* the mode change's pending DMA ended in a stop: the reverb stays off */
    (void)func_80085A64(1);
}

/* ----------------------------------------------------------- stream queue */

/* Append one 36-byte record to the command queue (index word 0x8009D2F4, base 0x800B8628): the
 * command word at +0 and the first `nargs` argument words (0x800BCD84, 88, 8C, 90) at +4, +8, +0xC, +0x10.
 * Returns the record address. */
static uint32_t queue_record(uint32_t command, unsigned nargs)
{
    uint32_t index = PE_LoadU32(W_QUEUE_INDEX);
    uint32_t record = W_QUEUE_BASE + index * QUEUE_STRIDE;
    unsigned k;

    PE_StoreU32(W_QUEUE_INDEX, index + 1u);
    for (k = 0; k < nargs; k++)
        PE_StoreU32(record + 4u + 4u * k, PE_LoadU32(W_ARG0 + 4u * k));
    PE_StoreU32(record, command);
    return record;
}

/* Payload commands 0x10 / 0x12 / 0x19 (the marker word is already 1): func_80085084 takes the payload pointer
 * (first argument word); a nonzero result ends the call with -1.  With result 0 the payload header is read
 * (halfword at +4, halfword at +8) and so is the halfword at +0x54 of the block 0x8009D2C8 points to; when
 * the +4 halfword equals the +0x54 halfword nothing more happens and the call returns 0.  Otherwise the
 * reverb mode transition to the +8 halfword runs, one record is queued (+4 = payload pointer + 0x10,
 * +0xC = the +4 halfword, +0x10 = second argument word for command 0x12 only, +0 = command) and the
 * call returns the +4 halfword. */
static int queue_payload(uint32_t cmd)
{
    uint32_t p = PE_LoadU32(W_ARG0), h4, h8, state, index, record;

    if (func_80085084(p) != 0u) {
        PE_StoreU32(W_QUEUE_BUSY, 0u);
        return -1;
    }
    h4 = PE_LoadU16(p + 4u);
    state = PE_LoadU32(W_STATE_BLOCK);
    h8 = PE_LoadU16(p + 8u);
    if (h4 == PE_LoadU16(state + 0x54u)) {
        PE_StoreU32(W_QUEUE_BUSY, 0u);
        return 0;
    }
    pe_plat_cdxspu_reverb_transition(h8);
    if (PE_Port_ShouldStop()) {                   /* the transition's DMA ended in a stop: nothing is published */
        PE_StoreU32(W_QUEUE_BUSY, 0u);
        return -1;
    }
    index = PE_LoadU32(W_QUEUE_INDEX);
    PE_StoreU32(W_QUEUE_INDEX, index + 1u);
    record = W_QUEUE_BASE + index * QUEUE_STRIDE;
    PE_StoreU32(record + 4u, p + 0x10u);
    PE_StoreU32(record + 0xCu, h4);
    if (cmd == 0x12u) PE_StoreU32(record + 0x10u, PE_LoadU32(W_ARG1));
    PE_StoreU32(record, cmd);
    PE_StoreU32(W_QUEUE_BUSY, 0u);
    return (int)h4;
}

/* Stream command dispatcher (the command word is 0x800BCD80, its arguments follow).  The marker word is 1
 * for the whole call.
 *   0x10, 0x12, 0x19  payload commands, see queue_payload()
 *   0x98 / 0x99       two argument-less records: 0x9A + 0x9C / 0x9B + 0x9D
 *   0xD8              two records with the first argument: 0xD0 and 0xD4
 *   0xD9              two records with the first two arguments: 0xD1 and 0xD5
 *   0xDA              two records with the first three arguments: 0xD2 and 0xD6
 *   0x24              one record (command, four arguments) plus a ticket at +0x14 taken from the counter
 *                     word 0x8009CDF0 (new value ((old + 1) & 0x1FF) + 0x400); returns the old value
 *   any other command one record with command and four arguments
 * Every other command value of 0..0xFFF was swept (docs: AUTHOR-REPORT) and behaves as the last line. */
int pe_plat_cdxspu_stream_dispatch(void)
{
    uint32_t cmd = PE_LoadU32(W_CMD);
    int result = 0;

    PE_StoreU32(W_QUEUE_BUSY, 1u);
    switch (cmd) {
    case 0x10u: case 0x12u: case 0x19u:
        return queue_payload(cmd);
    case 0x98u:
        (void)queue_record(0x9Au, 0); (void)queue_record(0x9Cu, 0);
        break;
    case 0x99u:
        (void)queue_record(0x9Bu, 0); (void)queue_record(0x9Du, 0);
        break;
    case 0xD8u:
        (void)queue_record(0xD0u, 1); (void)queue_record(0xD4u, 1);
        break;
    case 0xD9u:
        (void)queue_record(0xD1u, 2); (void)queue_record(0xD5u, 2);
        break;
    case 0xDAu:
        (void)queue_record(0xD2u, 3); (void)queue_record(0xD6u, 3);
        break;
    case 0x24u: {
        uint32_t record = queue_record(0x24u, 4);
        uint32_t ticket = PE_LoadU32(W_SEQUENCE);
        PE_StoreU32(record + 0x14u, ticket);
        PE_StoreU32(W_SEQUENCE, ((ticket + 1u) & 0x1FFu) + 0x400u);
        result = (int)ticket;
        break;
    }
    default:
        (void)queue_record(cmd, 4);
        break;
    }
    PE_StoreU32(W_QUEUE_BUSY, 0u);
    return result;
}

/* ---------------------------------------------------------------- SPU DMA */

/* Bounded halfword register poll of the DMA setup: reads register `reg` (offset in the SPU block; the base pointer word
 * is loaded once per poll loop) at most WAIT_POLLS times until (value & mask) == want, one port-clock tick per failed
 * read.  Returns 1 when the condition held, 0 after WAIT_POLLS reads that all failed (the original goes on silently,
 * see the timeout sequences of the cdgen48 fault set: 3841 reads), -1 when a stop was requested (host contract). */
static int spu_reg_poll(uint32_t reg, uint32_t mask, uint32_t want)
{
    uint32_t addr = PE_LoadU32(W_SPU_BASE) + reg, n;
    for (n = 0; n < WAIT_POLLS; n++) {
        if (PE_Port_ShouldStop()) return -1;
        if (((uint32_t)PE_LoadU16(addr) & mask) == want) return 1;
        pe_plat_sio_poll_tick();
    }
    return 0;
}

/* Start one DMA channel 4 transfer of `size` bytes (clamped) between guest RAM at `address` and the
 * SPU transfer address; `read` selects the direction.  The transfer address register takes the stored
 * address value masked to (32 - shift) bits and is polled back until it holds that value; SPUCNT takes the DMA
 * transfer mode and is polled until its transfer-mode bits (5-4) hold it; SPU_DELAY its DMA priority bits (shared
 * leaves), and the DMA channel registers are programmed with the 64-byte block count last.  Each poll is bounded
 * (WAIT_POLLS reads) and a timeout is silent.  Fault sequences (cdgen48 `upload_fault`, black-box):
 *   - the transfer address register never reads back the stored value: after the polls, the direction word is
 *     stored, SPUCNT is polled (still without being written) and the transfer is NOT started;
 *   - SPUCNT never shows the mode bits after its write: the transfer is NOT started.
 * Whether the SPUCNT poll compares the mode bits only or the whole register is not determined by the vectors (every
 * row's readback either equals the stored value or differs in the mode bits).  Returns the clamped size. */
static uint32_t spu_dma_issue(uint32_t address, uint32_t size, int read)
{
    uint32_t clamped = size > DMA_MAX_BYTES ? DMA_MAX_BYTES : size;
    uint32_t blocks = (clamped + (DMA_BLOCK - 1u)) / DMA_BLOCK;
    uint32_t shift = PE_LoadU32(W_SHIFT) & 31u;
    uint32_t tsa = ((uint32_t)PE_LoadU16(H_XFER_ADDR) << shift) >> shift;
    uint32_t base, mode = read ? SPUCNT_DMAR : SPUCNT_DMAW;
    uint16_t cnt;
    int w;

    PE_StoreU16(H_XFER_ADDR, (uint16_t)tsa);
    base = PE_LoadU32(W_SPU_BASE);
    PE_StoreU16(base + R_XFER_ADDR, (uint16_t)tsa);
    w = spu_reg_poll(R_XFER_ADDR, 0xFFFFu, tsa & 0xFFFFu);
    if (w < 0) return clamped;
    PE_StoreU32(W_DMA_DIR, read ? 1u : 0u);
    if (w == 0) {
        (void)spu_reg_poll(R_SPUCNT, SPUCNT_XFER, mode);
        return clamped;
    }
    cnt = PE_LoadU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT);
    PE_StoreU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT, (uint16_t)((cnt & ~SPUCNT_XFER) | mode));
    if (spu_reg_poll(R_SPUCNT, SPUCNT_XFER, mode) != 1) return clamped;
    if (read) func_8007DC84(); else func_8007DC5C();            /* SPU_DELAY priority bits */
    PE_StoreU32(W_DMA_SRC, address);
    PE_StoreU32(W_DMA_BLOCKS, blocks);
    PE_StoreU32(PE_LoadU32(W_MADR_PTR), address);
    PE_StoreU32(PE_LoadU32(W_BCR_PTR), (blocks << 16) | 0x10u);
    PE_StoreU32(PE_LoadU32(W_CHCR_PTR), read ? CHCR_READ : CHCR_WRITE);
    return clamped;
}

/* ------------------------------------------------------- programmed I/O write */

/* Wait until (SPUSTAT & mask) == want, at most WAIT_POLLS polls; on a timeout the original goes on
 * silently (the vectors show 3841 reads, then the next step).  Returns -1 when a stop was requested. */
static int spu_status_wait(uint32_t mask, uint32_t want)
{
    uint32_t n;
    for (n = 0; n < WAIT_POLLS; n++) {
        if (PE_Port_ShouldStop()) return -1;
        if ((PE_LoadU16(PE_LoadU32(W_SPU_BASE) + R_SPUSTAT) & mask) == want) return 1;
        pe_plat_sio_poll_tick();
    }
    return 0;
}

/* Manual sound RAM write of `size` bytes from guest RAM: the transfer address register takes the halfword
 * at 0x8009B414; the data goes through the FIFO register in chunks of at most 64 bytes (ceil(n / 2) halfwords,
 * an odd tail is read as a whole halfword), each chunk followed by SPUCNT transfer mode = manual write and a
 * bounded wait for SPUSTAT bit 10 to clear; at the end the transfer mode is cleared and SPUSTAT (bits 0-10)
 * is waited back to the value it had on entry.  Loads from `source` are plain halfword loads whatever the
 * alignment (the vectors have odd sources; the host loads are alignment-free). */
static void spu_pio_write(uint32_t source, uint32_t size)
{
    uint32_t entry = PE_LoadU16(PE_LoadU32(W_SPU_BASE) + R_SPUSTAT) & SPUSTAT_MASK;
    uint32_t remaining = size, cursor = source, i, halves;
    uint16_t cnt;

    PE_StoreU16(PE_LoadU32(W_SPU_BASE) + R_XFER_ADDR, PE_LoadU16(H_XFER_ADDR));
    while (remaining) {
        uint32_t chunk = remaining > 64u ? 64u : remaining;
        halves = (chunk + 1u) / 2u;
        for (i = 0; i < halves; i++)
            PE_StoreU16(PE_LoadU32(W_SPU_BASE) + R_XFER_FIFO, PE_LoadU16(cursor + 2u * i));
        cnt = PE_LoadU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT);
        PE_StoreU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT, (uint16_t)((cnt & ~SPUCNT_XFER) | SPUCNT_MWRITE));
        if (spu_status_wait(SPUSTAT_BUSY, 0u) < 0) return;
        cursor += chunk;
        remaining -= chunk;
    }
    cnt = PE_LoadU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT);
    PE_StoreU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT, (uint16_t)(cnt & ~SPUCNT_XFER));
    (void)spu_status_wait(SPUSTAT_MASK, entry);
}

/* The pending host DMA (if any) completes first; ends the call when that stopped the port. */
uint32_t pe_plat_cdxspu_spu_write(uint32_t source, uint32_t size)
{
    uint32_t result;

    if (!PE_SpuDma_DeliverPending()) return 0u;
    if (PE_LoadU32(W_B418) == 0u) {
        result = spu_dma_issue(source, size, 0);
    } else {
        spu_pio_write(source, size);
        result = size;
    }
    if (PE_LoadU32(W_CALLBACK) == 0u) PE_StoreU32(W_STATE_B430, 0u);
    return result;
}

uint32_t pe_plat_cdxspu_spu_read(int guest_destination, unsigned int size)
{
    uint32_t result;

    if (!PE_SpuDma_DeliverPending()) return 0u;
    result = spu_dma_issue((uint32_t)guest_destination, size, 1);
    if (PE_LoadU32(W_CALLBACK) == 0u) PE_StoreU32(W_STATE_B430, 0u);
    return result;
}

/* ------------------------------------------------------------ stream bring-up */

#define W_STREAM_HEAP 0x800B6958u      /* heap block buffer handed to the heap configuration */
#define W_EVENT_STREAM 0x8009CDE0u     /* handle of the stream counter event */
#define COUNTER_ID      0xF2000002u    /* root counter 2 */
#define COUNTER_TARGET  0x44E8u
#define COUNTER_FLAGS   0x1000u
#define STREAM_EVENT_SPEC 0x00000002u
#define STREAM_EVENT_HANDLER 0x8008E23Cu
#define RETRY_BUDGET    0x10000u       /* host stuck-wait budget of every retail "until it succeeds" loop */
#define SPUCNT_IRQ      0x0040u        /* SPUCNT bit 6: SPU IRQ enable (psx-spx) */
#define RECORD_BASE     0x800B8AF8u    /* 72 voice / channel records of 0x11C bytes */
#define RECORD_STRIDE   0x11Cu
#define BRINGUP_TSA_BYTES 0x1010u      /* SPU byte address of the bring-up upload: right after SpuInit's own 16-byte block at 0x1000 */

/* One retry step of a retail "call until it succeeds" loop: false when the port stopped or the stuck-wait
 * budget ran out (that requests a stop; the checkers never reach it). */
static int retry_step(uint32_t *tries)
{
    if (PE_Port_ShouldStop()) return 0;
    if ((*tries)++ >= RETRY_BUDGET) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    pe_plat_sio_poll_tick();
    return 1;
}

static void bringup_clear_a(void)
{
    PE_StoreU32(0x8009D2C8u, 0x800B6980u);
    PE_StoreU32(0x800B6A30u, 0x7F0000u);
    PE_StoreU32(0x800B69C8u, 0x7F0000u);
    PE_StoreU32(0x8009D2B4u, 0x7FFF0000u);
    PE_StoreU32(0x8009D2C0u, 0x1u);
    PE_StoreU32(0x800BCD50u, 0x0u);
    PE_StoreU32(0x800B6984u, 0x0u);
    PE_StoreU32(0x800B6988u, 0x0u);
    PE_StoreU16(0x800B69D4u, 0x0u);
    PE_StoreU32(0x800BCD60u, 0x0u);
    PE_StoreU32(0x800B699Cu, 0x0u);
    PE_StoreU32(0x800B6A04u, 0x0u);
    PE_StoreU32(0x800B69ECu, 0x0u);
    PE_StoreU32(0x800B69F0u, 0x0u);
    PE_StoreU16(0x800B6A3Cu, 0x0u);
    PE_StoreU16(0x800B6A38u, 0x0u);
    PE_StoreU16(0x800B69D0u, 0x0u);
    PE_StoreU16(0x8009D21Eu, 0x0u);
    PE_StoreU32(0x8009D2CCu, 0x0u);
    PE_StoreU16(0x8009D220u, 0x0u);
    PE_StoreU32(0x8009D2D0u, 0x0u);
    PE_StoreU16(0x8009D2A2u, 0x0u);
    PE_StoreU32(0x800BCD6Cu, 0x0u);
    PE_StoreU32(0x800B69B4u, 0x0u);
    PE_StoreU32(0x800BCD70u, 0x0u);
    PE_StoreU32(0x800B69B8u, 0x0u);
    PE_StoreU32(0x800BCD74u, 0x0u);
    PE_StoreU32(0x800B69BCu, 0x0u);
    PE_StoreU32(0x800B6A24u, 0x0u);
    PE_StoreU32(0x800B6A20u, 0x0u);
    PE_StoreU32(0x800B6A1Cu, 0x0u);
    PE_StoreU32(0x800B8628u, 0x0u);
    PE_StoreU16(0x800B69E0u, 0x0u);
    PE_StoreU16(0x800B69DEu, 0x0u);
    PE_StoreU16(0x800B69DCu, 0x0u);
    PE_StoreU16(0x800B69E4u, 0x0u);
    PE_StoreU32(0x800C0D90u, 0x3FCFu);
    PE_StoreU16(0x800C0D96u, 0x3FFFu);
    PE_StoreU16(0x800C0D94u, 0x3FFFu);
    PE_StoreU16(0x800C0D98u, 0x0u);
    PE_StoreU16(0x800C0D9Au, 0x0u);
    PE_StoreU16(0x800C0DA2u, 0x7FFFu);
    PE_StoreU16(0x800C0DA0u, 0x7FFFu);
    PE_StoreU32(0x800C0DA4u, 0x0u);
    PE_StoreU32(0x800C0DA8u, 0x1u);
    PE_StoreU16(0x800C0DAEu, 0x0u);
    PE_StoreU16(0x800C0DACu, 0x0u);
    PE_StoreU32(0x800C0DB0u, 0x0u);
    PE_StoreU32(0x800C0DB4u, 0x0u);
    func_80085F74(0x800C0D90u);
    PE_StoreU32(0x8009D268u, 0x0u);
    PE_StoreU32(0x8009D22Cu, 0x0u);
    PE_StoreU32(0x8009D2B8u, 0x0u);
    PE_StoreU32(0x800C0DD8u, 0x0u);
    PE_StoreU32(0x800C0DD4u, 0x0u);
    PE_StoreU32(0x800C0DD0u, 0x0u);
    PE_StoreU32(0x8009D2F4u, 0x0u);
    PE_StoreU32(0x8009D2DCu, 0x0u);
    PE_StoreU32(0x8009D2E0u, 0x0u);
}

static void bringup_clear_b(void)
{
    PE_StoreU32(0x800B6998u, 0x0u);
    PE_StoreU32(0x800B6994u, 0x0u);
    PE_StoreU32(0x800B6990u, 0x0u);
    PE_StoreU32(0x800B6A00u, 0x0u);
    PE_StoreU32(0x800B69FCu, 0x0u);
    PE_StoreU32(0x800B69F8u, 0x0u);
    PE_StoreU32(0x800BCD68u, 0x1u);
    PE_StoreU32(0x800BCD64u, 0x66A80000u);
    PE_StoreU32(0x800BCD5Cu, 0x0u);
    PE_StoreU32(0x800BCD58u, 0x0u);
    PE_StoreU32(0x800BCD54u, 0x0u);
    PE_StoreU32(0x800B6A28u, 0x3FFF0000u);
    PE_StoreU32(0x800B69C0u, 0x3FFF0000u);
    PE_StoreU32(0x800B6A2Cu, 0x0u);
    PE_StoreU32(0x800B69C4u, 0x0u);
    PE_StoreU16(0x800B6A40u, 0x0u);
    PE_StoreU16(0x800B69D8u, 0x0u);
}

/* Records 0..47: two passes over the 24 voices, each record cleared and its voice volume reset; records
 * 48..59: channel blocks 0xC..0x17. */
static void bringup_records(void)
{
    uint32_t i;

    for (i = 0; i < 48u; i++) {
        uint32_t r = RECORD_BASE + i * RECORD_STRIDE;
        PE_StoreU32(r, 0u);
        PE_StoreU32(r + 0xB8u, 0x18u);
        PE_StoreU16(r + 0x1Cu, 0u);
        PE_StoreU32(r + 0x18u, 0u);
        func_800862F4((int)(i % 24u), 0u, 0u, 0, 0);
    }
    for (i = 0; i < 12u; i++) {
        uint32_t r = RECORD_BASE + (48u + i) * RECORD_STRIDE;
        PE_StoreU32(r, 0u);
        PE_StoreU32(r + 0xB8u, 0xCu + i);
        PE_StoreU16(r + 0x1Cu, 1u);
        PE_StoreU32(r + 0x18u, 0u);
        PE_StoreU16(r + 0xA0u, 0x7F00u);
        PE_StoreU16(r + 0x3Cu, 0u);
        PE_StoreU16(r + 0x38u, 0u);
        PE_StoreU32(r + 4u, 0u);
    }
}

/* Stream (SPU) bring-up: installer, heap configuration, transfer address, the first SPU block write,
 * state / voice / channel record reset, reverb switched through mode 4 and on, SPU IRQ disabled, then the
 * root counter 2 setup + start and the counter event open + enable, each retried until it succeeds
 * (counter setup / start: zero = failure; OpenEvent: -1 = failure; EnableEvent: zero = failure).
 * Host contracts: the wait for the SPU DMA completion is PE_SpuDma_WaitForCompletion (the retail wait polls
 * a guest flag word the host model owns); every retry loop and the SPUCNT readback wait exit on
 * PE_Port_ShouldStop, tick the port clock per retry and carry a stuck-wait budget. */
void pe_plat_cdxspu_stream_bringup(void)
{
    uint32_t tries, cnt, reg;
    int handle;

    func_8007D15C();
    (void)func_80085A04(4, W_STREAM_HEAP);
    PE_StoreU32(W_B38C, 0u);
    PE_StoreU32(W_B418, 0u);
    (void)func_80085EB4(BRINGUP_TSA_BYTES);        /* transfer start = BYTE address 0x1010 (cdx-spu2) */
    func_800850F4(0x8009B7FCu, 0x20u);
    PE_SpuDma_WaitForCompletion();
    if (PE_Port_ShouldStop()) return;
    bringup_clear_a();
    bringup_records();
    bringup_clear_b();
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x80u);
    pe_plat_cdxspu_reverb_transition(4u);
    if (PE_Port_ShouldStop()) return;
    (void)func_80085A64(1);

    reg = PE_LoadU32(W_SPU_BASE) + R_SPUCNT;
    cnt = PE_LoadU16(reg);
    PE_StoreU16(reg, (uint16_t)(cnt & ~SPUCNT_IRQ));
    tries = 0;
    for (;;) {
        cnt = PE_LoadU16(PE_LoadU32(W_SPU_BASE) + R_SPUCNT);
        if (!(cnt & SPUCNT_IRQ)) break;
        if (!retry_step(&tries)) return;
    }
    if (PE_LoadU32(W_B438) != 0u) PE_StoreU32(W_B438, 0u);

    tries = 0;
    while (func_80085814((unsigned short)(COUNTER_ID & 0xFFFFu), COUNTER_TARGET, COUNTER_FLAGS) == 0)
        if (!retry_step(&tries)) return;
    tries = 0;
    while (func_800858E8(COUNTER_ID) == 0)
        if (!retry_step(&tries)) return;
    tries = 0;
    do {
        handle = PE_Event_Open(COUNTER_ID, STREAM_EVENT_SPEC, COUNTER_FLAGS, STREAM_EVENT_HANDLER);
        PE_StoreU32(W_EVENT_STREAM, (uint32_t)handle);
        if (PE_LoadU32(W_EVENT_STREAM) != 0xFFFFFFFFu) break;
        if (!retry_step(&tries)) return;
    } while (1);
    tries = 0;
    while (PE_Event_Enable((int)PE_LoadU32(W_EVENT_STREAM)) == 0)
        if (!retry_step(&tries)) return;
}

