#ifndef PE_PLAT_CD_STREAM_H
#define PE_PLAT_CD_STREAM_H
#include <stdint.h>

/* Guest word copy used by the CD stream; overlap copies forward. */
void pe_plat_cd_stream_copy_words(uint32_t dst, uint32_t src, uint32_t words);
void pe_plat_cd_decode_status(int status, uint32_t response);
void pe_plat_cd_command_progress(unsigned char status, int response);
void pe_plat_cd_reset_latches(void);
void pe_plat_cd_set_dma3_callback(unsigned int callback);
void pe_plat_cd_shutdown_stream_host(void);
int pe_plat_cd_initialize_device_host(void);
int pe_plat_cd_control(unsigned char command, uint32_t data, uint32_t result);
int pe_plat_cd_control_async(unsigned char command, uint32_t data);
int pe_plat_cd_control_wait(unsigned char command, uint32_t data, uint32_t result);
void pe_plat_cd_status_pump_host(void);
void pe_plat_cd_initialize_state(void);
#include "pe_plat/cdx_stream.h"
#endif
