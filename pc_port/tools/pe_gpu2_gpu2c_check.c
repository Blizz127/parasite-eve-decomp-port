/* sw-vec-gpu2 batch gpu2c checker: DrawPrim func_80075358 (+ worker func_80076B58), DrawOTag func_800753B4, ClearOTagR
 * func_800752AC (+ OTC worker func_80076354), record-pool init func_8007A214 (+ worker func_8007A244, range setter
 * func_8007C544), the five gpu_address leaves func_80071944/64/94 func_80077A64/AA4 and the three libgpu entries
 * func_80074F44 (ClearImage), func_80075424 (PutDrawEnv), func_800754E4 (DrawOTagEnv).
 * Generator: pc_port/tools/pe_gpu2_vectors.py gpu2c; contract: pe_gpu2_check_core.h.
 * Groups (select with -D; a caller and a callee it calls directly are in different groups, the callee is a checker stub):
 *   CHK_ADDRESS 80071944/64/94 80077A64/AA4      CHK_PRIM 80075358 80075358->76B58 callee stub, 800753B4
 *   CHK_OTC     800752AC (callee 80076354 stub)   CHK_OTCW  80076354 (callees 800773D0, 80077404 stubs)
 *   CHK_WORDS   80076B58 (pe_plat_ctl_gpu_transfer_words, clean-room pe_plat2_ctl.c, not part of gpu2c's provider)
 *   CHK_POOL    8007A214 (callee func_8007A244 stub)   CHK_POOLW 8007A244 (callee 8007C444 stub)   CHK_RANGE 8007C544
 *   CHK_LIBGPU  80074F44 80075424 800754E4 (callees 80074E28, 80075EE0, 80071A34 stubs)
 * Provider files (clean-room, branch sw-gpu2c-20261127): pe_plat2_gpuaddr.c (CHK_ADDRESS), pe_plat2_gpuprim.c (CHK_PRIM, CHK_OTC,
 * CHK_POOL, CHK_RANGE, CHK_LIBGPU), pe_plat2_gpuprim_otcw.c (CHK_OTCW), pe_plat2_gpuprim_poolw.c (CHK_POOLW).
 * Build lines (run from the repo root, verbatim; one per group, warning-clean):
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_ADDRESS -DG2_BATCH_NAME=\"gpu2c-address\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuaddr.c -o check_gpu2c_address
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_PRIM -DG2_BATCH_NAME=\"gpu2c-prim\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim.c -o check_gpu2c_prim
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_OTC -DG2_BATCH_NAME=\"gpu2c-otc\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim.c -o check_gpu2c_otc
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_OTCW -DG2_BATCH_NAME=\"gpu2c-otcw\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim_otcw.c -o check_gpu2c_otcw
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_POOL -DG2_BATCH_NAME=\"gpu2c-pool\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim.c -o check_gpu2c_pool
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_RANGE -DG2_BATCH_NAME=\"gpu2c-range\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim.c -o check_gpu2c_range
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_POOLW -DG2_BATCH_NAME=\"gpu2c-poolw\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim_poolw.c -o check_gpu2c_poolw
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_LIBGPU -DG2_BATCH_NAME=\"gpu2c-libgpu\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2c_check.c pc_port/platform/pe_plat2_gpuprim.c -o check_gpu2c_libgpu
 *   ./check_gpu2c_<group> vectors/gpu2c/gpu2c.txt        (CHK_WORDS: the same with -DCHK_WORDS and pc_port/platform/pe_plat2_ctl.c)
 * Mutants: -DGPU2_MUTANT=1 (no-op stub), =2 (return bit flip), =3 (store bit flip). */
#ifdef CHK_OTCW
#define G2_REAL_OT_WORKER
#endif
#ifdef CHK_WORDS
#define G2_REAL_WORDS
#endif
#ifdef CHK_POOLW
#define G2_REAL_POOL_WORKER
#endif
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
#ifdef CHK_ADDRESS
#include "pe_plat/gpuaddr.h"
#endif
typedef struct { int16_t x, y, w, h; } G2Rect;
/* Provider entry prototypes (the public header pe_plat/gpuprim.h needs psx_compat.h for RECT; the checker keeps its own). */
#if defined(CHK_PRIM) || defined(CHK_OTC) || defined(CHK_OTCW) || defined(CHK_POOL) || defined(CHK_POOLW) || defined(CHK_RANGE) || defined(CHK_LIBGPU)
int pe_plat_gpuprim_draw_prim(uint32_t packet);
int pe_plat_gpuprim_draw_otag(uint32_t ot);
void pe_plat_gpuprim_clear_otag_reverse(uint32_t ot, int count);
int pe_plat_gpuprim_otc_worker(uint32_t ot, int count);
int pe_plat_gpuprim_clear_image(G2Rect *rect, uint8_t red, uint8_t green, uint8_t blue);
uint32_t pe_plat_gpuprim_put_draw_env(uint32_t env);
void pe_plat_gpuprim_draw_otag_env(uint32_t ot, uint32_t env);
void pe_plat_gpuprim_pool_init(uint32_t record_base, uint32_t record_count);
void pe_plat_gpuprim_pool_clear(uint32_t record_base, uint32_t record_count);
void pe_plat_gpuprim_stream_set_range(uint32_t mode, uint32_t start, uint32_t end);
#endif
#ifdef CHK_WORDS
int pe_plat_ctl_gpu_transfer_words(uint32_t source, int count);
#endif
static unsigned g2_selected_list(uint32_t *out)
{
    unsigned n = 0;
#ifdef CHK_ADDRESS
    out[n++] = 0x80071944u; out[n++] = 0x80071964u; out[n++] = 0x80071994u; out[n++] = 0x80077A64u; out[n++] = 0x80077AA4u;
#endif
#ifdef CHK_PRIM
    out[n++] = 0x80075358u; out[n++] = 0x800753B4u;
#endif
#ifdef CHK_OTC
    out[n++] = 0x800752ACu;
#endif
#ifdef CHK_OTCW
    out[n++] = 0x80076354u;
#endif
#ifdef CHK_WORDS
    out[n++] = 0x80076B58u;
#endif
#ifdef CHK_POOL
    out[n++] = 0x8007A214u;
#endif
#ifdef CHK_POOLW
    out[n++] = 0x8007A244u;
#endif
#ifdef CHK_RANGE
    out[n++] = 0x8007C544u;
#endif
#ifdef CHK_LIBGPU
    out[n++] = 0x80074F44u; out[n++] = 0x80075424u; out[n++] = 0x800754E4u;
#endif
    (void)out;
    return n;
}
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    {
        uint32_t sel[32]; unsigned i, n = g2_selected_list(sel);
        for (i = 0; i < n; i++) if (sel[i] == entry) return 1;
        return -2;
    }
#else
    switch (entry) {
#ifdef CHK_ADDRESS
    case 0x80071944u: *ret = pe_plat_gpuaddr_optional_chunk(a[0]); return 1;
    case 0x80071964u: *ret = pe_plat_gpuaddr_pixel_chunk(a[0]); return 1;
    case 0x80071994u: *ret = pe_plat_gpuaddr_pixel_data(a[0]); return 1;
    case 0x80077A64u: *ret = (uint32_t)pe_plat_gpuaddr_texture_page((int)a[0], (int)a[1], (int)a[2], (int)a[3]); return 1;
    case 0x80077AA4u: *ret = (uint32_t)pe_plat_gpuaddr_clut((int)a[0], (int)a[1]); return 1;
#endif
#ifdef CHK_PRIM
    case 0x80075358u: *ret = (uint32_t)pe_plat_gpuprim_draw_prim(a[0]); return 1;
    case 0x800753B4u: *ret = (uint32_t)pe_plat_gpuprim_draw_otag(a[0]); return 1;
#endif
#ifdef CHK_OTC
    case 0x800752ACu: pe_plat_gpuprim_clear_otag_reverse(a[0], (int)a[1]); return 0;
#endif
#ifdef CHK_OTCW
    case 0x80076354u: *ret = (uint32_t)pe_plat_gpuprim_otc_worker(a[0], (int)a[1]); return 1;
#endif
#ifdef CHK_WORDS
    case 0x80076B58u: *ret = (uint32_t)pe_plat_ctl_gpu_transfer_words(a[0], (int)a[1]); return 1;
#endif
#ifdef CHK_POOL
    case 0x8007A214u: pe_plat_gpuprim_pool_init(a[0], a[1]); return 0;
#endif
#ifdef CHK_POOLW
    case 0x8007A244u: pe_plat_gpuprim_pool_clear(a[0], a[1]); return 0;
#endif
#ifdef CHK_RANGE
    case 0x8007C544u: pe_plat_gpuprim_stream_set_range(a[0], a[1], a[2]); return 0;
#endif
#ifdef CHK_LIBGPU
    case 0x80074F44u: *ret = (uint32_t)pe_plat_gpuprim_clear_image((G2Rect *)PE_Translate(a[0], 8), (uint8_t)a[1], (uint8_t)a[2], (uint8_t)a[3]); return 1;
    case 0x80075424u: *ret = pe_plat_gpuprim_put_draw_env(a[0]); return 1;
    case 0x800754E4u: pe_plat_gpuprim_draw_otag_env(a[0], a[1]); return 0;
#endif
    }
    return -2;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
