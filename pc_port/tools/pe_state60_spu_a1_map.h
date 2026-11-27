/* Checker-side name map for state60a rows 1-13 (author sw-s60a1): pulled in with
 * -include so that pe_state60_spu_check.c (unmodified) calls the clean-room provider
 * pe_plat2_spu_a.c (pe_plat_spua_*) instead of the quarantined pe_plat_spu_* symbols, and
 * so that the checker's raw 0x800857E4/0x800857F4 recorders take the names the provider
 * calls (pe_plat_bios_close_event / pe_plat_bios_disable_event, bios_event.h). */
#include "pe_plat/bios_event.h"
#define pe_plat_spu_initialize pe_plat_spua_initialize
#define pe_plat_spu_write_io pe_plat_spua_write_io
#define pe_plat_spu_store_value pe_plat_spua_store_value
#define pe_plat_spu_set_address_units pe_plat_spua_set_address_units
#define pe_plat_spu_address_value pe_plat_spua_address_value
#define pe_plat_spu_set_dma_control_priority pe_plat_spua_set_dma_control_priority
#define pe_plat_spu_dma_priority pe_plat_spua_dma_priority
#define pe_plat_spu_dma_priority_enabled pe_plat_spua_dma_priority_enabled
#define pe_plat_spu_wait_cycles pe_plat_spua_wait_cycles
#define pe_plat_spu_register_dma_callback pe_plat_spua_register_dma_callback
#define pe_plat_spu_shutdown pe_plat_spua_shutdown
#define pe_plat_spu_configure_heap pe_plat_spua_configure_heap
#define pe_plat_spu_set_reverb_enabled pe_plat_spua_set_reverb_enabled
