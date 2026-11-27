/* SPU register / heap / callback leaves, batch state60a rows 14-26, clean-room host
 * models in platform/pe_plat2_spu_b.c.  Written clean-room from the black-box vector
 * sets state60a (docs/evidence/native-state60-20261126) and psx-spx; no SDK source,
 * decompiled source, tables or disassembly were read.
 * Four behaviours, marked inline in pe_plat2_spu_b.c, follow the expected values of existing
 * native tests (test_native.c SPU_*_guest_dispatch and SPU_voice_volume_native; the reverb
 * tests), not the vectors.
 * The entry names use the prefix pe_plat_spub_ (not pe_plat2_) because the generator
 * and the canonical-prototype tool only accept `pe_plat_\w+` bindings.
 */
#ifndef PE_PLAT_SPU_B_H
#define PE_PLAT_SPU_B_H
#include <stdint.h>

int pe_plat_spub_heap_contains_raw(uint32_t address);
int pe_plat_spub_heap_contains(uint32_t address);
int pe_plat_spub_set_control_bit(int mode);
int pe_plat_spub_set_irq_callback(int callback);
void pe_plat_spub_register_irq(int callback);
uint32_t pe_plat_spub_set_transfer_address(uint32_t address);
int pe_plat_spub_set_transfer_mode(int mode);
void pe_plat_spub_set_callback(int callback);
void pe_plat_spub_set_common_attributes(uint32_t attrs);
void pe_plat_spub_voice_volume(int voice, uint16_t left, uint16_t right,
                                int16_t left_mode, int16_t right_mode);
int pe_plat_spub_set_control_level(int level);
void pe_plat_spub_copy_voice_field(unsigned int index, uint32_t dest);
void pe_plat_spub_set_pair(uint16_t left, uint16_t right);

#define func_80085B34 pe_plat_spub_heap_contains_raw
#define func_80085BB4 pe_plat_spub_heap_contains
#define func_80085C44 pe_plat_spub_set_control_bit
#define func_80085D84 pe_plat_spub_set_irq_callback
#define func_80085DC4 pe_plat_spub_register_irq
#define func_80085EB4 pe_plat_spub_set_transfer_address
#define func_80085F14 pe_plat_spub_set_transfer_mode
#define func_80085F44 pe_plat_spub_set_callback
#define func_80085F74 pe_plat_spub_set_common_attributes
#define func_800862F4 pe_plat_spub_voice_volume
#define func_80089EB8 pe_plat_spub_set_control_level
#define func_80089F08 pe_plat_spub_copy_voice_field
#define func_80089F28 pe_plat_spub_set_pair
/* Canonical guest-name prototypes for the bindings above. */
int func_80085B34(uint32_t address);
int func_80085BB4(uint32_t address);
int func_80085C44(int mode);
int func_80085D84(int callback);
void func_80085DC4(int callback);
uint32_t func_80085EB4(uint32_t address);
int func_80085F14(int mode);
void func_80085F44(int callback);
void func_80085F74(uint32_t attrs);
void func_800862F4(int voice, uint16_t left, uint16_t right, int16_t left_mode, int16_t right_mode);
int func_80089EB8(int level);
void func_80089F08(unsigned int index, uint32_t dest);
void func_80089F28(uint16_t left, uint16_t right);
#endif
