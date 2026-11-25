/* Checker-only stand-in for pc_port/include/psx_compat.h (used by
 * pe_gpu_rows_check.c via -Ipc_port/tools/chk_inc): declares the guest
 * callees the clean-room providers call, which the checker stubs. */
#ifndef PSX_COMPAT_H
#define PSX_COMPAT_H
#include <stdint.h>
#include "pe_guest_ram.h"
uint32_t func_80073A44(int mode);   /* VSync */
uint32_t func_800773D0(void);       /* GPU deadline seeder */
void func_80076B20(unsigned int command);                        /* GPU control write */
#endif
