/* libpad record state helpers (guest 0x800835C0 / 0x8008389C / 0x80083790 /
 * 0x80083D9C / 0x80084644 / 0x80084B44), written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_pad_leaf_vectors.py
 * batch2), with no SDK source.  Evidence:
 * docs/evidence/native-pad-state-20261030/. */
#include "pe_plat/pad.h"
#include "pe_guest_ram.h"

/* Record offsets (bytes from the record base). */
#define PAD_REC_HANDLER_A   0x14u /* word: handler address, cleared by init */
#define PAD_REC_HANDLER_B   0x18u /* word: second handler address, cleared by init */
#define PAD_REC_DATA_PTR    0x2Cu /* word: command data pointer (main handler) */
#define PAD_REC_STAGE       0x35u /* byte: command stage */
#define PAD_REC_CMD         0x36u /* byte: command letter */
#define PAD_REC_STATE       0x46u /* byte: record state selecting the step */
#define PAD_REC_ARG_47      0x47u /* byte: argument for commands G (send), L/F/G (query) */
#define PAD_REC_ARG_48      0x48u /* byte: nonzero selects command K when state is 4 */
#define PAD_REC_INIT_FLAG   0x49u /* byte: init does nothing when this is zero */
#define PAD_REC_CLR_0       0x00u /* word: cleared by init */
#define PAD_REC_CLR_1       0x04u /* word: cleared by init */
#define PAD_REC_CLR_2       0x08u /* word: cleared by init */
#define PAD_REC_DATA_D      0x51u /* main handler state 2: data pointer target */
#define PAD_REC_DATA_M      0x5Du /* main handler state 3: data pointer target */
#define PAD_REC_FILL        0x5Du /* first of six bytes init fills with 0xFF */
#define PAD_REC_ACT_BYTE    0xE3u /* byte: act_size term (also cleared by init) */
#define PAD_REC_ACT_SEL     0xE9u /* byte: act_size term (also cleared by init) */
#define PAD_REC_ACT_BASE    0xECu /* word: act_size base value */
#define PAD_REC_E4_BYTE     0xE4u /* byte: argument for command L (send step); cleared by init */
#define PAD_REC_E6_HALF     0xE6u /* halfword: cleared by init (twice) */
#define PAD_REC_EA_BYTE     0xEAu /* byte: cleared by init */

/* Values of the state byte at +0x46. */
#define PAD_STATE_2         2u    /* main handler: command D; send step: command E; query step: command L */
#define PAD_STATE_3         3u    /* main handler: command M; send step: command L; query step: command F */
#define PAD_STATE_4         4u    /* send step: command G; query step: command G or K */

/* Command letters and stage values the main handler stores itself. */
#define PAD_CMD_D           0x44u
#define PAD_CMD_M           0x4Du
#define PAD_STAGE_D         2u
#define PAD_STAGE_M         6u

/* record_init constants. */
#define PAD_INIT_FILL_COUNT 6u    /* bytes filled starting at PAD_REC_FILL (0x5D..0x62) */
#define PAD_INIT_FILL_VALUE 0xFFu

/* Reset a record: does nothing when the flag byte at +0x49 is zero,
 * otherwise clears the state, handler and action fields and fills six bytes
 * at +0x5D with 0xFF. */
void pe_plat_pad_record_init(uint32_t record)
{
    uint32_t i;
    if (PE_LoadU8(record + PAD_REC_INIT_FLAG) == 0u)
        return;
    PE_StoreU8(record + PAD_REC_INIT_FLAG, 0u);
    PE_StoreU8(record + PAD_REC_STATE, 0u);
    PE_StoreU16(record + PAD_REC_E6_HALF, 0u);
    PE_StoreU32(record + PAD_REC_HANDLER_A, 0u);
    PE_StoreU32(record + PAD_REC_HANDLER_B, 0u);
    PE_StoreU8(record + PAD_REC_ACT_BYTE, 0u);
    PE_StoreU8(record + PAD_REC_E4_BYTE, 0u);
    PE_StoreU16(record + PAD_REC_E6_HALF, 0u);
    PE_StoreU8(record + PAD_REC_ACT_SEL, 0u);
    PE_StoreU8(record + PAD_REC_EA_BYTE, 0u);
    PE_StoreU32(record + PAD_REC_CLR_0, 0u);
    PE_StoreU32(record + PAD_REC_CLR_1, 0u);
    PE_StoreU32(record + PAD_REC_CLR_2, 0u);
    for (i = 0; i < PAD_INIT_FILL_COUNT; i++)
        PE_StoreU8(record + PAD_REC_FILL + i, PAD_INIT_FILL_VALUE);
}

/* Send step: the state byte picks which command is queued. */
void pe_plat_pad_send_step(uint32_t record)
{
    switch (PE_LoadU8(record + PAD_REC_STATE)) {
    case PAD_STATE_2:
        pe_plat_pad_cmd_e(record);
        break;
    case PAD_STATE_3:
        pe_plat_pad_cmd_l(record, PE_LoadU8(record + PAD_REC_E4_BYTE));
        break;
    case PAD_STATE_4:
        pe_plat_pad_cmd_g(record, PE_LoadU8(record + PAD_REC_ARG_47));
        break;
    default:
        break;
    }
}

/* Query step: like the send step with a different command per state. */
void pe_plat_pad_query_step(uint32_t record)
{
    switch (PE_LoadU8(record + PAD_REC_STATE)) {
    case PAD_STATE_2:
        pe_plat_pad_cmd_l(record, PE_LoadU8(record + PAD_REC_ARG_47));
        break;
    case PAD_STATE_3:
        pe_plat_pad_cmd_f(record, PE_LoadU8(record + PAD_REC_ARG_47));
        break;
    case PAD_STATE_4:
        if (PE_LoadU8(record + PAD_REC_ARG_48) == 0u)
            pe_plat_pad_cmd_g(record, PE_LoadU8(record + PAD_REC_ARG_47));
        else
            pe_plat_pad_cmd_k(record);
        break;
    default:
        break;
    }
}

/* Size of the action buffer: the word at +0xEC plus four bytes per slot,
 * where the slots are one base slot, one per two (rounded up) E3 units and
 * 5/4 (rounded up) per E9 unit. */
int pe_plat_pad_act_size(uint32_t record)
{
    uint32_t e3 = PE_LoadU8(record + PAD_REC_ACT_BYTE);
    uint32_t e9 = PE_LoadU8(record + PAD_REC_ACT_SEL);
    uint32_t base = PE_LoadU32(record + PAD_REC_ACT_BASE);
    uint32_t slots = ((e3 + 1u) >> 1) + 1u + ((5u * e9 + 3u) >> 2);
    return (int)(base + 4u * slots);
}

/* Main handler: states 2 and 3 queue a command whose data pointer points
 * inside the record. */
void pe_plat_pad_main_handler(uint32_t record)
{
    uint8_t state = PE_LoadU8(record + PAD_REC_STATE);
    if (state == PAD_STATE_2) {
        PE_StoreU8(record + PAD_REC_CMD, PAD_CMD_D);
        PE_StoreU32(record + PAD_REC_DATA_PTR, record + PAD_REC_DATA_D);
        PE_StoreU8(record + PAD_REC_STAGE, PAD_STAGE_D);
    } else if (state == PAD_STATE_3) {
        PE_StoreU8(record + PAD_REC_CMD, PAD_CMD_M);
        PE_StoreU32(record + PAD_REC_DATA_PTR, record + PAD_REC_DATA_M);
        PE_StoreU8(record + PAD_REC_STAGE, PAD_STAGE_M);
    }
}

/* Hook table written at guest globals: three function addresses. */
#define PAD_HOOK_SLOT_0     0x8009B73Cu
#define PAD_HOOK_SLOT_1     0x8009B740u
#define PAD_HOOK_SLOT_2     0x8009B744u
#define PAD_HOOK_VALUE_0    0x80084B78u
#define PAD_HOOK_VALUE_1    0x80084F8Cu
#define PAD_HOOK_VALUE_2    0x80084C4Cu

void pe_plat_pad_install_hooks(void)
{
    PE_StoreU32(PAD_HOOK_SLOT_0, PAD_HOOK_VALUE_0);
    PE_StoreU32(PAD_HOOK_SLOT_1, PAD_HOOK_VALUE_1);
    PE_StoreU32(PAD_HOOK_SLOT_2, PAD_HOOK_VALUE_2);
}
