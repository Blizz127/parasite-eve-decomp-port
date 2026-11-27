/* Controller input for the host-only development menu (any controller: the
 * SDL GameController and Linux joystick connections from host_pad.c). */
#ifndef HOST_DEV_MENU_H
#define HOST_DEV_MENU_H
#include <stdint.h>
#include "host_pad_combo.h"
/* Once per host poll, after HostPad_Poll(). */
void HostDevMenu_Poll(void);
/* The same step with explicit controller connections and clock (tests). */
void HostDevMenu_Step(const HostPadOrigin *in, int n, uint64_t now_ms);
/* The menu is open or still draining: the game sees no controller input. */
int HostDevMenu_CapturesInput(void);
/* The controller bits the game may see this poll (held, with a lone
 * Select/Start withheld while it could still become the menu combo). */
uint16_t HostDevMenu_FilterPad(uint16_t held);
void HostDevMenu_Close(void);
#endif
