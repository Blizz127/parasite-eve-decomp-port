/*
 * The decomp boundary primitive on its own, for clean-room natives that
 * reach a guest function with no port implementation (the same call the
 * generated PE_D_COMP_BOUNDARYn macros make).  pe_guest_decomp.h includes
 * this header; natives include it directly, because pe_guest_decomp.h also
 * pulls in the hand-adapter prototypes.
 */
#ifndef PE_DECOMP_BOUNDARY_H
#define PE_DECOMP_BOUNDARY_H

#include <stdint.h>

/* Record a call to an unimplemented guest function at `vma` (`symbol`),
 * naming the native caller.  Under the native-only STOP policy
 * (PE_Decomp_SetBoundaryStop(1), the port binary's default) the first
 * boundary prints "CPU_BOUNDARY/REFUSED pc=<vma> ra=<caller> ..." and stops
 * the run. */
int PE_Decomp_BoundaryFrom(const char *caller, const char *symbol, unsigned vma,
                           unsigned arity, uintptr_t a0, uintptr_t a1,
                           uintptr_t a2, uintptr_t a3);

#endif
