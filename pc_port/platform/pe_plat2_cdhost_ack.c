/* CD-ROM controller interrupt acknowledge (retail entry 0x8007AAB4, public name
 * pe_plat_cd_acknowledge_host, implementation pe_plat_cdhost_ack; see pe_plat/cdhost.h).
 *
 * PROVENANCE (clean-room, black-box).  Written by the cdhost-ack author from:
 *   - the retail-derived black-box vector sets own/cdgen47/ack.txt and ack_silent.txt
 *     (batch cdgen47: 400 cases each, ordered register / RAM reads "L", stores "S",
 *     scripted callee returns "RETS", recorded callee calls "CALL"), read through the
 *     columns only; the later set own/cdgen47_credits/ack.txt is a regression input;
 *   - docs/evidence/native-cdx-20261126/cdgen47/REPORT.md (what the sets cover);
 *   - the checker pc_port/tools/pe_cdx_cdgen47_check.c and pe_cdx_common.h (what a case compares);
 *   - the public headers pe_plat/cd_regs.h, pe_plat/cdhost.h, pe_plat/cd.h, pe_guest_ram.h,
 *     pe_guestcode.h, pe_bios_string.h and psx-spx "CDROM Controller" (register map, index
 *     register bits, interrupt types INT1..INT5, status byte bits).
 * NOT read: the previous provider pe_plat_cd_ack_host.c, any translated or original code
 * (src/, asm/, vendor/, decomp_hand/, game/boot, pe_lib*.c, *_retail.c, oracle sources), any
 * capture log.  The behaviour below is exactly what the vectors show; nothing is claimed about
 * how the original was written.
 *
 * What the routine is.  The CD-ROM interrupt service step of the libcd driver: it reads the
 * interrupt flag register, drains the controller's response FIFO into an 8-byte buffer, acks
 * the interrupt, keeps a small status record and publishes the response in per-type result slots
 * for the command layer.  It returns a result code for the caller: 0 no / unknown interrupt,
 * 1 acknowledge only, 2 complete (or acknowledge with a later complete), 4 data (INT1 / INT4),
 * 6 error (INT5).
 *
 * Guest words and registers used (every one read at its use, like the vectors show; the vector
 * evidence is the L / S column of the cases named):
 *   0x8009B27C  pointer to the CD index/status register (normally 0x1F801800)   all cases
 *   0x8009B280  pointer to the response FIFO register   (normally 0x1F801801)   cases 3, 31
 *   0x8009B284  pointer to the interrupt-enable register (0x1F801802)           case 3
 *   0x8009B288  pointer to the interrupt-flag register  (0x1F801803)            case 3
 *   0x8009AFC0  debug level (signed; > 0 enables the INT5 error text)            cases 273, 372
 *   0x8009AFC4  last status byte (word)    0x8009AFC8  second response byte (word)   case 205
 *   0x8009AFCC  shell-open event counter (word, +1 on a rising status bit 4)         cases 205, 290
 *   0x8009AFD5  last command byte (byte)                                             cases 240, 346
 *   0x8009B17C + 4*cmd  "command returns status" table (word, 256 entries)          cases 240, 53
 *   0x8009B07C + 4*cmd  "command has a second interrupt" table (word, 256 entries)  cases 6, 240
 *   0x8009AFDC + 4*cmd  command-name pointer table (word, 256 entries)              cases 273, 285
 *   0x8009B294 / B295 / B296   result-type bytes of slots 0 / 1 / 2                  cases 3, 70, 314
 *   0x800A3460 / 3468 / 3470   8-byte response copies of slots 0 / 1 / 2             same cases
 *   0x80011B44, 0x80011B50, 0x80011B6C, 0x80011B80   the text addresses passed to the diagnostics
 *      (cases 273 "Disk Error: ", 273 "com=%s,code=(%02x:%02x)\n", 377 puts, 377 "(%d)\n")
 * The interrupt-type jump table (0x80011B88 + 4*type, types 1..5) is loaded in every case of those types; its entries are
 * the arm addresses 0x8007AE5C / AE10 / AD10 / AEDC / AF5C, and the arms are reproduced by a C switch after checking the entry.
 *
 * Flow, with the evidence:
 *   1. index register := 1; flag := *IF.  Low three bits zero -> return 0 (cases 2, 138, 83).
 *   2. Up to three more reads of the flag until its low three bits are nonzero (case 200 reads
 *      E1 E0 E0 E0: the type is then 0).  The type is the low three bits of the last read.
 *   3. Response drain: while fewer than 8 bytes: read the status register, stop when bit 5
 *      (response FIFO not empty) is clear, else read one byte (cases 3, 49: eight bytes end the
 *      loop without a ninth status read, case 298: seven bytes then a clear status).
 *   4. index register := 1, *IF := 7, *IE := 7 (every case after step 3).
 *   5. Status record.  Type 3 first reads the command byte and the "returns status" table; a
 *      zero entry skips the record (cases 240, 53).  Otherwise: old := word 0xAFC4; when bit 4 of
 *      the first response byte is set and bit 4 of old is clear, word 0xAFCC is incremented
 *      (cases 205, 327, 227; not 290, 302); then 0xAFC4 := byte 0, 0xAFC8 := byte 1.  "Error" is
 *      byte 0 & 0x1D (psx-spx stat bits Error, SeekError, IdError, ShellOpen); the error test only
 *      exists inside the record (cases 323, 359: a zero table entry with error bits stays slot 3/2).
 *   6. Per type:
 *        1  slot-1 byte := (error and more than one response byte) ? 5 : 1 (cases 284, 381: a lone
 *           status byte with error bits stays 1; cases 368, 317, 379: two or more bytes give 5); copy to slot 1; index := 0, *IF := 0; return 4
 *        2  slot-0 byte := error ? 5 : 2; copy to slot 0; return 2
 *        3  not error: re-read the command byte and the "second interrupt" table: nonzero -> slot-0
 *           byte 3, return 1; zero -> 2, return 2; error -> 5, return 2; copy to slot 0
 *        4  slot-2 and slot-1 bytes := 4, copies to slots 2 and 1; return 4
 *        5  optional text, slot-1 and slot-0 bytes := 5, copies to slots 1 and 0; return 6
 *        0, 6, 7  puts + printf of the type, no slot update; return 0
 *   The diagnostics are issued after the status record and before any slot store (the vectors'
 *   write count at each call is 6 distinct stores: index, index, IF, IE, 0xAFC4, 0xAFC8).
 *
 * Waits: both loops are bounded by the routine's own constants (4 flag reads, 8 response bytes);
 * there is no unbounded hardware wait here, so no stop exit or stuck-wait budget is needed. */
#include <stdint.h>
#include <string.h>

#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_bios_string.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/cdhost.h"
#include "game_port.h"

#define A_P_STATUS   0x8009B27Cu   /* pointer: index/status register */
#define A_P_RESPONSE 0x8009B280u   /* pointer: response FIFO */
#define A_P_ENABLE   0x8009B284u   /* pointer: interrupt enable */
#define A_P_FLAG     0x8009B288u   /* pointer: interrupt flag */
#define A_DEBUG      0x8009AFC0u
#define A_STATUS     0x8009AFC4u
#define A_STATUS2    0x8009AFC8u
#define A_SHELL_CNT  0x8009AFCCu
#define A_LAST_CMD   0x8009AFD5u
#define A_TAB_STATUS 0x8009B17Cu   /* + 4*cmd: command returns a status byte */
#define A_TAB_SECOND 0x8009B07Cu   /* + 4*cmd: command has a second interrupt */
#define A_TAB_NAME   0x8009AFDCu   /* + 4*cmd: command name pointer */
#define A_SLOT_TYPE0 0x8009B294u   /* result-type bytes of slots 0..2 */
#define A_SLOT_TYPE1 0x8009B295u
#define A_SLOT_TYPE2 0x8009B296u
#define A_SLOT_BUF0  0x800A3460u   /* response copies of slots 0..2, 8 bytes each */
#define A_SLOT_BUF1  0x800A3468u
#define A_SLOT_BUF2  0x800A3470u

#define A_TEXT_DISK_ERROR 0x80011B44u   /* "Disk Error: " */
#define A_TEXT_COMMAND    0x80011B50u   /* "com=%s,code=(%02x:%02x)\n" */
#define A_TEXT_UNKNOWN    0x80011B6Cu   /* puts text of the unknown-interrupt message */
#define A_TEXT_TYPE       0x80011B80u   /* "(%d)\n" */
#define A_JUMP_TABLE 0x80011B88u   /* + 4*type: interrupt-type jump table (types 1..5 used) */
#define A_GUEST_PUTS      0x80073C5Cu   /* retail puts, called through the guest-call registry */

#define STAT_ERROR_BITS 0x1Du           /* Error | SeekError | IdError | ShellOpen */
#define STAT_SHELL_OPEN 0x10u
#define IDX_RESPONSE_PENDING 0x20u      /* index register bit 5: response FIFO not empty */

static uint32_t word_at(uint32_t address) { return PE_LoadU32(address); }

static void copy_response(uint32_t destination, const uint8_t *response)
{
    unsigned i;
    for (i = 0; i < 8; i++) PE_StoreU8(destination + i, response[i]);
}

int pe_plat_cdhost_ack(void)
{
    uint8_t response[8];
    unsigned count, retry, type;
    uint32_t pointer;
    uint8_t flag;
    int error = 0, update = 1;

    /* host contract: a stop already requested means no CD register traffic (the vectors never request a stop) */
    if (PE_Port_ShouldStop())
        return 0;

    /* 1. select register bank 1 and look at the interrupt flag */
    PE_CdStoreU8(word_at(A_P_STATUS), 1);
    pointer = word_at(A_P_FLAG);
    flag = PE_CdLoadU8(pointer);
    if ((flag & 7u) == 0) return 0;

    /* 2. the flag is re-read (at most three more times) until it names an interrupt */
    for (retry = 0; retry < 3; retry++) {
        flag = PE_CdLoadU8(pointer);
        if (flag & 7u) break;
    }
    type = flag & 7u;

    /* 3. drain the response FIFO, at most eight bytes */
    memset(response, 0, sizeof response);
    count = 0;
    while (count < 8) {
        uint8_t status = PE_CdLoadU8(word_at(A_P_STATUS));
        if (!(status & IDX_RESPONSE_PENDING)) break;
        response[count++] = PE_CdLoadU8(word_at(A_P_RESPONSE));
    }

    /* 4. acknowledge: bank 1, clear the interrupt flags, enable bits 0..2 */
    PE_CdStoreU8(word_at(A_P_STATUS), 1);
    PE_CdStoreU8(word_at(A_P_FLAG), 7);
    PE_CdStoreU8(word_at(A_P_ENABLE), 7);

    /* 5. status record (an acknowledge of a command that returns no status skips it) */
    if (type == 3) {
        uint32_t command = PE_LoadU8(A_LAST_CMD);
        update = word_at(A_TAB_STATUS + 4u * command) != 0;
    }
    if (update) {
        uint32_t old = word_at(A_STATUS);
        if ((response[0] & STAT_SHELL_OPEN) && !(old & STAT_SHELL_OPEN))
            PE_StoreU32(A_SHELL_CNT, word_at(A_SHELL_CNT) + 1u);
        PE_StoreU32(A_STATUS, response[0]);
        PE_StoreU32(A_STATUS2, response[1]);
        error = (response[0] & STAT_ERROR_BITS) != 0;
    }

    /* 6. per interrupt type: the arm is reached through the jump table 0x80011B88 + 4*type (types 1..5; the vectors load the
     * entry in every such case and its value is always the arm address listed here).  An entry that names no known arm is a
     * computed jump this code cannot follow: the retail `jr` is handed to the guest-call boundary, which stops loudly. */
    if (type - 1u < 5u) {
        static const uint32_t arm[6] = { 0, 0x8007AE5Cu, 0x8007AE10u, 0x8007AD10u, 0x8007AEDCu, 0x8007AF5Cu };
        uint32_t target = word_at(A_JUMP_TABLE + 4u * type);
        if (target != arm[type]) {
            PE_GuestHook_Refuse("func_8007AAB4_jump_table", "pe_plat_cdhost_ack", target, 0, 0, 0, 0, 0,
                                "interrupt-type jump table entry names no known arm");
            return 0;
        }
    }
    switch (type) {
    case 1:     /* data ready */
        PE_StoreU8(A_SLOT_TYPE1, (error && count > 1) ? 5 : 1);
        copy_response(A_SLOT_BUF1, response);
        PE_CdStoreU8(word_at(A_P_STATUS), 0);
        PE_CdStoreU8(word_at(A_P_FLAG), 0);
        return 4;
    case 2:     /* command complete */
        PE_StoreU8(A_SLOT_TYPE0, error ? 5 : 2);
        copy_response(A_SLOT_BUF0, response);
        return 2;
    case 3: {   /* acknowledge */
        unsigned slot = 5;
        if (!error) {
            uint32_t command = PE_LoadU8(A_LAST_CMD);
            slot = word_at(A_TAB_SECOND + 4u * command) != 0 ? 3u : 2u;
        }
        PE_StoreU8(A_SLOT_TYPE0, (uint8_t)slot);
        copy_response(A_SLOT_BUF0, response);
        return slot == 3 ? 1 : 2;
    }
    case 4:     /* data end */
        PE_StoreU8(A_SLOT_TYPE2, 4);
        PE_StoreU8(A_SLOT_TYPE1, 4);
        copy_response(A_SLOT_BUF2, response);
        copy_response(A_SLOT_BUF1, response);
        return 4;
    case 5:     /* disk error */
        if ((int32_t)word_at(A_DEBUG) > 0) {
            PE_Printf_Guest(A_TEXT_DISK_ERROR, 0, 0, 0, 0, 0, 0, 0);
            if ((int32_t)word_at(A_DEBUG) > 0) {
                uint32_t command = PE_LoadU8(A_LAST_CMD);
                PE_Printf_Guest(A_TEXT_COMMAND, word_at(A_TAB_NAME + 4u * command), response[0], response[1], 0, 0, 0, 0);
            }
        }
        PE_StoreU8(A_SLOT_TYPE1, 5);
        PE_StoreU8(A_SLOT_TYPE0, 5);
        copy_response(A_SLOT_BUF0, response);
        copy_response(A_SLOT_BUF1, response);
        return 6;
    default:    /* 0, 6, 7: no such interrupt */
        PE_GuestCall("cd_ack_unknown_text", A_GUEST_PUTS, 1, A_TEXT_UNKNOWN, 0, 0, 0);
        PE_Printf_Guest(A_TEXT_TYPE, type, 0, 0, 0, 0, 0, 0);
        return 0;
    }
}
