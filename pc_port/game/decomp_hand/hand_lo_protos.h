/*
 * Host prototypes for the hand adapters in pc_port/game/decomp_hand/ whose
 * matched leaf sits at VRAM < 0x80060000 (port_absent lane).
 *
 * Every function here is a hand adaptation of the verified matched leaf
 * `src/func_XXXXXXXX.c` (see docs/ai_context/PC_PORT_FROM_DECOMP.md): retail
 * pointer parameters/returns become 32-bit guest addresses (`pe_addr_t`),
 * guest pointers read out of guest RAM are loaded with PE_LoadU32, and every
 * other parameter keeps the leaf's type.  gen_decomp_ports.py reads these
 * prototypes as the canonical host signatures, so generated callers bind to
 * the hand adapter instead of a loud boundary.
 *
 * Code-pointer arguments (`func_800638D8(slot, func_80050708)`) are passed as
 * the callee's retail VMA, which is what the retail `lui/addiu` pair loads.
 */
#ifndef PE_HAND_LO_PROTOS_H
#define PE_HAND_LO_PROTOS_H

#include <stdint.h>
#include "psx_compat.h"

/* Guest scratch for retail stack temporaries that a leaf hands to a callee
 * taking a guest address (a host stack address is never a guest address).
 * Storage-location substitution only: the bytes written and read back are
 * exactly retail's, and none of them outlives the call that uses them. */
#define PE_HAND_LO_STACK_TEMP 0x801FFD00u

/* ── field-VM opcode handlers / small helpers (absent_lo_small_port.c) ── */
int func_80015AB8(pe_addr_t a0);
int func_80017D18(void);
int func_80017E68(pe_addr_t a0);
int func_800182A0(void);
int func_800182C0(void);
int func_80018754(void);
int func_80018818(pe_addr_t a0);
int func_80018864(pe_addr_t arg0);
int func_80018B30(pe_addr_t a0);
int func_80018B98(pe_addr_t arg0);
int func_80018C10(pe_addr_t a0);
int func_80019450(pe_addr_t a0);
int func_800194F8(pe_addr_t a0);
int func_80019A1C(pe_addr_t a0);
int func_8001A0CC(int arg0);
int func_8001A114(int arg0);
int func_8001A43C(pe_addr_t a0);
int func_8001A474(pe_addr_t a0);
int func_80038CE4(unsigned int a0);
void func_8003E5F0(void);
void func_8004D690(int arg0);
void func_8004EC3C(pe_addr_t a0);
void func_8004EC78(pe_addr_t a0);
void func_8004EF30(int slot);
void func_8004F2E4(int slot);
void func_80050038(int slot);
void func_80050060(int slot);
void func_8005184C(unsigned int bit);
unsigned char func_8005421C(int value);
int func_8005D940(void);   /* retail $v0 of func_8006E454, see definition */
int func_8005DE70(void);
int func_80017A24(pe_addr_t arg0);
int func_80017C8C(pe_addr_t arg0);

/* ── state handlers / constructors / list walkers (absent_lo_state_port.c) ── */
int func_80017D3C(pe_addr_t arg0);
int func_80018598(void);
int func_80018660(pe_addr_t a0);
int func_80018894(pe_addr_t arg0);
int func_800190BC(pe_addr_t a0);
int func_800199CC(pe_addr_t a0);
int func_8001A064(void);
void func_8001A9F8(void);
void func_80020C74(void);
void func_80033430(void);
int func_80039970(void);
void func_8003E0D0(pe_addr_t arg0);
int func_8003E0FC(pe_addr_t a0, short key, pe_addr_t out);
int func_80042BC8(void);
void func_80047BEC(int a0);
void func_8004D978(int a0);
void func_8004EBCC(int a0);
void func_8004FAF8(int a0);
void func_8004FEEC(int a0);
void func_80050544(int unused, int enabled);
void func_80050690(int a0);   /* retail live $a0, see definition */
void func_80052EC0(void);
void func_800577E0(int a0, int a1);

/* ── constructors / record getters / stack-temp handlers (absent_lo_menu_port.c) ── */
int func_80017BB4(pe_addr_t a0);
int func_800183E8(pe_addr_t a0);
int func_800188C4(pe_addr_t a0);
pe_addr_t func_800424B4(int a0, int a1);
void func_8004784C(void);
int func_80047F48(int a0, int a1);
void func_8004F23C(void);
int func_8004FE58(int a0);
void func_80050E70(int a0);
int func_80053F90(int a0);
int func_8005DD8C(int a0);
pe_addr_t func_8005DE08(int a0);
void func_800434C0(int a);

/* ── record copiers / queue + packet helpers (absent_lo_records_port.c) ── */
void func_8003DBE4(pe_addr_t a0, pe_addr_t a1, short idx);
void func_8003DD08(pe_addr_t a0, pe_addr_t a1, short a2);
void func_8003DF50(pe_addr_t dst, pe_addr_t source, short index);
/* ── field lighting-state setters (gte_light_state_port.c) ── */
void func_8003F758(pe_addr_t dst, int r, int g, int b);
void func_8003F798(pe_addr_t state, int index, int r, int g, int b);
int func_80018460(pe_addr_t a0);
int func_80042170(int card, int slot);
void func_80047E94(int a0);
void func_80059C44(void);
void func_80059FD0(void);
int func_8005BF44(int id);
void func_8003E474(pe_addr_t m, int du, int dv, signed char dc);
int func_8005BBE4(int a0, int key);
void func_8005DEE4(int a0, int a1);
void func_8005E9C8(int a0, int a1, int a2, int a3);
void func_8005EA8C(void);

/* ── field-menu window builders / callbacks (absent_lo_window_port.c) ── */
int func_80019260(void);
void func_80047560(void);
void func_80047C50(void);
void func_80048F24(void);
void func_8004A570(int a0, int a1);
int func_8004D4C4(int a0, int a1);
int func_8004F30C(int a0, int a1);

/* ── menu builders / event-queue consumer (absent_lo_window2_port.c) ── */
void func_8004D084(int a0);
int func_80047D74(int a0, int a1);
void func_8005033C(int a0, int a1);
void func_8005DF6C(int mask, pe_addr_t out);
int func_80047A30(int a0, int a1);

/* ── item/menu panels, banner tick (absent_lo_panel_port.c) ── */
void func_80034DE0(void);
void func_800480AC(void);
void func_8004790C(void);
void func_80058AA8(int a0);
void func_800506E8(int a0);   /* retail live $a0, see definition */

/* ── field-VM actor-targeting handlers (absent_lo_actor_port.c) ── */
int func_80015108(pe_addr_t arg0);
int func_80015648(pe_addr_t arg0);
int func_80019170(pe_addr_t arg0);
int func_80019F04(pe_addr_t arg0);
void func_8001A680(pe_addr_t body, unsigned int id);

/* ── screen reset / save header / equip exit (absent_lo_misc_port.c) ── */
void func_8003F2FC(void);
pe_addr_t func_80040210(int a0, int secs);
void func_800490B0(void);
int func_80049354(int a0, int a1);
void func_80042264(void);
void func_8005C25C(void);
void func_8005C374(void);
int func_800491C8(pe_addr_t a, int f0);
int func_80015790(pe_addr_t a0);

/* ── large leaves (absent_lo_large_port.c) ── */
void func_800588EC(int a0);
int func_8004D6D4(int a, int flags);
void func_8005D020(void);
void func_8002F300(void);

/* ── batch-3 leaves (absent_lo_batch3_port.c) ── */
int func_80014E30(pe_addr_t a0);
void func_80029810(unsigned char id);
void func_8005F698(pe_addr_t p, int width);

/* func_80038954 is ported in absent_sdk_port.c (prototype in
 * hand_absent_sdk_protos.h): its retail fatal-error halt (`for (;;) {}` when
 * the fourth argument's low byte is 1) becomes a run-control stop request —
 * execution never continues past that point on retail either. */

#endif /* PE_HAND_LO_PROTOS_H */
