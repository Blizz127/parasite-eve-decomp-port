/* libcd CD_ready (guest 0x8007B290; two arguments: mode, result pointer),
 * written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen36) and psx-spx, with no SDK
 * source and without reading the previous provider; host contracts per the
 * lead's brief.  The behaviour below is exactly what the vectors show; no
 * claim is made about the original's source.
 *
 * Behaviour (all words little-endian guest RAM):
 *  - Entry: t = VSync(-1) (a counter query); the deadline word 0x800A3478 is
 *    stored as t + 960 (32-bit wrap), the pass counter 0x800A347C as 0 and
 *    the name word 0x800A3480 as the constant 0x80011BA8.
 *  - Each pass: now = VSync(-1); the deadline word is loaded and the pass
 *    times out when (int)deadline < (int)now (a signed compare of the two
 *    words, not of their difference: equality does not time out, and a
 *    deadline that wrapped negative against a positive count does).  A pass
 *    that does not time out loads the counter word and stores it plus one.
 *  - Timeout: puts(0x80011B18); printf(0x80011B28, name word, comstr, sync,
 *    ready) where comstr = word at 0x8009AFDC + 4 * byte[0x8009AFD5], sync =
 *    word at 0x8009B05C + 4 * byte[0x8009B294] and ready = word at
 *    0x8009B05C + 4 * byte[0x8009B295] (byte indices zero-extended); then
 *    CD_flush(); the result is -1 and nothing is written to the result.
 *  - Otherwise, when CheckCallback() (16 bits) is nonzero the controller
 *    interrupt events are drained: r = word at 0x8009B27C (CD register
 *    pointer), the byte at r is read first; then getintr() is called until it
 *    returns 0.  A result with bit 2 set calls the callback word 0x8009AFB8
 *    (when nonzero) with (byte 0x8009B295, 0x800A3468); then a result with
 *    bit 1 set calls the callback word 0x8009AFB4 (when nonzero) with
 *    (byte 0x8009B294, 0x800A3460).  The status byte is loaded only when the
 *    callback word is nonzero.  Afterwards the register pointer word is
 *    reloaded and (the first byte & 3) is stored at it.
 *  - Then: byte 0x8009B296 nonzero -> it is cleared, 8 bytes at 0x800A3470 are
 *    copied to the result (unless the result pointer is 0: then the source is
 *    not even read) and the byte is the return value.  Else byte 0x8009B295
 *    nonzero -> cleared, 8 bytes at 0x800A3468 copied likewise, returned.
 *    Else mode != 0 returns 0, and mode == 0 begins a new pass.
 *
 * Not determined by the vectors: any bound on the pass counter (the counter
 * word only ever increments within the 8192 callee calls a case may make), the
 * callbacks' return values (the dispatcher result is ignored), the exact
 * instruction order of the status-byte read against the first getintr call
 * (the contract below fixes it).
 *
 * Host contracts (port facts, not behaviour of the original; the vectors
 * never exercise them):
 *  - callbacks go through the address-keyed dispatcher
 *    PE_Cd_DispatchDataCallback (site labels "D_8009AFB4 (CD sync callback)"
 *    and "D_8009AFB8 (CD ready callback)");
 *  - the CD index/status byte is read before the getintr loop and its & 3
 *    write-back happens after it;
 *  - PE_Port_ShouldStop() is checked after each callback and each getintr
 *    pass; when set the register write-back is finished and 0 is returned
 *    without the status-byte tail or the response copy;
 *  - the VSync(-1) reads are pure counter queries (HostFB_GetState);
 *  - pe_plat_sio_poll_tick() once per blocking pass, and a stuck-wait budget
 *    of 0x40000 passes / getintr calls ends in PE_Port_RequestStop(
 *    PE_PORT_STOP_UNRESOLVED_BOUNDARY) (the loop is left; the normal
 *    write-back runs and 0 is returned).  The tick is also given once per
 *    getintr pass (as pe_plat_cd_gen33.c does), because the device clock
 *    only moves on ticks and production getintr can keep returning events
 *    until it does; the checker cannot tell the two apart. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_cdreg.h"
#include "pe_plat/cd.h"
#include "pe_plat/state.h"
#include "pe_plat/bios_file.h"
#include "game_port.h"
#include "pe_bios_string.h"
#include "host_framebuffer.h"
#include "pe_sdk.h"

#define POLL_BUDGET   0x40000u

#define W_DEADLINE    0x800A3478u
#define W_COUNTER     0x800A347Cu
#define W_NAME        0x800A3480u
#define NAME_VALUE    0x80011BA8u
#define STR_TIMEOUT   0x80011B18u
#define STR_FORMAT    0x80011B28u
#define TAB_COMSTR    0x8009AFDCu
#define B_LASTCOM     0x8009AFD5u
#define TAB_INTSTR    0x8009B05Cu
#define PTR_REGISTER  0x8009B27Cu
#define CB_SYNC       0x8009AFB4u   /* D_8009AFB4 (CD sync callback) */
#define CB_READY      0x8009AFB8u   /* D_8009AFB8 (CD ready callback) */
#define B_SYNC_STAT   0x8009B294u
#define B_SYNC_BYTE   0x8009B295u
#define B_READY_BYTE  0x8009B296u
#define RESP_SYNC_CB  0x800A3460u
#define RESP_READY_CB 0x800A3468u
#define RESP_READY    0x800A3470u
#define RESP_SYNC     0x800A3468u
#define DEADLINE_ADD  960u

/* Compile-time mutants for the sensitivity table in the evidence report; every
 * macro defaults to the verified behaviour.  Build with -DMUT_<NAME>=<value>. */
#ifndef MUT_DEADLINE_ADD          /* deadline constant offset (+/-1 ...) */
#define MUT_DEADLINE_ADD 0
#endif
#ifndef MUT_CMP                   /* 0 signed deadline < now; 1 <=; 2 unsigned <; 3 signed (deadline - now) < 0; 4 > (swapped) */
#define MUT_CMP 0
#endif
#ifndef MUT_CB                    /* 0 CheckCallback() != 0; 1 & 0xFF; 2 == 1; 3 > 0 (signed 16); 4 & 0x8000 == 0 */
#define MUT_CB 0
#endif
#ifndef MUT_BITS                  /* 0 bit2 ready / bit1 sync; 1 bit mask 8/2; 2 ==4 / ==2; 3 bit 4 -> bit 1 swapped masks */
#define MUT_BITS 0
#endif
#ifndef MUT_CBARG                 /* 0 status byte, response; 1 swap status bytes; 2 response addresses swapped; 3 status byte not loaded (0) */
#define MUT_CBARG 0
#endif
#ifndef MUT_NULLGUARD             /* 1 copy even when the result pointer is 0 */
#define MUT_NULLGUARD 0
#endif
#ifndef MUT_COPYLEN               /* copy length */
#define MUT_COPYLEN 8
#endif
#ifndef MUT_NOFLUSH               /* 1 skip CD_flush */
#define MUT_NOFLUSH 0
#endif
#ifndef MUT_COUNTER               /* 0 +1; 1 +2; 2 no store; 3 no load (store 1) */
#define MUT_COUNTER 0
#endif
#ifndef MUT_MODE                  /* 0 mode != 0; 1 mode == 1; 2 (mode & 1); 3 (mode & 0xFF); 4 (mode & 0xFFFF) */
#define MUT_MODE 0
#endif
#ifndef MUT_ORDER                 /* 1 test the sync byte before the ready byte */
#define MUT_ORDER 0
#endif
#ifndef MUT_INDEX                 /* 0 & 3; 1 & 7; 2 & 1; 3 no mask */
#define MUT_INDEX 0
#endif
#ifndef MUT_RET                   /* 1 timeout returns 0; 2 ready byte returns & 0x7F; 3 non-blocking return 1 */
#define MUT_RET 0
#endif
#ifndef MUT_TABLE                 /* 1 sync/ready table indices swapped; 2 comstr stride 1; 3 lastcom sign-extended */
#define MUT_TABLE 0
#endif
#ifndef MUT_CLEAR                 /* 1 do not clear the ready byte; 2 do not clear the sync byte */
#define MUT_CLEAR 0
#endif
#ifndef MUT_EXIT                  /* getintr loop exit: 0 bits == 0; 1 (bits & 0xFFFF) == 0; 2 (bits & 0xFF) == 0; 3 (int32_t)bits <= 0 */
#define MUT_EXIT 0
#endif
#ifndef MUT_CBGUARD               /* callback word guard: 0 != 0; 1 always call; 2 cb & 0x80000000; 3 cb & 0xFFFF; 4 (int32_t)cb > 0 */
#define MUT_CBGUARD 0
#endif
#ifndef MUT_BYTETEST              /* ready/sync byte tests: 0 != 0; 1 & 0x7F; 2 & 1; 3 (int8_t) > 0 */
#define MUT_BYTETEST 0
#endif
#ifndef MUT_INITNAME              /* 1 omit the name-word store */
#define MUT_INITNAME 0
#endif
#ifndef MUT_VSYNC                 /* 1 skip the first VSync (use 0) */
#define MUT_VSYNC 0
#endif

static int32_t vsync_count(void)
{
    int v = 0, d = 0, p = 0, m = 0;
    HostFB_GetState(&v, &d, &p, &m);
    return (int32_t)v;
}

static int deadline_passed(uint32_t deadline, int32_t now)
{
    switch (MUT_CMP) {
    case 1: return (int32_t)deadline <= now;
    case 2: return deadline < (uint32_t)now;
    case 3: return (int32_t)(deadline - (uint32_t)now) < 0;
    case 4: return (int32_t)deadline > now;
    default: return (int32_t)deadline < now;
    }
}

static int callback_pending(uint16_t v)
{
    switch (MUT_CB) {
    case 1: return (v & 0xFFu) != 0u;
    case 2: return v == 1u;
    case 3: return (int16_t)v > 0;
    case 4: return (v & 0x8000u) == 0u;
    default: return v != 0u;
    }
}

static int loop_done(uint32_t bits)
{
    switch (MUT_EXIT) {
    case 1: return (bits & 0xFFFFu) == 0u;
    case 2: return (bits & 0xFFu) == 0u;
    case 3: return (int32_t)bits <= 0;
    default: return bits == 0u;
    }
}

static int cb_installed(uint32_t cb)
{
    switch (MUT_CBGUARD) {
    case 1: return 1;
    case 2: return (cb & 0x80000000u) != 0u;
    case 3: return (cb & 0xFFFFu) != 0u;
    case 4: return (int32_t)cb > 0;
    default: return cb != 0u;
    }
}

static int byte_set(uint32_t b)
{
    switch (MUT_BYTETEST) {
    case 1: return (b & 0x7Fu) != 0u;
    case 2: return (b & 1u) != 0u;
    case 3: return (int8_t)b > 0;
    default: return b != 0u;
    }
}

static int mode_nonblocking(uint32_t mode)
{
    switch (MUT_MODE) {
    case 1: return mode == 1u;
    case 2: return (mode & 1u) != 0u;
    case 3: return (mode & 0xFFu) != 0u;
    case 4: return (mode & 0xFFFFu) != 0u;
    default: return mode != 0u;
    }
}

static int bits_hit(uint32_t bits, uint32_t mask)
{
    switch (MUT_BITS) {
    case 1: return (bits & (mask == 4u ? 8u : 1u)) != 0u;
    case 2: return bits == mask;
    case 3: return (bits & (mask == 4u ? 2u : 4u)) != 0u;
    default: return (bits & mask) != 0u;
    }
}

static void call_hook(const char *site, uint32_t target, uint32_t status, pe_addr_t response)
{
    (void)site;
    (void)PE_Cd_DispatchDataCallback(target, status, response);
}

static void copy_response(pe_addr_t dst, pe_addr_t src)
{
    unsigned i;
    if (!dst && !MUT_NULLGUARD) return;
    for (i = 0; i < (unsigned)MUT_COPYLEN; i++) PE_StoreU8(dst + i, PE_LoadU8(src + i));
}

static int32_t report_timeout(void)
{
    uint32_t sync_stat, sync_byte, name, comstr, a_sync, a_ready;

    PE_GuestCall("CD_ready timeout puts", 0x80073C5Cu, 1u, STR_TIMEOUT, 0u, 0u, 0u);
    sync_stat = PE_LoadU8(B_SYNC_STAT);
    sync_byte = PE_LoadU8(B_SYNC_BYTE);
    if (MUT_TABLE == 1) { uint32_t t = sync_stat; sync_stat = sync_byte; sync_byte = t; }
    a_ready = PE_LoadU32(TAB_INTSTR + 4u * sync_byte);
    name = PE_LoadU32(W_NAME);
    comstr = PE_LoadU32(TAB_COMSTR + (MUT_TABLE == 2 ? 1u : 4u) *
                        (MUT_TABLE == 3 ? (uint32_t)(int32_t)(int8_t)PE_LoadU8(B_LASTCOM) : (uint32_t)PE_LoadU8(B_LASTCOM)));
    a_sync = PE_LoadU32(TAB_INTSTR + 4u * sync_stat);
    PE_Printf_Guest(STR_FORMAT, name, comstr, a_sync, a_ready, 0u, 0u, 0u);
    if (!MUT_NOFLUSH) func_8007B9EC();
    return MUT_RET == 1 ? 0 : -1;
}

int pe_plat_cd_ready_poll(uint32_t mode, pe_addr_t result)
{
    uint32_t passes = 0u, budget = 0u;
    int32_t now;

    now = MUT_VSYNC ? 0 : vsync_count();
    PE_StoreU32(W_DEADLINE, (uint32_t)now + DEADLINE_ADD + (uint32_t)MUT_DEADLINE_ADD);
    PE_StoreU32(W_COUNTER, 0u);
    if (!MUT_INITNAME) PE_StoreU32(W_NAME, NAME_VALUE);

    for (;;) {
        uint32_t b;

        /* Host contract (lead integration edit, not original behaviour): a stop that is
         * already latched or raised during the wait ends the blocking wait with -1 and
         * keeps the earlier stop reason (test CDREADY_stop_exit). */
        if (PE_Port_ShouldStop()) return -1;
        now = vsync_count();
        if (deadline_passed(PE_LoadU32(W_DEADLINE), now)) return report_timeout();
        if (MUT_COUNTER == 3) PE_StoreU32(W_COUNTER, 1u);
        else if (MUT_COUNTER != 2) PE_StoreU32(W_COUNTER, PE_LoadU32(W_COUNTER) + (MUT_COUNTER == 1 ? 2u : 1u));
        else (void)PE_LoadU32(W_COUNTER);
        if (passes++ >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
        pe_plat_sio_poll_tick();

        if (callback_pending(func_80073DE8())) {
            uint32_t reg, bits, cb;
            uint8_t index;
            int stop = 0;

            reg = PE_LoadU32(PTR_REGISTER);
            index = PE_CdLoadU8(reg);
            for (;;) {
                bits = (uint32_t)func_8007AAB4();
                if (PE_Port_ShouldStop()) { stop = 1; break; }
                if (loop_done(bits)) break;
                if (budget++ >= POLL_BUDGET) {
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                    stop = 1;
                    break;
                }
                pe_plat_sio_poll_tick();
                if (bits_hit(bits, 4u)) {
                    cb = PE_LoadU32(CB_READY);
                    if (cb_installed(cb)) call_hook("D_8009AFB8 (CD ready callback)", cb,
                                      MUT_CBARG == 3 ? 0u : PE_LoadU8(MUT_CBARG == 1 ? B_SYNC_STAT : B_SYNC_BYTE),
                                      MUT_CBARG == 2 ? RESP_SYNC_CB : RESP_READY_CB);
                    if (PE_Port_ShouldStop()) { stop = 1; break; }
                }
                if (bits_hit(bits, 2u)) {
                    cb = PE_LoadU32(CB_SYNC);
                    if (cb_installed(cb)) call_hook("D_8009AFB4 (CD sync callback)", cb,
                                      PE_LoadU8(MUT_CBARG == 1 ? B_SYNC_BYTE : B_SYNC_STAT),
                                      MUT_CBARG == 2 ? RESP_READY_CB : RESP_SYNC_CB);
                    if (PE_Port_ShouldStop()) { stop = 1; break; }
                }
            }
            reg = PE_LoadU32(PTR_REGISTER);
            PE_CdStoreU8(reg, (uint8_t)(MUT_INDEX == 1 ? (index & 7u) : MUT_INDEX == 2 ? (index & 1u) : MUT_INDEX == 3 ? index : (index & 3u)));
            if (stop) return 0;
        }

        if (MUT_ORDER) {
            b = PE_LoadU8(B_SYNC_BYTE);
            if (byte_set(b)) {
                if (MUT_CLEAR != 2) PE_StoreU8(B_SYNC_BYTE, 0u);
                copy_response(result, RESP_SYNC);
                return (int)b;
            }
        }
        b = PE_LoadU8(B_READY_BYTE);
        if (byte_set(b)) {
            if (MUT_CLEAR != 1) PE_StoreU8(B_READY_BYTE, 0u);
            copy_response(result, RESP_READY);
            return MUT_RET == 2 ? (int)(b & 0x7Fu) : (int)b;
        }
        if (!MUT_ORDER) {
            b = PE_LoadU8(B_SYNC_BYTE);
            if (byte_set(b)) {
                if (MUT_CLEAR != 2) PE_StoreU8(B_SYNC_BYTE, 0u);
                copy_response(result, RESP_SYNC);
                return (int)b;
            }
        }
        if (mode_nonblocking(mode)) return MUT_RET == 3 ? 1 : 0;
    }
}
