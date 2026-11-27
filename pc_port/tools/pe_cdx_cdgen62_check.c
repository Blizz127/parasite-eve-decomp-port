/* Checker for the cdgen62 set (pe_cdx_cdgen62_vectors.py): the libc formatter 0x80071A84 (sprintf ABI: destination, format, argument words, words beyond
 * the fourth in the caller's outgoing stack area), provider under test PE_FormatterFrame (pc_port/game/boot/func_80071A84_port.c, a frame-level
 * transliteration; compiled out of tree, black box).  Scaffold pe_cdx_common_canary.h (canary seeding, store-footprint, controls noop / flip / retflip /
 * storeflip / poke0 / same).  Output bytes are compared exactly: the final-image diff (ram-image) covers every byte of the destination, the canary seeding
 * catches overruns and omitted bytes, the return value is the character count.
 * Frame contract: the provider is called with caller_sp = 0x801FF000 (the interpreter's entry stack pointer), so the varargs the original read from sp+0x10..
 * are the recorded stack loads the checker seeds, the home-area stores at 0x801FF004..0C are part of the footprint, and the frame the provider keeps below
 * caller_sp (caller_sp - 0x250 ..) lies in the exempt own-frame window 0x801FE000..FFF exactly as the original's frame does.  saved[9] (incoming s0..s7, ra)
 * are zero.
 * Seams (nm -u of the object): pe_plat_libc_move_bytes (= retail 0x80072334, the byte mover the formatter copies digits / strings through): the stand-in performs
 * its effect through PE_LoadU8 / PE_StoreU8 (memmove semantics: copy backward when the destination is above the source), the call is not recorded because the
 * original ran it inline; Bootstrap_ReturnInt4Indirect and PE_Port_RequestStop are the provider's unresolved-call boundary (the %s arms reach the BIOS strlen / memchr
 * veneers 0x80072314 / 0x80072324 that way): both are TRAPs, a call fails the case.  PE_Port_ShouldStop answers 0.
 * Build line (executed by pe_cdx_cdgen52_61_run.sh):
 * BUILD-formatter: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen62_check.c $O/func_80071A84_port.o -o $O/c62_formatter
 * Run: $O/c62_formatter <format.txt> [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
int Bootstrap_ReturnInt4Indirect(const char *symbol, const char *caller, int value, uintptr_t target, uintptr_t a0, uintptr_t a1, uintptr_t a2,
                                 uintptr_t a3, const void *payload, uint32_t payload_size)
{ (void)symbol; (void)caller; (void)target; (void)a0; (void)a1; (void)a2; (void)a3; (void)payload; (void)payload_size; bad_call = 1; trap_name = "Bootstrap_ReturnInt4Indirect"; return value; }
uint32_t pe_plat_libc_move_bytes(uint32_t dst, uint32_t src, uint32_t n)
{
    uint32_t i;
    if (dst < src) for (i = 0; i < n; i++) PE_StoreU8(dst + i, PE_LoadU8(src + i));
    else for (i = n; i > 0; i--) PE_StoreU8(dst + i - 1, PE_LoadU8(src + i - 1));
    return dst;
}
uint32_t PE_FormatterFrame(pe_addr_t destination, pe_addr_t format, uint32_t arg0, uint32_t arg1, pe_addr_t caller_sp, const uint32_t saved[9]);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    static const uint32_t saved[9];
    if (entry != 0x80071A84u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = PE_FormatterFrame(a[0], a[1], a[2], a[3], 0x801FF000u, saved);
    return 1;
}
