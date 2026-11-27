/*
 * Phase 6D-S — PS1 SDK compatibility layer.
 *
 * All BOOTSTRAP_RET stubs now go through the centralized Bootstrap_ReturnInt /
 * Bootstrap_ReturnVoid policy, so strict mode is enforced uniformly.
 */
#ifndef PSX_COMPAT_H
#define PSX_COMPAT_H

#include "host_framebuffer.h"
#include "stub_registry.h"
#include "pe_guest_ram.h"
#include "pe_callback.h"
#include "pe_bootstrap.h"
#include "pe_port_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Retail PS1 types ─────────────────────────────────────────────── */
#include "pe_plat/rect.h" /* RECT */
typedef struct { uint8_t disp[20]; } DISP_ENV;

/* ── Guest-RAM-resident named globals ─────────────────────────────────
 *
 * These symbols name bytes inside the contiguous 2 MiB guest RAM.
 * Each macro is a typed lvalue over PE_Translate, so reads AND writes
 * (e.g. `D_800B0CDC = 10;`) land in guest RAM with bounds checking.
 * Host endianness is little-endian on all supported build targets,
 * matching the PS1.  Address-of (`&D_800B0CD8`) yields a host pointer
 * into guest RAM; arithmetic from it stays inside the one allocation. */
#define PE_GUEST_U8(a)   (*(uint8_t  *)PE_Translate((a), 1))
#define PE_GUEST_S8(a)   (*(int8_t   *)PE_Translate((a), 1))
#define PE_GUEST_U16(a)  (*(uint16_t *)PE_Translate((a), 2))
#define PE_GUEST_S16(a)  (*(int16_t  *)PE_Translate((a), 2))
#define PE_GUEST_U32(a)  (*(uint32_t *)PE_Translate((a), 4))
/* Shared input/game flags have one guest-memory authority. */
#define D_8009D1A0 PE_GUEST_U32(0x8009D1A0u)
#define D_8009D1C4 PE_GUEST_U32(0x8009D1C4u)
#define D_8009D250 PE_GUEST_U32(0x8009D250u)
#define D_8009D280 PE_GUEST_U32(0x8009D280u)
#define PE_GUEST_S32(a)  (*(int32_t  *)PE_Translate((a), 4))

#define D_800BCE80     ((DISP_ENV *)PE_Translate(0x800BCE80u, sizeof(DISP_ENV)))

#define D_800B0CD8     PE_GUEST_U32(0x800B0CD8u)
#define D_800B0CDC     PE_GUEST_U16(0x800B0CDCu)
#define D_800B0CDE     PE_GUEST_S16(0x800B0CDEu)
#define D_800B0CE0     PE_GUEST_S8(0x800B0CE0u)
#define D_800B0CE1     PE_GUEST_S8(0x800B0CE1u)
#define D_800B0CE2     PE_GUEST_S8(0x800B0CE2u)
#define D_800B0CE3     PE_GUEST_S8(0x800B0CE3u)
#define D_800B0CE4     PE_GUEST_S8(0x800B0CE4u)
#define D_800B0CE5     PE_GUEST_S8(0x800B0CE5u)
#define D_800B0CE6     PE_GUEST_S8(0x800B0CE6u)
#define D_800B0CE7     PE_GUEST_S8(0x800B0CE7u)
#define D_800B0CE8     PE_GUEST_S8(0x800B0CE8u)
#define D_800B0CE9     PE_GUEST_S8(0x800B0CE9u)
#define D_800B0CEA     PE_GUEST_S8(0x800B0CEAu)
#define D_800B0CEB     PE_GUEST_S8(0x800B0CEBu)
#define D_800B0DB0     PE_GUEST_S8(0x800B0DB0u)
#define D_800B0DB1     PE_GUEST_S8(0x800B0DB1u)
#define D_800B0DB2     PE_GUEST_S8(0x800B0DB2u)
#define D_800B0DB3     PE_GUEST_S8(0x800B0DB3u)
#define D_800B0DB4     PE_GUEST_S8(0x800B0DB4u)
#define D_800B0DB5     PE_GUEST_S8(0x800B0DB5u)
#define D_800B0DB6     PE_GUEST_S8(0x800B0DB6u)
#define D_800B0DB7     PE_GUEST_S8(0x800B0DB7u)
#define D_800B0DC6     PE_GUEST_U8(0x800B0DC6u)
#define D_800B0DCD     PE_GUEST_U8(0x800B0DCDu)
#define D_800B0DD4     PE_GUEST_U16(0x800B0DD4u)
#define D_800B0DD8     PE_GUEST_S32(0x800B0DD8u)
#define D_80094488     PE_GUEST_U8(0x80094488u)
#define D_8009448C     ((uint8_t *)PE_Translate(0x8009448Cu, 64))

/* D_80011614 — retail guest pointer (global at 0x80011614 holding a guest
 * address).  Stored host-side as pe_addr_t; translated only at real access
 * sites. The initial value is bootstrap policy; authenticated real-image
 * startup adopts the executable-backed value (see pe_globals.c). */
extern pe_addr_t D_80011614;
int PE_Globals_AdoptRetailImage(void);

/* ── Host-owned scalar globals (plain data, no pointer arithmetic) ──── */

/* Story word persist[74]: one guest-memory authority (every writer stores
 * the guest word; a host copy left the main-loop volume gate stale). */
#define D_800A7918 PE_GUEST_U32(0x800A7918u)
extern unsigned short D_80093164[];

/* Resource-bank pointer/count owners below use canonical guest RAM. */
/* gp+0x2A8: one guest-memory authority, like its D_8009D03C..64 neighbours
 * (hand writers and generated readers such as func_80051E58 must agree). */
#define D_8009D018 PE_GUEST_U32(0x8009D018u)
#define D_8009D03C PE_GUEST_U32(0x8009D03Cu)
#define D_8009D048 PE_GUEST_U32(0x8009D048u)
#define D_8009D04C PE_GUEST_U32(0x8009D04Cu)
#define D_8009D050 PE_GUEST_U32(0x8009D050u)
#define D_8009D054 PE_GUEST_U32(0x8009D054u)
#define D_8009D058 PE_GUEST_U32(0x8009D058u)
#define D_8009D064 PE_GUEST_U32(0x8009D064u)

/* ── SDK IMPLEMENTED ──────────────────────────────────────────────── */
#include "pe_plat/hostfb_forward.h" /* func_80073A44 / func_80074D28 -> host framebuffer forwards */
int func_80074DC0(int mode);
#include "pe_plat/gpuprim.h" /* DrawPrim/DrawOTag/ClearOTagR/ClearImage/PutDrawEnv/DrawOTagEnv/record pool (clean-room) */
int pe_plat_gpu_clear_inline_host(uint32_t words[2], uint32_t color);
#define PE_ClearImageInline8 pe_plat_gpu_clear_inline_host
int PE_ClearImageInline8(uint32_t words[2], uint32_t color);
#include "pe_plat/gpuprim_clearrect.h" /* PE_DispatchClearRect (host-RECT ClearImage seam), clean-room */
/* Original PutDispEnv command/state sequence, followed by host presentation. */
extern pe_addr_t func_800755F0(pe_addr_t env);
/* func_800752AC (ClearOTagR) and func_80076354 (jtb[11] OTC worker) are
 * REAL translations: game/boot/func_800752AC_port.c (Phase 6E-OTC1). */

/* ── BOOTSTRAP_RET — func_8001220C callees ────────────────────────── */
/* func_8006AD40 is complete through its normal return (Phase 6E-B54K-M).
 * B54K-R enters func_801909B4 after both DMA checkpoints and executes the
 * generic MoveImage path and display setup. B54K-T enters overlay-local
 * func_80190660 and reaches its first DrawPrim call at retail 0x80190860. */
extern int func_8006ECEC(void);
void func_8006F044(void);   /* game/boot/func_8006F044_port.c (matched C port) */
/* func_80069B08 (disc-swap screen) is a hand port
 * (game/boot/func_80069B08_port.c); the other disc comes from the host tray
 * (platform/pe_drive_tray.h), otherwise it is a CPU_BOUNDARY STOP. */
void func_80069B08(int a0);
/* func_8003F3C4 / func_80066B60 / func_80068E24 are REAL native ports
 * (game/boot/func_8003F3C4_port.c, func_80066B60_port.c,
 * func_80068E24_port.c). Prototypes live in pe_port_compat.h. */
/* func_801235DC (credits main loop, ovl_0457) is reached from func_8001220C
 * through the guest-code registry (bootstrap/func_8001220C_port.c). */
/* func_8019234C is a REAL translation of the M0000I transition outer loop
 * (game/boot/func_8019234C_port.c); the former Bootstrap_ReturnVoid stub
 * was removed once its callees, VSync return contract and ordering-table
 * compaction were available. */
extern void func_8019234C(void);
extern int func_801909B4(void);
extern void func_801918F8(int buffer, int wide);
extern int func_80191FB8(int count, pe_addr_t source_list);
extern int PE_func_80191FB8_Values(int count, const pe_addr_t *sources);
extern int func_801924F8(int index);
extern int func_80192934(void);
extern void func_80191DC8(void);
extern int func_80192CE8(int index);
extern int func_80190660(void);
extern void func_8018F2F4(void);
extern pe_addr_t func_8018FBC0(int type);
extern void func_80190064(void);
extern void func_8018F468(void);
extern void func_80192FE8(pe_addr_t node);
extern void func_8019319C(pe_addr_t node);
#include "pe_plat/mdec_callback.h"
#include "pe_plat/mdec_expand.h" /* DecDCToutCallback */
#include "pe_plat/mdec_io_host.h" /* DecDCTin / DecDCTout wrappers */
#include "pe_plat/vlc_frame.h"     /* DecDCTvlc-role frame decoder */
extern void func_8007C304(uint32_t mode, int32_t start,
                          int32_t end, pe_addr_t callback, uint32_t auxiliary);
extern void func_8007C394(uint32_t sector);
/* func_80070E54 / func_80042FE8 / func_8006EBE4 are REAL translations
 * (game/boot/func_80070E54_port.c, Phase FTE1). */
extern void func_80070E54(void);
extern void func_80042FE8(void);
extern int func_8006EBE4(void);
/* func_8006E1C0 is REAL (Phase 6E-B51):
 * game/boot/func_8006E1C0_port.c — packed texture-entry LoadImage
 * dispatcher (68 retail words).  Phase 6E-B52 translates its two
 * func_8007506C (PsyQ LoadImage) wrapper calls through the read-only
 * func_80074E28 validator into the B53C-F func_80076C34 prefix.  B53E
 * translates the direct LoadImage worker func_80076664; B53F translates the
 * execution-proven installed-target path through callback wrapper
 * func_80073CF4, and B53G completes it by translating the installed setter
 * func_800746A0. B53H translates the busy-DMA prefix and B53I-C completes
 * the idle-DMA suffix of queue pump func_80076EE4; a later explicit IRQ
 * checkpoint now consumes the queued request and issues the second DMA.
 * B54D-M then complete every remaining archive/LoadImage walk, live poll,
 * func_80030894, F1/display call, conditional state reset, and bit-0 clear.
 * s2/poll are never assigned and no provider remains inside 6AD40. */

/* func_80038D1C is now a REAL translation too (Phase 6E-B15):
 * game/boot/func_80038D1C_port.c — D_80091A20 byte test-and-clear
 * status leaf (11 retail words; also a matched decomp C leaf).
 * func_8006A9E4 is REAL too (Phase 6E-B16):
 * game/boot/func_8006A9E4_port.c — PE.IMG streaming resource load
 * (215 retail words); B45: callee func_80087090 is TRANSLATED (retry
 * wrapper); B46: func_800851A8 is TRANSLATED (prefix: magic number
 * check + error path; hardware success path routes through the
 * centralized bootstrap boundary).
 * func_800527C8 is REAL too (Phase 6E-B17):
 * game/boot/func_800527C8_port.c — multi-subsystem bootstrap
 * dispatcher (49 retail words, 17 calls); unresolved callees route
 * through the centralized bootstrap boundary in retail order.
 * func_800528F0 is REAL too (Phase 6E-B18):
 * game/boot/func_800528F0_port.c — PRNG table generator
 * (143 retail words, 521-byte output table at D_800A1B90).
 * func_8005BCBC is REAL too (Phase 6E-B24):
 * game/boot/func_8005BCBC_port.c — resource-state pointer/count
 * selector (21 retail words; guest-resident D_8009D0C0/C4/C8).
 * func_8005D6F4 is REAL too (Phase 6E-B25):
 * game/boot/func_8005D6F4_port.c — resource-buffer + display-state
 * initializer (147 retail words; eight unresolved callees routed
 * through the centralized bootstrap boundary after B26).
 * func_8005DC4C is REAL too (Phase 6E-B26):
 * game/boot/func_8005DC4C_port.c — PE.IMG message/string-table lookup
 * (20 retail words; pure read-only lookup, returns the guest address of
 * a 0xFF-terminated record or retail's own 0 for an out-of-range
 * index). */

/* ── REAL translated functions ────────────────────────────────────── */
extern void func_800725DC(void);
extern void func_8001220C(void);
extern int func_80038D1C(void);
extern int  func_800698D4(void);
extern void func_8006A9E4(void);
extern int  func_8006AD40(void);
extern int  func_8006E1C0(pe_addr_t entry, pe_addr_t base);
#include "pe_plat/gpuimg.h"   /* LoadImage/StoreImage/MoveImage wrappers, rect report, image workers (clean-room, docs/evidence/native-gpu2-20261126) */
/* func_800718D0: clean-room provider pe_plat_gpu_tim.c (docs/evidence/native-tim-20261125). */
uint32_t pe_plat_gpu_load_tim(uint32_t tim);
#define func_800718D0 pe_plat_gpu_load_tim
extern pe_addr_t func_800718D0(pe_addr_t tim);
extern void PE_func_8006AD40_PackFontRecords(void);
#include "pe_plat/gpuq.h"   /* func_80076EE4 pump, func_800773D0 seed, PE_func_80076EE4_Pump (clean-room) */
/* Immediate (non-queued) libgpu entry points, platform/pe_libgpu.c. */
#include "pe_plat/bios_sys.h"  /* SetIntrMask 0x80073E10 -> pe_plat_irq_set_mask */
#include "pe_plat/irq_callback.h"
/* Read-only names for the two retail authorities func_800746A0 uses. */
extern pe_addr_t PE_DMA_CallbackSlotAddress(uint32_t dma_channel);
extern pe_addr_t PE_DMA_DicrPointerAddress(void);
extern uint32_t  PE_DMA_DicrMmioAddress(void);
/* Internal pump entry: *retail_returned is 1 only when translated retail
 * control flow reached the shared epilogue, and 0 when a nested typed
 * boundary did not return.  Callers must not infer that distinction from
 * the host stop reason, which keeps only the first reason requested. */
/* Read-only names for the retail authority the pump prefix uses. */
extern pe_addr_t PE_Pump_Dma2ChcrPointerAddress(void);
extern uint32_t  PE_Pump_Dma2ChcrMmioAddress(void);
typedef struct {
    uint64_t mask_disable_order;
    uint64_t callback_removal_order;
    uint64_t gpustat_ready_order;
    uint64_t worker_call_order;
    uint64_t consumer_advance_order;
    uint64_t mask_restore_order;
    uint64_t drawsync_clear_order;
    uint64_t drawsync_call_order;
    uint64_t worker_calls;
    uint64_t drawsync_calls;
    uint64_t gpustat_reads;
    uint32_t saved_mask;
    uint32_t restored_mask;
    pe_addr_t last_worker;
    pe_addr_t last_argument;
    uint32_t last_auxiliary;
    uint32_t consumer_before_worker;
    uint32_t consumer_after_worker;
    uint16_t mask_at_worker;
    uint32_t dma_callback_at_worker;
    uint32_t dicr_at_worker;
    uint32_t chcr_at_worker;
    int worker_returned;
} PeGpuPumpTrace;

/* Value-only evidence for actual typed-pump execution, including the silent
 * busy return.  These functions never participate in retail behavior. */
extern void      PE_Pump_TraceReset(void);
extern uint64_t  PE_Pump_EntryCount(void);
extern void      PE_Pump_GetTrace(PeGpuPumpTrace *out);
extern void func_80042798(void);
#include "pe_plat/gpu_swap.h" /* func_80076C34 + PE_func_80076C34_Inline8 -> pe_plat names (clean-room) */
extern int  func_80076B98(pe_addr_t packet, uint32_t auxiliary);
int pe_plat_gpu_clear_worker_host(pe_addr_t rect, uint32_t color);
#define func_80076434 pe_plat_gpu_clear_worker_host
extern int  func_80076434(pe_addr_t rect, uint32_t color);
extern void func_800527C8(void);
extern pe_addr_t func_8005332C(int32_t resource_id);
extern pe_addr_t func_80053968(int32_t resource_id);
extern int32_t func_80053B48(pe_addr_t record);
extern void func_80051CC4(void);
extern void func_80051980(int32_t unused, pe_addr_t out);
extern void func_80051E64(pe_addr_t out);
extern void func_80052F24(int32_t capacity);
extern void func_8005218C(void);
extern void func_800528F0(void);
extern void func_8005E588(void);
extern void func_80062568(void);
extern void func_80064964(void);
extern void func_8005DE88(void);
extern void func_80052C6C(void);
extern int  func_8005BCBC(pe_addr_t a0);
extern int  func_8005D6F4(void);
extern pe_addr_t func_8005DC4C(uint32_t idx);
extern void func_80052594(pe_addr_t src);  /* retail $v0 unread at every call site (port3 A1) */
extern pe_addr_t func_8005DB8C(int idx);
extern pe_addr_t func_8005DBAC(int arg);
extern void func_8005B91C(int32_t table_number, int32_t key,
                          pe_addr_t index_out, pe_addr_t fraction_out);
extern void PE_func_8005B91C_HostOut(int32_t table_number, int32_t key,
                                     int32_t *index_out,
                                     int32_t *fraction_out);
extern void func_8005CCA4(void);
extern int func_800438C0(int arg0);
extern void func_8005E850(int a0, int a1);
extern int func_8006A2E8(int a0, unsigned int a1);
extern void func_80052790(int value);
extern void func_800649D0(int a0);
extern signed char func_8005E884(void);
extern pe_addr_t func_80071A24(pe_addr_t dst, uint32_t len);
extern void func_8005E968(int a0);
extern void func_8005F844(int enabled);
extern int  func_8006E834(void);
extern int  func_8006E9A0(int);
extern int  func_80087090(pe_addr_t buffer, int count);
extern pe_addr_t func_80085EB4(pe_addr_t spu_addr);
extern void func_800850F4(pe_addr_t src, uint32_t size);
extern void func_80085098(void);
extern void func_80085174(void);   /* serviced DMA4 wait, func_800850F4_port.c */
extern int func_800851A8(pe_addr_t buffer, int count);

#endif
