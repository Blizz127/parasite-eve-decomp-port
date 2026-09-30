/* First-run disc check (pe_disc_check.h) and disc cache (pe_disc_cache.h).
 * Synthetic images cover every rejection path without any retail bytes;
 * the positive identification runs against the user's configured disc. */
#include "pe_disc_check.h"
#include "pe_disc_cache.h"

#define DCK_SECTORS 40u

/* Minimal MODE2/2352 image: PVD, a root directory with SYSTEM.CNF and one
 * boot file whose body is a synthetic PS-X EXE header (not retail bytes). */
static uint8_t *DCK_Build(int with_pvd, const char *boot_id, const char *cnf_text)
{
    uint8_t *img = (uint8_t *)calloc(DCK_SECTORS, PE_DISC_RAW_SECTOR);
    uint8_t *p;
    int n;
    uint32_t i;
    if (!img)
        return NULL;
    for (i = 0; i < DCK_SECTORS; i++) {
        uint8_t *raw = img + (size_t)i * PE_DISC_RAW_SECTOR;
        memset(raw + 1, 0xFF, 10);
        raw[15] = 0x02;
    }
    if (with_pvd) {
        p = FxUser(img, 16);
        p[0] = 1; memcpy(p + 1, "CD001", 5); p[6] = 1;
        FxPutDirRec(p + 156, 20, 2048, 0x02, "\0", 1);
    }
    p = FxUser(img, 20);
    n = FxPutDirRec(p, 20, 2048, 0x02, "\0", 1);
    n += FxPutDirRec(p + n, 20, 2048, 0x02, "\1", 1);
    n += FxPutDirRec(p + n, 22, (uint32_t)strlen(cnf_text), 0x00, "SYSTEM.CNF;1", 12);
    FxPutDirRec(p + n, 23, 4096, 0x00, boot_id, (int)strlen(boot_id));
    memcpy(FxUser(img, 22), cnf_text, strlen(cnf_text));
    memcpy(FxUser(img, 23), "PS-X EXE", 8);
    return img;
}

static int DCK_Identify(uint8_t *img, unsigned mask, char *why, size_t n)
{
    PE_Disc *d = PE_Disc_OpenMemory(img, (size_t)DCK_SECTORS * PE_DISC_RAW_SECTOR);
    PE_DiscIdentity id;
    int rc;
    why[0] = '\0';
    rc = PE_DiscCheck_Identify(d, mask, &id, why, n);
    if (d)
        PE_Disc_Close(d);
    return rc;
}

static void test_DISC_check_synthetic_rejections(void)
{
    char why[384];
    uint8_t *img;
    char sha[41];

    TEST("DISC_check_synthetic_rejections");
    PE_DiscCheck_Sha1Hex("abc", 3, sha);
    ASSERT(!strcmp(sha, "a9993e364706816aba3e25717850c26c9cd0d89d"), "SHA-1 FIPS vector");
    ASSERT(PE_DiscCheck_Identify(NULL, 1u, NULL, why, sizeof(why)) != 0 &&
           strstr(why, "no disc image"), "NULL disc");

    img = DCK_Build(0, "SLUS_006.62;1", "BOOT = cdrom:\\SLUS_006.62;1\r\n");
    ASSERT(img && DCK_Identify(img, 1u, why, sizeof(why)) != 0 &&
           strstr(why, "ISO9660"), "no PVD");
    free(img);

    img = DCK_Build(1, "OTHER.EXE;1", "BOOT = cdrom:\\OTHER.EXE;1\r\n");
    ASSERT(img && DCK_Identify(img, 1u, why, sizeof(why)) != 0 &&
           strstr(why, "not a Parasite Eve disc"), "foreign disc");
    free(img);

    img = DCK_Build(1, "SLUS_006.62;1", "BOOT = cdrom:\\SLUS_999.99;1\r\n");
    ASSERT(img && DCK_Identify(img, 1u, why, sizeof(why)) != 0 &&
           strstr(why, "SYSTEM.CNF does not boot SLUS_006.62"), "serial cross-check");
    free(img);

    img = DCK_Build(1, "SLUS_006.62;1", "BOOT = cdrom:\\SLUS_006.62;1\r\n");
    ASSERT(img && DCK_Identify(img, 1u, why, sizeof(why)) != 0 &&
           strstr(why, "does not match the expected 452fb033"), "EXE hash mismatch");
    free(img);

    img = DCK_Build(1, "SLPS_012.30;1", "BOOT = cdrom:\\SLPS_012.30;1\r\n");
    ASSERT(img && DCK_Identify(img, 1u, why, sizeof(why)) != 0 &&
           strstr(why, "JPN Disc 1") && strstr(why, "not supported yet"), "JPN recognised");
    free(img);
    PASS();
}

static void test_DISC_check_retail_disc1(void)
{
    char path[1024], err[384];
    PE_Disc *d;
    PE_DiscIdentity id, other;
    uint32_t size, w;
    const uint8_t *exe;

    TEST_RETAIL_DISC1("DISC_check_retail_disc1");
    ASSERT(PE_DiscCache_ImagePath(path, sizeof(path)) == 0, "configured image path");
    d = PE_Disc_Open(path, err, sizeof(err));
    ASSERT(d != NULL, err);
    ASSERT(PE_DiscCheck_Identify(d, 1u, &id, err, sizeof(err)) == 0, err);
    ASSERT(!strcmp(id.known->serial, "SLUS-00662") && id.known->disc == 1, "USA Disc 1");
    ASSERT(PE_DiscCheck_Identify(d, 2u, &other, err, sizeof(err)) != 0 && other.known == NULL &&
           strstr(err, "start the game with USA Disc 2"), "Disc 1 refused when only Disc 2 accepted");
    PE_Disc_Close(d);
    exe = PE_DiscCache_Exe(&size, err, sizeof(err));
    ASSERT(exe != NULL, err);
    ASSERT(size == id.exe_size && !memcmp(exe, "PS-X EXE", 8), "cache EXE is the disc EXE");
    ASSERT(PE_DiscCache_ExeWord(0x800960BCu, &w) == 0 && w != 0u, "libgte table word via cache");
    ASSERT(PE_DiscCache_ExeWord(0x80000000u, &w) != 0, "below taddr rejected");
    ASSERT(PE_DiscCache_FileRead(PE_DISC_SRC_PEIMG, 0u, &w, 4u, err, sizeof(err)) == 0, err);
    PASS();
}

static void test_DISC_check_all(void)
{
    test_DISC_check_synthetic_rejections();
    test_DISC_check_retail_disc1();
}
