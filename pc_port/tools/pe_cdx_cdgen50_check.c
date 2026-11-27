/* Checker for the cdgen50 sets (pe_cdx_cdgen50_vectors.py): the retail CD read poll 0x800811E4.  Scaffold pe_cdx_common_canary.h (the int3 pe_cdx_common.h:
 * canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per build, linked as a black box;
 * the object is pc_port/platform/pe_plat_cd_poll.c of the int3 tree (read-only), which defines BOTH
 *   pe_plat_cd_read_poll        the independent retail poll core (reaches the retrace clock, the abort entry and the ready entry through bound symbols),
 *   pe_plat_cd_read_poll_host   the legacy host adapter (reads the host counter through HostFB_GetState; clears 0x8009B6CC locally on expiry; calls nothing else).
 *   -DC50_HOST=1       entry = the legacy adapter.  Strict (default): HostFB_GetState reports the host's own VSync-CALL counter, which the checker holds at 0 (it is not
 *                      the retail retrace clock, so it cannot follow the vector's scripted VSync(-1) value);
 *                      -DC50_MAP_VSYNC=1 (mapped): HostFB_GetState reports the scripted VSync(-1) value of the case (peeked, not recorded as a call): the most
 *                      generous seam mapping, so only the adapter's own logic and footprint are measured.
 *   -DC50_CORE=1       entry = the retail poll core; pe_plat_hostfb_vsync = retail VSync 0x80073A44 (arity 1, scripted), pe_plat_ds_ready = 0x8007F608 (arity 1),
 *                      pe_plat_ds_81268 = 0x80081268 (arity 0).  Valid for poll_stop.txt (positive control: 0 mismatches expected); on poll.txt (abort entry inline)
 *                      the core fails by design.
 * The objects are built by pe_cdx_cdgen50_run.sh from the BUILD lines below ($O = scratch output directory):
 * BUILD-host: cc -std=gnu11 -Wall -Wextra -Werror -DC50_HOST=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen50_check.c $O/legacy_poll.o -o $O/c50_host
 * BUILD-host-mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC50_HOST=1 -DC50_MAP_VSYNC=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen50_check.c $O/legacy_poll.o -o $O/c50_host_mapped
 * BUILD-core: cc -std=gnu11 -Wall -Wextra -Werror -DC50_CORE=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen50_check.c $O/legacy_poll.o -o $O/c50_core
 * Run: $O/c50_<name> poll.txt|poll_stop.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"
#include "pe_plat/framebuffer_api.h"

#if !C50_HOST && !C50_CORE
#error "define C50_HOST or C50_CORE"
#endif

/* a stand-in the scripted VSync value can be peeked from (no recorded call) */
static __attribute__((unused)) uint32_t c50_peek(uint32_t fn)
{
    unsigned i;
    for (i = 0; i < nrets_t; i++) if (rets_fn[i] == fn) { unsigned k = rets_ctr[i]; return rets_val[i][k < rets_len[i] ? k : rets_len[i] - 1]; }
    return 0;
}
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask)
{
#if C50_MAP_VSYNC
    if (vsync) *vsync = (int)c50_peek(0x80073A44u);
#else
    if (vsync) *vsync = 0;       /* the host's VSync-call counter: unrelated to the retail retrace clock */
#endif
    if (drawsync) *drawsync = 0;
    if (presented) *presented = 0;
    if (mask) *mask = 0;
}
#if C50_CORE
int pe_plat_hostfb_vsync(int mode) { return (int)cdx_callee(0x80073A44u, 1, (uint32_t)mode, 0, 0, 0, 0); }
void pe_plat_ds_81268(void) { (void)cdx_callee(0x80081268u, 0, 0, 0, 0, 0, 0); }
int pe_plat_ds_ready(int a0) { return (int)cdx_callee(0x8007F608u, 1, (uint32_t)a0, 0, 0, 0, 0); }
#else
/* referenced by the retail core in the same object, never called by the host adapter: a call fails the case */
int pe_plat_hostfb_vsync(int mode) { (void)mode; bad_call = 1; trap_name = "pe_plat_hostfb_vsync"; return 0; }
void pe_plat_ds_81268(void) { bad_call = 1; trap_name = "pe_plat_ds_81268"; }
int pe_plat_ds_ready(int a0) { (void)a0; bad_call = 1; trap_name = "pe_plat_ds_ready"; return 0; }
#endif
int pe_plat_cd_read_poll(uint32_t fp);
int pe_plat_cd_read_poll_host(uint32_t fp);
#if C50_CORE
#define C50_ENTRY pe_plat_cd_read_poll
#else
#define C50_ENTRY pe_plat_cd_read_poll_host
#endif
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (entry != 0x800811E4u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)C50_ENTRY(a[0]);
    return 1;
}
