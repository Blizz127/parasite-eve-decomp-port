/* Original VSync algorithm with explicit device/BIOS operations.
 * The host clock adapter must supply these operations before live integration. */
#ifndef PE_VSYNC_H
#define PE_VSYNC_H
#include "pe_guest_ram.h"
typedef struct {
    void *context;
    uint32_t (*read_word)(void *context,pe_addr_t address);
    void (*write_word)(void *context,pe_addr_t address,uint32_t value);
    void (*bios_call)(void *context,uint32_t table,uint32_t service,uint32_t a0,uint32_t a1);
} PeVSyncClock;
uint32_t PE_RetailVSync(int32_t mode,const PeVSyncClock *clock);
#endif
