/* Card BIOS thunks (platform/pe_plat_bios_card.c) against the host
 * memory-card model.  Observation points are the eight libcard handler
 * flag words the retail InitCARD registers (func_800409B4):
 *   F4000001h spec 0004h -> D_800A1820   F0000011h spec 0004h -> D_800A182C
 *   F4000001h spec 8000h/0100h -> A1824  F0000011h spec 8000h/0100h -> A1830
 *   F4000001h spec 2000h -> D_800A1828   F0000011h spec 2000h -> D_800A1834
 * (8000h and 0100h share a flag; the model only produces 0100h.)  The card
 * file is isolated (TMPDIR, i.e. under the worktree build dir); the owner's
 * default card path is stat'ed before/after and must not change.  The thunks
 * perform no block I/O: block load/write goes through the PE_Memcard file
 * layer, used here only to prove the thunks leave a card holding a file
 * byte-identical. */
#ifndef _WIN32
#include <sys/stat.h>
#endif

#define BC_F4_OK   0x800A1820u
#define BC_F4_ERR  0x800A1824u
#define BC_F4_NEW  0x800A1828u
#define BC_F0_OK   0x800A182Cu
#define BC_F0_ERR  0x800A1830u
#define BC_F0_NEW  0x800A1834u

static void BC_ClearFlags(void)
{
    for (pe_addr_t a = BC_F4_OK; a <= BC_F0_NEW; a += 4u) PE_StoreU32(a, 0u);
}

/* Bitmask of the six delivered flags: bit n = flag word n set. */
static unsigned BC_Flags(void)
{
    unsigned m = 0;
    for (unsigned i = 0; i < 6; i++) if (PE_LoadU32(BC_F4_OK + 4u * i)) m |= 1u << i;
    return m;
}
#define BC_M_F4_OK  (1u << 0)
#define BC_M_F4_ERR (1u << 1)
#define BC_M_F4_NEW (1u << 2)
#define BC_M_F0_OK  (1u << 3)
#define BC_M_F0_ERR (1u << 4)
#define BC_M_F0_NEW (1u << 5)

static int BC_FileBytes(const char *path, uint8_t *buf, size_t cap)
{
    FILE *f = fopen(path, "rb");
    size_t n;
    if (!f) return -1;
    n = fread(buf, 1, cap, f);
    fclose(f);
    return (int)n;
}

static int BC_OwnerCard(long *size, long *mtime)
{
#ifndef _WIN32
    const char *home = getenv("HOME");
    char p[1024];
    struct stat st;
    if (!home) return 0;
    snprintf(p, sizeof p, "%s/.local/share/parasite-eve-port/memcard1.mcd", home);
    if (stat(p, &st) != 0) return 0;
    *size = (long)st.st_size; *mtime = (long)st.st_mtime;
    return 1;
#else
    (void)size; (void)mtime; return 0;
#endif
}

static void test_bios_card_thunks(void)
{
    char path[512];
    static uint8_t before[PE_MC_IMAGE_SIZE], after[PE_MC_IMAGE_SIZE];
    uint8_t raw[8], filebefore[PE_MC_IMAGE_SIZE], fileafter[PE_MC_IMAGE_SIZE];
    long osz0 = 0, omt0 = 0, osz1 = 0, omt1 = 0;
    int had0, had1, nb, fd;
    FILE *f;
    TEST("BIOS_card_thunks");
    had0 = BC_OwnerCard(&osz0, &omt0);

    /* 1. empty slot: both info and load time out (0100h), write times out,
     *    new_card is inert; every call returns 1 (request accepted). */
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    PE_Memcard_SetPath(0, NULL);
    func_800409B4();
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x00u) == 1, "info returns 1");
    ASSERT(BC_Flags() == BC_M_F4_ERR, "info on empty slot must deliver timeout only");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_load(0x00u) == 1 && BC_Flags() == BC_M_F4_ERR, "load on empty slot");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_write(0x00u, 0x3F, 0) == 1 && BC_Flags() == BC_M_F0_ERR,
           "write on empty slot fails closed (timeout), nothing else delivered");
    BC_ClearFlags();
    pe_plat_bios_card_new_card(0x00u);
    ASSERT(BC_Flags() == 0, "new_card delivers nothing");
    ASSERT(!PE_Port_ShouldStop(), "no boundary on empty slot");

    /* 2. present formatted file-backed card (isolated under TMPDIR) */
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    MC_TestTempPath(path, sizeof path, "bioscard.mcd");
    PE_Memcard_SetPath(0, path);
    ASSERT(PE_Memcard_Path(0) && strstr(PE_Memcard_Path(0), "pe_memcard_test_") &&
           !strstr(PE_Memcard_Path(0), "parasite-eve-port"), "test card path is isolated");
    func_800409B4();
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_load(0x00u) == 1 && BC_Flags() == BC_M_F4_OK,
           "load on formatted card delivers IOE");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x00u) == 1 && BC_Flags() == BC_M_F4_NEW,
           "first info after insertion reports new card (2000h)");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x00u) == 1 && BC_Flags() == BC_M_F4_NEW,
           "info does not clear the new-card state");
    /* sub-port / bad channel bits: timeout like an absent card */
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x01u) == 1 && BC_Flags() == BC_M_F4_ERR, "chan low bits -> timeout");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x10u) == 1 && BC_Flags() == BC_M_F4_ERR, "port 1 (absent) -> timeout");
    /* new_card on a blank (fresh) card changes nothing */
    BC_ClearFlags();
    pe_plat_bios_card_new_card(0x00u);
    ASSERT(BC_Flags() == 0, "new_card delivers nothing on a blank card");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x00u) == 1 && BC_Flags() == BC_M_F4_NEW, "new_card did not acknowledge");
    /* snapshot image + file; the acknowledging write must change neither */
    memcpy(before, PE_Memcard_Image(0), PE_MC_IMAGE_SIZE);
    nb = BC_FileBytes(path, filebefore, sizeof filebefore);
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_write(0x00u, 0x3F, 0) == 1 && BC_Flags() == BC_M_F0_OK,
           "write(3Fh) acknowledges with IOE");
    memcpy(after, PE_Memcard_Image(0), PE_MC_IMAGE_SIZE);
    ASSERT(memcmp(before, after, PE_MC_IMAGE_SIZE) == 0, "card_write modified the card image");
    ASSERT(BC_FileBytes(path, fileafter, sizeof fileafter) == nb &&
           (nb <= 0 || memcmp(filebefore, fileafter, (size_t)nb) == 0), "card_write modified the card file");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x00u) == 1 && BC_Flags() == BC_M_F4_OK,
           "after the acknowledging write info reports ready (IOE)");
    BC_ClearFlags();
    /* any sector/source: same acknowledgement, still no image change */
    ASSERT(pe_plat_bios_card_write(0x00u, 0x10, 0x80100000) == 1 && BC_Flags() == BC_M_F0_OK, "write other sector");
    memcpy(after, PE_Memcard_Image(0), PE_MC_IMAGE_SIZE);
    ASSERT(memcmp(before, after, PE_MC_IMAGE_SIZE) == 0, "other-sector write modified the image");

    /* 3. one block through the file layer (not the thunks), then the thunks
     *    must leave the stored file byte-identical */
    fd = PE_Memcard_Open("bu00:BASLUS-00662BCTEST", 0x10200u);
    ASSERT(fd >= 0, "file layer creates one-block file");
    {
        static uint8_t blk[PE_MC_BLOCK_SIZE], rd[PE_MC_BLOCK_SIZE];
        for (unsigned i = 0; i < sizeof blk; i++) blk[i] = (uint8_t)(i * 7u + 3u);
        PE_Memcard_Close(fd);                 /* create-only handle is not writable */
        fd = PE_Memcard_Open("bu00:BASLUS-00662BCTEST", 2u);
        ASSERT(fd >= 0, "open for write");
        ASSERT(PE_Memcard_Write(fd, blk, (int32_t)sizeof blk) == (int)sizeof blk, "write one block");
        PE_Memcard_Close(fd);
        memcpy(before, PE_Memcard_Image(0), PE_MC_IMAGE_SIZE);
        nb = BC_FileBytes(path, filebefore, sizeof filebefore);
        ASSERT(nb == (int)PE_MC_IMAGE_SIZE, "card file persisted at full size");
        BC_ClearFlags();
        ASSERT(pe_plat_bios_card_load(0x00u) == 1 && BC_Flags() == BC_M_F4_OK, "load with a file present");
        ASSERT(pe_plat_bios_card_write(0x00u, 0x3F, 0) == 1, "ack write with a file present");
        pe_plat_bios_card_new_card(0x00u);
        (void)pe_plat_bios_card_info(0x00u);
        memcpy(after, PE_Memcard_Image(0), PE_MC_IMAGE_SIZE);
        ASSERT(memcmp(before, after, PE_MC_IMAGE_SIZE) == 0, "thunks changed a card holding a file");
        ASSERT(BC_FileBytes(path, fileafter, sizeof fileafter) == nb && memcmp(filebefore, fileafter, (size_t)nb) == 0,
               "thunks changed the card file");
        fd = PE_Memcard_Open("bu00:BASLUS-00662BCTEST", 1u);
        ASSERT(fd >= 0 && PE_Memcard_Read(fd, rd, (int32_t)sizeof rd) == (int)sizeof rd &&
               memcmp(rd, blk, sizeof blk) == 0, "block reads back intact after the thunks");
        PE_Memcard_Close(fd);
    }
    remove(path);

    /* 4. unformatted card: load and info report new card (2000h); the
     *    acknowledging write fails closed (card stays unformatted, file bytes
     *    untouched) and the file layer refuses to open on it. */
    ResetTestState();
    PE_Memcard_SetBoundaryMode(0);
    MC_TestTempPath(path, sizeof path, "bioscard_raw.mcd");
    f = fopen(path, "wb"); fputs("x", f); fclose(f);
    PE_Memcard_SetPath(0, path);
    func_800409B4();
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_load(0x00u) == 1 && BC_Flags() == BC_M_F4_NEW, "load on unformatted -> 2000h");
    BC_ClearFlags();
    ASSERT(pe_plat_bios_card_info(0x00u) == 1 && BC_Flags() == BC_M_F4_NEW, "info on unformatted -> 2000h");
    BC_ClearFlags();
    (void)pe_plat_bios_card_write(0x00u, 0x3F, 0);
    ASSERT(!PE_Memcard_IsFormatted(0), "card_write formatted an unformatted card");
    ASSERT(BC_FileBytes(path, raw, sizeof raw) == 1 && raw[0] == 'x', "card_write rewrote the unformatted file");
    ASSERT(PE_Memcard_Open("bu00:BASLUS-00662BCTEST", 0x10200u) < 0, "file layer opened an unformatted card");
    ASSERT(BC_FileBytes(path, raw, sizeof raw) == 1 && raw[0] == 'x', "failed open rewrote the unformatted file");
    remove(path);

    Test_ResetMemcard();
    had1 = BC_OwnerCard(&osz1, &omt1);
    ASSERT(had0 == had1 && osz0 == osz1 && omt0 == omt1, "owner card file was touched");
    PASS();
}
