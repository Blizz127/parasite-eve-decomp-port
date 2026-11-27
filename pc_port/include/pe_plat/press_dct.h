/* libpress DecDCT size / sync entries (ovl_039F rows 0x8010BF94 DecDCTBufSize, 0x8010C86C DecDCTvlcSize,
 * 0x8010C078 DecDCToutSync): clean-room provider pc_port/platform/pe_plat2_press_dct.c, written from the black-box
 * vector set cdgen63 and the port's host MDEC/DMA model API.  These are host replacements called by name (no
 * func_XXXXXXXX binding); the declarations are the same ones pe_port_compat.h carries. */
#ifndef PE_PLAT_PRESS_DCT_H
#define PE_PLAT_PRESS_DCT_H
#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* First halfword of the bitstream at `bs`, zero-extended. */
uint32_t PE_DecDCTBufSize_Guest(pe_addr_t bs);
/* Set the VLC output bound word (guest 0x8011EB8C) and return its previous value. */
int32_t PE_DecDCTvlcSize_Guest(uint32_t size);
/* mode != 0: MDEC status bit 24 (0 or 1).  mode == 0: wait for the DMA1 channel to go idle; 0 when idle, -1 when
 * the wait ends in the retail time-out arm or a host stop. */
int32_t PE_DecDCToutSync_Guest(int32_t mode);

#ifdef __cplusplus
}
#endif
#endif
