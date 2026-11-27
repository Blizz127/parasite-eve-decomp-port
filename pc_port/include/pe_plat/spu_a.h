/* Bind the libspu leaves of state60 batch state60a (first 13 rows of
 * docs/evidence/native-state60-20261126/table.md) to the clean-room host
 * implementations in platform/pe_plat_spua_a.c.  Written clean-room from the
 * black-box vector sets state60a and psx-spx; no SDK source, decompiled source,
 * tables or disassembly were read. */
#ifndef PE_PLAT_SPU_A_H
#define PE_PLAT_SPU_A_H
#include <stdint.h>

int pe_plat_spua_initialize(int mode);
void pe_plat_spua_write_io(uint32_t source, uint32_t size);
void pe_plat_spua_store_value(int index, unsigned int value, int shift_mode);
int pe_plat_spua_set_address_units(int index, unsigned int address);
int pe_plat_spua_address_value(unsigned int index, int mode);
void pe_plat_spua_set_dma_control_priority(int mode);
void pe_plat_spua_dma_priority(void);
void pe_plat_spua_dma_priority_enabled(void);
void pe_plat_spua_wait_cycles(void);
void pe_plat_spua_register_dma_callback(uint32_t callback);
void pe_plat_spua_shutdown(void);
int pe_plat_spua_configure_heap(int count, uint32_t buffer);
int pe_plat_spua_set_reverb_enabled(int mode);

#define func_8007D1D4 pe_plat_spua_initialize
#define func_8007D454 pe_plat_spua_write_io
#define func_8007DAE0 pe_plat_spua_store_value
#define func_8007DB24 pe_plat_spua_set_address_units
#define func_8007DBC8 pe_plat_spua_address_value
#define func_8007DC04 pe_plat_spua_set_dma_control_priority
#define func_8007DC5C pe_plat_spua_dma_priority
#define func_8007DC84 pe_plat_spua_dma_priority_enabled
#define func_8007DCAC pe_plat_spua_wait_cycles
#define func_8007DD14 pe_plat_spua_register_dma_callback
#define func_80085984 pe_plat_spua_shutdown
#define func_80085A04 pe_plat_spua_configure_heap
#define func_80085A64 pe_plat_spua_set_reverb_enabled
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007D1D4(int mode);
void func_8007D454(uint32_t source, uint32_t size);
void func_8007DAE0(int index, unsigned int value, int shift_mode);
int func_8007DB24(int index, unsigned int address);
int func_8007DBC8(unsigned int index, int mode);
void func_8007DC04(int mode);
void func_8007DC5C(void);
void func_8007DC84(void);
void func_8007DCAC(void);
void func_8007DD14(uint32_t callback);
void func_80085984(void);
int func_80085A04(int count, uint32_t buffer);
int func_80085A64(int mode);
#endif
