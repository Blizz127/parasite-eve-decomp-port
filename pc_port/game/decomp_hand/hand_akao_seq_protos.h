/*
 * Hand-adapter prototypes, AKAO sequence start / bank restore (native-only
 * pass, 2026-10-07).  Included through decomp_hand_protos.h.  Definitions:
 * akao_seq_port.c.
 */
#ifndef PE_HAND_AKAO_SEQ_PROTOS_H
#define PE_HAND_AKAO_SEQ_PROTOS_H

/* src/func_8008A750.c: start one voice of a nested stream (Seq_StartNestedStreams). */
void func_8008A750(pe_addr_t a0, pe_addr_t s, unsigned int m, int a3);
/* src/func_8008AC40.c: restore the saved sequence-0 bank and shift its clocks. */
void func_8008AC40(int arg0);

#endif
