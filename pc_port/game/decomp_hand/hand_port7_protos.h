/*
 * Hand-adapter prototypes, port lane round 7 (port2, 2026-09-27).  Included
 * through decomp_hand_protos.h.  Definitions: port7_port.c.
 */
#ifndef PE_HAND_PORT7_PROTOS_H
#define PE_HAND_PORT7_PROTOS_H

/* src/func_80068710.c: grid cursor step over packed (x<<22 | y<<12) cells. */
int func_80068710(pe_addr_t o, pe_addr_t it, unsigned char dir, unsigned char wrap);
/* src/func_80083644.c: pad-record reply parser (default mode handler). */
int func_80083644(pe_addr_t a0);
/* src/func_80084C4C.c: pad-record receive hook (installed at D_8009B744). */
void func_80084C4C(pe_addr_t a0);

/* src/func_8001CBA0.c: push an object out across polygon edge i. */
void func_8001CBA0(pe_addr_t o, pe_addr_t p, unsigned short n, short i);
/* src/func_8001CE88.c: first polygon edge within D_8009CE2C of (x, z). */
int func_8001CE88(short x, short z, pe_addr_t p, unsigned short n);
/* src/func_8008A068.c: SEQ header parse + 24-voice channel init. */
void func_8008A068(pe_addr_t a0);
/* src/func_80081E70.c: CD data-ready (DsRead) interrupt callback. */
void func_80081E70(unsigned char intr, pe_addr_t result);
/* src/func_8001D170.c: actor vs boundary-polygon step (push-out / cancel). */
void func_8001D170(void);
/* src/func_800DE7A8.c: battle particle pool callback (move / draw). */
int func_800DE7A8(int a0, pe_addr_t a1);
/* src/func_80058E44.c: can inventory slot idx be moved (not equipped / not
 * the last of a kind-8 item)? */
int func_80058E44(int idx);
/* src/func_80058FEC.c: swap two inventory/equipment entries (menu 0x33/0x34). */
int func_80058FEC(int a, int b, int c, int d);
/* src/func_80054F58.c: equipment exchange compatibility code (1/3/4/5 or
 * the func_80054E4C verdict). */
int func_80054F58(int side, int k);
/* src/func_80088344.c: SEQ voice LFO / volume / pan / pitch update. */
void func_80088344(pe_addr_t v, unsigned int mask);
/* src/func_80048254.c: equipment-exchange confirm (menu bit 16). */
void func_80048254(void);

/* Guest scratch for round-7 retail stack buffers handed to guest-address
 * callees: 0x801FF5C0..0x801FF5FF, the unused top of the reserved round-4
 * window (PE_HAND_R4_STACK, 0x801FF500..0x801FF5FF, "needs none today"),
 * inside the portverify-masked scratch 0x801FF500..0x801FFD7F. */
#define PE_HAND_P7_STACK 0x801FF5C0u

#endif /* PE_HAND_PORT7_PROTOS_H */
