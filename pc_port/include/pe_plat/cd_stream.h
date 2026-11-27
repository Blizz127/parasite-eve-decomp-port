#ifndef PE_PLAT_CD_STREAM_H
#define PE_PLAT_CD_STREAM_H
#include <stdint.h>

/* Guest word copy used by the CD stream; overlap copies forward. */
void pe_plat_cd_stream_copy_words(uint32_t dst, uint32_t src, uint32_t words);
#define func_8007CE80 pe_plat_cd_stream_copy_words
void pe_plat_cd_decode_status(int status, uint32_t response);
#define func_8008080C pe_plat_cd_decode_status
void pe_plat_cd_command_progress(unsigned char status, int response);
#define func_80080404 pe_plat_cd_command_progress
void pe_plat_cd_reset_latches(void);
void pe_plat_cd_set_dma3_callback(unsigned int callback);
#define func_8007A3CC pe_plat_cd_reset_latches
void pe_plat_cd_shutdown_stream_host(void);
#define func_8007A2A4 pe_plat_cd_shutdown_stream_host
int pe_plat_cd_initialize_device_host(void);
#define func_8007BBFC pe_plat_cd_initialize_device_host
#define func_8007A8EC pe_plat_cd_set_dma3_callback
int pe_plat_cd_control(unsigned char command, uint32_t data, uint32_t result);
int pe_plat_cd_control_async(unsigned char command, uint32_t data);
int pe_plat_cd_control_wait(unsigned char command, uint32_t data, uint32_t result);
#define func_8007A4D0 pe_plat_cd_control
#define func_8007A60C pe_plat_cd_control_async
#define func_8007A740 pe_plat_cd_control_wait
void pe_plat_cd_status_pump_host(void);
#define func_8007FE24 pe_plat_cd_status_pump_host
void pe_plat_cd_initialize_state(void);
#define func_8007FA2C pe_plat_cd_initialize_state
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007CE80(uint32_t dst, uint32_t src, uint32_t words);
void func_8008080C(int status, uint32_t response);
void func_80080404(unsigned char status, int response);
void func_8007A3CC(void);
void func_8007A8EC(unsigned int callback);
int func_8007A4D0(unsigned char command, uint32_t data, uint32_t result);
int func_8007A60C(unsigned char command, uint32_t data);
int func_8007A740(unsigned char command, uint32_t data, uint32_t result);
void func_8007FA2C(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007FE24(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007A2A4(void);
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007BBFC(void);
#endif
