/* Isolated hook arity contract check; link with pe_guestcode.c and section GC.
 * Registry, RAM, diagnostics and stop service are doubles; no game runs. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guestcode.h"
#include "pe_bios_string.h"
#include "pe_bootstrap.h"
#include "pe_decomp_boundary.h"
#include "game_port.h"

static int calls, stack_calls, boundaries, refusals, stops;
static unsigned boundary_arity;
static uintptr_t boundary_args[4];
static int thunk(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d)
{ calls++; return (int)(a + b + c + d); }
static int stack_thunk(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d,
                       uintptr_t e, uintptr_t f, uintptr_t g, uintptr_t h)
{
    stack_calls++;
    if (a != 1 || b != 2 || c != 3 || d != 4 || e != 5 || f || g || h) abort();
    return 15;
}
static const PeGuestCodeEntry entries[] = {
    {0x80010000u, 4u, 0u, thunk, "synthetic_thunk", 0u, 0, 0u},
    {0x80010004u, 4u, 0u, 0, "synthetic_stack_thunk", 0u, stack_thunk, 5u}
};
static const PeGuestCodeTable exe = {"exe", 0x80010000u, 0x80010008u, 0, 0, entries, 2};
static const PeGuestCodeTable *tables[] = {&exe};
pe_addr_t PE_RamCanonical(pe_addr_t a, size_t n) { (void)n; return a; }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { (void)a; (void)n; return false; }
uint8_t PE_LoadU8(pe_addr_t a) { (void)a; return 0; }
void PE_Port_EscapePoint(void) { }
void PE_Port_RequestStop(PEPortStopReason reason)
{ if (reason != PE_PORT_STOP_UNRESOLVED_BOUNDARY) abort(); stops++; }
void PE_Decomp_NoteStopDetail(const char *text) { if (!text) abort(); }
void Bootstrap_ReturnVoid4Indirect(const char *s, const char *c, uintptr_t fn,
                                   uintptr_t a, uintptr_t b, uintptr_t d, uintptr_t e)
{ (void)s; (void)c; (void)fn; (void)a; (void)b; (void)d; (void)e; refusals++; }
int PE_Decomp_BoundaryFrom(const char *c, const char *s, unsigned fn, unsigned arity,
                           uintptr_t a, uintptr_t b, uintptr_t d, uintptr_t e)
{
    (void)c; (void)s; (void)fn;
    boundary_arity = arity;
    boundary_args[0] = a; boundary_args[1] = b;
    boundary_args[2] = d; boundary_args[3] = e;
    boundaries++;
    return 0;
}
int32_t PE_Printf_Guest(pe_addr_t f, uint32_t a, uint32_t b, uint32_t c,
                        uint32_t d, uint32_t e, uint32_t g, uint32_t h)
{ (void)f; (void)a; (void)b; (void)c; (void)d; (void)e; (void)g; (void)h; abort(); }

int main(void)
{
    int value = -123;
    PE_GuestCode_SetRegistry(tables, 1);
    if (!PE_GuestHook_Call("slot", "caller", 0x80010000u, 4, 1, 2, 3, 4, &value) ||
        value != 10 || calls != 1 || stops || refusals || boundaries) abort();
    const uintptr_t args[] = {1, 2, 3, 4, 5};
    if (PE_GuestCallN("site", 0x80010004u, 5, args) != 15 || stack_calls != 1 ||
        stops || refusals || boundaries) abort();
    if (PE_GuestCallN("site", 0x80010004u, 4, args) != 0 || stack_calls != 1 ||
        stops || refusals || boundaries != 1) abort();
    boundaries = 0;
    value = -123;
    if (PE_GuestHook_Call("slot", "caller", 0x80010004u, 4, 1, 2, 3, 4, &value) ||
        value != -123 || stack_calls != 1 || refusals != 1 || stops != 1 || boundaries) abort();
    refusals = stops = 0;
    value = -123;
    int ok = PE_GuestHook_Call("slot", "caller", 0x80010000u, 5, 1, 2, 3, 4, &value);
    printf("arity5: ok=%d ret=%d calls=%d refusals=%d stops=%d boundaries=%d\n",
           ok, value, calls, refusals, stops, boundaries);
    fflush(stdout);
    if (ok || value != -123 || calls != 1 || refusals != 1 || stops != 1 || boundaries) abort();
    value = -123;
    ok = PE_GuestHook_Call("slot", "caller", 0, 4, 1, 2, 3, 4, &value);
    if (ok || value != -123 || calls != 1 || refusals != 2 || stops != 2 || boundaries) abort();
    const uintptr_t too_many[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    if (PE_GuestCallN("site", 0x80010004u, 9, too_many) != 0) abort();
    printf("over-limit: boundary arity=%u calls=%d stack_calls=%d\n",
           boundary_arity, calls, stack_calls);
    fflush(stdout);
    if (boundaries != 1 || boundary_arity != 9 || calls != 1 || stack_calls != 1 ||
        boundary_args[0] != 1 || boundary_args[1] != 2 ||
        boundary_args[2] != 3 || boundary_args[3] != 4) abort();
    puts("four-register, stack-call arity, stack-hook refusal and unknown-target controls pass");
    return 0;
}
