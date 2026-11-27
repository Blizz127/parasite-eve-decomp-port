/* Vector check for batch cdgen42 (pe_cdx_vectors.py cdgen42): the CD-stream
 * family bound in game/boot/cd_stream_port.c (retail 0x8007FA2C CD startup state,
 * 0x8007A4D0 / 0x8007A60C / 0x8007A740 CD command control, 0x8007A3CC latch reset,
 * 0x8007A8EC DMA3 callback wrapper, 0x80080404 command progression, 0x8008080C
 * status decode, 0x8007CE80 stream word copy, 0x8007FE24 status pump, 0x8007A2A4
 * stream shutdown, 0x8007BBFC device init).  Scaffold: pe_cdx_common.h (contract).
 * The provider is linked as a built object, never read; every symbol it leaves
 * undefined is stubbed below (retail callees record their calls; host-only
 * symbols the retail code has no counterpart for are traps that fail the case).
 * Build (from the repo root; the object comes from `ninja -C build/vec
 * CMakeFiles/pe_field_runtime.dir/game/boot/cd_stream_port.c.o`):
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen42_check.c build/vec/CMakeFiles/pe_field_runtime.dir/game/boot/cd_stream_port.c.o -o check42
 * Run: ./check42 vectors.txt [noop|flip]
 */
#include "pe_cdx_common.h"
uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
/* retail callees (recorded; arguments as the retail callee takes them) */
CALLEE(pe_plat_bios_enter_critical, 0x80072714u, 0)
CALLEE(pe_plat_bios_exit_critical, 0x80072724u, 0)
CALLEE(pe_plat_cd_command_issue_host, 0x8007B558u, 4)
CALLEE(pe_plat_cd_ds_set_ready_callback, 0x800824C8u, 1)
CALLEE(pe_plat_cd_flush, 0x8007B9ECu, 0)
CALLEE(pe_plat_cd_int_to_pos, 0x8007A930u, 2)
CALLEE(pe_plat_cd_issue_wrapper, 0x8007FCFCu, 2)
CALLEE(pe_plat_cd_queue_len, 0x8007F778u, 0)
CALLEE(pe_plat_cd_retry_command, 0x800800F4u, 0)
CALLEE(pe_plat_cd_set_ready_callback, 0x8007A4BCu, 1)
CALLEE(pe_plat_cd_status_callback, 0x80080164u, 2)
CALLEE(pe_plat_cd_status_poll_host, 0x8007B010u, 2)
CALLEE(pe_plat_cd_stream_handler_host, 0x8007C564u, 0)
CALLEE(pe_plat_cd_text_13cc, 0x8007F960u, 2)
CALLEE(pe_plat_cd_try_issue_cmd, 0x8007FB44u, 2)
CALLEE(pe_plat_dma_callback, 0x80073CF4u, 2)
CALLEE(pe_plat_ds_data_callback, 0x800824F0u, 1)
CALLEE(pe_plat_ds_status_lane, 0x8007FBF0u, 1)
CALLEE(pe_plat_ds_system_status, 0x8007F72Cu, 0)
CALLEE(pe_plat_ds_text_170, 0x8007E704u, 2)
CALLEE(pe_plat_ds_text_3d0, 0x8007E964u, 2)
CALLEE(pe_plat_interrupt_callback, 0x80073CC4u, 2)
CALLEE(pe_plat_reset_callback, 0x80073C94u, 0)
CALLEE(pe_plat_vsync_callbacks, 0x80073D58u, 2)
/* host-only symbols: no retail counterpart */
TRAP(Bootstrap_ReturnVoid) TRAP(Bootstrap_ReturnVoid4Indirect) TRAP(PE_Callback_Bind)
TRAP(PE_CdReg_DetachDevice) TRAP(PE_CdReg_EnableDevice) TRAP(PE_GuestCall) TRAP(PE_GuestCode_Resolve)
TRAP(PE_GuestHook_Call)
IGNORE(PE_StrFeed_Close)   /* host stream-feed close: no retail counterpart, not compared */
uint32_t PE_CdReg_DeviceEnabled(void) { return 1; }   /* the retail device is always present */
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
void pe_plat_cd_initialize_state(void); void pe_plat_cd_reset_latches(void); void pe_plat_cd_stream_copy_words(uint32_t, uint32_t, uint32_t);
int pe_plat_cd_control(unsigned char, uint32_t, uint32_t); int pe_plat_cd_control_async(unsigned char, uint32_t);
int pe_plat_cd_control_wait(unsigned char, uint32_t, uint32_t); void pe_plat_cd_set_dma3_callback(unsigned int);
void pe_plat_cd_command_progress(unsigned char, int); void pe_plat_cd_decode_status(int, uint32_t);
void pe_plat_cd_status_pump_host(void); void pe_plat_cd_shutdown_stream_host(void); int pe_plat_cd_initialize_device_host(void);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    int hasret = entry == 0x8007A4D0u || entry == 0x8007A60Cu || entry == 0x8007A740u || entry == 0x8007BBFCu;
    if (mutant == 1) return hasret ? (*ret = 0, 1) : 0;
    switch (entry) {
    case 0x8007FA2Cu: pe_plat_cd_initialize_state(); return 0;
    case 0x8007A4D0u: *ret = (uint32_t)pe_plat_cd_control((unsigned char)a[0], a[1], a[2]); return 1;
    case 0x8007A60Cu: *ret = (uint32_t)pe_plat_cd_control_async((unsigned char)a[0], a[1]); return 1;
    case 0x8007A740u: *ret = (uint32_t)pe_plat_cd_control_wait((unsigned char)a[0], a[1], a[2]); return 1;
    case 0x8007A3CCu: pe_plat_cd_reset_latches(); return 0;
    case 0x8007A8ECu: pe_plat_cd_set_dma3_callback(a[0]); return 0;
    case 0x80080404u: pe_plat_cd_command_progress((unsigned char)a[0], (int)a[1]); return 0;
    case 0x8008080Cu: pe_plat_cd_decode_status((int)a[0], a[1]); return 0;
    case 0x8007CE80u: pe_plat_cd_stream_copy_words(a[0], a[1], a[2]); return 0;
    case 0x8007FE24u: pe_plat_cd_status_pump_host(); return 0;
    case 0x8007A2A4u: pe_plat_cd_shutdown_stream_host(); return 0;
    case 0x8007BBFCu: *ret = (uint32_t)pe_plat_cd_initialize_device_host(); return 1;
    }
    return -1;
}
