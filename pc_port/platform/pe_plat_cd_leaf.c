/* CD volume, flush and stream-ring leaves (guest 0x8007B964 / 0x8007B9EC /
 * 0x8007BAC0 / 0x8007C394 / 0x8007C484), written clean-room from black-box vectors of the
 * original code (pc_port/tools/pe_cd_leaf_vectors.py), with no SDK source.
 * Evidence: docs/evidence/native-cd-leaf-20261031/. */
#include "pe_plat/cd.h"
#include "pe_guest_ram.h"
#include "pe_cdreg.h"

/* Pointer globals holding the CD controller register addresses
 * (0x1F801800..0x1F801803) and the SPU base (0x1F801C00). */
#define CD_REG_IDX_PTR      0x8009B27Cu /* CD index/status register (+0) */
#define CD_REG1_PTR         0x8009B280u /* CD register 1 (+1) */
#define CD_REG2_PTR         0x8009B284u /* CD register 2 (+2) */
#define CD_REG3_PTR         0x8009B288u /* CD register 3 (+3) */
#define CD_DELAY_PTR        0x8009B28Cu /* word: 0x1F801020 (CD-ROM delay/size) */
#define SPU_BASE_PTR        0x8009B290u /* word: SPU register block base */

/* Offsets within the SPU block. */
#define SPU_MAIN_VOL_L      0x180u /* 0x1F801D80: main volume left */
#define SPU_MAIN_VOL_R      0x182u /* main volume right */
#define SPU_CD_VOL_L        0x1B0u /* 0x1F801DB0: CD input volume left */
#define SPU_CD_VOL_R        0x1B2u /* CD input volume right */
#define SPU_CTRL            0x1AAu /* 0x1F801DAA: SPU control */
#define SPU_CUR_VOL_L       0x1B8u /* 0x1F801DB8: current main volume left */
#define SPU_CUR_VOL_R       0x1BAu /* current main volume right */

#define SPU_VOL_MAX         0x3FFFu
#define SPU_CTRL_INIT       0xC001u

/* Values written to the CD registers (index register selects the bank). */
#define CD_BANK_FIRST       2u    /* first index written: regs 2,3 take b0,b1 */
#define CD_BANK_SECOND      3u    /* second index: regs 1,2 take b2,b3, reg 3 = apply */
#define CD_APPLY            0x20u /* final write to register 3 */
#define CD_INITVOL_VAL      0x80u /* b0 and b2 in the init call (b1 = b3 = 0) */

/* Write the four ATV bytes at addr+0..3 and the apply command:
 * index 2: reg2 = b0, reg3 = b1; index 3: reg1 = b2, reg2 = b3, reg3 = 0x20. */
void pe_plat_cd_set_vol(uint32_t atv)
{
    PE_StoreU8(PE_LoadU32(CD_REG_IDX_PTR), CD_BANK_FIRST);
    PE_StoreU8(PE_LoadU32(CD_REG2_PTR), PE_LoadU8(atv + 0u));
    PE_StoreU8(PE_LoadU32(CD_REG3_PTR), PE_LoadU8(atv + 1u));
    PE_StoreU8(PE_LoadU32(CD_REG_IDX_PTR), CD_BANK_SECOND);
    PE_StoreU8(PE_LoadU32(CD_REG1_PTR), PE_LoadU8(atv + 2u));
    PE_StoreU8(PE_LoadU32(CD_REG2_PTR), PE_LoadU8(atv + 3u));
    PE_StoreU8(PE_LoadU32(CD_REG3_PTR), CD_APPLY);
}

/* If both current main volumes read zero, open the main volume; always open
 * the CD input volume, set the SPU control word, and set the CD attenuation
 * to 0x80,0x00 / 0x80,0x00.  Returns 0. */
int pe_plat_cd_init_vol(void)
{
    uint32_t spu = PE_LoadU32(SPU_BASE_PTR);

    if (PE_LoadU16(spu + SPU_CUR_VOL_L) == 0u &&
        PE_LoadU16(spu + SPU_CUR_VOL_R) == 0u) {
        spu = PE_LoadU32(SPU_BASE_PTR);
        PE_StoreU16(spu + SPU_MAIN_VOL_L, SPU_VOL_MAX);
        PE_StoreU16(spu + SPU_MAIN_VOL_R, SPU_VOL_MAX);
    }
    spu = PE_LoadU32(SPU_BASE_PTR);
    PE_StoreU16(spu + SPU_CD_VOL_L, SPU_VOL_MAX);
    PE_StoreU16(spu + SPU_CD_VOL_R, SPU_VOL_MAX);
    PE_StoreU16(spu + SPU_CTRL, SPU_CTRL_INIT);

    PE_StoreU8(PE_LoadU32(CD_REG_IDX_PTR), CD_BANK_FIRST);
    PE_StoreU8(PE_LoadU32(CD_REG2_PTR), CD_INITVOL_VAL);
    PE_StoreU8(PE_LoadU32(CD_REG3_PTR), 0u);
    PE_StoreU8(PE_LoadU32(CD_REG_IDX_PTR), CD_BANK_SECOND);
    PE_StoreU8(PE_LoadU32(CD_REG1_PTR), CD_INITVOL_VAL);
    PE_StoreU8(PE_LoadU32(CD_REG2_PTR), 0u);
    PE_StoreU8(PE_LoadU32(CD_REG3_PTR), CD_APPLY);
    return 0;
}

/* Stream ring: a table of 32-byte entries followed by the sector slots.  The
 * vectors show slot i at base + count * 32 + i * 2016 (a 2048-byte sector
 * less the 32-byte entry size). */
#define RING_BASE           0x800C0DC8u /* word: address of the entry table */
#define RING_COUNT          0x800C20C4u /* word: number of entries */
#define RING_CURRENT        0x800BE9ECu /* word: index of the next entry */
#define RING_FLAG           0x800C0DBCu /* word: nonzero = clear a wrapped entry */
#define RING_ENTRY_SIZE     32u
#define RING_SLOT_STRIDE    2016u
#define RING_ENTRY_TYPE     0x00u /* halfword */
#define RING_ENTRY_SECTORS  0x06u /* halfword: sectors held */

#define RING_TYPE_END       1u  /* end marker: wrap to the first entry */
#define RING_TYPE_READY     2u  /* data ready to hand out */
#define RING_TYPE_BUSY      4u  /* handed out / in use */

/* Address of the entry at the given index. */
static uint32_t ring_entry(uint32_t base, uint32_t idx)
{
    return base + idx * RING_ENTRY_SIZE;
}

/* A sector address maps to the entry whose slot it falls in (signed division
 * by the slot stride, truncating toward zero).  If that entry is busy, clear it and the
 * following entries' type words (the sector count is a signed halfword, so
 * a value >= 0x8000 clears none) and make the next index the one after the
 * last cleared.  Both halfwords are read before the type test.  The original
 * returns 1 when it does nothing and 0 when it frees (even with a negative
 * count); the port's prototype is void. */
void pe_plat_cd_st_free_ring(uint32_t sector)
{
    uint32_t count = PE_LoadU32(RING_COUNT);
    uint32_t base = PE_LoadU32(RING_BASE);
    int32_t off = (int32_t)(sector - base - count * RING_ENTRY_SIZE);
    int32_t idx = off / (int32_t)RING_SLOT_STRIDE;
    uint32_t entry = ring_entry(base, (uint32_t)idx);
    uint32_t type = PE_LoadU16(entry + RING_ENTRY_TYPE);
    int16_t n = (int16_t)PE_LoadU16(entry + RING_ENTRY_SECTORS);
    int32_t i;

    if (type != RING_TYPE_BUSY)
        return;
    for (i = 0; i < n; i++) {
        base = PE_LoadU32(RING_BASE);
        PE_StoreU16(ring_entry(base, (uint32_t)(idx + i)) + RING_ENTRY_TYPE, 0u);
    }
    PE_StoreU32(RING_CURRENT, (uint32_t)(idx + i)); /* i == max(n, 0) */
}

/* Hand out the next ready entry: *data = its sector slot, *header = the
 * entry; mark it busy.  An end-marker entry wraps the index to 0 (clearing
 * the marker if the flag word is nonzero) and the first entry is tried.
 * Returns 0 on success, 1 if nothing is ready. */
int pe_plat_cd_st_get_next(uint32_t data, uint32_t header)
{
    uint32_t idx = PE_LoadU32(RING_CURRENT);
    uint32_t base = PE_LoadU32(RING_BASE);
    uint32_t entry = ring_entry(base, idx);
    uint32_t count;

    if (PE_LoadU16(entry + RING_ENTRY_TYPE) != RING_TYPE_READY) {
        if (PE_LoadU16(entry + RING_ENTRY_TYPE) != RING_TYPE_END)
            return 1;
        if (PE_LoadU32(RING_FLAG) != 0u) {
            PE_StoreU32(RING_CURRENT, 0u);
            PE_StoreU16(entry + RING_ENTRY_TYPE, 0u);
        } else {
            PE_StoreU32(RING_CURRENT, 0u);
        }
        idx = PE_LoadU32(RING_CURRENT);
        base = PE_LoadU32(RING_BASE);
        entry = ring_entry(base, idx);
        if (PE_LoadU16(entry + RING_ENTRY_TYPE) != RING_TYPE_READY)
            return 1;
    }
    count = PE_LoadU32(RING_COUNT);
    base = PE_LoadU32(RING_BASE);
    idx = PE_LoadU32(RING_CURRENT);
    PE_StoreU16(entry + RING_ENTRY_TYPE, RING_TYPE_BUSY);
    PE_StoreU32(data, base + count * RING_ENTRY_SIZE +
                      idx * RING_SLOT_STRIDE);
    PE_StoreU32(header, entry);
    return 0;
}

/* CD_flush (guest 0x8007B9EC): select register bank 1, then while the pending
 * interrupt type (low three bits of register 3) is nonzero, acknowledge it
 * (register 3 = 7), write register 2 = 7, and read register 3 again.  Then
 * clear three state bytes (the original also reads the first once and
 * ignores it as far as the vectors show), return to bank 0, clear register 3
 * and store the 0x1325 delay value through the word at CD_DELAY_PTR. */
#define CD_FLUSH_BANK       1u
#define CD_INT_TYPE_MASK    7u    /* bits 0-2 of reg 3 (bank 1): pending type */
#define CD_INT_ACK          7u    /* written to reg 3 (bank 1) to acknowledge */
#define CD_INT_ENABLE_ALL   7u    /* written to reg 2 (bank 1) inside the loop */
#define CD_DELAY_VALUE      0x1325u
#define CD_STATE_B294       0x8009B294u /* byte: set to 2 */
#define CD_STATE_B295       0x8009B295u /* byte: cleared */
#define CD_STATE_B296       0x8009B296u /* byte: read, then cleared */

void pe_plat_cd_flush(void)
{
    PE_CdStoreU8(PE_LoadU32(CD_REG_IDX_PTR), CD_FLUSH_BANK);
    while ((PE_CdLoadU8(PE_LoadU32(CD_REG3_PTR)) & CD_INT_TYPE_MASK) != 0u) {
        PE_CdStoreU8(PE_LoadU32(CD_REG_IDX_PTR), CD_FLUSH_BANK);
        PE_CdStoreU8(PE_LoadU32(CD_REG3_PTR), CD_INT_ACK);
        PE_CdStoreU8(PE_LoadU32(CD_REG2_PTR), CD_INT_ENABLE_ALL);
    }
    (void)PE_LoadU8(CD_STATE_B296);
    PE_StoreU8(CD_STATE_B296, 0u);
    PE_StoreU8(CD_STATE_B295, 0u);
    PE_StoreU8(CD_STATE_B294, 2u);
    PE_CdStoreU8(PE_LoadU32(CD_REG_IDX_PTR), 0u);
    PE_CdStoreU8(PE_LoadU32(CD_REG3_PTR), 0u);
    PE_CdStoreU32(PE_LoadU32(CD_DELAY_PTR), CD_DELAY_VALUE);
}
