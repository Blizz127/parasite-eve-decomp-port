/*
 * pe_guest_decomp.h — host shim for decomp-derived port TUs.
 *
 * This header is the only thing a TU under pc_port/game/decomp/ needs.  Those
 * TUs are generated, verbatim, from the verified matching C leaves in src/ by
 * tools/analysis/gen_decomp_ports.py.  The matching leaf is written for PSY-Q
 * cc1 targeting the PS1: fixed absolute data addresses (D_800XXXXX), pointer
 * globals, and its callees.  The generated TU is the leaf's logic unchanged;
 * every platform difference is absorbed by the macros this header provides:
 *
 *   * `D_XXXXXXXX` data symbols become either
 *       - a checked guest-RAM lvalue (`PE_DECOMP_SCALAR`), or
 *       - a checked guest-RAM base pointer (`PE_DECOMP_ARRAY`), or
 *       - a checked guest-RAM pointer *value* loaded from the slot and
 *         translated to host memory (`PE_DECOMP_PTRGLOBAL`),
 *     so a leaf's `D_800B0CD8 &= x` and `D_800942E4[i*0xA0C]` both keep
 *     retail semantics and land in the one 2 MiB guest-RAM authority.
 *
 *   * Any callee of the leaf that pc_port has not implemented becomes a
 *     *loud boundary*: PE_Decomp_NoteBoundary() records the symbol (so tests
 *     and the CLI can enumerate unresolved edges).  In the port binary
 *     (native-only STOP policy) it prints CPU_BOUNDARY/REFUSED and stops the
 *     run; in the record policy (tests) the call returns 0.  Never emulated.
 *
 * The matching `src/` tree is NEVER modified by this mechanism.
 */
#ifndef PE_GUEST_DECOMP_H
#define PE_GUEST_DECOMP_H

#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game/decomp_hand/decomp_hand_protos.h"  /* hand-adapter host prototypes */
#include "pe_guestcode.h"                          /* PE_GuestCall (guest code pointers) */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Guest-RAM data-symbol expanders ──────────────────────────────────
 *
 * Generated TU lines look like:
 *   PE_DECOMP_SCALAR(0x8009D2E0u, unsigned int)
 *   PE_DECOMP_ARRAY (0x800A7624u, SearchEntry)
 *   PE_DECOMP_PTRGLOBAL(0x800942E4u, unsigned char)
 */

/* A named scalar inside guest RAM: lvalue, volatile-qualified so repeated
 * reads are not CSE'd away (retail reloads these). */
#define PE_DECOMP_SCALAR(addr, type) \
    (*(volatile type *)PE_Translate((pe_addr_t)(addr), sizeof(type)))

/* A named array/struct base inside guest RAM.  The extent is unknown from an
 * `extern T name[];` declaration, so the base is translated with a one-byte
 * probe and indexed by the leaf; the tests pin the retail extents. */
#define PE_DECOMP_ARRAY(addr, type) \
    ((type *)PE_Translate((pe_addr_t)(addr), 1u))

/* Guest address -> host pointer for a pointer the matched leaf itself
 * null-tests (`if (p)`, `p != 0`, `!p`, `p && ...`, directly or through a
 * plain copy).  The generator uses this only for those names: a guest 0
 * becomes a host NULL so the leaf's own test sees "null" (func_80080998's
 * guard).  Every other pointer translates 0 through the retail RAM mirror
 * (PE_Translate folds KUSEG 0x00000000.. onto 0x80000000..), because on the
 * PS1 a dereference of guest 0 reads low RAM and retail does exactly that. */
static inline void *PE_DecompTranslateOrNull(pe_addr_t addr)
{
    return addr ? PE_Translate(addr, 1u) : (void *)0;
}

/* A pointer *global*: the word at the named slot holds a 32-bit guest address
 * (retail emits lui+lw for these bases).  The generated expression loads that
 * word and translates it (0 -> low RAM through the mirror, like retail); the
 * _NULLABLE form is emitted for slots the leaf null-tests. */
#define PE_DECOMP_PTRGLOBAL(addr, type) \
    ((type *)PE_Translate(PE_LoadU32((pe_addr_t)(addr)), 1u))
#define PE_DECOMP_PTRGLOBAL_NULLABLE(addr, type) \
    ((type *)PE_DecompTranslateOrNull(PE_LoadU32((pe_addr_t)(addr))))

/* Pointer-return adapter: the leaf's result is a host pointer; PE_HostToGuest
 * yields its KSEG0 address.  When the leaf has a single pointer root whose
 * guest value is a KUSEG/KSEG1 alias of main RAM, retail's result is in that
 * same segment (func_800C2B10: D_800E2248 = 0x65 -> returns 0x6D, not
 * 0x8000006D), so the root's segment is re-applied. */
static inline pe_addr_t PE_DecompReturnSegment(pe_addr_t result, pe_addr_t root)
{
    uint32_t seg = root & 0xE0000000u;

    if (result == 0u || (result & 0xE0000000u) != 0x80000000u)
        return result;
    if ((seg == 0x00000000u || seg == 0xA0000000u) &&
        (root & 0x1FFFFFFFu) < PE_RAM_SIZE && (result & 0x1FFFFFFFu) < PE_RAM_SIZE)
        return (result & 0x1FFFFFFFu) | seg;
    return result;
}

/* ── Boundary registry ──────────────────────────────────────────────── */

#define PE_DECOMP_MAX_BOUNDARIES 512

void PE_Decomp_NoteBoundary(const char *symbol);
void PE_Decomp_ResetBoundaries(void);
unsigned PE_Decomp_BoundaryCount(void);
const char *PE_Decomp_BoundaryName(unsigned index);

/* Record an unresolved call edge with its guest argument registers.  The
 * generated TU defines a per-callee boundary shim over this helper; a
 * non-zero `arity` forwards the vector into the established
 * Bootstrap_ReturnInt4Indirect log (unknown slots stay zero). */
int PE_Decomp_Boundary(const char *symbol, unsigned vma, unsigned arity,
                       uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3);
/* Same, naming the native caller (the generated boundary macros pass
 * __func__).  Under the native-only STOP policy (PE_Decomp_SetBoundaryStop(1),
 * the port binary's default) the first boundary prints
 * "CPU_BOUNDARY/REFUSED pc=<vma> ra=<caller> ..." and stops the run. */
int PE_Decomp_BoundaryFrom(const char *caller, const char *symbol, unsigned vma,
                           unsigned arity, uintptr_t a0, uintptr_t a1,
                           uintptr_t a2, uintptr_t a3);
void PE_Decomp_SetBoundaryStop(int stop);
int  PE_Decomp_BoundaryStopEnabled(void);

/* Argument pickers: the generated `#define callee(...) PE_D_COMP_BOUNDARYn(
 * ..., __VA_ARGS__)` selects the guest argument expressions the call site
 * actually passed (padded so a short call yields zero, never a syntax error). */
#define PE_D_COMP_ARG1(_1, ...) (uintptr_t)(_1)
#define PE_D_COMP_ARG2(_1, _2, ...) (uintptr_t)(_2)
#define PE_D_COMP_ARG3(_1, _2, _3, ...) (uintptr_t)(_3)
#define PE_D_COMP_ARG4(_1, _2, _3, _4, ...) (uintptr_t)(_4)

#define PE_D_COMP_BOUNDARY0(sym, vma) \
    PE_Decomp_BoundaryFrom(__func__, sym, vma, 0u, 0u, 0u, 0u, 0u)
#define PE_D_COMP_BOUNDARY1(sym, vma, ...) \
    PE_Decomp_BoundaryFrom(__func__, sym, vma, 1u, \
        PE_D_COMP_ARG1(__VA_ARGS__, 0, 0, 0, 0), 0u, 0u, 0u)
#define PE_D_COMP_BOUNDARY2(sym, vma, ...) \
    PE_Decomp_BoundaryFrom(__func__, sym, vma, 2u, \
        PE_D_COMP_ARG1(__VA_ARGS__, 0, 0, 0, 0), \
        PE_D_COMP_ARG2(__VA_ARGS__, 0, 0, 0, 0), 0u, 0u)
#define PE_D_COMP_BOUNDARY3(sym, vma, ...) \
    PE_Decomp_BoundaryFrom(__func__, sym, vma, 3u, \
        PE_D_COMP_ARG1(__VA_ARGS__, 0, 0, 0, 0), \
        PE_D_COMP_ARG2(__VA_ARGS__, 0, 0, 0, 0), \
        PE_D_COMP_ARG3(__VA_ARGS__, 0, 0, 0, 0), 0u)
#define PE_D_COMP_BOUNDARY4(sym, vma, ...) \
    PE_Decomp_BoundaryFrom(__func__, sym, vma, 4u, \
        PE_D_COMP_ARG1(__VA_ARGS__, 0, 0, 0, 0), \
        PE_D_COMP_ARG2(__VA_ARGS__, 0, 0, 0, 0), \
        PE_D_COMP_ARG3(__VA_ARGS__, 0, 0, 0, 0), \
        PE_D_COMP_ARG4(__VA_ARGS__, 0, 0, 0, 0))

#ifdef __cplusplus
}
#endif

#endif /* PE_GUEST_DECOMP_H */
