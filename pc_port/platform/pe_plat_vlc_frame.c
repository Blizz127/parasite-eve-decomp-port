/*
 * pe_plat_vlc_frame.c - guest-shaped adapter for the libpress VLC frame decoder (guest row 0x8010C89C).
 *
 * The decoding is pe_vlc_decode.c (clean-room from psx-spx by a fresh author; resumable begin/step API derived from the
 * black-box bound vectors).  This file maps the guest call onto it.  Contract, from black-box observation of the original
 * (pc_port/tools/pe_mdec_vlc_vectors.py, pe_mdec_vlc_bound_vectors.py, docs/evidence/native-press-20261125/ and
 * native-press-wire-20261125/):
 *   a0 != 0  starts a frame: a0 = guest pointer to one STR frame payload (BS header at +0), a1 = guest output pointer,
 *            a2 = VLC table pointer (ignored: the pure decoder needs no table), a3 ignored.  The first two output halfwords
 *            are the BS header's first two halfwords (the MDEC(1) command low word), then the RL data.
 *   a0 == 0  resumes after a bounded exit: continues at the saved input position and the saved output pointer; its a1 is
 *            ignored.  Without a preceding bounded exit the original never returns: here a named boundary stop.
 *   bound    the word at guest 0x8011EB8C (0x00FFFFFF in the shipped image; never written in real playback) is read at every
 *            call and counts output halfwords of THAT call; the call returns 1 at the decoder's checkpoint (right after a
 *            block's DC word) once the call's advance exceeds it, otherwise 0 with the FE00h padding written up to
 *            2 + 2*count halfwords.
 *   word 0x8011EBB4 = a1 + 4 + 4*count of the frame (the header's declared size, not the output end), stored on the first call.
 *   returned registers (observed): a0' = frame pointer + 8 + 2*ceil((end_bits + 7)/16) (input cursor, fitted to all vectors;
 *            meaning is read-ahead, not understood), a1' = output pointer after the call.  Production callers' outcomes do not
 *            depend on a0'/a1' (pe_c89c_caller_contract.py), caller use is unverified.
 * Real playback (native run, docs/evidence/native-press-wire-20261125/C89C_REAL_PLAYBACK.md): a0 never 0, bound never seeded,
 * ret always 0, one call per frame into two arenas 64,000 bytes apart; the largest real output is 56,836 bytes.
 * HOST SAFETY STOPS (not original behaviour, cannot fire on real data): a call whose output would leave guest RAM or exceed
 * 0x20000 bytes (more than twice the real maximum) ends in the named boundary C89C_output_overflow; decoder errors (streams on
 * which the original never terminates: no end-of-frame code, truncated data, unused code) and a resume without a bounded exit
 * end in named boundary stops, never in a silent wrong result.
 */
#include "pe_plat/vlc_frame.h"
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_bootstrap.h"
#include "pe_vlc_decode.h"
#include "pe_sdk.h"

#define VLC_BOUND_WORD   0x8011EB8Cu
#define VLC_END_WORD     0x8011EBB4u
/* Resume state the original keeps in the movie module's data at a bounded exit (found by RAM diffs over the bound vectors; not
 * rewritten when a call completes the frame): saved input pointer, saved output pointer, bit-window word, its shift count, quant<<10. */
#define VLC_SAVE_IN      0x8011EB90u
#define VLC_SAVE_OUT     0x8011EB94u
#define VLC_SAVE_WIN     0x8011EB98u
#define VLC_SAVE_CNT     0x8011EB9Cu
#define VLC_SAVE_QUANT   0x8011EBA0u
#define VLC_MAX_INPUT    (1u << 20)
#define VLC_MAX_HALFWORDS (1u << 17) /* 0x20000 halfwords = 0x40000 bytes per call: safety cap, > twice the real maximum */

static uint16_t g_vlc_out[VLC_MAX_HALFWORDS];
static pe_vlc_decoder_t g_st;       /* decoder state kept across calls (the original keeps it in its statics) */
static uint32_t g_frame_ptr;        /* guest pointer of the frame being decoded (a0 of the first call) */
static uint32_t g_out_ptr;          /* saved guest output pointer */
static int g_active;                /* a bounded exit left a decode to resume */
static PeC89CTelemetry g_c89c_telemetry; /* host-only: last entry arguments and this adapter's exit code */

void PE_C89C_GetTelemetry(PeC89CTelemetry *out)
{
    if (out) *out = g_c89c_telemetry;
}

static int Stop(const char *name, uint32_t arg)
{
    Bootstrap_ReturnVoid1(name, "C89C_decode", arg);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    g_active = 0;
    return 0;
}

/* The original's input pointer after a call that reached `end_bits`. */
static uint32_t Cursor(size_t end_bits)
{
    return (uint32_t)(8u + 2u * ((end_bits + 7u + 15u) / 16u));
}

int pe_plat_vlc_frame_host_regs(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t *a0_out, uint32_t *a1_out)
{
    size_t hw = 0, end_bits = 0, i;
    uint32_t bound, out_ptr, n_bytes, first = (a0 != 0u);
    int rc;
    (void)a3;
    g_c89c_telemetry.a0 = a0; g_c89c_telemetry.a1 = a1; g_c89c_telemetry.a2 = a2; g_c89c_telemetry.ret = 0;
    if (first) {
        pe_addr_t in = PE_RamCanonical(a0, 8u);
        size_t avail;
        if (!PE_RangeIsRam(in, 8u)) return Stop("C89C_input_range", a0);
        avail = (size_t)(PE_RAM_END - in);
        if (avail > VLC_MAX_INPUT) avail = VLC_MAX_INPUT;
        if (pe_vlc_decode_begin(&g_st, (const uint8_t *)PE_TranslateConst(in, avail), avail) != PE_VLC_OK)
            return Stop("C89C_header", a0);
        g_frame_ptr = a0;
        out_ptr = a1;
        g_active = 1;
    } else {
        if (!g_active) return Stop("C89C_resume_without_bound", 0u);
        out_ptr = g_out_ptr;
    }
    bound = PE_LoadU32(VLC_BOUND_WORD);
    rc = pe_vlc_decode_step(&g_st, g_vlc_out, VLC_MAX_HALFWORDS, bound, &hw, &end_bits);
    if (rc < 0) return Stop("C89C_decode_error", (uint32_t)rc);
    n_bytes = (uint32_t)(hw * 2u) + (first ? 4u : 0u);
    {
        pe_addr_t out = PE_RamCanonical(out_ptr, n_bytes);
        if (n_bytes > 0x40000u + 4u || !PE_RangeIsRam(out, n_bytes)) return Stop("C89C_output_overflow", out_ptr);
        if (first) {
            PE_StoreU16(out, PE_LoadU16(g_frame_ptr));          /* the MDEC(1) command low word, copied from the BS header */
            PE_StoreU16(out + 2u, PE_LoadU16(g_frame_ptr + 2u));
            out += 4u;
        }
        for (i = 0; i < hw; i++) PE_StoreU16(out + (pe_addr_t)(2u * i), g_vlc_out[i]);
    }
    if (first) PE_StoreU32(VLC_END_WORD, a1 + 4u + 4u * (uint32_t)PE_LoadU16(g_frame_ptr));
    g_out_ptr = out_ptr + n_bytes;
    g_active = (rc == 1);
    if (rc == 1) {                                  /* bounded exit: the original's saved resume state, bit for bit (vectors) */
        uint32_t cur = Cursor(end_bits), cnt = (uint32_t)((end_bits + 6u) & 15u), win = 0;
        if (cur >= 12u) win = ((uint32_t)PE_LoadU16(g_frame_ptr + cur - 4u) << 16) | PE_LoadU16(g_frame_ptr + cur - 2u);
        PE_StoreU32(VLC_SAVE_IN, g_frame_ptr + cur);
        PE_StoreU32(VLC_SAVE_OUT, g_out_ptr);
        PE_StoreU32(VLC_SAVE_WIN, win << cnt);
        PE_StoreU32(VLC_SAVE_CNT, cnt);
        PE_StoreU32(VLC_SAVE_QUANT, ((uint32_t)PE_LoadU16(g_frame_ptr + 4u) & 0x3Fu) << 10);
    }
    g_c89c_telemetry.ret = rc;
    if (a0_out) *a0_out = g_frame_ptr + Cursor(end_bits);
    if (a1_out) *a1_out = g_out_ptr;
    return rc;
}

int pe_plat_vlc_frame_host(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    return pe_plat_vlc_frame_host_regs(a0, a1, a2, a3, 0, 0);
}
