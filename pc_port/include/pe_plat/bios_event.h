/* Bind the BIOS event-service trampoline addresses (kernel B0 vector:
 * CloseEvent B(09h), TestEvent B(0Bh), DisableEvent B(0Dh), UnDeliverEvent
 * B(20h)) to the host event model in platform/pe_plat_bios_event.c
 * (psx-spx kernelbios, "BIOS Event Functions"). */
#ifndef PE_PLAT_BIOS_EVENT_H
#define PE_PLAT_BIOS_EVENT_H
#include <stdint.h>
int pe_plat_bios_test_event(uint32_t handle);
int pe_plat_bios_close_event(int handle);
int pe_plat_bios_disable_event(int handle);
void pe_plat_bios_undeliver_event(unsigned int cls, int spec);
#define func_800726F4 pe_plat_bios_test_event
#define func_800857E4 pe_plat_bios_close_event
#define func_800857F4 pe_plat_bios_disable_event
#define func_80085804 pe_plat_bios_undeliver_event
/* Canonical guest prototypes (read by the port generators; they expand to
 * the pe_plat declarations above). */
int func_800726F4(uint32_t handle);
int func_800857E4(int handle);
int func_800857F4(int handle);
void func_80085804(unsigned int cls, int spec);
#endif
