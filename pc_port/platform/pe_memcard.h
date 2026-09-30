/*
 * Host memory-card backend: a standard 128 KiB raw PS1 memory-card image
 * (".mcd", ".mcr": 16 blocks x 8 KiB = 1024 frames x 128 bytes) plus the
 * BIOS file-function semantics the game reaches through its B(32h..45h)
 * trampolines, and the libcard _card_info/_card_load/_new_card event
 * results.
 *
 * Clean-room host code written from the public PS1 documentation
 * (psx-spx "Memory Card Data Format", "BIOS File Functions", "BIOS Memory
 * Card Functions").  No Sony BIOS/libcard code or tables are reproduced.
 *
 * Image layout (psx-spx):
 *   frame 0        header: "MC", zero padding, XOR checksum at byte 7Fh
 *   frames 1..15   directory, one per block 1..15:
 *                    +00 u32 state (A0 free, 51 first, 52 middle, 53 last,
 *                        A1/A2/A3 deleted first/middle/last)
 *                    +04 u32 file size in bytes (first block only)
 *                    +08 u16 next block index 0..14, FFFFh = none
 *                    +0A filename, ASCII, NUL terminated (max 20 chars)
 *                    +7F XOR checksum of bytes 00..7E
 *   frames 16..35  broken-sector list (FFFFFFFFh = none), checksummed
 *   frames 36..55  broken-sector replacement data
 *   frame 63       write-test frame (copy of frame 0)
 *   blocks 1..15   file data, 8 KiB each, at offset block*2000h
 *
 * Card ports: port 0 ("bu00:") and port 1 ("bu10:").  Port 0 defaults to
 * ~/.local/share/parasite-eve-port/memcard1.mcd (PE_MEMCARD1 overrides);
 * port 1 is present only when PE_MEMCARD2 names an image.  A missing image
 * file is created freshly formatted; an image file that exists but is not a
 * formatted 128 KiB card is presented to the game as an unformatted card
 * and is only rewritten when the game formats it.  Every mutation is
 * written through to the file (temp file + rename).
 */
#ifndef PE_MEMCARD_H
#define PE_MEMCARD_H

#include <stddef.h>
#include <stdint.h>

#define PE_MC_IMAGE_SIZE   0x20000u
#define PE_MC_FRAME_SIZE   0x80u
#define PE_MC_BLOCK_SIZE   0x2000u
#define PE_MC_BLOCKS       16u
#define PE_MC_PORTS        2

/* Host copy of a BIOS DIRENTRY (guest layout: name[20] @0, attr @14h,
 * size @18h, next @1Ch, head @20h, system[4] @24h; 28h bytes). */
typedef struct {
    char     name[20];
    uint32_t attr;
    uint32_t size;
    uint32_t next;
    uint32_t head;
    uint8_t  system[4];
} PeMcDirEntry;

/* Configuration.  path == NULL: no card in that port.  path == "" : card
 * present, in-memory only (never written to disk; used by tests).  Any
 * other path: image file.  Calling this replaces the environment/default
 * resolution for that port and drops any cached image / open handles. */
void        PE_Memcard_SetPath(int port, const char *path);
/* Back to environment/default resolution; closes handles, drops caches. */
void        PE_Memcard_Reset(void);
/* Resolved image path for the port, or NULL when no card / in-memory. */
const char *PE_Memcard_Path(int port);

int  PE_Memcard_Present(int port);
int  PE_Memcard_IsFormatted(int port);
/* Raw 128 KiB image (loads it on demand); NULL when no card. */
const uint8_t *PE_Memcard_Image(int port);
/* Last host I/O error text for diagnostics ("" when none). */
const char *PE_Memcard_LastError(void);

/* Pure image helpers (exposed for tests / tools). */
void    PE_Memcard_FormatImage(uint8_t image[PE_MC_IMAGE_SIZE]);
uint8_t PE_Memcard_FrameChecksum(const uint8_t *frame);
int     PE_Memcard_ImageIsFormatted(const uint8_t image[PE_MC_IMAGE_SIZE]);

/* BIOS file functions (host strings/buffers; results as the BIOS returns
 * them).  Filenames are "buXY:NAME" with X the port digit. */
int PE_Memcard_Open(const char *name, uint32_t mode);          /* B(32h) */
int PE_Memcard_Lseek(int fd, int32_t offset, int whence);      /* B(33h) */
int PE_Memcard_Read(int fd, void *dst, int32_t length);        /* B(34h) */
int PE_Memcard_Write(int fd, const void *src, int32_t length); /* B(35h) */
int PE_Memcard_Close(int fd);                                  /* B(36h) */
int PE_Memcard_Format(const char *device);                     /* B(41h) */
int PE_Memcard_FirstFile(const char *pattern, PeMcDirEntry *out); /* B(42h) */
int PE_Memcard_NextFile(PeMcDirEntry *out);                    /* B(43h) */
int PE_Memcard_Delete(const char *name);                       /* B(45h) */

/* libcard-level card status for a chan (port<<4). */
typedef enum {
    PE_MC_EVENT_IOE = 0x0004,     /* ready / done */
    PE_MC_EVENT_TIMEOUT = 0x0100, /* no card */
    PE_MC_EVENT_NEWCARD = 0x2000  /* new card (info) / unformatted (load) */
} PeMcEvent;
int  PE_Memcard_CardInfoResult(uint32_t chan);   /* _card_info  */
int  PE_Memcard_CardLoadResult(uint32_t chan);   /* _card_load  */
int  PE_Memcard_CardClearResult(uint32_t chan);  /* _new_card + dummy write */

/* Test support: when enabled, the guest-facing BIOS trampolines record a
 * loud boundary and stop instead of performing card I/O (preserves the
 * first-unresolved-callee oracle fixtures).  Default: disabled. */
void PE_Memcard_SetBoundaryMode(int enabled);
int  PE_Memcard_BoundaryMode(void);

#endif
