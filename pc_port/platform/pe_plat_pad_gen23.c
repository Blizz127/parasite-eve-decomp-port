/* libpad "padif transfer" (guest 0x80084168), written clean-room from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen23, revision v3: vectors.v3.txt sha256
 * a928b8369be4701b6dd02bc7c7450ff554479a0f574a2e1ceeb77b2941c3b2f7, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is exactly the guest loads, stores, slot calls
 * and callee calls seen in the vectors.
 *
 * One call moves one byte of a card/pad transfer.  A countdown word bounds the
 * SIO exchange loops; a two-word pending-block array (selected by a state
 * word) drives extra slot calls in the first loop; the byte at *D_8009B7A0 is
 * finally appended to the object's buffer. */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
#include "pe_plat/bios_file.h"

/* Guest function-pointer slots (no null check). */
#define ADDR_SLOT_PREP     0x8009B730u  /* one argument (obj); called first */
#define ADDR_SLOT_BYTE     0x8009B72Cu  /* (obj, write-mode flag) -> byte to send */
#define ADDR_SLOT_BLOCK    0x8009B744u  /* one argument: pending-block record address */
#define ADDR_SLOT_BLOCK2   0x8009B748u  /* one argument: record address (retire) */
#define ADDR_SLOT_FINISH   0x8009B724u  /* one argument (0); called last */

/* Guest data words. */
#define ADDR_STATE_GATE    0x8009B770u  /* zero: skip the object flag test */
#define ADDR_STATE_SELECT  0x8009B764u  /* zero selects pending word 1, else word 0 */
#define ADDR_COUNTER       0x8009B79Cu  /* countdown shared by all loops */
#define ADDR_PENDING       0x8009B77Cu  /* two words: pending-block state per record */
#define ADDR_RECORDS       0x8009B758u  /* pointer to the record array */
#define ADDR_BYTE_SRC      0x8009B7A0u  /* pointer to the byte appended to the buffer */

/* Object fields. */
#define OBJ_RECORD_OFF     0x0Cu   /* word: base of a record array (write phase) */
#define OBJ_MODE_BYTE_OFF  0x36u   /* byte: zero selects the write phase */
#define OBJ_BUFFER_OFF     0x3Cu   /* word: buffer pointer; its first byte holds a flag nibble */
#define OBJ_INDEX_OFF      0x44u   /* byte: next buffer index */

#define BUFFER_FLAG_MASK    0xF0u
#define BUFFER_FLAG_SPECIAL 0x80u  /* high nibble 8: the object byte at +0x36 is tested */

#define RECORD_STRIDE      0xF0u   /* bytes per pending-block record */
#define RECORD_BASE_OFF    0x0Cu   /* word in each record: base address for slot arguments */
#define WRITE_PHASE_MAX    5u      /* the write phase runs at most this many passes */
#define TIMEOUT_LIMIT      0x3Cu   /* argument of the RCNT2 timeout start */
#define ERR_ACK_TIMEOUT    ((int)-3) /* result when the acknowledge wait returns 0 */

/* Pending-block states (word value, read as signed). */
#define PENDING_DONE       0xFFFFFFFFu /* stored once the block has been retired */
#define PENDING_AFTER_3    1u          /* stored after state 3 ran its two slot calls */
#define PENDING_AFTER_4    3u          /* stored after state 4 */

/* Decrement the shared countdown in guest memory and return the new value. */
static int32_t counter_step(void)
{
    uint32_t v = PE_LoadU32(ADDR_COUNTER) - 1u;

    PE_StoreU32(ADDR_COUNTER, v);
    return (int32_t)v;
}

/* One exchange pass: ask the slot for a byte (write-mode flag as second
 * argument), send it (SIO exchange, or SIO write in the write phase), start the
 * timeout and wait for the acknowledge.  Returns 1 to continue; otherwise
 * *result is the value the function returns (a negative send result, or -3
 * when the acknowledge wait returns 0). */
static int exchange_pass(uint32_t obj, uint32_t flag, int exchange, int *result)
{
    uint32_t byte = PE_Card_ICall("D_8009B72C (card frame exchange)",
                                  PE_LoadU32(ADDR_SLOT_BYTE), 2, obj, flag, 0);
    int sent = exchange ? pe_plat_card_sio_exchange(obj, (int)(byte & 0xFFu))
                        : pe_plat_card_sio_write_byte(obj, (int)(byte & 0xFFu));

    if (sent < 0) {
        *result = sent;
        return 0;
    }
    pe_plat_pad_rc2_wait_set(TIMEOUT_LIMIT);
    if (pe_plat_card_sio_ack_wait() == 0) {
        *result = ERR_ACK_TIMEOUT;
        return 0;
    }
    return 1;
}

/* Pending-block step for the state word at `pending`, whose record is `index`
 * entries into the record array.  `state` is the value the caller just loaded
 * (non-negative); the word is read once more after the first slot call, and
 * the slot may rewrite it (v3).  The branch is chosen by the re-read value
 * (`again`); the second block call's argument derives from the FIRST read
 * (`state`): base + (state - 2) * stride, with `base` kept from the first block
 * (no reload of D_8009B758 or the record word between those calls). */
static void pending_step(uint32_t pending, uint32_t index, uint32_t state)
{
    uint32_t base = 0;
    uint32_t again;

    if ((int32_t)state > 0) {
        uint32_t record = PE_LoadU32(ADDR_RECORDS);

        base = PE_LoadU32(record + index * RECORD_STRIDE + RECORD_BASE_OFF);
        PE_Card_ICall("D_8009B744 (card sector flush)", PE_LoadU32(ADDR_SLOT_BLOCK), 1,
                      base + (state - 1u) * RECORD_STRIDE, 0, 0);
    }
    again = PE_LoadU32(pending);
    if (again <= 1u) {   /* unsigned: a negative re-read (the block slot rewrote the pair) does nothing */
        uint32_t record = PE_LoadU32(ADDR_RECORDS) + index * RECORD_STRIDE;

        PE_Card_ICall("D_8009B744 (card sector flush)", PE_LoadU32(ADDR_SLOT_BLOCK), 1,
                      record, 0, 0);
        PE_Card_ICall("D_8009B748 (card record done)", PE_LoadU32(ADDR_SLOT_BLOCK2), 1,
                      record, 0, 0);
        PE_StoreU32(pending, PENDING_DONE);
    } else if (again == 3u) {
        PE_Card_ICall("D_8009B744 (card sector flush)", PE_LoadU32(ADDR_SLOT_BLOCK), 1,
                      base + (state - 2u) * RECORD_STRIDE, 0, 0);
        PE_StoreU32(pending, PENDING_AFTER_3);
    } else if (again == 4u) {
        PE_StoreU32(pending, PENDING_AFTER_4);
    }
}

/* 0x80084168.  Returns 0 on success, a negative send result, or -3. */
int pe_plat_pad_transfer(uint32_t obj)
{
    uint32_t flag = 0;      /* write-mode flag: 1 when the object byte at +0x36 is zero */
    uint32_t pending, index, i;
    int result;

    PE_Card_ICall("D_8009B730 (card sector prepare)", PE_LoadU32(ADDR_SLOT_PREP), 1,
                  obj, 0, 0);

    if (PE_LoadU32(ADDR_STATE_GATE) != 0) {
        uint32_t buffer = PE_LoadU32(obj + OBJ_BUFFER_OFF);

        if ((PE_LoadU8(buffer) & BUFFER_FLAG_MASK) == BUFFER_FLAG_SPECIAL &&
            PE_LoadU8(obj + OBJ_MODE_BYTE_OFF) == 0)
            flag = 1;
    }

    /* Write phase: up to five passes, each consuming one countdown step. */
    if (flag) {
        for (i = 0; i < WRITE_PHASE_MAX; i++) {
            if (counter_step() <= 0)
                break;
            if (i > 0)   /* from the second pass, a record-array walk precedes the byte request */
                PE_Card_ICall("D_8009B730 (card sector prepare)", PE_LoadU32(ADDR_SLOT_PREP), 1,
                              PE_LoadU32(obj + OBJ_RECORD_OFF) + (i - 1u) * RECORD_STRIDE, 0, 0);
            if (!exchange_pass(obj, flag, 0, &result))
                return result;
        }
    }

    /* Exchange phase.  index and flag come from single loads; no callee writes them. */
    index = PE_LoadU32(ADDR_STATE_SELECT) == 0 ? 1u : 0u;
    pending = ADDR_PENDING + index * 4u;

    if ((int32_t)PE_LoadU32(ADDR_COUNTER) > 1) {
        for (;;) {
            uint32_t state = PE_LoadU32(pending);

            if ((int32_t)state < 0)
                break;
            pending_step(pending, index, state);
            if (!exchange_pass(obj, flag, 1, &result))
                return result;
            if (counter_step() <= 1)
                break;
        }
    }
    for (;;) {
        if (counter_step() <= 0)
            break;
        if (!exchange_pass(obj, flag, 1, &result))
            return result;
    }

    pe_plat_card_sio_rx_wait();

    /* Append the byte at *D_8009B7A0 to the object buffer and advance the index. */
    {
        uint32_t at = PE_LoadU8(obj + OBJ_INDEX_OFF);
        uint32_t source = PE_LoadU32(ADDR_BYTE_SRC);
        uint32_t buffer = PE_LoadU32(obj + OBJ_BUFFER_OFF);
        uint8_t byte;

        PE_StoreU8(obj + OBJ_INDEX_OFF, (uint8_t)(at + 1u));
        byte = PE_LoadU8(source);   /* after the index store: *source may be obj+0x44 itself */
        PE_StoreU8(buffer + at, byte);
    }
    PE_Card_ICall("D_8009B724 (card error hook)", PE_LoadU32(ADDR_SLOT_FINISH), 1,
                  0, 0, 0);
    return 0;
}
