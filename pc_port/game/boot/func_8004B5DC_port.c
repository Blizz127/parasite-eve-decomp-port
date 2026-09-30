/*
 * Field-menu modal window draw: the callback func_8004B584 installs into the
 * modal window's draw slot (+0x30; dispatched by func_80062830 ->
 * menu_draw_callback).
 *
 * Original: [0x8004B5DC,0x8004B650), 0x74 bytes / 29 words, asm/disc1/3BD84.s.
 * This was the last named boundary in the field-menu tree
 * (`PE_MenuDrawCallback_8004B5DC`).
 *
 *   func_8005E8A4(0x10, 0x0A);              ; move the cursor down-right
 *   func_8005FCAC(8 - func_8005E884());     ; draw the alarm/step counter
 *   func_8005EB58(func_80073A44(-1) & 0x10); ; dim when the frame counter bit is set
 *   func_8005E8A4(-0x10, -0x69);
 *   func_8005EB64(0x7B);                    ; frame icon
 *   func_8005E8A4(0, 0xBE);
 *   func_8005ED18(0x7B, 2);                 ; modal body sprite (tiled UVs)
 *
 * func_8005E884() returns the signed alarm byte, so the printed value is the
 * remaining time.  func_80073A44 is the VSync host call (psx_compat.h); the
 * `& 0x10` selects the alternating dim for the blinking border.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8004B5DC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8004B5DC_port.c (src/func_8004B5DC.c); hand port retired (port3 switch-over I). */
