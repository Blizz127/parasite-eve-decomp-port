/* Link shim for checking the clean-room cdx-stream provider (pe_plat2_cdx_stream.c)
 * with pe_cdx_cdgen42_check.c.  The provider's ack loop gives the port clock one
 * tick per pass; the checker's other stand-ins do not include it.  The tick has no
 * retail counterpart and no guest-visible effect, so it is a no-op here.
 * Build (repo root):
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -O2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap -c pc_port/platform/pe_plat2_cdx_stream.c -o build/cdx/cdx_stream.o
 * BUILD: cc -std=gnu11 -Wall -Wextra -Werror -Dpe_plat_cd_initialize_state=pe_plat_cdxstream_initialize_state -Dpe_plat_cd_control=pe_plat_cdxstream_control -Dpe_plat_cd_control_async=pe_plat_cdxstream_control_async -Dpe_plat_cd_control_wait=pe_plat_cdxstream_control_wait -Dpe_plat_cd_reset_latches=pe_plat_cdxstream_reset_latches -Dpe_plat_cd_set_dma3_callback=pe_plat_cdxstream_set_dma3_callback -Dpe_plat_cd_command_progress=pe_plat_cdxstream_command_progress -Dpe_plat_cd_decode_status=pe_plat_cdxstream_decode_status -Dpe_plat_cd_stream_copy_words=pe_plat_cdxstream_stream_copy_words -Dpe_plat_cd_status_pump_host=pe_plat_cdxstream_status_pump -Dpe_plat_cd_shutdown_stream_host=pe_plat_cdxstream_shutdown_stream -Dpe_plat_cd_initialize_device_host=pe_plat_cdxstream_initialize_device -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen42_check.c pc_port/tools/pe_cdx_cdgen42_cdxstream_shim.c build/cdx/cdx_stream.o -o build/cdx/check42cdx
 * Run: ./build/cdx/check42cdx vectors.txt [noop|flip]
 */
void pe_plat_sio_poll_tick(void) {}
