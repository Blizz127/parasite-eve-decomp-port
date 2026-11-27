/* Bind the six CD host entries of the libcd interrupt / command / DMA path to the clean-room
 * implementations platform/pe_plat2_cdhost_{ack,poll,cmd,sector,stream,dma}.c.
 *
 *   retail entry   public name (unchanged for callers)        implementation
 *   0x8007AAB4     pe_plat_cd_acknowledge_host(void)           pe_plat_cdhost_ack
 *   0x8007B010     pe_plat_cd_status_poll_host(mode, dst)      pe_plat_cdhost_poll
 *   0x8007B558     pe_plat_cd_command_issue_host(cmd, ...)     pe_plat_cdhost_cmd
 *   0x8007BF44     pe_plat_cd_sector_dma_host(dst, count)      pe_plat_cdhost_sector
 *   0x8007C564     pe_plat_cd_stream_handler_host(void)        pe_plat_cdhost_stream
 *   0x8007CEAC     pe_plat_cd_dma_issue_host(channel, ...)     pe_plat_cdhost_dma
 *   0x800811E4     pe_plat_cd_read_poll_host(fp)               pe_plat_cdhost_readpoll
 *
 * The public names stay declared (object-like macros), so every caller that spells them keeps
 * compiling and linking; the retail names func_8007XXXX are bound by the same macros.
 * Written clean-room from black-box vector sets (cdgen47, docs/evidence/native-cdx-20261126),
 * the public headers and psx-spx; see the provenance header of each implementation file and
 * docs/evidence/native-cdx-20261126/cdhost-author/AUTHOR-REPORT.md. */
#ifndef PE_PLAT_CDHOST_H
#define PE_PLAT_CDHOST_H
#include <stdint.h>

int pe_plat_cdhost_ack(void);
int pe_plat_cdhost_poll(uint32_t mode, uint32_t dst);
int pe_plat_cdhost_cmd(uint32_t cmd, uint32_t input, uint32_t dst, uint32_t mode);
int pe_plat_cdhost_sector(uint32_t dst, uint32_t count);
void pe_plat_cdhost_stream(void);
void pe_plat_cdhost_dma(uint32_t channel, uint32_t address, uint32_t blocks, uint32_t words,
                        uint32_t control, uint32_t interrupt, uint32_t unused);
int pe_plat_cdhost_readpoll(uint32_t fp);

/* Loader device window (host model, platform/pe_plat2_cdhost_loader.c): opened by the DsRead command-sequence host when no
 * device is attached, serviced from HostFB_DevicePoll, closed when the read has finished. */
int PE_CdHost_LoaderOpen(void);
void PE_CdHost_LoaderClose(void);
void PE_CdHost_LoaderService(void);
void PE_CdHost_LoaderDrain(void);
int PE_CdHost_LoaderIsOpen(void);

#define func_8007AAB4 pe_plat_cdhost_ack
#define func_8007B010 pe_plat_cdhost_poll
#define func_8007B558 pe_plat_cdhost_cmd
#define func_8007BF44 pe_plat_cdhost_sector
#define func_8007C564 pe_plat_cdhost_stream
#define func_8007CEAC pe_plat_cdhost_dma
#define func_800811E4 pe_plat_cdhost_readpoll

/* Former public names of the same entries. */
#define pe_plat_cd_acknowledge_host pe_plat_cdhost_ack
#define pe_plat_cd_status_poll_host pe_plat_cdhost_poll
#define pe_plat_cd_command_issue_host pe_plat_cdhost_cmd
#define pe_plat_cd_sector_dma_host pe_plat_cdhost_sector
#define pe_plat_cd_stream_handler_host pe_plat_cdhost_stream
#define pe_plat_cd_dma_issue_host pe_plat_cdhost_dma
#define pe_plat_cd_read_poll_host pe_plat_cdhost_readpoll
#define pe_plat_cd_read_poll pe_plat_cdhost_readpoll

/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
int func_8007AAB4(void);
int func_8007B010(uint32_t mode, uint32_t dst);
int func_8007B558(uint32_t cmd, uint32_t input, uint32_t dst, uint32_t mode);
int func_8007BF44(uint32_t dst, uint32_t count);
void func_8007C564(void);
void func_8007CEAC(uint32_t channel, uint32_t address, uint32_t blocks, uint32_t words,
                   uint32_t control, uint32_t interrupt, uint32_t unused);
int func_800811E4(uint32_t fp);
#endif
