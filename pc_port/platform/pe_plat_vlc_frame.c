/*
 * pe_plat_vlc_frame.c - guest-shaped adapter for the libpress VLC frame decoder (guest row 0x8010C89C).
 *
 * The decoding itself is pe_vlc_decode.c (clean-room, written from psx-spx by a fresh author); this file only
 * maps the guest call onto it.  Contract, from black-box observation of the original (pc_port/tools/
 * pe_mdec_vlc_vectors.py and pe_c89c_caller_contract.py, docs/evidence/native-press-20261125/):
 *   a0  guest pointer to one complete STR frame payload (BS header at +0), always nonzero in the production
 *       callers (the "a0 == 0 resume" entry never occurs in them: 48 updater + 16 player scenarios);
 *   a1  guest output pointer: the original writes the MDEC run-length stream (2 header halfwords, data, FE00h
 *       padding to 2+2*size_field halfwords) -- pe_vlc_decode_frame_mdec()'s layout;
 *   a2  VLC table pointer built by 0x8010BD4C (ignored: the pure decoder needs no table);   a3 ignored;
 *   return 0 on normal completion; the word at guest 0x8011EBB4 (movie-module data) holds a1 + 4 + 4*count
 *   (the header's declared size, NOT the output end) afterwards (observed on 10 count/length variants;
 *   no caller reads it in the production graphs, but it is reproduced).
 * The original also returns an advanced input pointer / output end in a0/a1 (input cursor fits
 * 8 + 2*ceil((bits_consumed+7)/16), fitted to 224+ vectors); a C function cannot return those registers
 * and the production callers' outcomes do not depend on them (pe_c89c_caller_contract.py), so they are NOT
 * reproduced; caller use of the returned registers is unverified beyond those scenarios.
 * Streams on which the original never terminates (no end-of-frame code, unused code, ...) end in a named
 * boundary stop (PE_PORT_STOP_UNRESOLVED_BOUNDARY), never in a silent wrong result.
 */
#include "pe_plat/vlc_frame.h"
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_vlc_decode.h"
#include "pe_sdk.h"

#define VLC_END_WORD     0x8011EBB4u  /* movie-module data word: a1 + declared size after a decode */
#define VLC_MAX_INPUT    (1u << 20)
#define VLC_MAX_HALFWORDS (1u << 19)

static uint16_t g_vlc_out[VLC_MAX_HALFWORDS];
static PeC89CTelemetry g_c89c_telemetry; /* host-only: last entry arguments and this adapter's exit code */

/* Weak so that it coexists with a hand-port definition until that is removed (existing test_movie_updater.h link). */
#if defined(__GNUC__)
__attribute__((weak))
#endif
void PE_C89C_GetTelemetry(PeC89CTelemetry *out)
{
    if (out) *out = g_c89c_telemetry;
}

/* As pe_plat_vlc_frame_host, plus the registers the original returns: *a0_out = input pointer after the decode
 * (a0 + 8 + 2*ceil((bits_consumed+7)/16): an empirical fit to 284 vectors; the meaning is read-ahead and is not
 * understood), *a1_out = a1 + output bytes.  Production callers' outcomes do not depend on either register
 * (pe_c89c_caller_contract.py), so caller use of a0'/a1' is unverified beyond those scenarios. */
int pe_plat_vlc_frame_host_regs(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t *a0_out, uint32_t *a1_out)
{
    pe_addr_t in = PE_RamCanonical(a0, 8u);
    size_t avail, len = 0, i, end_bits = 0;
    int rc;
    (void)a3;
    g_c89c_telemetry.a0 = a0; g_c89c_telemetry.a1 = a1; g_c89c_telemetry.a2 = a2; g_c89c_telemetry.ret = 0;
    if (!PE_RangeIsRam(in, 8u)) { PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return 0; }
    avail = (size_t)(PE_RAM_END - in);
    if (avail > VLC_MAX_INPUT) avail = VLC_MAX_INPUT;
    rc = pe_vlc_decode_frame_mdec_ex((const uint8_t *)PE_TranslateConst(in, avail), avail, g_vlc_out, VLC_MAX_HALFWORDS, &len, &end_bits);
    if (rc != PE_VLC_OK) { PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return 0; }
    {
        pe_addr_t out = PE_RamCanonical(a1, (uint32_t)(len * 2u));
        if (!PE_RangeIsRam(out, len * 2u)) { PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY); return 0; }
        for (i = 0; i < len; i++) PE_StoreU16(out + (pe_addr_t)(2u * i), g_vlc_out[i]);
    }
    PE_StoreU32(VLC_END_WORD, a1 + 4u + 4u * (uint32_t)PE_LoadU16(a0)); /* a1 + declared size, not the end (vectors) */
    if (a0_out) *a0_out = a0 + (uint32_t)(8u + 2u * ((end_bits + 7u + 15u) / 16u));
    if (a1_out) *a1_out = a1 + (uint32_t)(len * 2u);
    return 0;
}

int pe_plat_vlc_frame_host(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    return pe_plat_vlc_frame_host_regs(a0, a1, a2, a3, 0, 0);
}
