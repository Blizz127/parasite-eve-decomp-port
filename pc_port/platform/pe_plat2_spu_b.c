/* SPU register / heap / callback leaves, batch state60a rows 14-26 (func_80085B34,
 * 80085BB4, 80085C44, 80085D84, 80085DC4, 80085EB4, 80085F14, 80085F44, 80085F74,
 * 800862F4, 80089EB8, 80089F08, 80089F28).
 * Written clean-room from the black-box vector sets state60a
 * (docs/evidence/native-state60-20261126) and psx-spx; no SDK source, decompiled
 * source, tables or disassembly were read.
 * Four behaviours, marked inline in pe_plat2_spu_b.c, follow the expected values of existing
 * native tests (test_native.c SPU_*_guest_dispatch and SPU_voice_volume_native; the reverb
 * tests), not the vectors.
 *
 * Guest RAM words used (little-endian): 0x8009B3FC SPU register base pointer
 * (0x1F801C00); 0x8009B464 allocation list head; 0x8009B424 address-unit shift;
 * 0x8009B420/428/42C transfer-address alignment state; 0x8009B414 transfer address
 * register shadow; 0x8009B438 IRQ callback; 0x8009B434 callback; 0x8009B38C /
 * 0x8009B418 transfer mode words; 0x8009B3A4/A6 reverb-volume shadow.
 * Behaviour per function is tabulated in
 * docs/evidence/native-state60-20261126/state60a/AUTHOR-REPORT-s60a2.md. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/spu_b.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/bios_file.h"

#define SPU_BASE_PTR   0x8009B3FCu
#define HEAP_HEAD      0x8009B464u
#define HEAP_SHIFT     0x8009B424u
#define XFER_MODE      0x8009B420u
#define XFER_UNIT      0x8009B428u
#define XFER_MASK      0x8009B42Cu
#define XFER_REG       0x8009B414u
#define IRQ_CB         0x8009B438u
#define CB_WORD        0x8009B434u
#define MODE_WORD      0x8009B38Cu
#define MODE_FLAG      0x8009B418u
#define PAIR_SHADOW    0x8009B3A4u

#define HEAP_STEP_BOUND 0x40000u   /* host guard: 8-byte entries in 2 MiB */
#define POLL_BUDGET     0x40000u

/* ---- allocation list walk (80085B34 / 80085BB4) -------------------------------- */
/* Entries are 8 bytes {word, size}.  A word with bit 31 set is skipped without its
 * size being read; a word with bit 30 set (bit 31 clear) ends the list (not found);
 * otherwise a below the word, or within [word, word+size), is a hit. */
static int heap_walk(uint32_t a)
{
    uint32_t p = PE_LoadU32(HEAP_HEAD), steps;
    /* A null list head is an empty list (no vector has one; the existing native tests
     * expect "no allocations" for it). */
    if (p == 0u) return 0;
    for (steps = 0; steps < HEAP_STEP_BOUND; steps++, p += 8u) {
        uint32_t w = PE_LoadU32(p);
        if (w & 0x80000000u) continue;
        if (w & 0x40000000u) return 0;
        if (a < w) return 1;
        if (a - w < PE_LoadU32(p + 4u)) return 1;
    }
    return 0;
}

int pe_plat_spub_heap_contains_raw(uint32_t address)
{
    return heap_walk(address);
}

int pe_plat_spub_heap_contains(uint32_t address)
{
    uint32_t sh = PE_LoadU32(HEAP_SHIFT) & 31u;
    return heap_walk(address << sh);
}

/* ---- SPUCNT bit 6 (80085C44) ----------------------------------------------------- */
/* SPUCNT is 1F801DAA (psx-spx); bit 6 = IRQ9 enable.  Read-modify-write, then read
 * the register back until bit 6 shows the written value (host contract: stop exit,
 * per-poll tick, stuck-wait budget; the vectors show exactly one read-back). */
static void spucnt_bit6(int set)
{
    uint32_t base = PE_LoadU32(SPU_BASE_PTR), budget = 0;
    uint16_t old = PE_LoadU16(base + 0x1AAu), nv;
    nv = set ? (uint16_t)(old | 0x40u) : (uint16_t)(old & ~0x40u);
    PE_StoreU16(base + 0x1AAu, nv);
    for (;;) {
        uint16_t rb = PE_LoadU16(base + 0x1AAu);
        if (!((rb ^ nv) & 0x40u)) break;
        if (PE_Port_ShouldStop()) break;
        if (budget++ >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            break;
        }
        pe_plat_sio_poll_tick();
    }
}

int pe_plat_spub_set_control_bit(int mode)
{
    if (mode == 0 || mode == 3) spucnt_bit6(0);
    if (mode == 1 || mode == 3) spucnt_bit6(1);
    return mode;
}

/* ---- callback words ----------------------------------------------------------- */
int pe_plat_spub_set_irq_callback(int callback)
{
    uint32_t old = PE_LoadU32(IRQ_CB);
    if ((uint32_t)callback != old) {
        PE_StoreU32(IRQ_CB, (uint32_t)callback);
        (void)pe_plat_interrupt_callback(9u, (uint32_t)callback);
    }
    return (int)old;
}

void pe_plat_spub_register_irq(int callback)
{
    (void)pe_plat_interrupt_callback(9u, (uint32_t)callback);
}

void pe_plat_spub_set_callback(int callback)
{
    uint32_t old = PE_LoadU32(CB_WORD);
    if ((uint32_t)callback != old) PE_StoreU32(CB_WORD, (uint32_t)callback);
}

/* ---- transfer address / mode ---------------------------------------------------- */
uint32_t pe_plat_spub_set_transfer_address(uint32_t address)
{
    uint32_t a = address, v;
    /* Addresses outside the usable SPU RAM return 0 with no memory access.  The vectors
     * only bracket the bounds (lower in (0xB61,0x116A], upper in [0x10000,0x1E28EC9)); the
     * exact limits 0x1010..0x7FFF8 are taken from the expectations of the existing native
     * test SPU_transfer_address_guest_dispatch (test_native.c), not from the vectors. */
    if (a < 0x1010u || a > 0x7FFF8u) return 0u;
    if (PE_LoadU32(XFER_MODE) != 0u) {
        uint32_t unit = PE_LoadU32(XFER_UNIT);
        /* psx-spx DIVU: dividend / 0 leaves HI = dividend, so the remainder is the address. */
        uint32_t rem = unit ? a % unit : a;
        if (rem != 0u) {
            uint32_t mask = PE_LoadU32(XFER_MASK);
            a = (a + unit) & ~mask;
        }
    }
    PE_StoreU16(XFER_REG, (uint16_t)(a >> (PE_LoadU32(HEAP_SHIFT) & 31u)));
    v = PE_LoadU16(XFER_REG);
    return v << (PE_LoadU32(HEAP_SHIFT) & 31u);
}

int pe_plat_spub_set_transfer_mode(int mode)
{
    int flag = (mode == 1);
    PE_StoreU32(MODE_WORD, (uint32_t)mode);
    PE_StoreU32(MODE_FLAG, (uint32_t)flag);
    return flag;
}

/* ---- common attributes (80085F74) ------------------------------------------------ */
/* attrs points at a guest record: mask word at +0; mask bit 0/1 master volume L/R
 * (u16 at +4/+6, written to base+0x180/0x182 with bit 15 cleared = volume mode);
 * bits 6,7 CD volume L/R (+0x10/+0x12 -> 0x1B0/0x1B2); bits 10,11 external volume
 * L/R (+0x1C/+0x1E -> 0x1B4/0x1B6); bits 8,9,12,13 flag words (+0x14,+0x18,+0x20,
 * +0x24; nonzero sets, zero clears) into SPUCNT bits 2,0,3,1.  Master-volume mode words
 * are at +8/+0xA (read only when mask bits 2/3 are set). */
static void put_reg16(uint32_t off, uint16_t v)
{
    PE_StoreU16(PE_LoadU32(SPU_BASE_PTR) + off, v);
}

static void cnt_flag(uint32_t field, uint16_t bit)
{
    uint32_t nz = PE_LoadU32(field);
    uint32_t base = PE_LoadU32(SPU_BASE_PTR);
    uint16_t cnt = PE_LoadU16(base + 0x1AAu);
    PE_StoreU16(base + 0x1AAu, nz ? (uint16_t)(cnt | bit) : (uint16_t)(cnt & ~bit));
}

/* Master volume with a mode word (mask bit 2 / 3 as well): mode 1..7 gives a sweep word,
 * bit 15 plus (mode-1) in bits 14-12, with the value clamped to 0..0x7F (signed); any
 * other mode (every one in the vectors) writes the value with bit 15 cleared.  The mode
 * 1..7 rule is taken from the expectations of the existing native test
 * SPU_common_attributes_guest_dispatch (test_native.c), not from the vectors. */
static uint16_t master_word(uint16_t v, uint16_t mode)
{
    if (mode >= 1u && mode <= 7u) {
        int sv = (int16_t)v;
        sv = sv < 0 ? 0 : (sv > 0x7F ? 0x7F : sv);
        return (uint16_t)(0x8000u + (uint32_t)(mode - 1u) * 0x1000u + (uint32_t)sv);
    }
    return (uint16_t)(v & 0x7FFFu);
}

void pe_plat_spub_set_common_attributes(uint32_t attrs)
{
    uint32_t m = PE_LoadU32(attrs);
    if (m == 0u) m = 0xFFFFFFFFu;   /* a zero mask selects every field (existing native test) */
    if (m & 0x1u) {
        uint16_t mode = (m & 0x4u) ? PE_LoadU16(attrs + 8u) : 0u;
        put_reg16(0x180u, master_word(PE_LoadU16(attrs + 4u), mode));
    }
    if (m & 0x2u) {
        uint16_t mode = (m & 0x8u) ? PE_LoadU16(attrs + 0xAu) : 0u;
        put_reg16(0x182u, master_word(PE_LoadU16(attrs + 6u), mode));
    }
    if (m & 0x40u)   put_reg16(0x1B0u, PE_LoadU16(attrs + 0x10u));
    if (m & 0x80u)   put_reg16(0x1B2u, PE_LoadU16(attrs + 0x12u));
    if (m & 0x400u)  put_reg16(0x1B4u, PE_LoadU16(attrs + 0x1Cu));
    if (m & 0x800u)  put_reg16(0x1B6u, PE_LoadU16(attrs + 0x1Eu));
    if (m & 0x100u)  cnt_flag(attrs + 0x14u, 0x4u);
    if (m & 0x200u)  cnt_flag(attrs + 0x18u, 0x1u);
    if (m & 0x1000u) cnt_flag(attrs + 0x20u, 0x8u);
    if (m & 0x2000u) cnt_flag(attrs + 0x24u, 0x2u);
}

/* ---- voice volume (800862F4) ----------------------------------------------------- */
/* Voice registers are 16 bytes apart from the base (psx-spx): +0 left, +2 right.
 * Mode 1..7 sets bit 15 (sweep) and puts mode-1 in bits 14-12 (OR into the value);
 * any other mode writes the value with bit 15 cleared.  The address arithmetic wraps
 * in 32 bits and is not range-checked (out-of-range voices store into RAM, as shown
 * by the vectors). */
static uint16_t volume_word(uint16_t v, int16_t mode)
{
    if (mode >= 1 && mode <= 7)
        return (uint16_t)(v | (0x8000u + (uint32_t)(mode - 1) * 0x1000u));
    return (uint16_t)(v & 0x7FFFu);
}

void pe_plat_spub_voice_volume(int voice, uint16_t left, uint16_t right,
                                int16_t left_mode, int16_t right_mode)
{
    uint32_t off = (uint32_t)voice << 4;
    PE_StoreU16(PE_LoadU32(SPU_BASE_PTR) + off, volume_word(left, left_mode));
    PE_StoreU16(PE_LoadU32(SPU_BASE_PTR) + off + 2u, volume_word(right, right_mode));
}

/* ---- control level / voice field / reverb pair ----------------------------------- */
int pe_plat_spub_set_control_level(int level)
{
    uint32_t base = PE_LoadU32(SPU_BASE_PTR);
    uint16_t old = PE_LoadU16(base + 0x1AAu);
    int lv = level < 0 ? 0 : (level > 63 ? 63 : level);
    PE_StoreU16(base + 0x1AAu, (uint16_t)((old & ~0x3F00u) | ((uint32_t)lv << 8)));
    return lv;
}

void pe_plat_spub_copy_voice_field(unsigned int index, uint32_t dest)
{
    uint32_t base = PE_LoadU32(SPU_BASE_PTR);
    PE_StoreU16(dest, PE_LoadU16(base + (index << 4) + 0xCu));
}

void pe_plat_spub_set_pair(uint16_t left, uint16_t right)
{
    uint32_t base = PE_LoadU32(SPU_BASE_PTR);
    PE_StoreU16(base + 0x184u, left);
    PE_StoreU16(base + 0x186u, right);
    PE_StoreU16(PAIR_SHADOW, left);
    PE_StoreU16(PAIR_SHADOW + 2u, right);
}
