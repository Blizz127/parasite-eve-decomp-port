/*
 * Windows controller input: the host_pad.h API implemented over XInput
 * (host_pad_xinput.c), plus the analog-stick query the Win32 window backend
 * needs.  XInput is loaded at run time (xinput1_4.dll, then xinput1_3.dll,
 * then xinput9_1_0.dll), so the executable has no link dependency on it; with
 * no DLL or no controller the game is keyboard only.
 *
 * Layout (Xbox positional, the same as the SDL GameController mapping in
 * host_pad_sdl.c):
 *   A=Cross  B=Circle  X=Square  Y=Triangle  LB/RB=L1/R1  LT/RT=L2/R2
 *   Back/View=Select  Start/Menu=Start  D-pad + left stick = directions
 *   R3 = fast-forward while held, L3 = toggle fast-forward (port hotkeys)
 * Environment (as on Linux):
 *   PE_JOYSTICK=off   no controller input
 *   PE_PAD_SWAP_XY=1  swap Square/Triangle
 *   PE_PAD_DEBUG=1    log connections and every held-mask change
 */
#ifndef HOST_PAD_WIN32_H
#define HOST_PAD_WIN32_H

#include <stdint.h>
#include "host_pad.h"

/* XInput gamepad state to HOST_PAD_* bits (pure; exposed for tests).
 * buttons: XINPUT_GAMEPAD wButtons; triggers 0..255; thumbs in XInput's
 * convention (up = positive Y).  stick_dpad = 0 drops the left stick ->
 * d-pad bits (opt-in ANALOG STICK). */
uint16_t HostPadXInput_Map(uint16_t buttons, uint8_t left_trigger, uint8_t right_trigger,
                           int16_t lx, int16_t ly, int swap_xy, int stick_dpad,
                           uint8_t *hotkeys);

/* Opt-in ANALOG STICK: 1 when at least one controller is connected, with
 * out[] = RX, RY, LX, LY DualShock bytes (0x80 = centre, 0x00 = up/left;
 * per axis the most-deflected connected controller); 0 otherwise. */
int HostPadXInput_AnalogSticks(uint8_t out[4]);

#endif
