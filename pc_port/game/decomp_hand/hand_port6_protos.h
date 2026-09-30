/*
 * Hand-adapter prototypes, port lane round 6 (2026-09-27).  Included through
 * decomp_hand_protos.h.  Definitions: port6_port.c.
 */
#ifndef PE_HAND_PORT6_PROTOS_H
#define PE_HAND_PORT6_PROTOS_H

/* src/func_80048918.c: item/equipment window constructor. */
void func_80048918(int a0, int a1, int a2);
/* src/func_8007E4E0.c: exception-return stub install (0xDF80). */
void func_8007E4E0(void);
/* src/func_8004EF58.c: 0x36 item-frame window input handler. */
int func_8004EF58(pe_addr_t self, int key);

#endif /* PE_HAND_PORT6_PROTOS_H */
