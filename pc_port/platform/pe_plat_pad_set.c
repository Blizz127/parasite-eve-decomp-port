/* libpad PadSetAct / PadSetActAlign / PadSetMainMode (guest 0x80082974 /
 * 0x800828F4 / 0x8008292C), written clean-room by an author who saw neither
 * SDK source nor this port's earlier code: 28080 seeded black-box vectors of
 * the original (pc_port/tools/pe_pad_set_vectors.py); see
 * docs/evidence/native-pad-setters-20261030/. */
/* libpad setters written from black-box vectors of the original code, not
 * from SDK source.  Each setter finds the port's controller record with
 * pe_plat_pad_record and edits fields of that record in guest RAM. */
#include "pe_plat/pad.h"
#include "pe_guest_ram.h"

/* Record offsets (bytes from the record base). */
#define PAD_REC_HANDLER_A   0x14u /* word: handler address, set by ALIGN/MAIN */
#define PAD_REC_HANDLER_B   0x18u /* word: second handler address */
#define PAD_REC_ACT_ALIGN   0x20u /* word: actuator data pointer (ALIGN) */
#define PAD_REC_ACT_DATA    0x28u /* word: actuator data pointer (ACT) */
#define PAD_REC_ACT_COUNT   0x34u /* byte: actuator size (ACT) */
#define PAD_REC_REQUEST     0x46u /* byte: request flag, set to 1 */
#define PAD_REC_MODE        0x51u /* byte: requested main mode */
#define PAD_REC_LOCK        0x52u /* byte: mode lock flag */
#define PAD_REC_MODE_FLAG   0x53u /* byte: 1 when the mode byte equals the E4 byte */
#define PAD_REC_E4_BYTE     0xE4u /* byte: compared with the new mode by MAIN */

#define PAD_ALIGN_HANDLER_A 0x80083C20u /* ALIGN: value stored at +0x14 */
#define PAD_ALIGN_HANDLER_B 0x80083C3Cu /* ALIGN: value stored at +0x18 */
#define PAD_MAIN_HANDLER_A  0x80083D9Cu /* MAIN: value stored at +0x14 */
#define PAD_MAIN_HANDLER_B  0x80083DF0u /* MAIN: value stored at +0x18 */

void pe_plat_pad_set_act(uint32_t port, uint32_t data, uint32_t count)
{
    uint32_t rec = pe_plat_pad_record(port);
    if (rec == 0u)
        return;
    PE_StoreU32(rec + PAD_REC_ACT_DATA, data);
    PE_StoreU8(rec + PAD_REC_ACT_COUNT, (uint8_t)count);
}

int pe_plat_pad_set_act_align(uint32_t port, uint32_t data)
{
    uint32_t rec = pe_plat_pad_record(port);
    if (rec == 0u || pe_plat_pad_busy(rec) != 0)
        return 0;
    PE_StoreU32(rec + PAD_REC_HANDLER_A, PAD_ALIGN_HANDLER_A);
    PE_StoreU32(rec + PAD_REC_HANDLER_B, PAD_ALIGN_HANDLER_B);
    PE_StoreU32(rec + PAD_REC_ACT_ALIGN, data);
    PE_StoreU8(rec + PAD_REC_REQUEST, 1u);
    return 1;
}

int pe_plat_pad_set_main_mode(uint32_t port, uint32_t mode, uint32_t lock)
{
    uint32_t rec = pe_plat_pad_record(port);
    if (rec == 0u || pe_plat_pad_busy(rec) != 0)
        return 0;
    PE_StoreU32(rec + PAD_REC_HANDLER_A, PAD_MAIN_HANDLER_A);
    PE_StoreU32(rec + PAD_REC_HANDLER_B, PAD_MAIN_HANDLER_B);
    PE_StoreU8(rec + PAD_REC_REQUEST, 1u);
    PE_StoreU8(rec + PAD_REC_MODE, (uint8_t)mode);
    PE_StoreU8(rec + PAD_REC_LOCK, (uint8_t)lock);
    PE_StoreU8(rec + PAD_REC_MODE_FLAG,
               ((uint8_t)mode == PE_LoadU8(rec + PAD_REC_E4_BYTE)) ? 1u : 0u);
    return 1;
}
