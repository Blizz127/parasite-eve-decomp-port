/* libspu leaves of state60 batch state60a (13 functions), written clean-room from
 * the black-box vector sets state60a (docs/evidence/native-state60-20261126) and
 * psx-spx; no SDK source, decompiled source, tables or disassembly were read.
 * The behaviour below is exactly what the vectors show; no claim is made about
 * the original's source.  The derivation table with vector evidence for every
 * line is docs/evidence/native-state60-20261126/state60a/AUTHOR-REPORT.md.
 *
 * Guest words used (all little-endian guest RAM, read at every use):
 *   0x8009B3FC  pointer to the SPU register block (1F801C00)
 *   0x8009B40C  pointer to DPCR (1F8010F0)        0x8009B410  pointer to SPU_DELAY (1F801014)
 *   0x8009B414  halfword: SPU transfer address register value (address / 8)
 *   0x8009B418/1C, B430, B434, B438  state words cleared / set by initialize
 *   0x8009B420  mode word, B424 shift, B428 unit (round-up add), B42C unit mask
 *   0x8009B390  reverb flag, B394 / B398 reverb parameters, B3EC "initialized" flag
 *   0x8009B45C/B460/B464  heap bookkeeping (count, zero, heap-block table pointer)
 *
 * Host contracts (port facts, not behaviour of the original):
 *  - every blocking wait on a status register exits when PE_Port_ShouldStop() is set
 *    and gives pe_plat_sio_poll_tick() once per poll (the port clock only moves on ticks);
 *  - the waits the vectors bound keep the original's bound exactly: 3841 polls, then the
 *    recorded printf and the recorded continuation;
 *  - the unbounded FIFO-busy wait (SPUSTAT bit 10 after a chunk) is not bounded by any
 *    vector; it carries a stuck-wait budget of 0x40000 polls that ends in
 *    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY).
 *  - func_8007DCAC has no observable output (the vectors record a pure delay); it is a
 *    bounded host wait of 15 poll ticks (15 x 64 cycles = 960 cycles, the order of the
 *    recorded 910 retired instructions) with the stop contract. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "pe_guest_decomp.h"
#include "pe_plat/spu_a.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/bios_event.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/bios_file.h"

#define W_SPU_BASE    0x8009B3FCu
#define W_DPCR_PTR    0x8009B40Cu
#define W_DELAY_PTR   0x8009B410u
#define H_XFER_ADDR   0x8009B414u
#define W_STATE_B418  0x8009B418u
#define W_STATE_B41C  0x8009B41Cu
#define W_MODE        0x8009B420u
#define W_SHIFT       0x8009B424u
#define W_UNIT        0x8009B428u
#define W_UNIT_MASK   0x8009B42Cu
#define W_STATE_B430  0x8009B430u
#define W_STATE_B434  0x8009B434u
#define W_STATE_B438  0x8009B438u
#define W_REVERB      0x8009B390u
#define W_REVERB_P1   0x8009B394u
#define W_REVERB_P2   0x8009B398u
#define W_INIT_FLAG   0x8009B3ECu
#define W_EVENT       0x8009B384u
#define W_HEAP_COUNT  0x8009B45Cu
#define W_HEAP_ZERO   0x8009B460u
#define W_HEAP_TABLE  0x8009B464u
#define SILENCE_TABLE 0x8009B43Cu      /* 8 halfwords copied to the first SPU RAM block */
#define RAM_CLEAR     0x800B6900u      /* 10 halfwords cleared by initialize */

#define SPUSTAT_MASK  0x7FFu           /* SPUSTAT bits 0-10 (psx-spx: mode, IRQ, DMA request, transfer busy) */
#define SPUSTAT_BUSY  0x400u           /* SPUSTAT bit 10: data transfer busy */
#define SPUCNT_XFER   0x30u            /* SPUCNT bits 5-4: sound RAM transfer mode */
#define SPUCNT_MWRITE 0x10u            /* ... 1 = manual write */
#define WAIT_POLLS    3841u            /* polls the vectors show before a wait gives up */
#define STUCK_BUDGET  0x40000u
#define FMT_TIMEOUT   0x80011C4Cu
#define STR_INIT_WAIT 0x80011C5Cu
#define STR_FIFO_WAIT 0x80011C80u

/* SPU register block offsets (psx-spx "SPU Control Registers"). */
#define R_MAIN_VOL_L  0x180u
#define R_MAIN_VOL_R  0x182u
#define R_REVERB_VOL_L 0x184u
#define R_REVERB_VOL_R 0x186u
#define R_KON_LO      0x188u
#define R_KON_HI      0x18Au
#define R_KOFF_LO     0x18Cu
#define R_KOFF_HI     0x18Eu
#define R_PMON_LO     0x190u
#define R_PMON_HI     0x192u
#define R_NON_LO      0x194u
#define R_NON_HI      0x196u
#define R_EON_LO      0x198u
#define R_EON_HI      0x19Au
#define R_XFER_ADDR   0x1A6u
#define R_XFER_FIFO   0x1A8u
#define R_SPUCNT      0x1AAu
#define R_XFER_CTRL   0x1ACu
#define R_SPUSTAT     0x1AEu
#define R_CD_VOL_L    0x1B0u
#define R_CD_VOL_R    0x1B2u
#define R_EXT_VOL_L   0x1B4u
#define R_EXT_VOL_R   0x1B6u

static uint32_t spu_base(void) { return PE_LoadU32(W_SPU_BASE); }
static uint16_t reg_load(uint32_t off) { return PE_LoadU16(spu_base() + off); }
static void reg_store(uint32_t off, uint16_t v) { PE_StoreU16(spu_base() + off, v); }

/* Read-modify-write of a register through a RAM pointer word (re-read at each use). */
static uint32_t ptr_load(uint32_t ptr_word) { return PE_LoadU32(PE_LoadU32(ptr_word)); }
static void ptr_store(uint32_t ptr_word, uint32_t v) { PE_StoreU32(PE_LoadU32(ptr_word), v); }

/* ------------------------------------------------------------------ waits */

/* Wait until (SPUSTAT & mask) == want, at most WAIT_POLLS polls.  Returns 1 when the
 * condition was met, 0 on timeout, -1 when a stop was requested. */
static int wait_status_equal(uint32_t mask, uint32_t want)
{
    uint32_t n;
    for (n = 0; n < WAIT_POLLS; n++) {
        uint32_t stat;
        if (PE_Port_ShouldStop()) return -1;
        stat = reg_load(R_SPUSTAT);
        if ((stat & mask) == want) return 1;
        pe_plat_sio_poll_tick();
    }
    return 0;
}

/* Wait while SPUSTAT bit 10 (transfer busy) is set; unbounded in the vectors, so the
 * host stuck-wait budget applies.  Returns 1 done, -1 stop. */
static int wait_transfer_idle(void)
{
    uint32_t n = 0;
    for (;;) {
        uint32_t stat;
        if (PE_Port_ShouldStop()) return -1;
        stat = reg_load(R_SPUSTAT);
        if (!(stat & SPUSTAT_BUSY)) return 1;
        if (n++ >= STUCK_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
        pe_plat_sio_poll_tick();
    }
}

/* ------------------------------------------------------ manual FIFO write */

/* Manual sound-RAM write of `size` bytes from guest RAM: transfer address register from the
 * halfword at 0x8009B414, the data through the FIFO register in chunks of at most 64 bytes
 * (ceil(n/2) halfwords, odd tails read as a whole halfword), each chunk followed by SPUCNT
 * transfer mode = manual write and a wait for SPUSTAT bit 10 to clear; finally the transfer
 * mode is cleared and SPUSTAT (bits 0-10) is waited back to the value it had on entry.
 * Returns 0 normally, -1 when a stop ended it. */
static int fifo_write(uint32_t source, uint32_t size)
{
    uint32_t entry_status = reg_load(R_SPUSTAT);
    uint32_t remaining = size;
    uint32_t cursor = source;
    int w;

    reg_store(R_XFER_ADDR, PE_LoadU16(H_XFER_ADDR));
    while (remaining) {
        uint32_t chunk = remaining > 64u ? 64u : remaining;
        uint32_t halves = (chunk + 1u) / 2u, i;
        uint16_t cnt;
        for (i = 0; i < halves; i++)
            reg_store(R_XFER_FIFO, PE_LoadU16(cursor + 2u * i));
        cnt = reg_load(R_SPUCNT);
        reg_store(R_SPUCNT, (uint16_t)((cnt & ~SPUCNT_XFER) | SPUCNT_MWRITE));
        if (wait_transfer_idle() < 0) return -1;
        cursor += chunk;
        remaining -= chunk;
    }
    {
        uint16_t cnt = reg_load(R_SPUCNT);
        reg_store(R_SPUCNT, (uint16_t)(cnt & ~SPUCNT_XFER));
    }
    w = wait_status_equal(SPUSTAT_MASK, entry_status & SPUSTAT_MASK);
    if (w < 0) return -1;
    if (w == 0) PE_Printf_Guest(FMT_TIMEOUT, STR_FIFO_WAIT, 0, 0, 0, 0, 0, 0);
    return 0;
}

void pe_plat_spua_write_io(uint32_t source, uint32_t size)
{
    /* Host contract: an odd source address would raise an address-error exception on the CPU
     * (psx-spx, COP0 exceptions) at the first halfword load; the port names it as a boundary
     * stop instead of reading.  No vector has an odd source. */
    if (size != 0u && (source & 1u)) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    (void)fifo_write(source, size);
}

/* ---------------------------------------------------------- initialization */

int pe_plat_spua_initialize(int mode)
{
    uint32_t i, v;
    int w;

    v = ptr_load(W_DPCR_PTR);
    ptr_store(W_DPCR_PTR, v | 0xB0000u);          /* DPCR: DMA4 (SPU) enable + priority 3 */
    PE_StoreU32(W_STATE_B418, 0);
    PE_StoreU32(W_STATE_B41C, 0);
    PE_StoreU16(H_XFER_ADDR, 0);
    reg_store(R_MAIN_VOL_L, 0);
    reg_store(R_MAIN_VOL_R, 0);
    reg_store(R_SPUCNT, 0);
    reg_store(R_MAIN_VOL_L, 0);
    reg_store(R_MAIN_VOL_R, 0);
    w = wait_status_equal(SPUSTAT_MASK, 0);
    if (w < 0) return 0;
    if (w == 0) PE_Printf_Guest(FMT_TIMEOUT, STR_INIT_WAIT, 0, 0, 0, 0, 0, 0);
    PE_StoreU32(W_MODE, 2);
    PE_StoreU32(W_SHIFT, 3);
    PE_StoreU32(W_UNIT, 8);
    PE_StoreU32(W_UNIT_MASK, 7);
    reg_store(R_XFER_CTRL, 4);                    /* sound RAM data transfer control = 0004h */
    reg_store(R_REVERB_VOL_L, 0);
    reg_store(R_REVERB_VOL_R, 0);
    reg_store(R_KOFF_LO, 0xFFFF);
    reg_store(R_KOFF_HI, 0xFFFF);
    reg_store(R_EON_LO, 0);
    reg_store(R_EON_HI, 0);
    for (i = 0; i < 10; i++) PE_StoreU16(RAM_CLEAR + 2u * i, 0);

    if (mode == 0) {                              /* cold start: clear the whole register file */
        uint32_t voice;
        PE_StoreU16(H_XFER_ADDR, 0x200);          /* first free sound RAM block (0x1000 / 8) */
        reg_store(R_PMON_LO, 0);
        reg_store(R_PMON_HI, 0);
        reg_store(R_NON_LO, 0);
        reg_store(R_NON_HI, 0);
        reg_store(R_CD_VOL_L, 0);
        reg_store(R_CD_VOL_R, 0);
        reg_store(R_EXT_VOL_L, 0);
        reg_store(R_EXT_VOL_R, 0);
        if (fifo_write(SILENCE_TABLE, 16) < 0) return 0;
        for (voice = 0; voice < 24; voice++) {
            uint32_t b = 0x10u * voice;
            reg_store(b + 0x0, 0);                /* volume L */
            reg_store(b + 0x2, 0);                /* volume R */
            reg_store(b + 0x4, 0x3FFF);           /* pitch */
            reg_store(b + 0x6, 0x200);            /* start address */
            reg_store(b + 0x8, 0);                /* ADSR lo */
            reg_store(b + 0xA, 0);                /* ADSR hi */
        }
        reg_store(R_KON_LO, 0xFFFF);
        reg_store(R_KON_HI, 0xFF);
        reg_store(R_KOFF_LO, 0xFFFF);
        reg_store(R_KOFF_HI, 0xFF);
    }
    PE_StoreU32(W_STATE_B430, 1);
    reg_store(R_SPUCNT, 0xC000);                  /* SPU enable + unmute */
    PE_StoreU32(W_STATE_B434, 0);
    PE_StoreU32(W_STATE_B438, 0);
    return 0;
}

/* ------------------------------------------------------- register setters */

/* Store the 16-bit value (value >> shift when shift_mode != 0) at register index `index`
 * (halfword index from the block base; 32-bit wrap of index * 2). */
void pe_plat_spua_store_value(int index, unsigned int value, int shift_mode)
{
    unsigned int v = value;
    if (shift_mode) v = value >> (PE_LoadU32(W_SHIFT) & 31u);
    PE_StoreU16(spu_base() + (uint32_t)index * 2u, (uint16_t)v);
}

/* Round an address up to the transfer unit (when the mode word is nonzero), convert it to
 * register units with the shift word and store it at register index `index` (index -1: no
 * store, the 16-bit register value is returned instead).  Returns the rounded address. */
int pe_plat_spua_set_address_units(int index, unsigned int address)
{
    uint32_t a = address, shift;
    if (PE_LoadU32(W_MODE) != 0) {
        uint32_t unit = PE_LoadU32(W_UNIT), rem;
        if (unit == 0) {                          /* unsigned remainder by zero: break 7, HI = dividend */
            PE_Decomp_GuestTrapDivZero("func_8007DB24");
            rem = a;
        } else {
            rem = a % unit;
        }
        if (rem != 0) a = (a + unit) & ~PE_LoadU32(W_UNIT_MASK);
    }
    shift = PE_LoadU32(W_SHIFT) & 31u;
    if (index == -1) return (int)((a >> shift) & 0xFFFFu);
    PE_StoreU16(spu_base() + (uint32_t)index * 2u, (uint16_t)(a >> shift));
    return (int)a;
}

/* An address the host can load from: the I/O page, the scratchpad or main RAM (KUSEG, KSEG0,
 * KSEG1 aliases of the 2 MiB).  Used only where the vectors show a skipped load. */
static int load_address_valid(uint32_t a)
{
    uint32_t seg = a & 0xE0000000u;
    if (seg != 0u && seg != 0x80000000u && seg != 0xA0000000u) return 0;
    if (((a & 0x1FFFF000u) == 0x1F801000u)) return 1;
    if (((a & 0x7FFFFFFFu) - 0x1F800000u) < 0x400u) return 1;
    return (a & 0x1FFFFFFFu) < 0x200000u;
}

/* Read register index `index` and scale it left by the shift word (mode -1: unscaled). */
int pe_plat_spua_address_value(unsigned int index, int mode)
{
    uint32_t addr = spu_base() + index * 2u;
    uint32_t v = load_address_valid(addr) ? PE_LoadU16(addr) : 0u;
    if (mode == -1) return (int)v;
    return (int)(v << (PE_LoadU32(W_SHIFT) & 31u));
}

/* ------------------------------------------------------------ DMA control */

/* DPCR channel 4 (SPU) priority field (bits 16-18, psx-spx): clear it, then set it again to
 * 3 when mode != 0, else 5.  The register is re-read through its pointer word in between. */
void pe_plat_spua_set_dma_control_priority(int mode)
{
    uint32_t v = ptr_load(W_DPCR_PTR);
    ptr_store(W_DPCR_PTR, v & ~0x70000u);
    v = ptr_load(W_DPCR_PTR);
    ptr_store(W_DPCR_PTR, v | ((mode ? 3u : 5u) << 16));
}

/* SPU_DELAY (1F801014) through the pointer word: bits 24-27 cleared, then bit 29 set;
 * the _enabled variant also sets bit 25. */
void pe_plat_spua_dma_priority(void)
{
    uint32_t v = ptr_load(W_DELAY_PTR);
    ptr_store(W_DELAY_PTR, (v & 0xF0FFFFFFu) | 0x20000000u);
}

void pe_plat_spua_dma_priority_enabled(void)
{
    uint32_t v = ptr_load(W_DELAY_PTR);
    ptr_store(W_DELAY_PTR, (v & 0xF0FFFFFFu) | 0x22000000u);
}

/* Pure delay of the original (no loads, stores or calls).  Host: 15 bounded poll ticks. */
void pe_plat_spua_wait_cycles(void)
{
    unsigned i;
    for (i = 0; i < 15u; i++) {
        if (PE_Port_ShouldStop()) return;
        pe_plat_sio_poll_tick();
    }
}

void pe_plat_spua_register_dma_callback(uint32_t callback)
{
    (void)func_80073CF4(4u, callback);            /* DMA channel 4 callback slot */
}

void pe_plat_spua_shutdown(void)
{
    if (PE_LoadU32(W_INIT_FLAG) != 1u) return;
    PE_StoreU32(W_INIT_FLAG, 0);
    (void)func_80072714();                        /* enter critical section */
    PE_StoreU32(W_STATE_B434, 0);
    PE_StoreU32(W_STATE_B438, 0);
    (void)func_80073CF4(4u, 0u);                  /* clear the DMA4 callback */
    (void)func_800857E4((int)PE_LoadU32(W_EVENT));
    (void)func_800857F4((int)PE_LoadU32(W_EVENT));
    func_80072724();                              /* exit critical section */
}

/* ------------------------------------------------------------------- heap */

/* Heap block table walk used by set_reverb_enabled (derived from the vector sets of the
 * neighbouring heap queries 80085B34 / 80085BB4, read-only, 249 + 313 cases): the table
 * pointer word 0x8009B464 points at 4-byte-aligned records.  A record whose word has bit 31
 * set is 8 bytes long and skipped; bit 30 (with bit 31 clear) ends the table with 0; otherwise
 * the word (bits 0-29) is a start and the next word a size: 1 when address < start or
 * address < start + size (64-bit sum), else the walk continues 8 bytes on. */
static int heap_raw_query(uint32_t address)
{
    uint32_t p = PE_LoadU32(W_HEAP_TABLE);
    if (p == 0u) return 0;                        /* host guard: no table configured (no vector has one) */
    for (;;) {
        uint32_t w = PE_LoadU32(p);
        uint32_t start;
        if (w & 0x80000000u) { p += 8u; continue; }
        if (w & 0x40000000u) return 0;
        start = w & 0x3FFFFFFFu;
        if (address < start) return 1;
        if ((uint64_t)address < (uint64_t)start + PE_LoadU32(p + 4u)) return 1;
        p += 8u;
    }
}

/* heap_raw_query on an address given in register units: scaled left by the shift word. */
static int heap_unit_query(uint32_t address)
{
    return heap_raw_query(address << (PE_LoadU32(W_SHIFT) & 31u));
}

int pe_plat_spua_configure_heap(int count, uint32_t buffer)
{
    uint32_t shift;
    if (count <= 0) return 0;                     /* vectors: 0 only; negative counts follow the port's existing contract */
    shift = PE_LoadU32(W_SHIFT) & 31u;
    PE_StoreU32(buffer, 0x40001010u);              /* first record: end-of-table flag + reserved start */
    PE_StoreU32(W_HEAP_TABLE, buffer);
    PE_StoreU32(W_HEAP_ZERO, 0);
    PE_StoreU32(W_HEAP_COUNT, (uint32_t)count);
    PE_StoreU32(buffer + 4u, (0x10000u << shift) - 0x1010u);   /* sound RAM size minus the reserved 0x1010 */
    return count;
}

/* Reverb master enable (SPUCNT bit 7).  mode 0: off; mode 1: on when the reverb parameter
 * word B394 == 1 or the reverb work area word B398 is not a heap-unit hit; any other mode only
 * returns the current flag.  The flag word is stored and then re-read as the result. */
int pe_plat_spua_set_reverb_enabled(int mode)
{
    if ((uint32_t)mode <= 1u) {
        uint32_t on = 0, cnt;
        if (mode == 1)
            on = (PE_LoadU32(W_REVERB_P1) == 1u) || !heap_unit_query(PE_LoadU32(W_REVERB_P2));
        cnt = reg_load(R_SPUCNT);
        PE_StoreU32(W_REVERB, on);
        reg_store(R_SPUCNT, (uint16_t)(on ? (cnt | 0x80u) : (cnt & ~0x80u)));
    }
    return (int)PE_LoadU32(W_REVERB);
}
