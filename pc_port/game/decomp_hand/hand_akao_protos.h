/*
 * Hand-adapter prototypes, AKAO sound-driver tick (port lane, port3,
 * 2026-09-28).  Included through decomp_hand_protos.h.  Definitions:
 * akao_tick_port.c.
 */
#ifndef PE_HAND_AKAO_PROTOS_H
#define PE_HAND_AKAO_PROTOS_H

/* src/func_8008DB7C.c: AKAO driver tick (RCNT2 event, 240 Hz). */
void func_8008DB7C(void);
/* src/func_8008E8D0.c: sequencer step for one voice (opcode loop + note). */
void func_8008E8D0(pe_addr_t v, unsigned int bit);
/* src/func_80089328.c: key-on/off and voice parameter flush. */
void func_80089328(void);
/* src/func_80088980.c: per-voice LFO + volume/pan/pitch mix. */
void func_80088980(pe_addr_t v, unsigned int mask);
/* src/func_80087AA8.c: per-voice slide / LFO-depth tick. */
void func_80087AA8(pe_addr_t v, unsigned int mask);

#endif
