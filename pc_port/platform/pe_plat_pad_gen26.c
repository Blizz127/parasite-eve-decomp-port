/* libpad pad-record step (guest 0x80084C4C), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen26; vectors.txt sha256
 * 319985f8dcbd7202d657e0abc6b7c8cac96f84b19f3d36b6534b7bdc34f5f2d3, 6000
 * cases, checked by pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * An independent re-implementation: behaviour is exactly the guest loads,
 * stores and callee/slot calls seen in the vectors.
 *
 * The record step is installed in slot 0x8009B744.  It takes the bytes the
 * record last received (buffer pointer at +0x3C), mirrors them into the
 * record's own buffer (+0x30), settles the record's busy / reset condition
 * and then advances the state byte at +0x46 (one state-machine step).
 *
 * Beyond the checker's set test, the ordered load list (address, width) and the
 * ordered store list (address, width, value) of this file were compared with
 * every vector's L / S lists and are identical in all 6000 cases.
 *
 * The function is void in the port (the original's return register is a
 * leftover that none of its callers use).  It polls no hardware itself. */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_plat/pad.h"

#define ADDR_RESET_SLOT   0x8009B728u  /* guest function pointer, one argument (obj), no null check */

#define OBJ_DST_OFF       0x30u  /* word: destination buffer pointer */
#define OBJ_MODE_OFF      0x36u  /* byte: scripted by the callees; non-zero lets the state advance */
#define OBJ_B37_OFF       0x37u  /* byte: tested for zero after a zero busy result */
#define OBJ_SRC_OFF       0x3Cu  /* word: source buffer pointer (received bytes) */
#define OBJ_COUNT_OFF     0x44u  /* byte: number of bytes mirrored */
#define OBJ_STATE_OFF     0x46u  /* byte: state */
#define OBJ_B47_OFF       0x47u  /* byte: cleared when state 1 advances */
#define OBJ_B49_OFF       0x49u  /* byte: set to 1 when state 0 advances */
#define OBJ_B4A_OFF       0x4Au  /* byte: tested for zero, then cleared */
#define OBJ_B50_OFF       0x50u  /* byte: tested for zero */
#define OBJ_HOOK_OFF      0x18u  /* word: guest function pointer, one argument (obj), or 0 */
#define OBJ_WORD14_OFF    0x14u  /* word: tested for zero in state 1 */
#define OBJ_ID_OFF        0xE8u  /* byte: terminal id nibble (high nibble of the first received byte) */

#define ID_NONE           0xFu   /* first-byte high nibble that means "no id": copy skipped */
#define BYTE_SPECIAL      0xF3u  /* first received byte that does not trigger the second reset */

#define STATE_IDLE        0x00u
#define STATE_ONE         0x01u
#define STATE_FE          0xFEu
#define STATE_FF          0xFFu

/* The state-dependent tail.  The state byte is read afresh here (the reset slot
 * may have rewritten it since the test below). */
static void pad_record_advance(uint32_t obj)
{
    uint32_t st = PE_LoadU8(obj + OBJ_STATE_OFF);

    if (st == STATE_IDLE) {
        if (PE_LoadU8(obj + OBJ_ID_OFF) != 0u) {
            PE_StoreU8(obj + OBJ_B49_OFF, 1u);
            PE_StoreU8(obj + OBJ_STATE_OFF, (uint8_t)(PE_LoadU8(obj + OBJ_STATE_OFF) + 1u));
        }
    } else if (st == STATE_ONE) {
        if (PE_LoadU8(obj + OBJ_MODE_OFF) != 0u) {
            PE_StoreU8(obj + OBJ_B47_OFF, 0u);
            PE_StoreU8(obj + OBJ_STATE_OFF, (uint8_t)(PE_LoadU8(obj + OBJ_STATE_OFF) + 1u));
        }
    } else if (st == STATE_FE) {
        if (PE_LoadU8(obj + OBJ_MODE_OFF) != 0u)
            PE_StoreU8(obj + OBJ_STATE_OFF, 0xFFu);
    } else if (st != STATE_FF) {
        if (PE_LoadU8(obj + OBJ_MODE_OFF) != 0u) {
            uint32_t hook = PE_LoadU32(obj + OBJ_HOOK_OFF);
            uint32_t r;

            if (hook != 0u)
                r = (uint32_t)PE_GuestCall("func_80084C4C a0->+0x18 hook", hook, 1, obj, 0, 0, 0);
            else
                r = (uint32_t)pe_plat_pad_recv_step(obj);
            PE_StoreU8(obj + OBJ_STATE_OFF, (uint8_t)(PE_LoadU8(obj + OBJ_STATE_OFF) + r));
        }
    }
}

void pe_plat_pad_record_step(uint32_t obj)
{
    uint32_t src, dst, first, old_id, st, second;
    int reset_first = 0;

    /* First received byte; its high nibble becomes the id byte. */
    src = PE_LoadU32(obj + OBJ_SRC_OFF);
    first = PE_LoadU8(src);
    old_id = PE_LoadU8(obj + OBJ_ID_OFF);
    PE_StoreU8(obj + OBJ_ID_OFF, (uint8_t)(first >> 4));
    if ((first >> 4) != ID_NONE) {
        uint32_t i = 1u;

        dst = PE_LoadU32(obj + OBJ_DST_OFF);
        PE_StoreU8(dst, 0u);
        do {
            uint32_t s = PE_LoadU32(obj + OBJ_SRC_OFF);
            uint32_t d = PE_LoadU32(obj + OBJ_DST_OFF);

            PE_StoreU8(d + i, PE_LoadU8(s + (i == 1u ? 0u : i)));
            i++;
        } while (i < PE_LoadU8(obj + OBJ_COUNT_OFF));
    } else {
        PE_StoreU8(obj + OBJ_ID_OFF, (uint8_t)old_id);
    }

    src = PE_LoadU32(obj + OBJ_SRC_OFF);
    second = PE_LoadU8(src + 1u);

    /* A zero second byte consults the state (and, in state 1, the +0x14 word) and +0x50. */
    if (second == 0u) {
        uint32_t st1 = PE_LoadU8(obj + OBJ_STATE_OFF);

        if (st1 != STATE_ONE || PE_LoadU32(obj + OBJ_WORD14_OFF) != 0u) {
            if (PE_LoadU8(obj + OBJ_B50_OFF) == 0u)
                reset_first = 1;
        }
    }

    if (reset_first) {
        PE_Card_ICall("D_8009B728 (card record reset)", PE_LoadU32(ADDR_RESET_SLOT), 1, obj, 0, 0);
    } else {
        if (pe_plat_pad_record_busy(obj) == 0) {
            if (PE_LoadU8(obj + OBJ_B37_OFF) == 0u && PE_LoadU8(obj + OBJ_B4A_OFF) == 0u &&
                PE_LoadU8(obj + OBJ_ID_OFF) != old_id)
                PE_Card_ICall("D_8009B728 (card record reset)", PE_LoadU32(ADDR_RESET_SLOT), 1, obj, 0, 0);
        }
    }
    PE_StoreU8(obj + OBJ_B4A_OFF, 0u);

    /* Generic-state test: the busy / reset phase may have rewritten the state. */
    st = PE_LoadU8(obj + OBJ_STATE_OFF);
    if (st != STATE_IDLE && st != STATE_ONE && st != STATE_FE && st != STATE_FF) {
        src = PE_LoadU32(obj + OBJ_SRC_OFF);
        if (PE_LoadU8(src) != BYTE_SPECIAL)
            PE_Card_ICall("D_8009B728 (card record reset)", PE_LoadU32(ADDR_RESET_SLOT), 1, obj, 0, 0);
    }
    pad_record_advance(obj);
}
