/* Phase 6C — Game port shared declarations */
#ifndef GAME_PORT_H
#define GAME_PORT_H

#include <stdint.h>

/* Host-owned run-control state. */
extern int g_port_stop_requested;
extern int g_port_main_iterations;

typedef int (*PEPortQuitPoll)(void);

typedef enum PEPortStopReason {
    PE_PORT_STOP_NONE = 0,
    PE_PORT_STOP_EXPLICIT,
    PE_PORT_STOP_HOST_QUIT,
    PE_PORT_STOP_FRAME_LIMIT,
    PE_PORT_STOP_MAIN_ITERATION_LIMIT,
    PE_PORT_STOP_UNRESOLVED_BOUNDARY,
    PE_PORT_STOP_GUEST_TRAP,
} PEPortStopReason;

typedef enum PEPortDmaIrqCheckpointResult {
    PE_PORT_DMA_IRQ_CHECKPOINT_IDLE = 0,
    PE_PORT_DMA_IRQ_CHECKPOINT_RETURNED,
    PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY,
    PE_PORT_DMA_IRQ_CHECKPOINT_STALE,
} PEPortDmaIrqCheckpointResult;

/* Value-only evidence for one-token checkpoint admission.  These counters
 * do not participate in scheduling or hardware authority. */
typedef struct PEPortDmaIrqCheckpointTrace {
    uint64_t checkpoint_calls;
    uint64_t token_queries;
    uint64_t service_calls;
    uint64_t last_captured_token;
    uint64_t last_serviced_token;
} PEPortDmaIrqCheckpointTrace;

void PE_Port_RunControlReset(void);
void PE_Port_SetFrameLimit(int frames);
void PE_Port_SetMainIterationLimit(int iterations);
void PE_Port_SetQuitPoll(PEPortQuitPoll poll);
/* Live-window present hook (Phase 6E-PRS1).  The host framebuffer invokes
 * the installed hook after counting each presentation so a windowed run
 * can blit the newest pixels without polling guest state.  The hook runs
 * on host data only and must never touch guest RAM/VRAM. */
typedef void (*PEPortPresentHook)(void);
void PE_Port_SetPresentHook(PEPortPresentHook hook);
void PE_Port_InvokePresentHook(void);
void PE_Port_RequestStop(PEPortStopReason reason);
int  PE_Port_BeginMainIteration(void);
int  PE_Port_ShouldStop(void);
int  PE_Port_FramePresentationAllowed(void);
void PE_Port_FramePresented(int presented);
PEPortStopReason PE_Port_GetStopReason(void);
/* Monotonic stop-request counter.  Compare it across a call to learn
 * whether that call requested a stop; PE_Port_GetStopReason cannot answer
 * that, because only the first reason is retained. */
unsigned PE_Port_StopEpoch(void);
const char *PE_Port_StopReasonName(PEPortStopReason reason);

/* Stop escape for a long-running generated loop that never tests
 * PE_Port_ShouldStop (the ending credits; PE_Main_EnterCredits is the only
 * user).  The caller arms it around the loop and disarms it on every path.
 * While it is armed, PE_Port_EscapePoint() polls PE_Port_ShouldStop(), so a
 * host quit is seen.  Once a stop is requested it longjmps back to the armed
 * caller, and taking the escape disarms it.  The escape point sits at the very
 * entry of HostFB_VSync, PE_GuestCall and HostFB_DevicePoll, before any
 * device or audio servicing.  It is never taken from PE_Port_RequestStop,
 * which runs part-way through device updates.  It can be taken inside IRQ
 * servicing: the CD data-ready dispatch (PE_GuestCall from
 * HostFB_ServiceDeviceIrq) and a VSync(-1) inside the VBlank dispatch both
 * reach a point.  The longjmp then abandons that dispatch's guard and mask
 * state.  That is acceptable only because a stop ends the run: after main
 * returns, port_main's shutdown is host-only (blit, screenshot, logs, stop
 * report) and runs no guest code. */
#include <setjmp.h>
void PE_Port_EscapeArm(jmp_buf *escape);
void PE_Port_EscapeDisarm(void);
void PE_Port_EscapePoint(void);

/* Credits pad refill (HOST_ADAPTED).  Retail's VSync interrupt refills the
 * BIOS pad buffer every VBlank.  The credits loop (ovl_0457 func_801235DC)
 * reads it through func_8003EB04 but, unlike the title, movie and field
 * loops, has no host fill site of its own.  PE_Main_EnterCredits arms this
 * around the loop; while it is armed, PE_Port_CreditsPadPoint (at the entry
 * of HostFB_VSync) refills the buffer from the pad source, so the EX
 * ending's exit press and the pad-code channel switches reach the loop. */
void PE_Port_CreditsPadArm(int armed);
void PE_Port_CreditsPadPoint(void);

/* Deterministic B53I-B2 hardware opportunity.  It captures at most one
 * already-active DMA token, then keeps completion, DICR-edge bridging, and
 * CPU IRQ service as separately callable phases. */
void PE_Port_SetDmaIrqCheckpointEnabled(int enabled);
int PE_Port_DmaIrqCheckpointEnabled(void);
PEPortDmaIrqCheckpointResult PE_Port_ServiceDmaIrqCheckpoint(void);
void PE_Port_DmaIrqCheckpointTraceReset(void);
void PE_Port_GetDmaIrqCheckpointTrace(PEPortDmaIrqCheckpointTrace *out);

/* Host dev entry (plan: skip the opening FMV to reach the field first).
 * When enabled, func_801909B4 does NOT invoke the movie driver
 * func_80192CE8(1); instead it returns the New-Game selector so the real
 * func_8006E9A0(1) publishes the field token 0xA80830C8 and the main loop
 * dispatches into the field tick.  This bypasses the untranslated title
 * menu and the STR/MDEC movie pipeline; it is a documented HOST_ADAPTED
 * shortcut, never a claim that the retail movie/title ran. */
void PE_Port_SetSkipMovie(int enabled);
int  PE_Port_SkipMovie(void);

/* Host-only continue-frame budget for opening/field movie media loops.
 * -1 (default) = unlimited retail loop.  N > 0 allows N func_80192934 /
 * continue-frame iterations before the named media_loop frontier.  0
 * cuts before the next continue.  ResetTestState / RunControlReset
 * restore -1. */
void PE_Port_SetMovieContinueBudget(int continues);
int  PE_Port_MovieContinueBudget(void);
/* Returns 1 if another continue-frame is allowed; if a finite budget
 * remains it is decremented.  Returns 0 when budget is exhausted. */
int  PE_Port_ConsumeMovieContinue(void);

/* Host-only budget for func_801909B4's title main loop (0x801911C0).
 * -1 (default) = retail loop (repeat until Circle confirm / 1000-frame
 * attract timeout).  N >= 0 allows N further loop-continues before the
 * func_801909B4_title_main_loop frontier.  RunControlReset restores -1. */
void PE_Port_SetTitleLoopBudget(int continues);
int  PE_Port_ConsumeTitleLoop(void);

/* Opt-in demo shortcut for M0010's EF(0) menu after Aya's profile.
 * Preserve existing defaults; the full 16F10/4DCA4 menu is not translated. */
void PE_Port_SetSkipOpeningMenu(int enabled);
int  PE_Port_SkipOpeningMenu(void);

/* Title-loop card pump: allow Psy-Q TestEvent (0x800726F4) to return
 * "no event" without PE_PORT_STOP so func_800425DC can complete one
 * frame.  DAY1_card_status keeps the default stop path (flag clear). */
void PE_Port_SetCardTestEventHostReturn(int enabled);
int  PE_Port_CardTestEventHostReturn(void);

/* Host pad fill for D_800BE9A2 (active-low Sony bits).  Retail writes
 * this from libpad/StartPAD at VSync; the port has no SIO, so a source
 * installed here is polled at the field-tick pad site (before 3EB04).
 * Windowed runs install HostWindow_PadRaw; tests/headless may install a
 * scripted source.  NULL source leaves guest RAM unchanged except the
 * existing idle-zero → 0xFFFF normalize. */
typedef uint16_t (*PEPortPadSource)(void);
/* Installing a pad source drops the analog source (below), so route-pad and
 * PE_PAD_SCRIPT sources always reply as the digital pad. */
void PE_Port_SetPadSource(PEPortPadSource source);
int  PE_Port_HasPadSource(void);
uint16_t PE_Port_ReadPadRaw(void);

/* Opt-in ANALOG STICK (pe_plat/enhance.h).  An analog source fills the four
 * DualShock stick bytes RX, RY, LX, LY (0x80 = centre) and returns 1 while an
 * analog controller is connected, else 0.  Only the windowed host installs
 * one (port_main.c, after PE_Port_SetPadSource(HostWindow_PadRaw)). */
typedef int (*PEPortPadAnalogSource)(uint8_t sticks[4]);
void PE_Port_SetPadAnalogSource(PEPortPadAnalogSource source);
/* Pad-buffer header at the fill sites (D_800BE9A0: status byte 0, ID byte).
 * Default: stores 0x4100 (ID 41h, digital), exactly the fill before this
 * option existed.  With ANALOG STICK on and the analog source reporting:
 * 0x7300 (ID 73h, DualShock analog) and the stick bytes at 0x800BE9A4..A7.
 * Returns 1 when it stored the analog reply. */
int  PE_Port_StorePadId(void);
/* The overlay fill sites (title, world map, movie loops): when a pad source
 * is installed, PE_Port_StorePadId() then the raw buttons at 0x800BE9A2. */
void PE_Port_FillPadBuffer(void);

/* Trace helper available to game code */
void Trace_Direct(const char *event);

/* func_8001220C's credits arm (state 0xAA108448 with flag F5 bit 1): runs the
 * ovl_0457 credits loop and returns 1, or records the credits_cd_xa_service
 * boundary, requests a stop and returns 0 (bootstrap/func_8001220C_port.c). */
int PE_Main_EnterCredits(void);

#endif
