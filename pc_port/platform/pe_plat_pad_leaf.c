/* Small libpad internals (guest 0x80084B20 / 0x80084F8C / 0x80084AE8 /
 * 0x800847A0 / 0x80083E70 / 0x80083EE4 / 0x80083C20 / 0x80083E50 /
 * 0x80083E84 / 0x80083EA4 / 0x80083EC4), written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_pad_leaf_vectors.py), with
 * no SDK source.  Evidence: docs/evidence/native-pad-leaf-20261030/. */
#include "pe_plat/pad.h"
#include "pe_guest_ram.h"

/* Controller record table (two records, 0xF0 bytes apart). */
#define PAD_RECORD_0        0x800A5B70u /* record for ports with no 0xF0 bits */
#define PAD_RECORD_1        0x800A5C60u /* record for every other port */
#define PAD_PORT_MASK       0xF0u       /* port bits that select record 1 */

/* Port ids reported for a record; any other address reports "none". */
#define PAD_ID_RECORD_0     0x10
#define PAD_ID_RECORD_1     0x20
#define PAD_ID_NONE         0xFF

/* Record offsets (bytes from the record base). */
#define PAD_REC_SRC_PTR     0x20u /* word: data pointer copied by cmd_m */
#define PAD_REC_ARG_BYTE    0x24u /* byte: command argument (cmd c/l/f/g) */
#define PAD_REC_DATA_PTR    0x2Cu /* word: command data pointer */
#define PAD_REC_STAGE       0x35u /* byte: command stage, set by every cmd */
#define PAD_REC_CMD         0x36u /* byte: command letter */
#define PAD_REC_CMD_PREV    0x37u /* byte: previous command (cmd_shift) */
#define PAD_REC_BUSY_BYTE   0x46u /* byte: compared with 0xFF by record_busy */
#define PAD_REC_BUSY_HALF   0xE6u /* halfword: 0 means idle in record_busy */

#define PAD_BUSY_NONE       0xFFu /* record_busy: byte value that means ready */

/* Command letters stored at +0x36. */
#define PAD_CMD_C           0x43u
#define PAD_CMD_E           0x45u
#define PAD_CMD_F           0x46u
#define PAD_CMD_G           0x47u
#define PAD_CMD_K           0x4Bu
#define PAD_CMD_L           0x4Cu
#define PAD_CMD_M           0x4Du

/* Stage byte values written alongside the command letter. */
#define PAD_STAGE_CLEAR     0u /* cmd_e / cmd_k */
#define PAD_STAGE_ARG       1u /* cmd_c / cmd_l / cmd_f / cmd_g */
#define PAD_STAGE_COPY      6u /* cmd_m */

uint32_t pe_plat_pad_port_record(int port)
{
    return ((uint32_t)port & PAD_PORT_MASK) != 0u ? PAD_RECORD_1 : PAD_RECORD_0;
}

int pe_plat_pad_record_busy(uint32_t record)
{
    if (PE_LoadU16(record + PAD_REC_BUSY_HALF) == 0u)
        return 1;
    return PE_LoadU8(record + PAD_REC_BUSY_BYTE) != PAD_BUSY_NONE;
}

int pe_plat_pad_record_port(uint32_t record)
{
    if (record == PAD_RECORD_0)
        return PAD_ID_RECORD_0;
    if (record == PAD_RECORD_1)
        return PAD_ID_RECORD_1;
    return PAD_ID_NONE;
}

/* Move the current command letter to the previous-command slot. */
void pe_plat_pad_cmd_shift(uint32_t record)
{
    uint8_t cmd = PE_LoadU8(record + PAD_REC_CMD);
    PE_StoreU8(record + PAD_REC_CMD, 0u);
    PE_StoreU8(record + PAD_REC_CMD_PREV, cmd);
}

/* Commands with no argument: point the data pointer at zero. */
static void pad_cmd_plain(uint32_t record, uint8_t letter)
{
    PE_StoreU8(record + PAD_REC_CMD, letter);
    PE_StoreU32(record + PAD_REC_DATA_PTR, 0u);
    PE_StoreU8(record + PAD_REC_STAGE, PAD_STAGE_CLEAR);
}

/* Commands with one byte argument kept inside the record. */
static void pad_cmd_arg(uint32_t record, uint8_t letter, unsigned char arg)
{
    PE_StoreU8(record + PAD_REC_CMD, letter);
    PE_StoreU32(record + PAD_REC_DATA_PTR, record + PAD_REC_ARG_BYTE);
    PE_StoreU8(record + PAD_REC_ARG_BYTE, arg);
    PE_StoreU8(record + PAD_REC_STAGE, PAD_STAGE_ARG);
}

void pe_plat_pad_cmd_e(uint32_t record) { pad_cmd_plain(record, PAD_CMD_E); }
void pe_plat_pad_cmd_k(uint32_t record) { pad_cmd_plain(record, PAD_CMD_K); }

void pe_plat_pad_cmd_m(uint32_t record)
{
    uint32_t src = PE_LoadU32(record + PAD_REC_SRC_PTR);
    PE_StoreU8(record + PAD_REC_CMD, PAD_CMD_M);
    PE_StoreU8(record + PAD_REC_STAGE, PAD_STAGE_COPY);
    PE_StoreU32(record + PAD_REC_DATA_PTR, src);
}

void pe_plat_pad_cmd_c(uint32_t record, unsigned char arg) { pad_cmd_arg(record, PAD_CMD_C, arg); }
void pe_plat_pad_cmd_l(uint32_t record, unsigned char arg) { pad_cmd_arg(record, PAD_CMD_L, arg); }
void pe_plat_pad_cmd_f(uint32_t record, unsigned char arg) { pad_cmd_arg(record, PAD_CMD_F, arg); }
void pe_plat_pad_cmd_g(uint32_t record, unsigned char arg) { pad_cmd_arg(record, PAD_CMD_G, arg); }
