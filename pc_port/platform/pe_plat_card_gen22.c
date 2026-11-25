/* libcard SIO byte exchange (guest 0x800830DC) and byte write (guest
 * 0x800832B4), written clean-room as natives from black-box vectors of the
 * original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen22, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads, stores and timeout-pair calls seen in the
 * vectors; no claim is made about any source. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "game_port.h"   /* PE_Port_ShouldStop */
#include "pe_guestcode.h"
#include "pe_plat/pad.h"
#include "pe_plat/bios_file.h"


/* Port contract (not behaviour of the original; the vectors never exercise
 * it): PE_Port_ShouldStop() is checked at the top of every iteration of all
 * seven waits below (three SIO_STAT bit spins, two 0x80084FE4 spins, the
 * 0x800830DC I_STAT poll and the 0x800832B4 I_STAT / inline RCNT2 loop, whose
 * RCNT2 timeout alone would not end once a stop is requested).  A
 * non-zero result leaves the function at once with no further guest access:
 * the exchange returns its timeout value -20 and the write returns its
 * timeout value -2.  The check itself does no guest load or store.
 *
 * Second port contract (likewise not behaviour of the original): the port's
 * clock is deterministic CPU-cycle time that nothing advances while native
 * code spins, so a wait that ends on an RCNT2 timeout would never end.
 * pe_plat_sio_poll_tick() is therefore called once at the top of each
 * iteration of every wait loop (all seven), after the PE_Port_ShouldStop()
 * check.  The tick does
 * no guest load or store.
 *
 * Third port contract (likewise not behaviour of the original): the three
 * SIO_STAT bit spins (not the 0x80084FE4 spins, not the I_STAT loops, which
 * end on the RCNT2 timeout) count polls; once 4096 polls have passed without
 * the bit, PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY) is called
 * and the spin is left as if the bit had been seen, so the function runs on
 * to its own result.  That budget exit comes before the next
 * PE_Port_ShouldStop() check; an external stop there still returns -20 / -2
 * at once.  Vector behaviour is unchanged: no vector reaches 4096 polls. */

#define ADDR_PTR_ISTAT      0x8009B784u  /* word: address of the interrupt-status word */
#define ADDR_PTR_SIO        0x8009B788u  /* word: address of the SIO register block (re-read) */
#define ADDR_STATE_MODE     0x8009B768u  /* word: 2 selects the short settle delay */
#define ADDR_RC2_START      0x800A76D0u  /* word: RCNT2 counter at the start of the wait */
#define ADDR_RC2_LIMIT      0x800BD02Cu  /* word: elapsed-time limit */

#define RC2_COUNTER         0x1F801120u  /* halfword */
#define RC2_MODE            0x1F801124u  /* halfword: bit 9 set = elapsed taken as is, else / 8 */
#define RC2_TARGET          0x1F801128u  /* halfword */

#define SIO_DATA_OFF        0x00u        /* byte */
#define SIO_STAT_OFF        0x04u        /* halfword: bit 0, bit 1 polled */
#define SIO_RATE_OFF        0x0Eu        /* halfword: 0x22 or 0x88 */

#define OBJ_BUF_OFF         0x3Cu        /* word: pointer to the byte buffer */
#define OBJ_PTR_OFF         0x40u        /* word: pointer to a byte (send path) */
#define OBJ_IDX_OFF         0x44u        /* byte: buffer index */
#define OBJ_CNT_OFF         0x45u        /* byte: count */
#define OBJ_WAIT_OFF        0xE8u        /* byte */

#define ISTAT_RX_BIT        0x80u
#define RC2_LIMIT_START     0x190u       /* argument of the timeout start in 0x800832B4 */
#define RC2_LIMIT_SETTLE    0x3Cu
#define EXCH_STATE_EXCHANGE 0x1AEu       /* stored to ADDR_RC2_LIMIT by 0x800830DC */
#define SIO_STAT_POLL_BUDGET 4096u       /* port contract: polls before an unresolvable stat wait is stopped */
#define EXCH_TIMEOUT        (-20)
#define WRITE_TIMEOUT       (-2)

/* Spin until (SIO_STAT & mask) is non-zero.  Returns 1 when an external stop
 * ends the wait (the caller returns its timeout value at once), else 0.
 * Port contract: after 4096 polls without the bit the wait is
 * unresolvable on the host SIO model, so PE_Port_RequestStop() is called and
 * the wait is left as if the bit had been seen (before the next
 * PE_Port_ShouldStop() check, so the function runs on to its own result). */
static int sio_wait_stat(uint32_t sio, uint32_t mask)
{
    unsigned polls;

    for (polls = 0u;; polls++) {
        if (polls >= SIO_STAT_POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        if (PE_Port_ShouldStop() != 0)
            return 1;
        pe_plat_sio_poll_tick();
        if ((PE_LoadU16(sio + SIO_STAT_OFF) & mask) != 0u)
            return 0;
    }
}

/* Serial rate: 0x22 when the buffer's first byte has high nibble 8 and the
 * index byte exceeds 8, otherwise 0x88. */
static uint16_t sio_rate(uint32_t obj)
{
    uint8_t b0 = PE_LoadU8(PE_LoadU32(obj + OBJ_BUF_OFF));
    if ((b0 >> 4) == 8u && PE_LoadU8(obj + OBJ_IDX_OFF) > 8u)
        return 0x22u;
    return 0x88u;
}

/* 0x800830DC.  A negative byte selects the send path: read the data
 * register, set index 0xFF / count 1, store the complement of the byte to
 * the byte pointer at obj+0x40, wait for stat bit 0, spin on the timeout
 * check until it is non-zero, then write the complement to the data
 * register; the result is the byte read first.  Otherwise: record the RCNT2
 * counter, choose the serial rate, wait for stat bit 1, read the data
 * register, wait for interrupt-status bit 7 (timeout check per failed poll,
 * non-zero returns -20), write the byte, bump count and index and store the
 * data byte at buffer[index]; the result is the byte read. */
int pe_plat_card_sio_exchange(uint32_t obj, int byte)
{
    uint32_t sio, istat_ptr, buf;
    uint8_t data, idx, cnt;

    if (byte < 0) {
        uint32_t p;
        uint8_t out = (uint8_t)~(uint32_t)byte;

        sio = PE_LoadU32(ADDR_PTR_SIO);
        p = PE_LoadU32(obj + OBJ_PTR_OFF);
        data = PE_LoadU8(sio + SIO_DATA_OFF);
        PE_StoreU8(obj + OBJ_IDX_OFF, 0xFFu);
        PE_StoreU8(obj + OBJ_CNT_OFF, 1u);
        PE_StoreU8(p, out);
        sio = PE_LoadU32(ADDR_PTR_SIO);
        if (sio_wait_stat(sio, 1u) != 0)
            return EXCH_TIMEOUT;
        sio = PE_LoadU32(ADDR_PTR_SIO);
        for (;;) {
            if (PE_Port_ShouldStop() != 0)
                return EXCH_TIMEOUT;
            pe_plat_sio_poll_tick();
            if (func_80084FE4() != 0)
                break;
        }
        PE_StoreU8(sio + SIO_DATA_OFF, out);
        return (int)data;
    }

    {
        uint16_t rate = sio_rate(obj);
        uint32_t counter;

        sio = PE_LoadU32(ADDR_PTR_SIO);
        counter = PE_LoadU16(RC2_COUNTER);
        PE_StoreU32(ADDR_RC2_LIMIT, EXCH_STATE_EXCHANGE);
        PE_StoreU32(ADDR_RC2_START, counter);
        PE_StoreU16(sio + SIO_RATE_OFF, rate);
        if (sio_wait_stat(sio, 2u) != 0)
            return EXCH_TIMEOUT;
        sio = PE_LoadU32(ADDR_PTR_SIO);
        istat_ptr = PE_LoadU32(ADDR_PTR_ISTAT);
        data = PE_LoadU8(sio + SIO_DATA_OFF);
        while ((PE_LoadU32(istat_ptr) & ISTAT_RX_BIT) == 0u) {
            if (PE_Port_ShouldStop() != 0)
                return EXCH_TIMEOUT;
            pe_plat_sio_poll_tick();
            if (func_80084FE4() != 0)
                return EXCH_TIMEOUT;
            istat_ptr = PE_LoadU32(ADDR_PTR_ISTAT);
        }
    }
    sio = PE_LoadU32(ADDR_PTR_SIO);
    cnt = PE_LoadU8(obj + OBJ_CNT_OFF);
    idx = PE_LoadU8(obj + OBJ_IDX_OFF);
    PE_StoreU8(sio + SIO_DATA_OFF, (uint8_t)byte);
    PE_StoreU8(obj + OBJ_CNT_OFF, (uint8_t)(cnt + 1u));
    buf = PE_LoadU32(obj + OBJ_BUF_OFF);
    idx = PE_LoadU8(obj + OBJ_IDX_OFF);
    PE_StoreU8(buf + idx, data);
    PE_StoreU8(obj + OBJ_IDX_OFF, (uint8_t)(idx + 1u));
    return (int)data;
}

/* 0x800832B4.  Choose the serial rate (as above, or 0x22 when the index is 0
 * and the data byte read next has high nibble 8), wait for stat bit 1, read
 * the data register, start the RCNT2 timeout (limit 0x190), store the rate, then
 * poll interrupt-status bit 7.  While it is clear, elapsed = counter -
 * start (counter + target - start when the counter is below the start: the
 * target is read once, and if non-zero read again and used, if zero 0x10000
 * is used);
 * when the mode's bit 9 is clear elapsed is divided by 8; elapsed at or
 * above the limit returns -2.  On success: if obj+0xE8 is not 8 and the
 * state word 0x8009B768 is 2, start a 0x3C timeout and spin on the check;
 * then write the byte, bump count and index and store the data byte at
 * buffer[index] unless the index is 0xFF; the result is the byte read. */
int pe_plat_card_sio_write_byte(uint32_t obj, int byte)
{
    uint32_t sio, istat_ptr, buf, ctrl;
    uint16_t rate = sio_rate(obj);
    uint8_t data, idx, idx2, cnt;

    sio = PE_LoadU32(ADDR_PTR_SIO);
    if (sio_wait_stat(sio, 2u) != 0)
        return WRITE_TIMEOUT;
    sio = PE_LoadU32(ADDR_PTR_SIO);
    data = PE_LoadU8(sio + SIO_DATA_OFF);
    if (PE_LoadU8(obj + OBJ_IDX_OFF) == 0u && (data >> 4) == 8u)
        rate = 0x22u;
    func_80084FC4(RC2_LIMIT_START);
    PE_StoreU16(sio + SIO_RATE_OFF, rate);
    istat_ptr = PE_LoadU32(ADDR_PTR_ISTAT);
    if ((PE_LoadU32(istat_ptr) & ISTAT_RX_BIT) == 0u) {
        uint32_t start = PE_LoadU32(ADDR_RC2_START);
        uint32_t limit = PE_LoadU32(ADDR_RC2_LIMIT);

        for (;;) {
            uint32_t counter, elapsed;

            if (PE_Port_ShouldStop() != 0)
                return WRITE_TIMEOUT;
            pe_plat_sio_poll_tick();
            counter = PE_LoadU16(RC2_COUNTER);

            if (counter < start) {
                uint32_t target = PE_LoadU16(RC2_TARGET);
                if (target != 0u)
                    target = PE_LoadU16(RC2_TARGET);
                else
                    target = 0x10000u;
                elapsed = counter + target - start;
            } else {
                elapsed = counter - start;
            }
            ctrl = PE_LoadU16(RC2_MODE);
            if ((ctrl & 0x200u) == 0u)
                elapsed >>= 3;
            if (elapsed >= limit)
                return WRITE_TIMEOUT;
            if ((PE_LoadU32(istat_ptr) & ISTAT_RX_BIT) != 0u)
                break;
        }
    }
    if (PE_LoadU8(obj + OBJ_WAIT_OFF) != 8u && PE_LoadU32(ADDR_STATE_MODE) == 2u) {
        func_80084FC4(RC2_LIMIT_SETTLE);
        for (;;) {
            if (PE_Port_ShouldStop() != 0)
                return WRITE_TIMEOUT;
            pe_plat_sio_poll_tick();
            if (func_80084FE4() != 0)
                break;
        }
    }
    sio = PE_LoadU32(ADDR_PTR_SIO);
    cnt = PE_LoadU8(obj + OBJ_CNT_OFF);
    idx = PE_LoadU8(obj + OBJ_IDX_OFF);
    idx2 = PE_LoadU8(obj + OBJ_IDX_OFF);
    PE_StoreU8(sio + SIO_DATA_OFF, (uint8_t)byte);
    PE_StoreU8(obj + OBJ_CNT_OFF, (uint8_t)(cnt + 1u));
    if (idx2 != 0xFFu) {
        buf = PE_LoadU32(obj + OBJ_BUF_OFF);
        idx2 = PE_LoadU8(obj + OBJ_IDX_OFF);
        PE_StoreU8(buf + idx2, data);
    }
    PE_StoreU8(obj + OBJ_IDX_OFF, (uint8_t)(idx + 1u));
    return (int)data;
}
