/* Focused synthetic test for floor zero-divide propagation through the actor list.
 * Link only the named floor, actor-list, guest-RAM, guest-decomp and run-control
 * sources with function-section garbage collection and a linker wrap for the
 * existing fixed-point multiply provider. GTE, MMIO and unrelated callbacks
 * abort if reached.
 * The fixture does not exercise game data, the PS1 exception handler or devices.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_guest_decomp.h"
#include "pe_sdk.h"
#include "pe_mmio.h"
#include "game_port.h"

#define MESH_P 0x8009D1FCu
#define PLANES 0x8009D1D8u
#define EDGES  0x8009CE14u
#define AYA    0x8009D254u
#define INPUT  0x8009D2E8u

int PE_MMIO_InShadowBlock(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void PE_MMIO_Store(pe_addr_t a, size_t n, uint32_t v)
{ (void)a; (void)n; (void)v; abort(); }
void *PE_MMIO_Translate(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }

static unsigned multiply_calls;
extern uint32_t __real_func_8003708C(uint32_t, uint32_t);
uint32_t __wrap_func_8003708C(uint32_t a, uint32_t b)
{ multiply_calls++; return __real_func_8003708C(a, b); }
int func_8001C614(pe_addr_t rec, int x, int z)
{ (void)rec; (void)x; (void)z; abort(); }
void func_8001D170(void) { abort(); }
void func_8001A9F8(void);
PeGteState g_pe_gte;
int32_t PE_GTE_NCLIP(void) { abort(); }

int main(void)
{
    for (int dim = 0; dim < 2; ++dim) for (int stop = 1; stop >= 0; --stop)
    for (int zero = 1; zero >= 0; --zero) {
        PE_RamReset();
        g_port_stop_requested = 0;
        multiply_calls = 0;
        PE_Decomp_SetBoundaryStop(stop);
        unsigned traps = PE_Decomp_GuestTrapCount();
        unsigned epoch = PE_Port_StopEpoch();

        PE_StoreU32(MESH_P, 0x80130000u);
        PE_StoreU16(0x80130004u, 2);
        PE_StoreU16(0x80130008u, 1);
        PE_StoreU32(0x80130018u, 0x80130100u);
        PE_StoreU32(0x8013001cu, 0x80130200u);
        PE_StoreU32(EDGES, 0x80130300u);
        PE_StoreU32(PLANES, dim ? 0x80130400u : 0u);
        PE_StoreU32(AYA, 0);
        PE_StoreU32(INPUT, 0x100);
        PE_StoreU32(0x8009d20cu, 0x80120000u);
        PE_StoreU32(0x80120004u, 0x80121000u);
        PE_StoreU32(0x80121004u, 0);
        PE_StoreU16(0x80120026u, 4096);
        PE_StoreU16(0x80120224u, 65);
        PE_StoreU32(0x80120028u, 950u << 16);
        PE_StoreU32(0x80120030u, 315u << 16);
        PE_StoreU32(0x80120040u, 930u << 16);
        PE_StoreU32(0x80120048u, 300u << 16);
        PE_StoreU32(0x801201a4u, 0x80130200u);
        PE_StoreU32(0x80120098u, 0x80000u);
        PE_StoreU32(0x801211a4u, 0x80130200u);
        PE_StoreU32(0x80121098u, 0x80000u);
        unsigned vertex_offset = dim ? 8u : 2u;
        unsigned edge_offset = dim ? 14u : 8u;
        unsigned neighbor_offset = dim ? 20u : 14u;
        unsigned stride = dim ? 6u : 4u;
        for (unsigned i = 0; i < 3; ++i) {
            PE_StoreU16(0x80130200u + vertex_offset + 2u*i, (uint16_t)i);
            PE_StoreU16(0x80130200u + edge_offset + 2u*i, (uint16_t)i);
            PE_StoreU16(0x80130200u + neighbor_offset + 2u*i, 0xffffu);
            PE_StoreU16(0x80130100u + stride*i, i ? 1000 : 0);
            PE_StoreU16(0x80130100u + stride*i + (dim ? 4u : 2u), i == 2 ? 1000 : 0);
            PE_StoreU32(0x80130300u + 12u*i, zero ? 0u : 65536000u);
        }
        func_8001A9F8();
        int expected_stop = zero && stop;
        if (PE_Decomp_GuestTrapCount() != traps + (unsigned)zero ||
            PE_Port_StopEpoch() != epoch + (unsigned)expected_stop ||
            !!g_port_stop_requested != expected_stop ||
            (expected_stop && (PE_Port_GetStopReason() != PE_PORT_STOP_GUEST_TRAP ||
                      strstr(PE_Decomp_StopDetail(), "floor collision") == NULL)) ||
            PE_LoadU32(0x80121098u) != (expected_stop ? 0x80000u : 0u) ||
            multiply_calls != (expected_stop ? 0u : 2u))
            abort();
        printf("PASS floor %s %s %s: next actor %s\n", dim ? "3D" : "2D",
               zero ? "zero" : "normal", stop ? "STOP" : "record",
               expected_stop ? "untouched" : "processed");
        PE_RamDestroy();
    }
    return 0;
}
