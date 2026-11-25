/*
 * Host memory-card BIOS entry points: the BIOS file-function wrappers
 * (B(32h) open, B(33h) lseek, B(34h) read, B(35h) write, B(36h) close,
 * B(41h) format, B(42h)/B(43h) firstfile/nextfile, B(45h) erase) and the
 * delivery of memory-card events to the game's handlers, backed by the .mcd
 * image layer (pe_memcard.c).
 *
 * Provenance: written clean-room from the psx-spx kernel/BIOS documentation
 * ("BIOS File Functions", "BIOS Memory Card Functions", "BIOS Event Summary",
 * https://psx-spx.consoledev.net/kernelbios/) and the port's host
 * memory-card model, with no SDK source, tables or disassembly.  The guest
 * entry names are bound in pe_plat/bios_file.h.  The card events are
 * delivered synchronously as the classes/specs the game registers at init
 * (software F4000001h, hardware F0000011h; 0004h ok, 0100h no card, 2000h
 * new/unformatted); psx-spx does not say which call raises which, so that
 * mapping is the host model's (see docs/evidence/native-card-bios-20261125/).
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_memcard.h"
#include "pe_guest_ram.h"

#include <string.h>

extern int func_80042BD8(void);
extern int func_80042BEC(void);
extern int func_80042C00(void);
extern int func_80042C14(void);
extern int func_80042C28(void);
extern int func_80042C3C(void);
extern int func_80042C50(void);
extern int func_80042C64(void);

void PE_BiosCard_Deliver(uint32_t cls, uint32_t spec)
{
    switch (PE_Event_DeliverCard(cls, spec)) {
    case 0x80042BD8u: (void)func_80042BD8(); break;
    case 0x80042BECu: (void)func_80042BEC(); break;
    case 0x80042C00u: (void)func_80042C00(); break;
    case 0x80042C14u: (void)func_80042C14(); break;
    case 0x80042C28u: (void)func_80042C28(); break;
    case 0x80042C3Cu: (void)func_80042C3C(); break;
    case 0x80042C50u: (void)func_80042C50(); break;
    case 0x80042C64u: (void)func_80042C64(); break;
    default: break;
    }
}

/* Guest NUL-terminated string -> host buffer (truncating). */
static void guest_string(pe_addr_t address, char *out, size_t cap)
{
    size_t i = 0;
    if (!address) { out[0] = 0; return; }
    for (; i + 1 < cap; i++) {
        uint8_t c = PE_LoadU8(address + (uint32_t)i);
        if (!c) break;
        out[i] = (char)c;
    }
    out[i] = 0;
}

static void store_direntry(pe_addr_t dir, const PeMcDirEntry *e)
{
    for (uint32_t i = 0; i < 20u; i++) PE_StoreU8(dir + i, (uint8_t)e->name[i]);
    PE_StoreU32(dir + 0x14u, e->attr);
    PE_StoreU32(dir + 0x18u, e->size);
    PE_StoreU32(dir + 0x1Cu, e->next);
    PE_StoreU32(dir + 0x20u, e->head);
    for (uint32_t i = 0; i < 4u; i++) PE_StoreU8(dir + 0x24u + i, e->system[i]);
}

/* ── libcard / kernel card service ───────────────────────────────────
 * _card_info/_card_load/_card_write/_new_card now live in
 * pe_plat_bios_card.c (bound through pe_plat/bios_card.h); this file keeps
 * PE_BiosCard_Deliver, which they call. */

/* ── BIOS file functions ───────────────────────────────────────────── */

/* B(32h) open */
int pe_plat_bios_file_open(pe_addr_t name, uint32_t mode)
{
    char s[64];
    guest_string(name, s, sizeof s);
    return PE_Memcard_Open(s, mode);
}

/* B(33h) lseek */
int pe_plat_bios_file_lseek(int fd, int32_t offset, int whence)
{
    return PE_Memcard_Lseek(fd, offset, whence);
}

/* B(34h) read */
int pe_plat_bios_file_read(int fd, pe_addr_t dst, int32_t length)
{
    uint8_t buf[PE_MC_BLOCK_SIZE];
    int n;
    if (length < 0 || length > (int32_t)sizeof buf) return -1;
    n = PE_Memcard_Read(fd, buf, length);
    for (int i = 0; i < n; i++) PE_StoreU8(dst + (uint32_t)i, buf[i]);
    return n;
}

/* B(35h) write */
int pe_plat_bios_file_write(int fd, pe_addr_t src, int32_t length)
{
    uint8_t buf[PE_MC_BLOCK_SIZE];
    if (length < 0 || length > (int32_t)sizeof buf) return -1;
    for (int32_t i = 0; i < length; i++) buf[i] = PE_LoadU8(src + (uint32_t)i);
    return PE_Memcard_Write(fd, buf, length);
}

/* B(36h) close */
int pe_plat_bios_file_close(int fd)
{
    return PE_Memcard_Close(fd);
}

/* B(41h) format */
int pe_plat_bios_file_format(pe_addr_t device)
{
    char s[64];
    guest_string(device, s, sizeof s);
    return PE_Memcard_Format(s);
}

/* B(42h) firstfile(name, direntry): the result the libapi wrapper at
 * 0x800727B4 returns (the direntry address, or 0). */
pe_addr_t PE_Bios_FirstFile(pe_addr_t pattern, pe_addr_t dir)
{
    char s[64];
    PeMcDirEntry e;
    guest_string(pattern, s, sizeof s);
    if (!PE_Memcard_FirstFile(s, &e)) return 0;
    store_direntry(dir, &e);
    return dir;
}

/* B(43h) nextfile */
pe_addr_t pe_plat_bios_file_nextfile(pe_addr_t dir)
{
    PeMcDirEntry e;
    if (!PE_Memcard_NextFile(&e)) return 0;
    store_direntry(dir, &e);
    return dir;
}

/* B(45h) delete */
int pe_plat_bios_file_delete(pe_addr_t name)
{
    char s[64];
    guest_string(name, s, sizeof s);
    return PE_Memcard_Delete(s);
}
