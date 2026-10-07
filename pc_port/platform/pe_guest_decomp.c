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

#include <stdio.h>
#include <string.h>

#include "pe_bootstrap.h"
#include "game_port.h"

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

/* ── Native-only boundary policy (owner, 2026-10-07) ──────────────────
 * A boundary is retail code the port has no native C for.  The port never
 * emulates it.  With the STOP policy (the parasite-eve-port default, see
 * port_main.c) the first boundary prints
 *     CPU_BOUNDARY/REFUSED pc=0x<callee vma> ra=<native caller> sym=<name> ...
 * and requests PE_PORT_STOP_UNRESOLVED_BOUNDARY, so the run ends on the
 * missing function instead of continuing with an invented 0.  The library
 * default stays RECORD (tests enumerate boundaries without stopping). */
static int g_boundary_stop;

void PE_Decomp_SetBoundaryStop(int stop) { g_boundary_stop = stop ? 1 : 0; }
int  PE_Decomp_BoundaryStopEnabled(void) { return g_boundary_stop; }

int PE_Decomp_BoundaryFrom(const char *caller, const char *symbol, unsigned vma,
                           unsigned arity, uintptr_t a0, uintptr_t a1,
                           uintptr_t a2, uintptr_t a3)
{
    PE_Decomp_NoteBoundary(symbol);
    if (g_boundary_stop) {
        static unsigned printed;
        if (printed < 8u) {
            printed++;
            fprintf(stderr,
                    "CPU_BOUNDARY/REFUSED pc=0x%08X ra=%s sym=%s arity=%u "
                    "a0=0x%08X a1=0x%08X a2=0x%08X a3=0x%08X (native only: "
                    "no C for this callee; not emulated)\n",
                    vma, caller ? caller : "?", symbol ? symbol : "?", arity,
                    (unsigned)(uint32_t)a0, (unsigned)(uint32_t)a1,
                    (unsigned)(uint32_t)a2, (unsigned)(uint32_t)a3);
            fflush(stderr);
        }
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    }
    if (arity != 0u) {
        Bootstrap_ReturnInt4Indirect(symbol, "decomp-port", 0, (uintptr_t)vma,
                                     a0, a1, a2, a3, NULL, 0u);
    }
    return 0;
}

int PE_Decomp_Boundary(const char *symbol, unsigned vma, unsigned arity,
                       uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{
    return PE_Decomp_BoundaryFrom(NULL, symbol, vma, arity, a0, a1, a2, a3);
}

/*
 * There is deliberately no fallback implementation of any retail function
 * here.  A callee pc_port has not implemented stays a loud boundary: the macro
 * records the symbol and returns 0.  Nothing is invented.
 */
