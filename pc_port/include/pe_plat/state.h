#ifndef PE_PLAT_STATE_H
#define PE_PLAT_STATE_H
#include <stdint.h>

uint16_t pe_plat_callback_state(void);
int32_t pe_plat_video_mode(void);
int32_t pe_plat_pad_initialized(void);
uint32_t pe_plat_video_set_mode(uint32_t mode);
uint8_t pe_plat_graph_debug(void);
void pe_plat_pad_set_initialized(uint32_t value);
int pe_plat_counter_get(int counter);
int pe_plat_counter_reset(int counter);
int pe_plat_counter_start(unsigned int counter);
int pe_plat_counter_stop(unsigned int counter);
int pe_plat_counter_setup(unsigned short counter, short target, int flags);
void pe_plat_spu_set_callback(int callback);
void pe_plat_spu_register_irq(int callback);
void pe_plat_spu_register_dma_callback(uint32_t callback);
void pe_plat_spu_shutdown(void);
int pe_plat_spu_set_irq_callback(int callback);
int pe_plat_spu_set_transfer_mode(int mode);
uint16_t pe_plat_interrupt_mask_read(void);
int pe_plat_gpu_status_read(void);
void pe_plat_gpu_control_write(unsigned int command);
unsigned char pe_plat_gpu_control_cached(int index);
unsigned int pe_plat_gpu_query(unsigned int query);
int pe_plat_gpu_transfer_words(uint32_t source, int count);
void pe_plat_spu_dma_priority(void);
void pe_plat_spu_dma_priority_enabled(void);
int pe_plat_spu_address_value(unsigned int index, int mode);
int pe_plat_spu_set_address_units(int index, unsigned int address);
uint32_t pe_plat_spu_set_transfer_address(uint32_t address);
void pe_plat_spu_store_value(int index, unsigned int value, int shift_mode);
void pe_plat_spu_set_dma_control_priority(int mode);
void pe_plat_spu_copy_voice_field(unsigned int index, uint32_t dest);
void pe_plat_spu_set_pair(uint16_t left, uint16_t right);
void pe_plat_spu_set_common_attributes(uint32_t attrs);
void pe_plat_spu_voice_volume(int voice, uint16_t left, uint16_t right,
                              int16_t left_mode, int16_t right_mode);
int pe_plat_spu_set_control_level(int level);
int pe_plat_spu_set_reverb_enabled(int mode);
int pe_plat_spu_set_control_bit(int mode);
void pe_plat_spu_write_io(uint32_t source, uint32_t size);
int pe_plat_spu_initialize(int mode);
void pe_plat_spu_wait_cycles(void);
void pe_plat_spu_install_dma_event(void);
void pe_plat_spu_init_wrapper(int mode);
void pe_plat_spu_init_default(void);
int pe_plat_spu_configure_heap(int count, uint32_t buffer);
int pe_plat_spu_heap_contains(uint32_t address);
int pe_plat_spu_heap_contains_raw(uint32_t address);
uint32_t pe_plat_spu_write(uint32_t source, uint32_t size);
uint32_t pe_plat_spu_read(int guest_destination, unsigned int size);

#define func_8007D15C pe_plat_spu_install_dma_event
#define func_8007D054 pe_plat_spu_init_default
#define func_8007D074 pe_plat_spu_init_wrapper
#define func_80085C44 pe_plat_spu_set_control_bit
#define func_80085EB4 pe_plat_spu_set_transfer_address
#define func_80085B34 pe_plat_spu_heap_contains_raw
#define func_80085BB4 pe_plat_spu_heap_contains
#define func_80085D84 pe_plat_spu_set_irq_callback
#define func_80085DC4 pe_plat_spu_register_irq
#define func_80085F14 pe_plat_spu_set_transfer_mode
#define func_80085F44 pe_plat_spu_set_callback
#define func_80085F74 pe_plat_spu_set_common_attributes
#define func_800862F4 pe_plat_spu_voice_volume
#define func_80089EB8 pe_plat_spu_set_control_level
#define func_80089F28 pe_plat_spu_set_pair
#define func_80089F08 pe_plat_spu_copy_voice_field
#define func_80085E54 pe_plat_spu_write
#define func_80085DF4 pe_plat_spu_read
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007D15C(void);
void func_8007D054(void);
void func_8007D074(int mode);
int func_80085C44(int mode);
uint32_t func_80085EB4(uint32_t address);
int func_80085B34(uint32_t address);
int func_80085BB4(uint32_t address);
int func_80085D84(int callback);
void func_80085DC4(int callback);
int func_80085F14(int mode);
void func_80085F44(int callback);
void func_80085F74(uint32_t attrs);
void func_800862F4(int voice, uint16_t left, uint16_t right, int16_t left_mode, int16_t right_mode);
int func_80089EB8(int level);
void func_80089F28(uint16_t left, uint16_t right);
void func_80089F08(unsigned int index, uint32_t dest);
uint32_t func_80085E54(uint32_t source, uint32_t size);
uint32_t func_80085DF4(int guest_destination, unsigned int size);
/* The SPU leaves re-derived clean-room (state60a) bind in pe_plat/spu_a.h; users of these guest
 * names keep seeing the bindings through this include. */
#include "pe_plat/spu_a.h"
#endif
