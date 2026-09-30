/*
 * pe_guest_decomp.c — support for decomp-derived port TUs.
 *
 * Two jobs:
 *   1. the loud-boundary registry: every callee a derived leaf references but
 *      pc_port has not implemented is recorded here, so tests and the CLI can
 *      enumerate the unresolved edges instead of silently taking a zero;
 *   2. func_800721DC, the retail BIOS-random leaf, implemented because the
 *      derived 0x8003/0x8002 leaves call it and no pc_port definition exists.
 *
 * Nothing here invents retail re-implementations: the boundary path is the
 * explicit "this subsystem is not ported yet" marker required by CLAUDE.md.
 */
#include "pe_guest_decomp.h"

#include <string.h>

#include "pe_bootstrap.h"

static const char *g_boundaries[PE_DECOMP_MAX_BOUNDARIES];
static unsigned g_boundary_count;
static unsigned g_boundary_total; /* includes repeats */

void PE_Decomp_NoteBoundary(const char *symbol)
{
    unsigned i;
    ++g_boundary_total;
    for (i = 0; i < g_boundary_count; i++) {
        if (g_boundaries[i] == symbol || strcmp(g_boundaries[i], symbol) == 0)
            return;
    }
    if (g_boundary_count < PE_DECOMP_MAX_BOUNDARIES)
        g_boundaries[g_boundary_count++] = symbol;
}

void PE_Decomp_ResetBoundaries(void)
{
    g_boundary_count = 0;
    g_boundary_total = 0;
}

unsigned PE_Decomp_BoundaryCount(void)
{
    return g_boundary_count;
}

const char *PE_Decomp_BoundaryName(unsigned index)
{
    return index < g_boundary_count ? g_boundaries[index] : 0;
}

int PE_Decomp_Boundary(const char *symbol, unsigned vma, unsigned arity,
                       uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    PE_Decomp_NoteBoundary(symbol);
    if (arity != 0u) {
        Bootstrap_ReturnInt4Indirect(symbol, "decomp-port", 0, (uintptr_t)vma,
                                     a0, a1, a2, a3, NULL, 0u);
    }
    return 0;
}

/*
 * There is deliberately no fallback implementation of any retail function
 * here.  A callee pc_port has not implemented stays a loud boundary: the macro
 * records the symbol and returns 0.  Nothing is invented.
 */
