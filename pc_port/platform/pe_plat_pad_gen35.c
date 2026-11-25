/* libpad/libcard "SIO0 start" routine (guest 0x80082E00), written clean-room
 * from black-box vectors of the original code (pc_port/tools/
 * pe_cd_leaf_vectors.py cdgen35; checked by pc_port/tools/
 * pe_spu_caller_check.c, CHK_HAVE_PAD_GEN35), with no SDK source.  A fresh
 * author's design: the vectors were designed from harness observations only
 * (no hand port, test body or disassembly), see
 * docs/evidence/native-pad-bios-rows-20261125/.
 *
 * One call restarts the serial port for the 0xF0-byte record `record`.  It
 * reads the SIO base pointer word and writes the control register 0x40 then
 * 0, the mode register 0x0D and the baud register 0x88, starts the RCNT2
 * timeout with limit 0x91 (0x50 when the record byte at +0xE8 is 8) and writes the control register again: 0x1003 when
 * the port-select word (0x8009B764) is 0, else 0x3003.
 *
 * The word 0x8009B77C + 4 * select (no bound check, 32-bit wrap) is a
 * countdown, re-read on every use.  While it is positive it is decremented
 * and stored and the flush slot 0x8009B744 is called with
 * (new count * 0xF0 + the record word at +0xC).  Then, if the count is not
 * negative, it is set to -1 and the slots 0x8009B744 and 0x8009B748 are
 * called with the record.
 *
 * Finally the status register is read.  When its bit 9 (0x200) is set the
 * control register is read, and the status register is read a second time;
 * the control value | 0x10 is written back.  If bit 9 is still set the data
 * register (byte at the base) is written 1 and read back (value unused), the
 * RX-ready wait is run and the result is 0.  If it is clear, 0xFFFFFF7F is written through the interrupt
 * status pointer word 0x8009B784.  The routine ends (also when bit 9 was
 * clear at the first read) by returning 0 when both the record bytes +0x50
 * and +0x36 are nonzero (+0x36 is only read when +0x50 is nonzero); else the
 * byte at the pointer in record word +0x3C is cleared (unconditionally: with
 * a null or low pointer the original makes the same store, into low RAM the
 * harness does not log, so such cases are dropped from the vectors and are
 * not checked) and the result is 1. */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_plat/pad.h"
#include "pe_plat/bios_file.h"

#define ADDR_SELECT     0x8009B764u  /* port select word */
#define ADDR_COUNT_BASE 0x8009B77Cu  /* countdown cells, 4 * select further on */
#define ADDR_STAT_PTR   0x8009B784u  /* pointer to the interrupt status word */
#define ADDR_SIO_PTR    0x8009B788u  /* pointer to the SIO0 register block */
#define ADDR_SLOT_FLUSH 0x8009B744u  /* one argument */
#define ADDR_SLOT_DONE  0x8009B748u  /* one argument */

#define SIO_STAT        0x4u
#define SIO_MODE        0x8u
#define SIO_CTRL        0xAu
#define SIO_BAUD        0xEu
#define STAT_BIT9       0x200u
#define RECORD_STRIDE   0xF0u

static uint32_t count_cell(void)
{
    return ADDR_COUNT_BASE + 4u * PE_LoadU32(ADDR_SELECT);
}

static void call_flush(uint32_t arg)
{
    PE_Card_ICall("D_8009B744 (card sector flush)", PE_LoadU32(ADDR_SLOT_FLUSH), 1, arg, 0, 0);
}

int pe_plat_pad_sio_start(uint32_t record)
{
    uint32_t sio = PE_LoadU32(ADDR_SIO_PTR);
    uint32_t cell, count;

    PE_StoreU16(sio + SIO_CTRL, 0x40u);
    PE_StoreU16(sio + SIO_CTRL, 0u);
    PE_StoreU16(sio + SIO_MODE, 0x0Du);
    PE_StoreU16(sio + SIO_BAUD, 0x88u);
    pe_plat_pad_rc2_wait_set(PE_LoadU8(record + 0xE8u) == 8u ? 0x50u : 0x91u);
    PE_StoreU16(sio + SIO_CTRL, PE_LoadU32(ADDR_SELECT) == 0u ? 0x1003u : 0x3003u);

    while ((int32_t)PE_LoadU32(count_cell()) > 0) {
        cell = count_cell();
        count = PE_LoadU32(cell) - 1u;
        PE_StoreU32(cell, count);
        call_flush(count * RECORD_STRIDE + PE_LoadU32(record + 0xCu));
    }
    if ((int32_t)PE_LoadU32(count_cell()) >= 0) {
        PE_StoreU32(count_cell(), 0xFFFFFFFFu);
        call_flush(record);
        PE_Card_ICall("D_8009B748 (card record done)", PE_LoadU32(ADDR_SLOT_DONE), 1, record, 0, 0);
    }

    sio = PE_LoadU32(ADDR_SIO_PTR);
    if (PE_LoadU16(sio + SIO_STAT) & STAT_BIT9) {
        uint16_t ctrl = PE_LoadU16(sio + SIO_CTRL);
        uint16_t again = PE_LoadU16(sio + SIO_STAT);
        PE_StoreU16(sio + SIO_CTRL, (uint16_t)(ctrl | 0x10u));
        if (again & STAT_BIT9) {
            sio = PE_LoadU32(ADDR_SIO_PTR);
            PE_StoreU8(sio, 1u);
            (void)PE_LoadU8(sio);
            pe_plat_card_sio_rx_wait();
            return 0;
        }
        PE_StoreU32(PE_LoadU32(ADDR_STAT_PTR), 0xFFFFFF7Fu);
    }

    if (PE_LoadU8(record + 0x50u) != 0u && PE_LoadU8(record + 0x36u) != 0u)
        return 0;
    PE_StoreU8(PE_LoadU32(record + 0x3Cu), 0u);
    return 1;
}
