/* Host memory-card image layer (platform/pe_memcard.c), its BIOS/libcard
 * trampolines (platform/pe_bios_card.c) and the card operation processor
 * running against it (game/boot/func_80041108_port.c). */
#include "pe_memcard.h"
#include "pe_bios_string.h"
#include <sys/stat.h>
#if defined(_WIN32)
#include <process.h>
#define MC_GETPID() _getpid()
#define MC_SETENV(k, v) _putenv_s((k), (v))
#define MC_UNSETENV(k) _putenv_s((k), "")
#else
#include <unistd.h>
#define MC_GETPID() getpid()
#define MC_SETENV(k, v) setenv((k), (v), 1)
#define MC_UNSETENV(k) unsetenv(k)
#endif

static void MC_TestTempPath(char *out, size_t cap, const char *leaf)
{
    const char *dir = getenv("TMPDIR");
    snprintf(out, cap, "%s/pe_memcard_test_%ld_%s", dir && dir[0] ? dir : ".",
             (long)MC_GETPID(), leaf);
    remove(out);
}

static int MC_FrameOk(const uint8_t *f)
{
    return PE_Memcard_FrameChecksum(f) == f[127];
}

static void test_MC_format_layout(void)
{
    static uint8_t img[PE_MC_IMAGE_SIZE];
    TEST("MC_format_layout");
    memset(img, 0x5A, sizeof img);
    PE_Memcard_FormatImage(img);
    ASSERT(img[0] == 'M' && img[1] == 'C' && img[127] == ('M' ^ 'C'), "header frame MC + checksum 0Eh");
    for (unsigned i = 2; i < 127; i++) ASSERT(img[i] == 0, "header padding zero");
    for (unsigned b = 1; b <= 15; b++) {
        const uint8_t *f = img + b * 128u;
        ASSERT(f[0] == 0xA0 && f[1] == 0 && f[2] == 0 && f[3] == 0, "free directory state A0h");
        ASSERT(f[4] == 0 && f[8] == 0xFF && f[9] == 0xFF, "free: size 0, next FFFFh");
        ASSERT(f[127] == 0xA0 && MC_FrameOk(f), "free directory checksum");
    }
    for (unsigned b = 16; b <= 35; b++) {
        const uint8_t *f = img + b * 128u;
        ASSERT(f[0] == 0xFF && f[3] == 0xFF && f[8] == 0xFF && f[9] == 0xFF && MC_FrameOk(f),
               "broken-sector list entries empty");
    }
    ASSERT(memcmp(img + 63u * 128u, img, 128) == 0, "write-test frame copies header");
    ASSERT(PE_Memcard_ImageIsFormatted(img), "formatted predicate");
    img[0] = 0;
    ASSERT(!PE_Memcard_ImageIsFormatted(img), "unformatted predicate");
    PASS();
}

static void test_MC_sprintf(void)
{
    const pe_addr_t fmt = 0x80160000u, dst = 0x80160100u, str = 0x80160200u;
    const char *f = "bu%d0:BASLUS-00662TESTSV%c%c|%ld|%s|%04x|%-3d|%3d|";
    char host[80];
    uint32_t args[8];
    TEST("MC_host_sprintf");
    ResetTestState();
    for (unsigned i = 0; i <= strlen(f); i++) PE_StoreU8(fmt + i, (uint8_t)f[i]);
    PE_StoreU8(str, 'o'); PE_StoreU8(str + 1, 'k'); PE_StoreU8(str + 2, 0);
    args[0] = 1; args[1] = '1'; args[2] = 'C'; args[3] = 7; args[4] = str; args[5] = 0xBEu;
    args[6] = (uint32_t)-5; args[7] = 42;
    ASSERT(PE_Bios_Sprintf(dst, fmt, args, 8) == 44, "formatted length");
    for (unsigned i = 0; i < sizeof host; i++) host[i] = (char)PE_LoadU8(dst + i);
    ASSERT(strcmp(host, "bu10:BASLUS-00662TESTSV1C|7|ok|00be|-5 | 42|") == 0, "sprintf text");
    ASSERT(PE_Bios_SprintfHost(host, 6, fmt, args, 8) == 44 && strcmp(host, "bu10:") == 0,
           "host variant truncates with NUL");
    PASS();
}

static void test_MC_file_roundtrip(void)
{
    static uint8_t data[0x2000], back[0x2000];
    PeMcDirEntry d;
    const uint8_t *img;
    int fd;
    TEST("MC_file_roundtrip");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    for (unsigned i = 0; i < sizeof data; i++) data[i] = (uint8_t)(i * 7u + 3u);
    ASSERT(PE_Memcard_Present(0) && !PE_Memcard_Present(1), "port 0 in-memory card, port 1 empty");
    ASSERT(PE_Memcard_IsFormatted(0), "in-memory card starts formatted");
    ASSERT(PE_Memcard_Open("bu00:BASLUS-00662SAVE00A", 1) == -1, "open missing file fails");
    fd = PE_Memcard_Open("bu00:BASLUS-00662SAVE00A", 0x10200u);
    ASSERT(fd >= 0, "create one block");
    ASSERT(PE_Memcard_Open("bu00:BASLUS-00662SAVE00A", 0x10200u) == -1, "create existing fails");
    ASSERT(PE_Memcard_Write(fd, data, 1024) == -1, "create-only handle is not writable");
    ASSERT(PE_Memcard_Close(fd) == fd && PE_Memcard_Close(fd) == -1, "close once");
    img = PE_Memcard_Image(0);
    ASSERT(img[128] == 0x51 && img[128 + 4] == 0x00 && img[128 + 5] == 0x20 &&
           img[128 + 8] == 0xFF && img[128 + 9] == 0xFF, "dir frame 1: first block, 2000h bytes, no next");
    ASSERT(memcmp(img + 128 + 10, "BASLUS-00662SAVE00A", 20) == 0 && img[128 + 30] == 0,
           "dir frame filename");
    ASSERT(MC_FrameOk(img + 128), "dir frame checksum");
    fd = PE_Memcard_Open("bu00:BASLUS-00662SAVE00A", 2);
    ASSERT(fd >= 0, "open for write");
    ASSERT(PE_Memcard_Write(fd, data, 100) == -1, "write length must be sector multiple");
    for (unsigned o = 0; o < 0x2000u; o += 1024u)
        ASSERT(PE_Memcard_Write(fd, data + o, 1024) == 1024, "1024-byte write");
    ASSERT(PE_Memcard_Write(fd, data, 128) == 0, "write past end transfers nothing");
    ASSERT(PE_Memcard_Close(fd) == fd, "close writer");
    ASSERT(memcmp(img + 0x2000, data, 0x2000) == 0, "block 1 holds file data");
    fd = PE_Memcard_Open("bu00:BASLUS-00662SAVE00A", 1);
    ASSERT(fd >= 0 && PE_Memcard_Lseek(fd, 0x100, 0) == 0x100, "seek 100h");
    ASSERT(PE_Memcard_Read(fd, back, 128) == 128 && memcmp(back, data + 0x100, 128) == 0, "header read after seek");
    ASSERT(PE_Memcard_Lseek(fd, 0, 0) == 0, "rewind");
    for (unsigned o = 0; o < 0x2000u; o += 1024u)
        ASSERT(PE_Memcard_Read(fd, back + o, 1024) == 1024, "1024-byte read");
    ASSERT(memcmp(back, data, sizeof data) == 0, "read back equals write");
    ASSERT(PE_Memcard_Read(fd, back, 128) == 0, "read at end returns 0");
    PE_Memcard_Close(fd);
    /* multi-block chain */
    fd = PE_Memcard_Open("bu00:MULTI", 0x30200u | 2u);
    ASSERT(fd >= 0, "create 3-block file");
    ASSERT(img[256] == 0x51 && img[256 + 8] == 2 && img[384] == 0x52 && img[384 + 8] == 3 &&
           img[512] == 0x53 && img[512 + 8] == 0xFF && MC_FrameOk(img + 384) && MC_FrameOk(img + 512),
           "chain blocks 2,3,4: first/middle/last with next links");
    PE_Memcard_Close(fd);
    ASSERT(PE_Memcard_FirstFile("bu00:*", &d) && d.head == 1 && d.size == 0x2000 &&
           memcmp(d.name, "BASLUS-00662SAVE00A", 20) == 0, "firstfile *");
    ASSERT(PE_Memcard_NextFile(&d) && d.head == 2 && d.size == 0x6000 && strcmp(d.name, "MULTI") == 0,
           "nextfile finds the chain head only");
    ASSERT(!PE_Memcard_NextFile(&d), "listing ends");
    ASSERT(PE_Memcard_FirstFile("bu00:BASLUS-00662??????A", &d) && d.head == 1 && !PE_Memcard_NextFile(&d),
           "? wildcard");
    ASSERT(!PE_Memcard_FirstFile("bu10:*", &d), "empty port lists nothing");
    ASSERT(PE_Memcard_Delete("bu00:MULTI") == 1 && PE_Memcard_Delete("bu00:MULTI") == 0, "delete once");
    ASSERT(img[256] == 0xA1 && img[384] == 0xA2 && img[512] == 0xA3 && MC_FrameOk(img + 256),
           "deleted chain marked A1/A2/A3 with checksums");
    ASSERT(PE_Memcard_FirstFile("bu00:*", &d) && !PE_Memcard_NextFile(&d), "deleted file not listed");
    fd = PE_Memcard_Open("bu00:REUSE", 0x10200u);
    ASSERT(fd >= 0 && img[256] == 0x51, "deleted blocks reusable");
    PE_Memcard_Close(fd);
    ASSERT(PE_Memcard_Format("bu00:") == 1 && !PE_Memcard_FirstFile("bu00:*", &d) && img[128] == 0xA0,
           "format empties the directory");
    PASS();
}

static void test_MC_persistence(void)
{
    char path[512];
    static uint8_t data[0x2000];
    struct stat st;
    PeMcDirEntry d;
    FILE *f;
    int fd;
    TEST("MC_persistence");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    MC_TestTempPath(path, sizeof path, "card.mcd");
    for (unsigned i = 0; i < sizeof data; i++) data[i] = (uint8_t)(i ^ 0x5Au);
    PE_Memcard_SetPath(0, path);
    ASSERT(PE_Memcard_IsFormatted(0), "missing image created formatted");
    ASSERT(stat(path, &st) == 0 && st.st_size == (off_t)PE_MC_IMAGE_SIZE, "128 KiB file written");
    fd = PE_Memcard_Open("bu00:BASLUS-00662SAVE00B", 0x10200u);
    PE_Memcard_Close(fd);
    fd = PE_Memcard_Open("bu00:BASLUS-00662SAVE00B", 2);
    for (unsigned o = 0; o < 0x2000u; o += 1024u) (void)PE_Memcard_Write(fd, data + o, 1024);
    PE_Memcard_Close(fd);
    /* "Quit and relaunch": drop all state, reload from the file. */
    PE_Memcard_Reset();
    PE_Memcard_SetPath(0, path);
    ASSERT(PE_Memcard_FirstFile("bu00:*", &d) && memcmp(d.name, "BASLUS-00662SAVE00B", 20) == 0,
           "file listed after reload");
    {
        static uint8_t back[0x2000];
        fd = PE_Memcard_Open("bu00:BASLUS-00662SAVE00B", 1);
        for (unsigned o = 0; o < 0x2000u; o += 1024u) (void)PE_Memcard_Read(fd, back + o, 1024);
        PE_Memcard_Close(fd);
        ASSERT(memcmp(back, data, sizeof data) == 0, "data persisted across reload");
    }
    /* A non-card file is an unformatted card and is not overwritten. */
    remove(path);
    f = fopen(path, "wb"); fputs("not a card", f); fclose(f);
    PE_Memcard_Reset();
    PE_Memcard_SetPath(0, path);
    ASSERT(PE_Memcard_Present(0) && !PE_Memcard_IsFormatted(0), "garbage file = unformatted card");
    ASSERT(PE_Memcard_Open("bu00:X", 0x10200u) == -1, "no file ops on unformatted card");
    ASSERT(stat(path, &st) == 0 && st.st_size == 10, "unformatted image untouched");
    ASSERT(PE_Memcard_Format("bu00:") == 1 && stat(path, &st) == 0 &&
           st.st_size == (off_t)PE_MC_IMAGE_SIZE, "format rewrites the file");
    PE_Memcard_Reset();
    PE_Memcard_SetPath(0, path);
    ASSERT(PE_Memcard_IsFormatted(0), "formatted after reload");
    remove(path);
    /* PE_MEMCARD1 override and default resolution */
    PE_Memcard_Reset();
    MC_SETENV("PE_MEMCARD1", path);
    ASSERT(PE_Memcard_Path(0) && strcmp(PE_Memcard_Path(0), path) == 0, "PE_MEMCARD1 overrides");
    MC_UNSETENV("PE_MEMCARD1");
    PE_Memcard_Reset();
    {
        const char *p = PE_Memcard_Path(0);
        const char *tail = "parasite-eve-port/memcard1.mcd";
        ASSERT(p && strlen(p) > strlen(tail) && strcmp(p + strlen(p) - strlen(tail), tail) == 0,
               "default path under the data dir");
        ASSERT(!PE_Memcard_Present(1), "slot 2 empty by default");
    }
    Test_ResetMemcard();
    PASS();
}

static void test_MC_bios_trampolines(void)
{
    const pe_addr_t name = 0x80160000u, buf = 0x80161000u, dir = 0x80163000u, pat = 0x80160100u;
    const char *n = "bu00:BASLUS-00662SAVE001C", *p = "bu00:*";
    int fd;
    TEST("MC_bios_trampolines");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    for (unsigned i = 0; i <= strlen(n); i++) PE_StoreU8(name + i, (uint8_t)n[i]);
    for (unsigned i = 0; i <= strlen(p); i++) PE_StoreU8(pat + i, (uint8_t)p[i]);
    for (unsigned i = 0; i < 0x2000u; i++) PE_StoreU8(buf + i, (uint8_t)(i * 3u));
    fd = func_80072734(name, 0x10200u);
    ASSERT(fd >= 0 && func_80072774(fd) == fd, "B(32h) create / B(36h) close");
    fd = func_80072734(name, 2u);
    ASSERT(func_80072764(fd, buf, 0x400) == 0x400, "B(35h) write from guest");
    func_80072774(fd);
    fd = func_80072734(name, 1u);
    ASSERT(func_80072744(fd, 0x100, 0) == 0x100, "B(33h) lseek");
    ASSERT(func_80072754(fd, buf + 0x1000u, 0x80) == 0x80, "B(34h) read to guest");
    for (unsigned i = 0; i < 0x80u; i++)
        ASSERT(PE_LoadU8(buf + 0x1000u + i) == (uint8_t)((i + 0x100u) * 3u), "read bytes");
    func_80072774(fd);
    ASSERT(PE_Bios_FirstFile(pat, dir) == dir, "B(42h) firstfile returns direntry");
    ASSERT(PE_LoadU8(dir + 18) == '1' && PE_LoadU8(dir + 19) == 'C' && PE_LoadU32(dir + 0x18u) == 0x2000u &&
           PE_LoadU32(dir + 0x20u) == 1u, "DIRENTRY name[18..19], size @18h, head @20h");
    ASSERT(func_80072794(dir) == 0, "B(43h) nextfile end");
    ASSERT(func_800727A4(name) == 1 && PE_Bios_FirstFile(pat, dir) == 0, "B(45h) delete");
    for (unsigned i = 0; i < 6; i++) PE_StoreU8(pat + i, (uint8_t)"bu00:"[i]);
    ASSERT(func_80072784(pat) == 1, "B(41h) format");
    PASS();
}

/* libcard status machine (func_800405A4) against the host card events. */
static int MC_RunStatus(unsigned frames)
{
    for (unsigned i = 0; i < frames; i++) {
        func_800405A4(0);
        if (PE_Port_ShouldStop()) return 0;
        if (PE_LoadU8(0x800A0EDCu) == 4u) return 1;
    }
    return 0;
}

static void test_MC_card_status_events(void)
{
    char path[512];
    FILE *f;
    TEST("MC_card_status_events");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    func_800409B4();                                  /* InitCARD: open + enable events */
    ASSERT(MC_RunStatus(16), "formatted card reaches status 4");
    ASSERT(!(PE_LoadU8(0x800A0ED4u) & 4u), "no unformatted flag");
    ASSERT(!PE_Port_ShouldStop(), "status machine runs without boundaries");
    /* poll again: known card stays ready */
    PE_StoreU32(0x800A1840u, 0u); PE_StoreU32(0x800A183Cu, 0u);
    func_800405A4(0); func_800405A4(0);
    ASSERT(PE_LoadU8(0x800A0EDCu) == 4u && (PE_LoadU8(0x800A0ED4u) & 1u), "re-poll keeps card ready");
    /* unformatted card */
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    MC_TestTempPath(path, sizeof path, "raw.mcd");
    f = fopen(path, "wb"); fputs("x", f); fclose(f);
    PE_Memcard_SetPath(0, path);
    func_800409B4();
    ASSERT(MC_RunStatus(16) && (PE_LoadU8(0x800A0ED4u) & 4u), "unformatted card flagged (bit 2)");
    remove(path);
    /* no card */
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    PE_Memcard_SetPath(0, NULL);
    func_800409B4();
    ASSERT(!MC_RunStatus(16) && PE_LoadU8(0x800A0EDCu) <= 1u && !(PE_LoadU8(0x800A0ED4u) & 1u),
           "empty slot never ready");
    Test_ResetMemcard();
    PASS();
}

/* Directory scan (state 2) then save (states 4/6/9) through the operation
 * processor, with the matched-C record layout. */
static void test_MC_operation_scan_and_save(void)
{
    const pe_addr_t c = 0x800A0ED4u;
    const char *fmt = "bu%d0:BASLUS-00662TESTSV%c%c", *pat = "bu 0:*";
    const pe_addr_t fmt_at = 0x80170000u, pat_at = 0x80170040u;
    PeMcDirEntry d;
    int guard;
    TEST("MC_operation_scan_and_save");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    for (unsigned i = 0; i <= strlen(fmt); i++) PE_StoreU8(fmt_at + i, (uint8_t)fmt[i]);
    for (unsigned i = 0; i <= strlen(pat); i++) PE_StoreU8(pat_at + i, (uint8_t)pat[i]);
    PE_StoreU32(0x80092224u, fmt_at);
    PE_StoreU32(0x80092230u, pat_at);
    /* pre-existing save in entry C, variant '1' */
    {
        int fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV1C", 0x10200u);
        PE_Memcard_Close(fd);
    }
    PE_StoreU32(0x800A1838u, 1u);
    PE_StoreU32(0x800A186Cu, 0u);            /* list only: scan then stop (state 15) */
    PE_StoreU8(c, 1u); PE_StoreU8(c + 1u, 2u); PE_StoreU32(c + 0xCu, 0xFFFFFFFFu);
    func_80041108(0);
    ASSERT(!PE_Port_ShouldStop(), "scan runs without boundaries");
    ASSERT(PE_LoadU8(0x80170042u) == '0', "device digit patched into pattern");
    ASSERT(PE_LoadU8(c + 1u) == 15u && PE_LoadU32(0x800A1838u) == 0u, "list-only scan ends in state 15");
    ASSERT(PE_LoadU8(c + 0x1Cu + 2u * 0x44u) == 1u && PE_LoadU8(c + 0x1Cu + 2u * 0x44u + 0x29u) == 1u,
           "entry C marked used, variant 1");
    ASSERT(PE_LoadU8(c + 4u) == 1u && PE_LoadU8(c + 0xAu) == 1u, "one save seen, one block counted");
    ASSERT(PE_LoadU8(c + 0x1Cu) == 3u || PE_LoadU8(c + 0x1Cu) == 2u, "other entries free/unused");
    /* save into fresh entry A: 42020's record arming, without its UI calls */
    for (unsigned i = 0; i < 0x2000u; i++) PE_StoreU8(0x8009EED0u + i, (uint8_t)(i * 5u + 1u));
    PE_StoreU8(c + 3u, 0u); PE_StoreU8(c + 1u, 4u); PE_StoreU16(c + 0x14u, 0x2000u);
    PE_StoreU16(c + 0x16u, 10u); PE_StoreU32(0x800A1704u, 0u);
    for (guard = 0; guard < 32 && PE_LoadU8(c + 1u) != 3u; guard++) {
        func_80041108(0);
        if (PE_Port_ShouldStop()) break;
    }
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(c + 1u) == 3u, "create/open/write reached state 3");
    ASSERT((int32_t)PE_LoadU32(c + 0xCu) == -1 && PE_LoadU16(c + 0x14u) == 0u, "handle closed, all written");
    ASSERT(PE_Memcard_FirstFile("bu00:BASLUS-00662TESTSV0A", &d) && d.size == 0x2000u, "save file on card");
    {
        const uint8_t *img = PE_Memcard_Image(0);
        int ok = 1;
        for (unsigned i = 0; i < 0x2000u; i++)
            if (img[(size_t)d.head * 0x2000u + i] != (uint8_t)(i * 5u + 1u)) ok = 0;
        ASSERT(ok, "save bytes on card equal D_8009EED0");
    }
    Test_ResetMemcard();
    PASS();
}

/* Overwrite an existing save (owner report on r4b: "it closes if i overwrite a
 * save already saved").  Retail chain, all matched C except the card driver:
 *   func_8004D6D4 (save over a used slot) arms the 0x29 confirm window with
 *   update func_80044E98 and D_8009CFA8 = func_80050544;
 *   func_80044E98 (confirm, row 0) calls D_8009CFA8(window, 1);
 *   func_80050544 -> func_80042B50(func_800504F4): D_800A1870/1874 = cb, 1;
 *   func_80042B6C runs cb on its 3rd tick after that -> func_800504F4 ->
 *   func_80042020(D_8009CF44, D_8009CF48): used entry -> next state 0xB;
 *   func_80041108 state 1 -> 11: open old name, close, B(45h) delete it,
 *   -> 4: B(32h) create the new variant (D_800A1704), write 0x2000 bytes.
 * The r4b hand func_80044E98 had a fixed callback list without 0x80050544
 * and stopped the port (unresolved boundary) at the confirm. */
static void test_MC_overwrite_existing_save(void)
{
    const pe_addr_t c = 0x800A0ED4u, win = 0x80171000u, list = 0x80171100u;
    const char *fmt = "bu%d0:BASLUS-00662TESTSV%c%c";
    const pe_addr_t fmt_at = 0x80170000u;
    char path[512];
    static uint8_t back[0x2000];
    PeMcDirEntry d;
    int guard, fd, n;
    TEST("MC_overwrite_existing_save");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    MC_TestTempPath(path, sizeof path, "overwrite.mcd");
    PE_Memcard_SetPath(0, path);
    for (unsigned i = 0; i <= strlen(fmt); i++) PE_StoreU8(fmt_at + i, (uint8_t)fmt[i]);
    PE_StoreU32(0x80092224u, fmt_at);
    /* first save: entry A, variant '0', old bytes */
    fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV0A", 0x10200u);
    PE_Memcard_Close(fd);
    fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV0A", 2);
    for (unsigned i = 0; i < 0x2000u; i++) back[i] = (uint8_t)(i * 3u + 7u);
    ASSERT(PE_Memcard_Write(fd, back, 0x2000) == 0x2000, "first save written");
    PE_Memcard_Close(fd);
    /* card record: port 0 inserted, ready (f8 == 4), entry A used, variant 0 */
    PE_StoreU8(c, 1u); PE_StoreU8(c + 1u, 0u); PE_StoreU8(c + 8u, 4u);
    PE_StoreU32(c + 0xCu, 0xFFFFFFFFu);
    PE_StoreU8(c + 0x1Cu, 1u); PE_StoreU8(c + 0x1Cu + 0x29u, 0u);
    PE_StoreU32(0x8009CF44u, 0u); PE_StoreU32(0x8009CF48u, 0u);
    PE_StoreU32(0x800A1704u, 1u);                /* next variant digit */
    /* the confirm window: row 0 (list+0x44 base 0, +0x48 factor 0) = "Yes" */
    for (pe_addr_t a = win; a < list + 0x80u; a += 4u) PE_StoreU32(a, 0u);
    PE_StoreU32(win + 8u, list); PE_StoreU32(win + 0x24u, 0x29u);
    PE_StoreU32(list + 0x34u, 1u);
    PE_StoreU32(0x8009CFA8u, 0x80050544u);
    PE_StoreU32(0x800A1870u, 0u); PE_StoreU32(0x800A1874u, 0u);
    ASSERT(func_80044E98(win, 0x10000u) == 1, "confirm handled");
    ASSERT(!PE_Port_ShouldStop(), "confirming the overwrite does not stop the port");
    ASSERT(PE_LoadU32(0x800A1870u) == 0x800504F4u && PE_LoadU32(0x800A1874u) == 1u,
           "func_80050544 queued func_800504F4");
    for (n = 0; n < 3; n++) func_80042B6C();
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU32(0x800A1870u) == 0u, "queued callback ran on the 4th count");
    ASSERT(PE_LoadU8(c + 1u) == 1u && PE_LoadU8(c + 0xBu) == 0xBu, "func_80042020 armed the overwrite (state 1 -> 0xB)");
    for (guard = 0; guard < 64 && PE_LoadU8(c + 1u) != 3u; guard++) {
        func_80041108(0);
        if (PE_Port_ShouldStop()) break;
    }
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(c + 1u) == 3u, "delete/create/write reached state 3");
    ASSERT(!PE_Memcard_FirstFile("bu00:BASLUS-00662TESTSV0A", &d), "old variant deleted");
    ASSERT(PE_Memcard_FirstFile("bu00:BASLUS-00662TESTSV1A", &d) && d.size == 0x2000u, "new variant on card");
    ASSERT(!PE_Memcard_NextFile(&d), "exactly one save file");
    /* func_80042020 -> func_80040B80 builds the save image into D_8009EED0
     * (header + CRC); the card must hold exactly that image, not the old one. */
    {
        static uint8_t want[0x2000];
        int differs = 0;
        for (unsigned i = 0; i < 0x2000u; i++) {
            want[i] = PE_LoadU8(0x8009EED0u + i);
            if (want[i] != (uint8_t)(i * 3u + 7u)) differs = 1;
        }
        ASSERT(differs, "second save image differs from the first save");
        PE_Memcard_Reset();
        PE_Memcard_SetPath(0, path);
        fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV1A", 1);
        ASSERT(fd >= 0 && PE_Memcard_Read(fd, back, 0x2000) == 0x2000, "reload reads the overwritten save");
        PE_Memcard_Close(fd);
        ASSERT(memcmp(back, want, sizeof want) == 0, "reloaded bytes are the second save's image (D_8009EED0)");
    }
    ASSERT(!PE_Memcard_FirstFile("bu00:BASLUS-00662TESTSV0A", &d), "old variant still gone after reload");
    remove(path);
    Test_ResetMemcard();
    PASS();
}

/* The same confirm answered "No" (row 1): the port keeps running, nothing is
 * queued or deleted, and the first save is byte-identical after a reload. */
static void test_MC_overwrite_declined(void)
{
    const pe_addr_t c = 0x800A0ED4u, win = 0x80171000u, list = 0x80171100u;
    char path[512];
    static uint8_t back[0x2000];
    PeMcDirEntry d;
    int fd, n;
    TEST("MC_overwrite_declined");
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    MC_TestTempPath(path, sizeof path, "declined.mcd");
    PE_Memcard_SetPath(0, path);
    fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV0A", 0x10200u);
    PE_Memcard_Close(fd);
    fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV0A", 2);
    for (unsigned i = 0; i < 0x2000u; i++) back[i] = (uint8_t)(i * 3u + 7u);
    ASSERT(PE_Memcard_Write(fd, back, 0x2000) == 0x2000, "first save written");
    PE_Memcard_Close(fd);
    PE_StoreU8(c, 1u); PE_StoreU8(c + 1u, 0u); PE_StoreU8(c + 8u, 4u);
    PE_StoreU8(c + 0x1Cu, 1u);
    PE_StoreU32(0x8009CF44u, 0u); PE_StoreU32(0x8009CF48u, 0u);
    for (pe_addr_t a = win; a < list + 0x80u; a += 4u) PE_StoreU32(a, 0u);
    PE_StoreU32(win + 8u, list); PE_StoreU32(win + 0x24u, 0x29u);
    PE_StoreU32(list + 0x34u, 1u); PE_StoreU32(list + 0x48u, 1u);   /* row 1 = "No" */
    PE_StoreU32(0x8009CFA8u, 0x80050544u);
    PE_StoreU32(0x800A1870u, 0u); PE_StoreU32(0x800A1874u, 0u);
    ASSERT(func_80044E98(win, 0x10000u) == 1, "confirm handled");
    ASSERT(!PE_Port_ShouldStop(), "declining the overwrite does not stop the port");
    ASSERT(PE_LoadU32(0x800A1870u) == 0u, "nothing queued (func_80050544 enabled = 0)");
    for (n = 0; n < 4; n++) func_80042B6C();
    for (n = 0; n < 8; n++) func_80041108(0);
    ASSERT(!PE_Port_ShouldStop() && PE_LoadU8(c + 1u) == 0u, "card driver stays idle");
    PE_Memcard_Reset();
    PE_Memcard_SetPath(0, path);
    ASSERT(PE_Memcard_FirstFile("bu00:*", &d) && memcmp(d.name, "BASLUS-00662TESTSV0A", 20) == 0 &&
           !PE_Memcard_NextFile(&d), "only the first save, not deleted");
    fd = PE_Memcard_Open("bu00:BASLUS-00662TESTSV0A", 1);
    ASSERT(fd >= 0 && PE_Memcard_Read(fd, back, 0x2000) == 0x2000, "first save readable");
    PE_Memcard_Close(fd);
    {
        int ok = 1;
        for (unsigned i = 0; i < 0x2000u; i++) if (back[i] != (uint8_t)(i * 3u + 7u)) ok = 0;
        ASSERT(ok, "first save bytes unchanged after reload");
    }
    remove(path);
    Test_ResetMemcard();
    PASS();
}

static void test_MC_all(void)
{
    test_MC_format_layout();
    test_MC_sprintf();
    test_MC_file_roundtrip();
    test_MC_persistence();
    test_MC_bios_trampolines();
    test_MC_card_status_events();
    test_MC_operation_scan_and_save();
    test_MC_overwrite_existing_save();
    test_MC_overwrite_declined();
}
