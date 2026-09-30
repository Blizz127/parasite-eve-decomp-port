/*
 * PE-CH2 — func_80065954: opcode 0x75 camera-slot flag update.
 *
 * Complete retail body (18 words / 0x48, exe 0x80065954–0x8006599C,
 * file offset 0x56154).  The sole caller is opcode handler
 * func_80018B98 at jal 0x80018BB0.
 *
 * The slot address is *(D_800B1624) + offset_at_+0x10 + index*16.
 * A nonzero flag ORs byte 0 with 6; zero clears those bits with 0xF9.
 * The indexed sb at 0x80065990 is the only store.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B1624 0x800B1624u

/* func_80065954: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80065954_port.c (src/func_80065954.c); hand port retired (switch1 lane, audit klass a-replaceable, generator-verified eligible). */
