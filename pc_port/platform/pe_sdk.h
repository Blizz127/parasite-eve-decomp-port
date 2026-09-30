/*
 * Phase 6E-A — PsyQ SDK provider layer.
 *
 * Real host implementations of the SDK providers on the native boot path,
 * replacing the Phase 6D-S BOOTSTRAP_RET stubs.  Every function is classified
 * per the Phase 6E-A provider-frontier audit:
 *
 *   class 1 — translated game logic (guest-RAM state transcribed verbatim)
 *   class 2 — PsyQ/SDK behavior requiring a host implementation
 *   class 3 — deterministic platform provider with documented ordering
 *
 * Convention: retail guest addresses stay pe_addr_t; all guest-state effects
 * go through the bounds-checked PE_Load / PE_Store / PE_Translate API.
 * Hardware-only effects (SPU/CD/GPU registers, kernel events, DMA) are
 * represented by narrow named providers or explicitly documented no-ops.
 * Nothing
 * here returns a fabricated value merely to advance strict mode: state
 * transitions reproduce the retail-observable guest-RAM effects.
 */
#ifndef PE_SDK_H
#define PE_SDK_H

#include <stdint.h>
#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

int func_8007B290(uint32_t mode, pe_addr_t result);
int func_8007A488(int mode, pe_addr_t pe_result);
void func_8007C564(void);
void func_8007C13C(void);
void func_8007CEAC(uint32_t channel,pe_addr_t address,uint32_t blocks,uint32_t words,
                    uint32_t control,uint32_t interrupt,uint32_t unused);
void func_8007F960(uint32_t status, pe_addr_t response);
int func_8007F994(void);
int func_8007BAC0(void);
void func_800812F4(unsigned int value);
uint32_t func_80073D58(uint32_t slot, pe_addr_t handler);
void func_8007FE24(void);
void func_800800F4(void);
int func_8007BBFC(void);
void func_8007FA2C(void);
void func_8007E5C4(void);
void func_8007E964(uint32_t status, pe_addr_t response);
void func_80080164(uint32_t status, pe_addr_t response);
void func_8007EB88(uint32_t sequence, uint32_t status, pe_addr_t response);
void func_8007E704(uint32_t status, pe_addr_t response);
/* Address-keyed data-ready callback dispatcher owned by cd_stream_port.c.
 * Retail dispatches D_8009AFB8 / D_8009AFB4 / D_800A36A4 / D_800B8AB4
 * through indirect calls; the translated-tree targets are native leaves, so
 * every poll loop that consumes func_8007AAB4 events (7C13C, 7B010, 7B558,
 * 7E964, 7F88C) shares this one dispatcher instead of stopping. Returns 1
 * when the target was handled, 0 when the default arm raised the explicit
 * unresolved-callback boundary. */
int PE_Cd_DispatchDataCallback(pe_addr_t target, uint32_t status,
                              pe_addr_t response);

/* ── libgte (pc_port/platform/pe_gte.c) ─────────────────────────────── */
/* cop2 control-register state has no guest-RAM backing; it is host-owned. */
typedef struct {
    int32_t ofx;   /* $24 screen offset X (16.16) */
    int32_t ofy;   /* $25 screen offset Y (16.16) */
    int32_t h;     /* $26 projection plane distance */
    int32_t dqa;   /* $27 depth cueing coefficient */
    int32_t dqb;   /* $28 depth cueing offset */
    int32_t zsf3;  /* $29 average-z scale (3 terms) */
    int32_t zsf4;  /* $30 average-z scale (4 terms) */
    int16_t rt[3][3]; /* C2CTRL 0-4 rotation, 12.12 */
    int32_t tr[3];    /* C2CTRL 5-7 TRX/TRY/TRZ */
    int16_t llm[3][3]; /* C2CTRL 8-12 light matrix */
    int16_t lcm[3][3]; /* C2CTRL 16-20 color matrix */
    int32_t bk[3];    /* C2CTRL 13-15 RBK/GBK/BBK */
    int32_t fc[3];    /* C2CTRL 21-23 RFC/GFC/BFC */
    int32_t ir[3];    /* C2DR 9-11 IR1-3 */
    int32_t mac[3];   /* C2DR 25-27 MAC1-3 */
    int32_t ir0;      /* C2DR 8 interpolation / depth cue */
    int32_t mac0;     /* C2DR 24 scalar result */
    uint32_t otz;     /* C2DR 7 averaged depth */
    uint32_t sxy[3], sz[4]; /* projection FIFOs */
    uint32_t projection_flags; /* RTPS/RTPT result; other commands do not retain FLAG here */
    int16_t v0[3];    /* C2DR 0-1 VXY0/VZ0 */
    int16_t v1[3];    /* C2DR 2-3 VXY1/VZ1 */
    int16_t v2[3];    /* C2DR 4-5 VXY2/VZ2 */
    uint32_t rgbc;    /* C2DR 6 RGBC */
    uint32_t lzcs, lzcr; /* C2DR 30-31 leading-sign-bit source/result */
    uint32_t rgb_fifo[3]; /* C2DR 20-22 RGB0/1/2 */
} PeGteState;
extern PeGteState g_pe_gte;

void func_80077F7C(void);            /* InitGeom */
void func_80079004(int a, int b);    /* SetGeomOffset: OFX=a<<16, OFY=b<<16 */
void func_80079024(int a);           /* SetGeomScreen: H=a */

/* Exact 32-bit GTE LZCS/LZCR arithmetic (Phase 6E-B4): count of leading
 * bits equal to the sign bit.  Defined for every input (0 -> 32,
 * 0xFFFFFFFF -> 32); pure arithmetic, no g_pe_gte state. */
uint32_t PE_GTE_LZCR(uint32_t v);
void PE_GTE_SetLZCS(uint32_t v);

/* Exact integer MVMVA (psx-spx): no host float.
 * cmd bits: sf@19, mx@17-18, v@15-16, cv@13-14, lm@10. */
void PE_GTE_LoadRT(pe_addr_t matrix);
void PE_GTE_LoadRT33(pe_addr_t matrix);
void PE_GTE_LoadLCM(pe_addr_t matrix);
void PE_GTE_LoadLLM_halfs(const int16_t *halfs);
void PE_GTE_SetIR(int16_t ir1, int16_t ir2, int16_t ir3);
void PE_GTE_SetV0(int16_t vx, int16_t vy, int16_t vz);
void PE_GTE_SetV1(int16_t vx, int16_t vy, int16_t vz);
void PE_GTE_SetV2(int16_t vx, int16_t vy, int16_t vz);
void PE_GTE_SetRGBC(uint32_t rgbc);
void PE_GTE_SetBK(int32_t rbk, int32_t gbk, int32_t bbk);
void PE_GTE_MVMVA(uint32_t cmd);
void PE_GTE_GPF(int sf, int lm);
void PE_GTE_GPL(int sf, int lm);
void PE_GTE_OP(int sf, int lm);
/* Exact integer NCCT (psx-spx COP2 0x118043F). No NCLIP. No host float. */
void PE_GTE_NCCT(void);
void PE_GTE_RTPS_coordinates(uint32_t *xy, uint32_t *z);
void PE_GTE_RTPT_coordinates(uint32_t xy[3], uint32_t z[3]);
void PE_GTE_AVSZ3(const uint32_t z[3]);
void PE_GTE_AVSZ4(const uint32_t z[4]);
int32_t PE_GTE_NCLIP(void);

/* ── memory-card BIOS / libcard (pc_port/platform/pe_bios_card.c) ──── */
int  func_800726F4(uint32_t handle);                 /* B(0Bh) TestEvent */
int  func_8007DD44(uint32_t chan);                   /* A(ABh) _card_info */
int  func_8007DD54(uint32_t chan);                   /* A(ACh) _card_load */
void func_8007DD74(pe_addr_t chan);                  /* _new_card + sector-3Fh write (generated TU) */
void func_8007DDC4(pe_addr_t chan);                  /* B(50h) allow_new_card */
int  func_8007DDB4(pe_addr_t chan, int sector, int src); /* B(4Eh) */
int  func_80072734(pe_addr_t name, uint32_t mode);   /* B(32h) open */
int  func_80072744(int fd, int32_t offset, int whence); /* B(33h) lseek */
int  func_80072754(int fd, pe_addr_t dst, int32_t length); /* B(34h) read */
int  func_80072764(int fd, pe_addr_t src, int32_t length); /* B(35h) write */
int  func_80072774(int fd);                          /* B(36h) close */
int  func_80072784(pe_addr_t device);                /* B(41h) format */
pe_addr_t PE_Bios_FirstFile(pe_addr_t pattern, pe_addr_t dir); /* B(42h) */
pe_addr_t func_80072794(pe_addr_t dir);              /* B(43h) nextfile */
int  func_800727A4(pe_addr_t name);                  /* B(45h) delete */

/* ── libetc (pc_port/platform/pe_libetc.c) ──────────────────────────── */
void func_80073C74(uint32_t flag); /* BIOS ChangeClearPAD */
uint32_t func_80073C84(uint32_t counter,uint32_t flag); /* ChangeClearRCnt,0..3 */
void func_80073C94(void);            /* ResetCallback */
pe_addr_t func_80073CC4(uint32_t source, pe_addr_t handler);
pe_addr_t func_800740D0(uint32_t source, pe_addr_t handler);
uint32_t func_80073D24(pe_addr_t handler); /* VBlank callback slot 4 setter */
uint16_t func_80073E10(uint16_t new_mask); /* I_MASK exchange (PE_IRQ authority) */
int  func_80072714(void);            /* EnterCriticalSection */
void func_80072724(void);            /* ExitCriticalSection */
void func_800726C4(void);            /* BIOS A0(44h) FlushCache host adapter */
int  PE_Irq_LockDepth(void);         /* diagnostic: current critical depth */

/* B53I-B1 bounded host equivalents of source 0's BIOS auto-ack controls.
 * This value-only snapshot includes live guest slot/mask observations at
 * each BIOS call; it is diagnostic state, never a callback authority. */
typedef struct {
    uint32_t pad_clear_mode;
    uint32_t vblank_clear_mode;
    uint32_t pad_calls;
    uint32_t vblank_calls;
    uint32_t pad_argument;
    uint32_t vblank_counter;
    uint32_t vblank_argument;
    uint16_t mask_at_pad_call;
    uint16_t mask_at_vblank_call;
    pe_addr_t source0_slot_at_pad_call;
    pe_addr_t source0_slot_at_vblank_call;
    uint16_t registered_mask_at_pad_call;
    uint16_t registered_mask_at_vblank_call;
    uint64_t pad_call_order;
    uint64_t vblank_call_order;
    uint64_t source0_mask_restore_order;
} PeIrqSource0BiosState;

void PE_Irq_GetSource0BiosState(PeIrqSource0BiosState *out);

/* OpenEvent / EnableEvent host shims (BIOS B(08h)/B(0Ch)).
 * Retail allocates kernel Event Control Blocks outside the 2 MiB guest
 * window; the host models handles as an opaque deterministic counter.
 * Never returns -1 at boot (event classes used never exhaust). */
int  PE_Event_Open(uint32_t cls, uint32_t spec, uint32_t mode, pe_addr_t handler);
int  PE_Event_Enable(int handle);
/* Card events: returns the EvMdINTR handler to run for (cls, spec), or 0
 * (not opened/enabled, or recorded for TestEvent instead). */
pe_addr_t PE_Event_DeliverCard(uint32_t cls, uint32_t spec);
int  PE_Event_Test(int handle);
int  PE_Event_Disable(int handle);
int  PE_Event_Close(int handle);
int  PE_Event_UnDeliver(uint32_t cls, uint32_t spec);
int  PE_Event_SpuDmaEnabled(void);
int  PE_Event_DeliverSpuDma(void);
int  PE_Event_ConsumeSpuDma(int handle);
/* Host scheduling of the command-RAM portion of the audio timer. */
void PE_Event_ServiceAudioCommands(void);
/* Audio lane: 1 when the retail RCNT2 audio event (func_8008E23C) is
 * registered+enabled and no critical section is held — the gate under which
 * the native audio driver (pe_audio_driver.h) runs func_8008DB7C. */
int PE_Event_AudioTimerArmed(void);
void func_8008CA84(void);

/* ── libgpu (pc_port/platform/pe_libgpu.c) ──────────────────────────── */
pe_addr_t func_80074924(pe_addr_t env, int x, int y, int w, int h); /* SetDefDrawEnv */
pe_addr_t func_800749D8(pe_addr_t pe_env, int x, int y, int w, int h); /* SetDefDispEnv */
int       func_80074A44(int mode);   /* ResetGraph */
unsigned char func_80074BB8(unsigned char a0);  /* SetGraphDebug */
pe_addr_t func_80075424(pe_addr_t env); /* PutDrawEnv */
void PE_SetTexWindowValues75B4C(pe_addr_t packet,const int16_t rect[4]);
void PE_SetDrawAreaValues75B84(pe_addr_t packet,const int16_t rect[4]);
void func_80075B4C(pe_addr_t packet,pe_addr_t rect);
void func_80075B84(pe_addr_t packet,pe_addr_t rect);
void      func_800754E4(pe_addr_t ot, pe_addr_t env); /* DrawOTagEnv software */

/* ── libsnd (pc_port/platform/pe_libsnd.c) ──────────────────────────── */
void func_8007D054(void);            /* SsInit wrapper (tail-call 7D074(0)) */
void func_8007D15C(void);            /* SPU IRQ event install */

/* ── streaming (pc_port/platform/pe_stream.c) ───────────────────────── */
void func_80085644(void);            /* streaming bring-up */
void func_80086FF8(void);            /* stream command 0xF0 */
void func_80087024(void);            /* stream command 0xF1 */
void func_8008682C(int a0);           /* stream command select */
int  func_8008CBA8(void);            /* streaming command dispatcher */
void func_8008D140(pe_addr_t attributes); /* masked SPU mode registers */
void func_8008CB54(uint32_t mode); /* SPU reverb mode transition */

/* ── libcard (pc_port/platform/pe_libcard.c) ────────────────────────── */
void func_800409B4(void);            /* InitCARD + StartCARD */

/* ── libcd (pc_port/platform/pe_libcd.c) ────────────────────────────── */
int  func_8007EC14(void);            /* CdInit */
int  func_8007ED58(void);            /* Cd reset + state clear (returns 1) */
int  func_8007FBF0(int idx);         /* CdStatus lane getter D_8009B574[idx] */
int  func_8007F72C(void);            /* CdReady */
int func_8007F778(void);            /* CdReady queue-depth getter D_800A3608 */
void func_800870F0(unsigned int a0);
void func_8010BD4C(pe_addr_t dst, uint32_t count);
int  func_8010C89C(uint32_t a0, pe_addr_t a1, pe_addr_t a2,
                   uint32_t a3); /* VLC frame decoder; a3 dead */
typedef struct PeC89CTelemetry {
    uint32_t a0;                 /* entry cursor (0 = resume) */
    pe_addr_t a1;                /* entry arena */
    pe_addr_t a2;                /* entry table word */
    int ret;                     /* exit code (0 pad, 1 bound) */
} PeC89CTelemetry;
void PE_C89C_GetTelemetry(PeC89CTelemetry *out); /* host-only, never guest */
void func_8007C214(void);
int func_8007A88C(pe_addr_t pe_buffer);
void func_8007B964(pe_addr_t p);
int32_t func_80191B64(pe_addr_t a0);
int func_80080CC8(int value);           /* exchange D_8009AFC0 */
int func_8007F7A8(void);            /* getter D_8009B590 */
int func_80080C48(pe_addr_t pe_a0);    /* CdPosToInt: BCD mm/ss/ff @fp → LBA */
uint32_t func_8007EE84(uint32_t command,pe_addr_t param,uint32_t extra,uint32_t callback);
void func_80080998(pe_addr_t pe_a0, pe_addr_t pe_a1);
int func_8007FC64(pe_addr_t pe_result);
int func_8007F418(uint32_t sequence,pe_addr_t response);
int func_80080DC4(int command,pe_addr_t param,pe_addr_t response);
int  func_80080D5C(int command, pe_addr_t param, pe_addr_t result);
uint32_t PE_Cd_GetSetlocRaw(void);    /* host telemetry: last proven CdlLOC */
int func_800824C8(int value);
pe_addr_t func_800824F0(pe_addr_t callback);
int  func_80081314(pe_addr_t location, uint32_t mode);
int  func_80082314(void);            /* PVD verify; result word D_800B28F8 */
int  func_80081414(pe_addr_t fp, const char *name); /* DsSearchFile */
int  func_8006E6D4(int lba_base, int lba_off, pe_addr_t dest, int sectors);
int  func_800811E4(pe_addr_t fp);    /* read poll: 0 done, -1 timeout */
pe_addr_t func_8007E6B0(void);       /* request-slot ring allocator */
void func_80080950(pe_addr_t pe_a0, pe_addr_t pe_a1); /* 4-byte copy-or-clear */
void func_8007C214(void);            /* streaming DMA-completion callback */
pe_addr_t func_8007A930(int arg0, pe_addr_t pe_arg1);
int func_8007AA34(pe_addr_t pe_a0);
int func_8007C2A0(int a0);
/* matched src/func_8007A4BC.c: `func_8009AFB8_t func_8007A4BC(func_8009AFB8_t)`;
 * a PS1 function pointer is a 32-bit guest code address (pe_guestcode.h). */
pe_addr_t func_8007A4BC(pe_addr_t callback);
void func_8007A8EC(unsigned int value);
void func_8007A2A4(void);
void func_8007C394(uint32_t sector); /* stream-record index update */
int  func_8007F0C8(uint32_t mode, pe_addr_t loc, int count, uint32_t a3,
                    pe_addr_t buf);  /* CdlReadS queue issue */
int  func_8007E8F4(void);            /* completion-queue predicate */
int  func_8007FB44(uint32_t cmd, uint32_t data); /* completion-dispatch */
unsigned short func_80073DE8(void);  /* D_800945E6 halfword getter */
void func_8007B9EC(void);            /* CD latch block */
int  func_8007AAB4(void);            /* CD acknowledge-poll worker (stop) */
int  func_8007B010(uint32_t cmdi, pe_addr_t buf); /* CD status-poll prefix */
int  func_8007B558(uint32_t cmd, uint32_t data, pe_addr_t dst,
                   uint32_t mode);   /* CD command-issue controller */
int  func_8007FCFC(uint32_t cmd, uint32_t data); /* CD issue wrapper */
char *func_800719F4(char *destination, const char *source); /* BIOS A(15h) strcat */
void func_8010BFA0(pe_addr_t command_block,uint32_t mode);
void func_8010C01C(pe_addr_t destination,uint32_t words);
void func_8010BE3C(int mode);         /* libpress DecDCTReset wrapper */

/* ── streaming wrappers (pc_port/game/boot/, Phase 6E-B16) ────────── */
int func_8006E6A8(int lba, pe_addr_t pe_dest, int sectors); /* issue */
int       func_8006E7E8(void);              /* poll + D_800B0CD8 RMW */
pe_addr_t func_8006E498(pe_addr_t base, uint32_t key); /* archive lookup */

/* ── dispatcher leaves (pc_port/game/boot/, Phase 6E-B17/18) ──────── */
void func_8005B890(int a0);                 /* D_8009D028 = a0 */
void func_8005BC98(void);         /* D_8009D218 = 1 */
void func_8004F808(void);                   /* ten-word clear */
void func_80042B38(void);                   /* D_800A1870/1874 = 0 */
void func_80051084(void);                   /* D_8009D014 = 0x800A1AA0 */
pe_addr_t func_8005332C(int32_t resource_id); /* resource-record lookup */
pe_addr_t func_80053968(int32_t resource_id); /* materialize archive record */
void func_80051CC4(void);                   /* resource command-state init */
void func_800528F0(void);                   /* PRNG table generator, 521 bytes */
void func_8005E588(void);                   /* display environment setup */
void func_80062568(void);                   /* free-list pool init, 24 0x90-byte slots */
void func_80064964(void);                   /* A(28h) clear + 8 0xFF flag bytes */
void func_8005DE88(void);                   /* resource-list/state initializer */
pe_addr_t func_80071A24(pe_addr_t dst, uint32_t len);  /* BIOS A(28h) bzero trampoline */
void func_8005E968(int a0);         /* pack-color halver */
void func_8005F844(int enabled);                 /* conditional constant stores */
void func_80052E30(uint32_t a0);            /* resource-buffer init/reuse */
uint32_t func_80052F0C(void);                /* buffer-identity comparison */

/* ── save manager (pc_port/platform/pe_save.c) ──────────────────────── */
void func_800844E4(pe_addr_t base, pe_addr_t base2);
void func_80082534(void);

/* ── game boot (pc_port/game/boot/) ─────────────────────────────────── */
void func_8003E754(int w, int h);    /* video init (DISPENV/DRAWENV setup) */
void func_8003E944(void);            /* save-manager bring-up */

/* ── test determinism ───────────────────────────────────────────────── */
/* Reset every host-owned SDK state block (GTE state, IRQ lock depth,
 * event handles, I_STAT/I_MASK authority, SPU RAM and pending DMA).  It also
 * invalidates the guest DMA busy/callback/IRQ-handle state owned by those
 * host resources and coherently clears the guest-backed CPU IRQ registration
 * block.  Other guest RAM is reset via PE_RamReset. */
void PE_Sdk_ResetState(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_SDK_H */
