/*
 * pe_plat CD stream ring / issue / sync / ack leaves (cdx-stream family):
 * startup state, the command control entries (CdControl-style, async,
 * and control-then-wait), latch reset, DMA3 callback wrapper, command
 * progression, status decode, stream word copy, status pump, stream
 * shutdown and device init.
 *
 * PROVENANCE: written clean-room from the black-box vector sets cdgen42 and
 * cdgen42b (docs/evidence/native-cdx-20261126, including the 48 runtime-observed
 * pump tuples), the recorded seam contracts and psx-spx; no SDK source,
 * decompiled source, tables or disassembly were read.  The first revision was
 * written by a fresh author from cdgen42 (branch sw-cdx-stream-20261127); the
 * pump, progress handler and status decoder were redone from cdgen42b by
 * sw-cdx-stream2 (see docs/evidence/native-cdx-20261126/cdx-stream2/AUTHOR-REPORT.md
 * for the files read).  Provider: pc_port/platform/pe_plat2_cdx_stream.c.  The retail
 * names are bound to the entries below by object-like macros, so no
 * func_XXXXXXXX symbol is defined by the port for these leaves.
 *
 * Naming: the provider symbols use the prefix pe_plat_cdxstream_ because
 * gen_decomp_ports.py and pe_plat_canonical_protos.py only accept
 * `#define func_X pe_plat_<word>` bindings.
 */
#ifndef PE_PLAT_CDX_STREAM_H
#define PE_PLAT_CDX_STREAM_H
#include <stdint.h>

void pe_plat_cdxstream_stream_copy_words(uint32_t dst, uint32_t src, uint32_t words);
void pe_plat_cdxstream_decode_status(int status, uint32_t response);
void pe_plat_cdxstream_command_progress(unsigned char status, int response);
void pe_plat_cdxstream_reset_latches(void);
void pe_plat_cdxstream_set_dma3_callback(unsigned int callback);
void pe_plat_cdxstream_shutdown_stream(void);
int pe_plat_cdxstream_initialize_device(void);
int pe_plat_cdxstream_control(unsigned char command, uint32_t data, uint32_t result);
int pe_plat_cdxstream_control_async(unsigned char command, uint32_t data);
int pe_plat_cdxstream_control_wait(unsigned char command, uint32_t data, uint32_t result);
void pe_plat_cdxstream_status_pump(void);
void pe_plat_cdxstream_initialize_state(void);
/* pe_plat2_cdx_stream_slots.c: the callback-slot targets (cdgen42c / cdgen42ci) */
uint32_t pe_plat_cdxstream_vblank_hook(void);
uint32_t pe_plat_cdxstream_status_callback(int status, uint32_t response);
uint32_t pe_plat_cdxstream_ready_callback(int status, uint32_t response);

#define func_8007CE80 pe_plat_cdxstream_stream_copy_words
#define func_8008080C pe_plat_cdxstream_decode_status
#define func_80080404 pe_plat_cdxstream_command_progress
#define func_8007A3CC pe_plat_cdxstream_reset_latches
#define func_8007A8EC pe_plat_cdxstream_set_dma3_callback
#define func_8007A2A4 pe_plat_cdxstream_shutdown_stream
#define func_8007BBFC pe_plat_cdxstream_initialize_device
#define func_8007A4D0 pe_plat_cdxstream_control
#define func_8007A60C pe_plat_cdxstream_control_async
#define func_8007A740 pe_plat_cdxstream_control_wait
#define func_8007FE24 pe_plat_cdxstream_status_pump
#define func_8007FA2C pe_plat_cdxstream_initialize_state
#define func_8007F7E8 pe_plat_cdxstream_vblank_hook
#define func_8007F88C pe_plat_cdxstream_status_callback
#define func_80080778 pe_plat_cdxstream_ready_callback

/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007CE80(uint32_t dst, uint32_t src, uint32_t words);
void func_8008080C(int status, uint32_t response);
void func_80080404(unsigned char status, int response);
void func_8007A3CC(void);
void func_8007A8EC(unsigned int callback);
void func_8007A2A4(void);
int func_8007BBFC(void);
int func_8007A4D0(unsigned char command, uint32_t data, uint32_t result);
int func_8007A60C(unsigned char command, uint32_t data);
int func_8007A740(unsigned char command, uint32_t data, uint32_t result);
void func_8007FE24(void);
void func_8007FA2C(void);
uint32_t func_8007F7E8(void);
uint32_t func_8007F88C(int status, uint32_t response);
uint32_t func_80080778(int status, uint32_t response);
#endif
