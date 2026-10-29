/* ── pe_drive_tray: host drive tray for the retail disc-swap screen ──── *
 *
 * Parasite Eve ships on two discs.  When the story word persist[74] crosses
 * 0x258 (or New Game init runs without disc 2), the main loop calls the
 * retail disc-swap screen func_80069B08 (game/boot/func_80069B08_port.c),
 * which stops the motor, shows "insert disc N", waits for the lid to open,
 * waits for the drive to be ready again and re-mounts (func_800698D4 finds
 * \FMV2\PEDISC02.IDF on disc 2 and sets D_800B0DCD bit 2).
 *
 * The other disc is a host input, like the pad: --disc2-image names the
 * user's own Disc 2 dump (opened read-only, identified as Disc 2 by
 * pe_disc_check; never copied).  The tray holds it as the spare disc and
 * plays the user's part while the screen waits for the lid: open the lid,
 * swap the spare in as the active disc, close the lid.  The disc that comes
 * out becomes the spare, so a later "insert disc 1" works the same way.
 *
 * Observable drive effects follow the SDK's own reaction to a shell-open
 * status (func_8008068C): the latched status byte D_8009B56C carries 0x10
 * and the CdReady lane D_8009B574 reads 2 while the lid is open; on close
 * the lane reads 1 again and the status byte is clear.
 *
 * Without a spare disc for the requested number nothing is faked: the
 * screen is a CPU_BOUNDARY STOP (PE_Port_DiscSwapBoundary). */
#ifndef PE_DRIVE_TRAY_H
#define PE_DRIVE_TRAY_H

#include <stdint.h>
#include "pe_disc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Frames the tray waits after the prompt appears before opening the lid,
 * and while the lid is open before closing it on the other disc. */
#define PE_TRAY_OPEN_DELAY  60u
#define PE_TRAY_CLOSE_DELAY 60u

/* Register the spare disc (number 1 or 2).  NULL clears it. */
void PE_Tray_SetSpare(PE_Disc *disc, int number);
PE_Disc *PE_Tray_Spare(void);
int  PE_Tray_SpareNumber(void);
/* Number of the active disc as registered (1 at boot; 0 if unknown). */
void PE_Tray_SetActiveNumber(int number);
int  PE_Tray_ActiveNumber(void);
/* 1 when the tray can put disc `wanted` in the drive. */
int  PE_Tray_CanServe(int wanted);
/* Start a swap session (resets the lid timer). */
void PE_Tray_Begin(int wanted);
/* One frame of the swap screen.  waiting_for_lid is 1 while the screen is
 * in its "open the lid" state.  Returns 1 on the frame the lid closes on
 * the other disc. */
int  PE_Tray_Frame(int waiting_for_lid);
/* Motor stop with the CD device disabled (the fmv-lane precedent in
 * func_80080DC4: CdlStop completes immediately).  Publishes a completed
 * command (status 2) in the SDK completion ring and returns its sequence
 * for func_8007F418. */
uint32_t PE_Tray_HostStop(void);
/* The swap screen's CdlStop (func_80069B08 case 0): the queued SDK command
 * func_8007EE84(8,0,0,-1) when the register-level CD device is enabled,
 * PE_Tray_HostStop otherwise.  Returns the sequence for func_8007F418. */
uint32_t PE_Tray_Stop(void);
/* Put disc `wanted` in the drive now (dev-warp harness: the player swapped
 * discs before the warp).  Returns 1 when the tray swapped. */
int  PE_Tray_InsertNow(int wanted);
/* Swaps performed (for tests and logs). */
unsigned PE_Tray_Swaps(void);
void PE_Tray_Reset(void);

#ifdef __cplusplus
}
#endif
#endif
