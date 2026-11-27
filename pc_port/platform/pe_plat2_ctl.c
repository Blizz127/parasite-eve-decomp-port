/*
 * pe_plat control-word leaves, batch state60b (17 retail leaves).
 *
 * PROVENANCE: written clean-room from the black-box vector sets state60b
 * (docs/evidence/native-state60-20261126) and psx-spx; no SDK source,
 * decompiled source, tables or disassembly were read.
 *
 * Every access goes through the PE_Load/PE_Store accessors, so the I/O page (psx-spx
 * "Memory Map": 0x1F801070 I_STAT/I_MASK, 0x1F801100.. root counters,
 * 0x1F801810/0x1F801814 GP0/GP1 / GPUREAD/GPUSTAT) reaches the modelled
 * hardware exactly as the vectors record it.  Guest RAM words used (all
 * located from the vectors; the pointer words always held the I/O address
 * in the vector sets):
 *   0x800945E6 u16  callback-state value             (func_80073DE8)
 *   0x80095674 ptr  -> interrupt mask register       (func_80073DF8, u16 read)
 *   0x800956EC u32  video mode                       (func_80074A14 / A28)
 *   0x8009574E u8   graphics debug level             (func_80074CB8)
 *   0x80095850 ptr  -> GP0 / GPUREAD word            (func_80076B58 / BE0)
 *   0x80095854 ptr  -> GP1 / GPUSTAT word            (func_8007633C, B20, B58, BE0)
 *   0x800A3348 u8[256]  last value written per GP1 command number (B20 / B44)
 *   0x8009B4AC u32  pad-initialised flag             (func_8007DEA4 / DEB0)
 *   0x8009B7CC ptr  -> I_STAT (the mask is the word at +4) (func_800858E8 / 85918)
 *   0x8009B7D0 ptr  -> root counter 0 register block (func_80085814 / 858B0 / 8594C)
 *   0x8009B7D4 u32[] interrupt-mask bit per index    (func_800858E8 / 85918)
 *
 * Derivation table: docs/evidence/native-state60-20261126/state60b/AUTHOR-REPORT.md.
 *
 * Behaviour notes (vectors):
 *  - The register-pointer words are loaded only on the paths the vectors show
 *    them loaded (e.g. the counter leaves never touch 0x8009B7D0 when the
 *    counter number is out of range; the transfer leaf never reads the GP0
 *    pointer for a zero count).
 *  - Counter numbers are 16-bit: the leaves look at the low halfword of the
 *    argument (0x10000 behaves as 0, 0xFFFFFFFF as 0xFFFF).
 *  - func_80085814 computes the mode word from the flags; see counter_mode().
 *    The bit layout follows psx-spx "Timers" (mode bit 3 reset on target,
 *    bit 4 IRQ on target, bit 6 repeat IRQ, bits 8-9 clock source, bit 0
 *    sync enable).
 *  - Undefined edges, none of which a vector shows: func_80076B58 with a
 *    negative count writes only the GP1 word (count <= 0 transfers nothing).
 *    The table indices of func_80076B44 / func_800858E8 / func_80085918 are
 *    unchecked in the vectors (any 32-bit / 16-bit value reads the guest word
 *    it addresses), so they are not bounds-checked here either.
 *
 * No waits, polls or callees: the host stop / tick contracts do not apply.
 */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/ctl.h"

#define W_CALLBACK_STATE   0x800945E6u
#define P_IRQ_MASK         0x80095674u
#define W_VIDEO_MODE       0x800956ECu
#define B_GRAPH_DEBUG      0x8009574Eu
#define P_GP0              0x80095850u
#define P_GP1              0x80095854u
#define TAB_GP1_CACHE      0x800A3348u
#define W_PAD_INIT         0x8009B4ACu
#define P_ISTAT            0x8009B7CCu
#define P_RCNT             0x8009B7D0u
#define TAB_MASK_BITS      0x8009B7D4u

#define GP1_DMA_OFF        0x04000000u
#define GP1_GET_INFO       0x10000000u

uint16_t pe_plat_ctl_callback_state(void)
{
    return PE_LoadU16(W_CALLBACK_STATE);
}

uint16_t pe_plat_ctl_irq_mask_read(void)
{
    return PE_LoadU16(PE_LoadU32(P_IRQ_MASK));
}

uint32_t pe_plat_ctl_video_set_mode(uint32_t mode)
{
    uint32_t old = PE_LoadU32(W_VIDEO_MODE);
    PE_StoreU32(W_VIDEO_MODE, mode);
    return old;
}

int32_t pe_plat_ctl_video_mode(void)
{
    return (int32_t)PE_LoadU32(W_VIDEO_MODE);
}

uint8_t pe_plat_ctl_graph_debug(void)
{
    return PE_LoadU8(B_GRAPH_DEBUG);
}

int pe_plat_ctl_gpu_status_read(void)
{
    return (int)PE_LoadU32(PE_LoadU32(P_GP1));
}

/* GP1 command write; the low byte of the word is remembered under the
 * command number (top byte). */
void pe_plat_ctl_gpu_control_write(unsigned int command)
{
    PE_StoreU32(PE_LoadU32(P_GP1), command);
    PE_StoreU8(TAB_GP1_CACHE + (command >> 24), (uint8_t)command);
}

unsigned char pe_plat_ctl_gpu_control_cached(int index)
{
    return PE_LoadU8(TAB_GP1_CACHE + (uint32_t)index);
}

/* GP1(0x10 | arg) "get GPU info", then the answer from GPUREAD (24 bits). */
unsigned int pe_plat_ctl_gpu_query(unsigned int query)
{
    uint32_t gp1 = PE_LoadU32(P_GP1);
    uint32_t gp0 = PE_LoadU32(P_GP0);
    PE_StoreU32(gp1, query | GP1_GET_INFO);
    return PE_LoadU32(gp0) & 0x00FFFFFFu;
}

/* GP1(0x04) (DMA direction off), then `count` words from guest RAM pushed
 * through GP0 by the CPU, in order. */
int pe_plat_ctl_gpu_transfer_words(uint32_t source, int count)
{
    PE_StoreU32(PE_LoadU32(P_GP1), GP1_DMA_OFF);
    if (count > 0) {
        uint32_t gp0 = PE_LoadU32(P_GP0);
        int i;
        for (i = 0; i < count; i++)
            PE_StoreU32(gp0, PE_LoadU32(source + 4u * (uint32_t)i));
    }
    return 0;
}

void pe_plat_ctl_pad_set_initialized(uint32_t value)
{
    PE_StoreU32(W_PAD_INIT, value);
}

int32_t pe_plat_ctl_pad_initialized(void)
{
    return (int32_t)PE_LoadU32(W_PAD_INIT);
}

/* ---- root counters (psx-spx "Timers": base + n*0x10: count, +4 mode, +8 target) ---- */

#define RCNT_COUNT  0x0u
#define RCNT_MODE   0x4u
#define RCNT_TARGET 0x8u
#define RCNT_N      3u

/* Mode word for a counter from the caller's flags (derived from the 308
 * valid-counter cases of func_80085814's vectors):
 *   base 0x48: reset counter on target (bit 3) + repeat IRQ mode (bit 6)
 *   flags bit 12            -> mode bit 4 (IRQ when the target is reached)
 *   counters 0 and 1:  flags bit 4 -> mode bit 0 (sync enable);
 *                      flags bit 0 clear -> mode bit 8 (alternate clock source)
 *   counter 2:         flags bit 0 clear -> mode bit 9 (clock source sysclk/8) */
static uint16_t counter_mode(unsigned counter, uint32_t flags)
{
    uint32_t mode = 0x48u;
    if (flags & 0x1000u) mode |= 0x10u;
    if (counter < 2u) {
        if (flags & 0x10u) mode |= 0x01u;
        if (!(flags & 0x01u)) mode |= 0x100u;
    } else if (!(flags & 0x01u)) {
        mode |= 0x200u;
    }
    return (uint16_t)mode;
}

int pe_plat_ctl_counter_setup(unsigned short counter, short target, int flags)
{
    uint32_t reg;
    if (counter >= RCNT_N) return 0;
    reg = PE_LoadU32(P_RCNT) + 0x10u * counter;
    PE_StoreU16(reg + RCNT_MODE, 0);
    PE_StoreU16(reg + RCNT_TARGET, (uint16_t)target);
    PE_StoreU16(reg + RCNT_MODE, counter_mode(counter, (uint32_t)flags));
    return 1;
}

int pe_plat_ctl_counter_get(int counter)
{
    uint16_t n = (uint16_t)counter;
    if (n >= RCNT_N) return 0;
    return PE_LoadU16(PE_LoadU32(P_RCNT) + 0x10u * n + RCNT_COUNT);
}

/* I_MASK |= the bit for the (16-bit) index; 1 for a real counter, else 0.
 * The mask register is the word after the interrupt-status pointer. */
int pe_plat_ctl_counter_start(unsigned int counter)
{
    uint16_t n = (uint16_t)counter;
    uint32_t bit = PE_LoadU32(TAB_MASK_BITS + 4u * n);
    uint32_t mask = PE_LoadU32(P_ISTAT) + 4u;
    PE_StoreU32(mask, PE_LoadU32(mask) | bit);
    return n < RCNT_N;
}

/* I_MASK &= ~bit for the (16-bit) index; always 1. */
int pe_plat_ctl_counter_stop(unsigned int counter)
{
    uint16_t n = (uint16_t)counter;
    uint32_t bit = PE_LoadU32(TAB_MASK_BITS + 4u * n);
    uint32_t mask = PE_LoadU32(P_ISTAT) + 4u;
    PE_StoreU32(mask, PE_LoadU32(mask) & ~bit);
    return 1;
}

int pe_plat_ctl_counter_reset(int counter)
{
    uint16_t n = (uint16_t)counter;
    if (n >= RCNT_N) return 0;
    PE_StoreU16(PE_LoadU32(P_RCNT) + 0x10u * n + RCNT_COUNT, 0);
    return 1;
}
