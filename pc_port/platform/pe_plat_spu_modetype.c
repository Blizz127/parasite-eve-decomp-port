/* SPU reverb mode select (guest 0x8008CF70), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * modetype), with no SDK source.  Evidence: docs/evidence/native-spu-modetype-20261031/. */
#include "pe_plat/audio.h"
#include "pe_plat/state.h"
#include "pe_spu_dma.h"
#include "game_port.h"
#include "pe_guest_ram.h"

/* Pointer global holding the SPU register block base (0x1F801C00). */
#define SPU_BASE_PTR        0x8009B3FCu
#define SPU_CTRL_REG        0x1AAu  /* halfword, offset from the block base */
#define SPU_CTRL_BIT7       0x0080u /* cleared while the preset loads */
#define SPU_VOL_L_REG       0x184u  /* halfword pair, zeroed on a mode change */
#define SPU_VOL_R_REG       0x186u

/* Mode argument: low part selects the preset, bit 8 asks for the work-area clear. */
#define MODE_CLEAR_FLAG     0x100u
#define MODE_COUNT          10u

/* Per-mode tables: one size word each, one 68-byte attribute record each. */
#define SIZE_TABLE          0x8009B46Cu
#define PRESET_TABLE        0x8009C8C0u
#define PRESET_SIZE         0x44u

/* Reverb state words written on a successful change. */
#define STATE_SIZE          0x8009B398u /* word: size word of the current mode */
#define STATE_MODE          0x8009B3A0u /* word: current mode */
#define STATE_DEPTH_B       0x8009B3A8u /* word: 0x7F for modes 7 and 8, else 0 */
#define STATE_DEPTH_A       0x8009B3ACu /* word: 0x7F for mode 7 only, else 0 */
#define STATE_VOL_L         0x8009B3A4u /* halfword, zeroed */
#define STATE_VOL_R         0x8009B3A6u /* halfword, zeroed */

#define DEPTH_FULL          0x7Fu
#define FSET_INDEX          0xD1u /* _spu_FsetRXX index that takes the size word */

/* Fails (-1) for a mode outside 0..9 (flag bit aside), or when the mode's size
 * word lies in an allocated area (the table is read once for that check).
 * Otherwise, all state stores come before the first callee after the check:
 * the mode, its size and the two depth words, SPU control bit 7 dropped if it
 * was set, then the volumes zeroed (SPU registers and shadow halfwords).  Then
 * the preset record (first word forced to 0) goes through the attribute setter, the work area is
 * cleared if asked, the size word goes to FsetRXX, and bit 7 is restored from
 * a fresh read. */
int pe_plat_spu_set_reverb_mode_type(int mode)
{
    uint32_t arg = (uint32_t)mode;
    uint32_t m = arg & ~MODE_CLEAR_FLAG;
    uint32_t size, spu, ctrl, i;
    int ctrl_was_set;
    pe_addr_t rec;

    if (m >= MODE_COUNT)
        return -1;
    size = PE_LoadU32(SIZE_TABLE + m * 4u);
    if (pe_plat_spu_heap_contains(size))
        return -1;

    PE_StoreU32(STATE_MODE, m);
    PE_StoreU32(STATE_SIZE, size);
    PE_StoreU32(STATE_DEPTH_A, m == 7u ? DEPTH_FULL : 0u);
    PE_StoreU32(STATE_DEPTH_B, (m == 7u || m == 8u) ? DEPTH_FULL : 0u);

    spu = PE_LoadU32(SPU_BASE_PTR);
    ctrl_was_set = (PE_LoadU16(spu + SPU_CTRL_REG) & SPU_CTRL_BIT7) != 0u;
    if (ctrl_was_set) {
        spu = PE_LoadU32(SPU_BASE_PTR);
        ctrl = PE_LoadU16(spu + SPU_CTRL_REG);
        PE_StoreU16(spu + SPU_CTRL_REG, (uint16_t)(ctrl & ~SPU_CTRL_BIT7));
    }

    spu = PE_LoadU32(SPU_BASE_PTR);
    PE_StoreU16(spu + SPU_VOL_L_REG, 0u);
    PE_StoreU16(spu + SPU_VOL_R_REG, 0u);
    PE_StoreU16(STATE_VOL_L, 0u);
    PE_StoreU16(STATE_VOL_R, 0u);

    rec = PE_GuestFrame_Enter(PRESET_SIZE);
    for (i = 0u; i < PRESET_SIZE; i += 4u)
        PE_StoreU32(rec + i, PE_LoadU32(PRESET_TABLE + m * PRESET_SIZE + i));
    PE_StoreU32(rec, 0u); /* the record's first word (field mask) always goes out as 0 */
    pe_plat_spu_set_reverb_attr(rec);
    PE_GuestFrame_Leave(rec, PRESET_SIZE);

    if (arg & MODE_CLEAR_FLAG)
        PE_SpuReverb_ClearWorkArea((int)m);
    /* Port integration (not original behaviour): the host clear can request
     * a port stop; stop here as the replaced hand port did. */
    if (PE_Port_ShouldStop())
        return -1;
    pe_plat_spua_store_value((int)FSET_INDEX, PE_LoadU32(STATE_SIZE), 0);

    if (ctrl_was_set) {
        spu = PE_LoadU32(SPU_BASE_PTR);
        ctrl = PE_LoadU16(spu + SPU_CTRL_REG);
        PE_StoreU16(spu + SPU_CTRL_REG, (uint16_t)(ctrl | SPU_CTRL_BIT7));
    }
    return 0;
}
