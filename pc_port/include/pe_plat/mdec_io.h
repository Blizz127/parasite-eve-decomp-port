/*
 * pe_plat/mdec_io.h - host MDEC I/O wrappers (ovl_039F rows 0x8010BFA0 and
 * 0x8010C01C).
 *
 * Provenance: written clean-room from psx-spx and black-box vectors of the
 * original code, with no SDK source.  psx-spx pages used:
 *   https://psx-spx.consoledev.net/macroblockdecodermdec/ (MDEC(1) command word,
 *       MDEC status register bit 29 = command busy)
 *   https://psx-spx.consoledev.net/dmachannels/ (DPCR, D#_MADR/BCR/CHCR)
 *
 * Pure functions: no globals, no hardware or guest-RAM access.  Each call
 * returns the ordered list of accesses the original performs; the wiring
 * layer applies them in array order.  An access whose address is in guest RAM
 * (>= 0x80000000, e.g. the command block) is a RAM store; addresses
 * 0x1F8010xx / 0x1F801820 are hardware registers.  All accesses are 32-bit.
 */
#ifndef PE_PLAT_MDEC_IO_H
#define PE_PLAT_MDEC_IO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_MDECIO_REG_DPCR   0x1F8010F0u
#define PE_MDECIO_REG_DMA0_MADR 0x1F801080u
#define PE_MDECIO_REG_DMA0_BCR  0x1F801084u
#define PE_MDECIO_REG_DMA0_CHCR 0x1F801088u
#define PE_MDECIO_REG_DMA1_MADR 0x1F801090u
#define PE_MDECIO_REG_DMA1_BCR  0x1F801094u
#define PE_MDECIO_REG_DMA1_CHCR 0x1F801098u
#define PE_MDECIO_REG_MDEC0  0x1F801820u /* command/parameter port */
#define PE_MDECIO_REG_MDEC1  0x1F801824u /* status (read) / control (write) */

#define PE_MDECIO_STATUS_BUSY 0x20000000u /* psx-spx status bit 29 */

#define PE_MDECIO_MAX_WRITES 8
#define PE_MDECIO_EXIT_OK 0u
#define PE_MDECIO_EXIT_BLOCK_COUNT 1u /* guarded output only: refused, no write */

typedef struct {
    uint32_t reg;   /* address written */
    uint32_t value; /* 32-bit value */
} pe_mdec_io_write;

typedef struct {
    pe_mdec_io_write writes[PE_MDECIO_MAX_WRITES]; /* program order */
    uint32_t count;
    uint32_t edited_command; /* row 1: command word after the call */
    uint32_t upload_source;  /* row 1: DMA0 source (command_block + 4) */
    uint32_t upload_words;   /* row 1: parameter words (command bits 15-0) */
    uint32_t would_wait;     /* 1: the original polls and does not finish; no writes valid */
    uint32_t exit_code;
} pe_mdec_io_result;

/* Row 0x8010BFA0, func_8010BFA0(command_block, mode).  `command` is the word
 * at command_block on entry, `dpcr_in` the DPCR read, `mdec_status` the MDEC
 * status read.  If mdec_status has the busy bit (bit 29) set the original
 * waits for it to clear; the result then has would_wait = 1, count = 0 and
 * the wiring layer must apply its host wait contract and call again.
 * Otherwise (7 writes, in order):
 *   [block]=c1, [block]=c2 (two RAM stores: c1 = bit 27 step, c2 = + bit 25 step),
 *   DPCR = dpcr_in | 0x88, DMA0 MADR = block + 4,
 *   DMA0 BCR = 0x20 | ((words >> 5) << 16)  with words = c2 & 0xFFFF,
 *   MDEC0 = c2 (the MDEC(1) command), DMA0 CHCR = 0x01000201 (to device,
 *   SyncMode 1, start).
 * c1 = bit 27 := !(mode & 1); c2 = c1 with bit 25 := (mode & 2) != 0;
 * every other command bit is preserved. */
pe_mdec_io_result pe_mdec_io_input_trace(uint32_t command, uint32_t mode,
                                         uint32_t command_block,
                                         uint32_t dpcr_in, uint32_t mdec_status);

/* Row 0x8010C01C, func_8010C01C(destination, words) - the original's exact
 * ordered writes for every input (5 writes):
 *   DPCR = dpcr_in | 0x88, DMA1 CHCR = 0 (stop), DMA1 MADR = destination
 *   (unmasked), DMA1 BCR = 0x20 | ((words >> 5) << 16) (32-bit wrap),
 *   DMA1 CHCR = 0x01000200 (to RAM, SyncMode 1, start - last).
 * The vectors do not seed DMA1 CHCR, so any busy poll of it is unrepresented. */
pe_mdec_io_result pe_mdec_io_output_trace(uint32_t destination, uint32_t words,
                                          uint32_t dpcr_in);

/* Same, but with the port boundary from the integration spec: a block count
 * (words >> 5) of 0 (hardware would use 0x10000) or above 0xFFFF is refused
 * before any write (count = 0, exit_code = PE_MDECIO_EXIT_BLOCK_COUNT). */
pe_mdec_io_result pe_mdec_io_output_guarded(uint32_t destination, uint32_t words,
                                            uint32_t dpcr_in);

#ifdef __cplusplus
}
#endif
#endif
