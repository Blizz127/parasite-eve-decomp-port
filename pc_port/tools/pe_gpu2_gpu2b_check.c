/* sw-vec-gpu2 batch gpu2b checker: image transfer workers/wrappers - LoadImage worker func_80076664 and wrapper
 * func_8007506C, StoreImage wrapper func_800750CC and worker func_800768A0, rect validator func_80074E28, MoveImage
 * func_8007512C.  Generator: pc_port/tools/pe_gpu2_vectors.py gpu2b; contract: pe_gpu2_check_core.h.
 * Groups (select with -D): CHK_WRAP (func_8007506C, func_800750CC, func_8007512C; callee func_80074E28 is a stub),
 * CHK_RECT (func_80074E28), CHK_WORK (func_80076664, func_800768A0).
 * Build lines (run from the repo root, verbatim; the clean-room provider of this family is pe_plat2_gpuimg*.c, one
 * translation unit per group so that a caller and its callee are never in the same build):
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_WRAP -DG2_BATCH_NAME=\"gpu2b-wrap\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2b_check.c pc_port/platform/pe_plat2_gpuimg.c -o check_gpu2b_wrap
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_RECT -DG2_BATCH_NAME=\"gpu2b-rect\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2b_check.c pc_port/platform/pe_plat2_gpuimg_rect.c -o check_gpu2b_rect
 *   cc -std=gnu11 -Wall -Wextra -Werror -DCHK_WORK -DG2_BATCH_NAME=\"gpu2b-work\" -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_gpu2_gpu2b_check.c pc_port/platform/pe_plat2_gpuimg_work.c -o check_gpu2b_work
 *   ./check_gpu2b_<group> vectors/gpu2b/gpu2b.txt
 * (Before the replacement the same groups ran against the translated boot files with -DG2_SMOKE; those lines are in
 * docs/evidence/native-gpu2-20261126/REPORT.md.)
 * Callees are checker stubs (func_80074E28 = pe_plat_gpuimg_report_rect, func_80076C34 = pe_plat_gpu_swap, the
 * timeout func_80077404 = pe_plat_gpu_timeout, VSync = HostFB_VSync, ...).  The RECT argument of the wrapper entries
 * is a host pointer into the checker's RAM image; PE_HostToGuest recovers its guest address.
 * Mutants: -DGPU2_MUTANT=1 (no-op stub), =2 (return bit flip), =3 (store bit flip). */
#ifdef CHK_WORK
#define G2_REAL_UPLOAD_WORKER
#define G2_REAL_READBACK_WORKER
#define G2_REAL_UPLOAD_INLINE
#define G2_REAL_READBACK_INLINE
#endif
#ifdef CHK_RECT
#define G2_REAL_REPORT_RECT
#endif
#include "pe_gpu2_check_core.h"
#include "pe_guest_ram.h"
#include "pe_gpu2_host_stubs.h"
#ifdef G2_SMOKE
#include "pe_gpu2_smoke_host.h"
#endif
#ifdef CHK_WORK
/* Host hooks the workers call that are not guest-visible (the vectors cannot show them): the host DMA tick of a
 * failed poll, the KUSEG/KSEG1 alias fold used by the buffer range check, and the host GPU model's image preflight
 * (always able here: the vectors hold only buffers and states the original accepted).  PE_GPU_DMA2Issue is the
 * host model's DMA2 start; the provider calls it in place of the CHCR register store when the CHCR pointer word holds
 * the real register address, so this stub stands for that store (address 0x1F8010A8, the value the call carries): the
 * CHCR store of a vector line is checked through it, the pointer-aimed-elsewhere lines through the plain store. */
int pe_plat_gpu_host_wait_tick(void) { return 0; }
pe_addr_t PE_RamCanonical(pe_addr_t a, size_t n) { (void)n; return a; }
int PE_GPU_CanBeginImageLoad(int needs_dma) { (void)needs_dma; return 1; }
int PE_GPU_DMA2Issue(pe_addr_t madr, uint32_t bcr, uint32_t chcr) { (void)madr; (void)bcr; g2_st(0x1F8010A8u, 4, chcr); return 1; }
#endif
typedef struct { int16_t x, y, w, h; } G2Rect;
#ifdef CHK_WRAP
int pe_plat_gpuimg_load_image(const G2Rect *rect, uint32_t data);
int pe_plat_gpuimg_store_image(const G2Rect *rect, uint32_t data);
int pe_plat_gpuimg_move_image(const G2Rect *rect, int dx, int dy);
#endif
#ifdef CHK_RECT
void pe_plat_gpuimg_report_rect(uint32_t name, const G2Rect *rect);
#endif
#ifdef CHK_WORK
int pe_plat_gpuimg_upload_worker(pe_addr_t rect, pe_addr_t source);
int pe_plat_gpuimg_readback_worker(pe_addr_t rect, pe_addr_t destination);
#endif
static unsigned g2_selected_list(uint32_t *out)
{
    unsigned n = 0;
#ifdef CHK_WRAP
    out[n++] = 0x8007506Cu; out[n++] = 0x800750CCu; out[n++] = 0x8007512Cu;
#endif
#ifdef CHK_RECT
    out[n++] = 0x80074E28u;
#endif
#ifdef CHK_WORK
    out[n++] = 0x80076664u; out[n++] = 0x800768A0u;
#endif
    (void)out;
    return n;
}
static int g2_entry(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret;
#if defined(GPU2_MUTANT) && GPU2_MUTANT == 1
    {
        uint32_t sel[8]; unsigned i, n = g2_selected_list(sel);
        for (i = 0; i < n; i++) if (sel[i] == entry) return 1;
        return -2;
    }
#else
    switch (entry) {
#ifdef CHK_WRAP
    case 0x8007506Cu: *ret = (uint32_t)pe_plat_gpuimg_load_image((const G2Rect *)PE_Translate(a[0], 8), a[1]); return 1;
    case 0x800750CCu: *ret = (uint32_t)pe_plat_gpuimg_store_image((const G2Rect *)PE_Translate(a[0], 8), a[1]); return 1;
    case 0x8007512Cu: *ret = (uint32_t)pe_plat_gpuimg_move_image((const G2Rect *)PE_Translate(a[0], 8), (int)a[1], (int)a[2]); return 1;
#endif
#ifdef CHK_RECT
    case 0x80074E28u: pe_plat_gpuimg_report_rect(a[0], (const G2Rect *)PE_Translate(a[1], 8)); return 0;
#endif
#ifdef CHK_WORK
    case 0x80076664u: *ret = (uint32_t)pe_plat_gpuimg_upload_worker(a[0], a[1]); return 1;
    case 0x800768A0u: *ret = (uint32_t)pe_plat_gpuimg_readback_worker(a[0], a[1]); return 1;
#endif
    }
    return -2;
#endif
}
int main(int c, char **v) { return g2_main(c, v); }
