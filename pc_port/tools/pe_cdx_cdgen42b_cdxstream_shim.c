/* Link shim (canary replay) for checking the clean-room cdx-stream provider (pe_plat2_cdx_stream.c) with
 * pe_cdx_cdgen42b_check.c / pe_cdx_cdgen42c_check.c: the provider calls the pe_plat_cdxetc_* critical-section veneers
 * (the bios_sys.h macro targets); the checker's recorded stand-ins carry the retail names pe_plat_bios_*.  Map one to the other
 * (a recorded call 0x80072714 / 0x80072724, as the retail code makes it).  Used together with pe_cdx_cdgen42_cdxstream_shim.c. */
extern unsigned pe_plat_bios_enter_critical(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
extern unsigned pe_plat_bios_exit_critical(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
int pe_plat_cdxetc_enter_critical(void) { return (int)pe_plat_bios_enter_critical(0, 0, 0, 0, 0); }
void pe_plat_cdxetc_exit_critical(void) { (void)pe_plat_bios_exit_critical(0, 0, 0, 0, 0); }
