/*
 * pe_plat control-word leaves (state60b): RCNT counters, GPU control/query
 * words, interrupt-mask read, pad-init flag, video mode, callback state.
 *
 * PROVENANCE: written clean-room from the black-box vector sets state60b
 * (docs/evidence/native-state60-20261126) and psx-spx; no SDK source,
 * decompiled source, tables or disassembly were read.  Provider:
 * pc_port/platform/pe_plat2_ctl.c.  The retail names are bound to the
 * entries below by object-like macros, so no func_XXXXXXXX symbol is defined
 * by the port for these leaves.
 *
 * Naming: the provider symbols use the prefix pe_plat_ctl_ (not pe_plat2_)
 * because gen_decomp_ports.py and pe_plat_canonical_protos.py only accept
 * `#define func_X pe_plat_<word>` bindings.
 */
#ifndef PE_PLAT_CTL_H
#define PE_PLAT_CTL_H
#include <stdint.h>

uint16_t pe_plat_ctl_callback_state(void);
uint16_t pe_plat_ctl_irq_mask_read(void);
uint32_t pe_plat_ctl_video_set_mode(uint32_t mode);
int32_t pe_plat_ctl_video_mode(void);
uint8_t pe_plat_ctl_graph_debug(void);
int pe_plat_ctl_gpu_status_read(void);
void pe_plat_ctl_gpu_control_write(unsigned int command);
unsigned char pe_plat_ctl_gpu_control_cached(int index);
unsigned int pe_plat_ctl_gpu_query(unsigned int query);
int pe_plat_ctl_gpu_transfer_words(uint32_t source, int count);
void pe_plat_ctl_pad_set_initialized(uint32_t value);
int32_t pe_plat_ctl_pad_initialized(void);
int pe_plat_ctl_counter_setup(unsigned short counter, short target, int flags);
int pe_plat_ctl_counter_get(int counter);
int pe_plat_ctl_counter_start(unsigned int counter);
int pe_plat_ctl_counter_stop(unsigned int counter);
int pe_plat_ctl_counter_reset(int counter);

#define func_80073DE8 pe_plat_ctl_callback_state
#define func_80073DF8 pe_plat_ctl_irq_mask_read
#define func_80074A14 pe_plat_ctl_video_set_mode
#define func_80074A28 pe_plat_ctl_video_mode
#define func_80074CB8 pe_plat_ctl_graph_debug
#define func_8007633C pe_plat_ctl_gpu_status_read
#define func_80076B20 pe_plat_ctl_gpu_control_write
#define func_80076B44 pe_plat_ctl_gpu_control_cached
#define func_80076B58 pe_plat_ctl_gpu_transfer_words
#define func_80076BE0 pe_plat_ctl_gpu_query
#define func_8007DEA4 pe_plat_ctl_pad_set_initialized
#define func_8007DEB0 pe_plat_ctl_pad_initialized
#define func_80085814 pe_plat_ctl_counter_setup
#define func_800858B0 pe_plat_ctl_counter_get
#define func_800858E8 pe_plat_ctl_counter_start
#define func_80085918 pe_plat_ctl_counter_stop
#define func_8008594C pe_plat_ctl_counter_reset

/* Canonical guest-name prototypes for the bindings above (copied from the
 * pe_plat_ctl_ prototypes; tools read callee signatures from these). */
uint16_t func_80073DE8(void);
uint16_t func_80073DF8(void);
uint32_t func_80074A14(uint32_t mode);
int32_t func_80074A28(void);
uint8_t func_80074CB8(void);
int func_8007633C(void);
void func_80076B20(unsigned int command);
unsigned char func_80076B44(int index);
int func_80076B58(uint32_t source, int count);
unsigned int func_80076BE0(unsigned int query);
void func_8007DEA4(uint32_t value);
int32_t func_8007DEB0(void);
int func_80085814(unsigned short counter, short target, int flags);
int func_800858B0(int counter);
int func_800858E8(unsigned int counter);
int func_80085918(unsigned int counter);
int func_8008594C(int counter);

#endif
