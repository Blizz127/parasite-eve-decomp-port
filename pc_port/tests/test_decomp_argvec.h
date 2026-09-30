/*
 * Decomp-derived guest-pointer-vector (E7c) port regression tests.
 *
 * Field-VM opcode handlers receive `T **args`: a guest array of 32-bit guest
 * pointers to their operands.  gen_decomp_ports.py adapts each dereferenced
 * element use (`args[k]`, `*(args + k)`, `*args`) to
 * PE_DECOMP_PTRGLOBAL(pe_args + 4u * (k), T) — the retail `lw` of the element
 * followed by the access through it.  These tests pin the matched-leaf
 * behaviour and prove the element stride is 4 guest bytes (not a host
 * pointer's 8):
 *
 *   * `func_80017F44`  `*a0[1] = (D_8009D1A0 & **a0) ? 1 : 0`
 *   * `func_80016FE0`  `*a0[0] = (D_8009D2E8 & 1) ? 0 : 1`   (inverted gate)
 *   * `func_80017A78`  `D_8009D2E8 &= ~*arg0[0]`
 *   * `func_80017AC0`  `D_8009D2E8 |= *arg0[0]`
 *   * `func_80017AA4`  `*arg0[0] = D_8009D2E8`
 */
#include "pe_guest_decomp.h"

extern int func_80017F44(pe_addr_t a0);
extern int func_80016FE0(pe_addr_t a0);
extern int func_80017A78(pe_addr_t arg0);
extern int func_80017AC0(pe_addr_t arg0);
extern int func_80017AA4(pe_addr_t arg0);

/* Guest scratch: the argument vector, then operands placed out of order so a
 * wrong element stride would read the wrong operand. */
#define ARGV_VEC   0x80152000u
#define ARGV_OP0   0x80152100u
#define ARGV_OP1   0x80152180u

static void ArgvSeed(void)
{
    PE_StoreU32(ARGV_VEC + 0u, ARGV_OP0);
    PE_StoreU32(ARGV_VEC + 4u, ARGV_OP1);
    /* A 64-bit-stride misread would land here; keep it poisoned. */
    PE_StoreU32(ARGV_VEC + 8u, 0u);
}

static void test_DECOMPVEC_bit_test_to_second_operand(void)
{
    TEST("DECOMPVEC_bit_test_to_second_operand");
    ResetTestState();
    ArgvSeed();

    PE_StoreU32(0x8009D1A0u, 0x00002000u);
    PE_StoreU32(ARGV_OP0, 0x00002000u);
    PE_StoreU32(ARGV_OP1, 0xDEADBEEFu);
    ASSERT(func_80017F44(ARGV_VEC) == 1, "handler must return 1");
    ASSERT(PE_LoadU32(ARGV_OP1) == 1u, "a set bit must store 1 through args[1]");
    ASSERT(PE_LoadU32(ARGV_OP0) == 0x00002000u, "args[0] operand is read-only");

    PE_StoreU32(ARGV_OP0, 0x00000001u);
    (void)func_80017F44(ARGV_VEC);
    ASSERT(PE_LoadU32(ARGV_OP1) == 0u, "a clear bit must store 0 through args[1]");
    PASS();
}

static void test_DECOMPVEC_inverted_flag_gate(void)
{
    TEST("DECOMPVEC_inverted_flag_gate");
    ResetTestState();
    ArgvSeed();

    PE_StoreU32(0x8009D2E8u, 1u);
    PE_StoreU32(ARGV_OP0, 0x55u);
    ASSERT(func_80016FE0(ARGV_VEC) == 1, "handler must return 1");
    ASSERT(PE_LoadU32(ARGV_OP0) == 0u, "bit 0 set must write 0 (inverted)");

    PE_StoreU32(0x8009D2E8u, 2u);
    (void)func_80016FE0(ARGV_VEC);
    ASSERT(PE_LoadU32(ARGV_OP0) == 1u, "bit 0 clear must write 1 (inverted)");
    PASS();
}

static void test_DECOMPVEC_flag_word_rmw(void)
{
    TEST("DECOMPVEC_flag_word_rmw");
    ResetTestState();
    ArgvSeed();

    PE_StoreU32(0x8009D2E8u, 0xF0F0u);
    PE_StoreU32(ARGV_OP0, 0x0F10u);
    (void)func_80017AC0(ARGV_VEC);
    ASSERT(PE_LoadU32(0x8009D2E8u) == 0xFFF0u, "OR-in must merge the operand");

    PE_StoreU32(ARGV_OP0, 0x00F0u);
    (void)func_80017A78(ARGV_VEC);
    ASSERT(PE_LoadU32(0x8009D2E8u) == 0xFF00u, "AND-NOT must clear the operand bits");

    PE_StoreU32(ARGV_OP0, 0u);
    (void)func_80017AA4(ARGV_VEC);
    ASSERT(PE_LoadU32(ARGV_OP0) == 0xFF00u, "getter must copy the flag word out");
    PASS();
}

static void test_DECOMPVEC_all(void)
{
    test_DECOMPVEC_bit_test_to_second_operand();
    test_DECOMPVEC_inverted_flag_gate();
    test_DECOMPVEC_flag_word_rmw();
}
