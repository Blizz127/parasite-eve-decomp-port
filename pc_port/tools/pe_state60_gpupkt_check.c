/* State60 vector checker, family gpupkt (batch state60c): GPU packet setters / link / draw-page (pe_plat2_gpupkt.c).
 * Build (run from the repository root; provider object is linked as a black box):
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -O1 -fno-stack-protector -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -Ipc_port/tools pc_port/tools/pe_state60_gpupkt_check.c build/state60/prov/pe_plat2_gpupkt.c.o -Wl,--unresolved-symbols=ignore-all -o build/state60/check_gpupkt
 * Run:  ./build/state60/check_gpupkt <vector files of state60c>
 * Provider object under test: build/state60/prov/pe_plat2_gpupkt.c.o (compiled from pc_port/platform/pe_plat2_gpupkt.c, see AUTHOR-REPORT.md).
 * Replay rules are in pe_state60_check_core.h; vectors come from pc_port/tools/pe_state60_vectors.py state60c. */
#define _POSIX_C_SOURCE 200809L
#include "pe_state60_check_core.h"
#include "pe_plat/gpupkt.h"
static unsigned family_arity(uint32_t fn) { (void)fn; return 0; }
static int family_call(uint32_t entry, const uint32_t *a, uint32_t *ret, int *has_ret, unsigned *rw)
{
    *rw = 4; *has_ret = 0;
    switch (entry) {
    case 0x80077CB4u: *ret = (uint32_t)pe_plat_gpupkt_append_packet(a[0], a[1]); *has_ret = 1; return 1;
    case 0x80077B04u: pe_plat_gpupkt_set_semitrans(a[0], (int)a[1]); return 1;
    case 0x80077B34u: pe_plat_gpupkt_set_shade_texture(a[0], (int)a[1]); return 1;
    case 0x80077B64u: pe_plat_gpupkt_poly_f3(a[0]); return 1;
    case 0x80077B84u: pe_plat_gpupkt_poly_g3(a[0]); return 1;
    case 0x80077BA4u: pe_plat_gpupkt_poly_ft4(a[0]); return 1;
    case 0x80077BC4u: pe_plat_gpupkt_poly_g4(a[0]); return 1;
    case 0x80077BE4u: pe_plat_gpupkt_poly_gt4(a[0]); return 1;
    case 0x80077C04u: pe_plat_gpupkt_sprite(a[0]); return 1;
    case 0x80077C24u: pe_plat_gpupkt_tile1(a[0]); return 1;
    case 0x80077C44u: pe_plat_gpupkt_tile(a[0]); return 1;
    case 0x80077C64u: pe_plat_gpupkt_line_f2(a[0]); return 1;
    case 0x80077C84u: *ret = pe_plat_gpupkt_draw_page(a[0], a[1], a[2], a[3]); *has_ret = 1; return 1;
    }
    return -1;
}
