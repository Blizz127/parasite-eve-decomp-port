/* Synthetic POSIX card persistence checks. Only newly made private files are
 * accessed. Includes the real backend, with syscall fault/ordering wrappers.
 * This verifies pre-replacement flush/sync and failure behavior, not crash
 * survival, directory durability, game-level transactions or writer exclusion.
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int fixture_fflush(FILE *stream);
int fixture_fsync(int fd);
int fixture_fclose(FILE *stream);
int fixture_rename(const char *source, const char *destination);
#define fflush fixture_fflush
#define fsync fixture_fsync
#define fclose fixture_fclose
#define rename fixture_rename
#ifndef PE_MEMCARD_SOURCE
#define PE_MEMCARD_SOURCE "../platform/pe_memcard.c"
#endif
#include PE_MEMCARD_SOURCE
#undef fflush
#undef fsync
#undef fclose
#undef rename

enum Fault { FAULT_NONE, FAULT_FLUSH, FAULT_SYNC, FAULT_EINTR };
enum Operation { OP_CREATE, OP_WRITE, OP_DELETE, OP_FORMAT };
static const char *const operation_names[] = { "create", "write", "delete", "format" };
static const char *case_name = "setup";
static enum Fault injected;
static int auditing, sync_interrupts, flushed_fd;
static unsigned checks;
static char events[64];
static size_t event_count;

#define REQUIRE(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s line %d: %s (events=%s)\n", \
                case_name, __LINE__, message, events); \
        exit(1); \
    } \
} while (0)

static void record(char event)
{
    REQUIRE(event_count + 1u < sizeof events, "bounded syscall trace");
    events[event_count++] = event;
    events[event_count] = 0;
}

int fixture_fflush(FILE *stream)
{
    if (auditing) {
        record('F');
        flushed_fd = fileno(stream);
        if (injected == FAULT_FLUSH) { errno = EIO; return EOF; }
    }
    return fflush(stream);
}

int fixture_fsync(int fd)
{
    if (auditing) {
        record('S');
        REQUIRE(fd >= 0 && fd == flushed_fd, "sync the stream that was flushed");
        if (injected == FAULT_SYNC) { errno = EIO; return -1; }
        if (injected == FAULT_EINTR && sync_interrupts++ < 2) {
            errno = EINTR;
            return -1;
        }
    }
    return fsync(fd);
}

int fixture_fclose(FILE *stream)
{
    if (auditing) record('C');
    return fclose(stream);
}

int fixture_rename(const char *source, const char *destination)
{
    if (auditing) record('R');
    return rename(source, destination);
}

static void audit_begin(enum Fault fault)
{
    injected = fault;
    event_count = 0;
    events[0] = 0;
    sync_interrupts = 0;
    flushed_fd = -1;
    auditing = 1;
}

static void audit_end(const char *expected)
{
    auditing = 0;
    injected = FAULT_NONE;
    REQUIRE(strcmp(events, expected) == 0, "expected flush/sync/close/rename order");
}

static void file_bytes(const char *name, const unsigned char *expected, size_t length)
{
    unsigned char actual[256];
    int fd = PE_Memcard_Open(name, 1u);
    REQUIRE(length <= sizeof actual, "bounded file expectation");
    REQUIRE(fd >= 0, "expected file remains readable");
    REQUIRE(PE_Memcard_Read(fd, actual, (int32_t)length) == (int)length,
            "read expected file bytes");
    REQUIRE(memcmp(actual, expected, length) == 0, "file payload matches");
    REQUIRE(PE_Memcard_Close(fd) == fd, "close expected file");
}

static void disk_bytes(const char *path, const unsigned char *expected)
{
    unsigned char actual[PE_MC_IMAGE_SIZE];
    FILE *f = fopen(path, "rb");
    REQUIRE(f != NULL, "old or new disk image exists");
    REQUIRE(fread(actual, 1, sizeof actual, f) == sizeof actual,
            "complete disk image");
    REQUIRE(fgetc(f) == EOF && !ferror(f), "exact disk image size");
    REQUIRE(fclose(f) == 0, "close disk readback");
    REQUIRE(memcmp(actual, expected, sizeof actual) == 0,
            "complete disk image matches expected bytes");
}

static void no_temp(const char *path)
{
    char temp[1100];
    struct stat st;
    int n = snprintf(temp, sizeof temp, "%s.tmp", path);
    REQUIRE(n >= 0 && (size_t)n < sizeof temp, "temp path fits");
    errno = 0;
    REQUIRE(stat(temp, &st) != 0 && errno == ENOENT, "no temporary image remains");
}

static void reset_to(const char *path)
{
    PE_Memcard_Reset();
    PE_Memcard_SetPath(0, path);
    PE_Memcard_SetPath(1, NULL);
}

static void seed_card(const char *path, unsigned char *before,
                      unsigned char *a, unsigned char *b)
{
    int fd;
    reset_to("");
    for (unsigned i = 0; i < 256u; i++) a[i] = (unsigned char)(i * 17u + 3u);
    for (unsigned i = 0; i < 128u; i++) b[i] = (unsigned char)(i * 11u + 9u);
    fd = PE_Memcard_Open("bu00:A", 0x10203u);
    REQUIRE(fd >= 0 && PE_Memcard_Write(fd, a, 256) == 256,
            "create independent synthetic A");
    REQUIRE(PE_Memcard_Close(fd) == fd, "close seed A");
    fd = PE_Memcard_Open("bu00:B", 0x10203u);
    REQUIRE(fd >= 0 && PE_Memcard_Write(fd, b, 128) == 128,
            "create independent synthetic B");
    REQUIRE(PE_Memcard_Close(fd) == fd, "close seed B");
    REQUIRE(PE_Memcard_Image(0) != NULL, "synthetic image available");
    memcpy(before, PE_Memcard_Image(0), PE_MC_IMAGE_SIZE);
    FILE *f = fopen(path, "wb");
    REQUIRE(f != NULL, "create private fixture card");
    REQUIRE(fwrite(before, 1, PE_MC_IMAGE_SIZE, f) == PE_MC_IMAGE_SIZE,
            "write complete fixture image");
    REQUIRE(fclose(f) == 0, "close fixture image");
    reset_to(path);
    REQUIRE(PE_Memcard_IsFormatted(0), "load synthetic persistent card before injection");
}

static int operation(enum Operation op, int fd, const unsigned char *replacement)
{
    switch (op) {
    case OP_CREATE: return PE_Memcard_Open("bu00:NEW", 0x10203u);
    case OP_WRITE: return PE_Memcard_Write(fd, replacement, 128);
    case OP_DELETE: return PE_Memcard_Delete("bu00:A");
    case OP_FORMAT: return PE_Memcard_Format("bu00:");
    }
    REQUIRE(0, "known operation");
    return -1;
}

static void successful_effect(enum Operation op, unsigned char *a,
                              const unsigned char *b, const unsigned char *replacement)
{
    if (op == OP_WRITE) memcpy(a + 128, replacement, 128);
    if (op == OP_CREATE || op == OP_WRITE) file_bytes("bu00:A", a, 256);
    else REQUIRE(PE_Memcard_Open("bu00:A", 1u) < 0, "A absent after delete/format");
    if (op == OP_FORMAT) {
        PeMcDirEntry entry;
        REQUIRE(PE_Memcard_IsFormatted(0), "formatted image remains valid");
        REQUIRE(!PE_Memcard_FirstFile("bu00:*", &entry), "formatted card has no files");
    } else file_bytes("bu00:B", b, 128);
    if (op == OP_CREATE) {
        const unsigned char zeros[128] = {0};
        file_bytes("bu00:NEW", zeros, sizeof zeros);
    } else REQUIRE(PE_Memcard_Open("bu00:NEW", 1u) < 0, "unrequested new file absent");
}

static void run_case(const char *directory, enum Operation op, enum Fault fault)
{
    unsigned char before[PE_MC_IMAGE_SIZE], after[PE_MC_IMAGE_SIZE];
    unsigned char a[256], b[128], replacement[128], current[128];
    char path[1024], label[48];
    const char *fault_name = fault == FAULT_FLUSH ? "flush" :
                             fault == FAULT_SYNC ? "sync" : "eintr";
    int n = snprintf(label, sizeof label, "%s-%s", fault_name, operation_names[op]);
    REQUIRE(n >= 0 && (size_t)n < sizeof label, "case label fits");
    case_name = label;
    n = snprintf(path, sizeof path, "%s/%s.mcd", directory, label);
    REQUIRE(n >= 0 && (size_t)n < sizeof path, "private card path fits");
    seed_card(path, before, a, b);
    for (unsigned i = 0; i < sizeof replacement; i++) replacement[i] = (unsigned char)(0xd1u ^ i);
    int fd = PE_Memcard_Open("bu00:A", 3u);
    REQUIRE(fd >= 0 && PE_Memcard_Lseek(fd, 128, 0) == 128, "live handle at sector one");

    audit_begin(fault);
    int result = operation(op, fd, replacement);
    auditing = 0;
    if (fault != FAULT_EINTR) {
        REQUIRE(op < OP_DELETE ? result < 0 : result == 0, "injected I/O error is reported");
        audit_end(fault == FAULT_FLUSH ? "FC" : "FSC");
        REQUIRE(PE_Memcard_LastError()[0] != 0, "I/O error has a diagnostic");
        REQUIRE(memcmp(before, PE_Memcard_Image(0), sizeof before) == 0,
                "failed persistence restores complete cached card");
        disk_bytes(path, before);
        no_temp(path);
        REQUIRE(PE_Memcard_Lseek(fd, 0, 1) == 128, "failed operation preserves open handle and position");
        REQUIRE(PE_Memcard_Read(fd, current, sizeof current) == (int)sizeof current &&
                memcmp(current, a + 128, sizeof current) == 0,
                "old handle reads old payload after failed mutation");
        REQUIRE(PE_Memcard_Lseek(fd, 128, 0) == 128, "restore observed handle position");
        REQUIRE(PE_Memcard_Open("bu00:NEW", 1u) < 0, "failed create leaves no phantom file");
        file_bytes("bu00:B", b, sizeof b);
        audit_begin(FAULT_NONE);
        result = operation(op, fd, replacement);
        audit_end("FSCR");
    } else audit_end("FSSSCR");

    REQUIRE(op == OP_CREATE ? result >= 0 :
            op == OP_WRITE ? result == 128 : result == 1,
            "same operation succeeds once fault clears or EINTR retries");
    if (op == OP_CREATE) REQUIRE(PE_Memcard_Close(result) == result, "close new file");
    if (op == OP_CREATE || op == OP_WRITE)
        REQUIRE(PE_Memcard_Close(fd) == fd, "unrelated live handle retained on success");
    else REQUIRE(PE_Memcard_Lseek(fd, 0, 1) < 0, "successful removal invalidates old handle");
    successful_effect(op, a, b, replacement);
    memcpy(after, PE_Memcard_Image(0), sizeof after);
    REQUIRE(memcmp(before, after, sizeof before) != 0, "success actually changes card");
    disk_bytes(path, after);
    no_temp(path);
    reset_to(path);
    REQUIRE(PE_Memcard_Image(0) != NULL &&
            memcmp(after, PE_Memcard_Image(0), sizeof after) == 0,
            "fresh reload recovers the committed image");
    successful_effect(op, a, b, replacement);
    printf("PASS %s: old-state preservation, retry/reload and syscall order\n", case_name);
    checks++;
    case_name = "cleanup";
    reset_to(NULL);
    REQUIRE(unlink(path) == 0, "remove only this generated card");
}

static void missing_card(const char *directory, enum Fault fault)
{
    char path[1024];
    struct stat st;
    case_name = fault == FAULT_FLUSH ? "flush-missing" : "sync-missing";
    int n = snprintf(path, sizeof path, "%s/%s.mcd", directory, case_name);
    REQUIRE(n >= 0 && (size_t)n < sizeof path, "private card path fits");
    reset_to(path);
    audit_begin(fault);
    REQUIRE(PE_Memcard_Image(0) == NULL, "failed initial creation stays unloaded");
    audit_end(fault == FAULT_FLUSH ? "FC" : "FSC");
    errno = 0;
    REQUIRE(stat(path, &st) != 0 && errno == ENOENT, "failed initial creation leaves no card");
    no_temp(path);
    audit_begin(FAULT_NONE);
    REQUIRE(PE_Memcard_Image(0) != NULL, "initial creation retries after failure");
    audit_end("FSCR");
    REQUIRE(PE_Memcard_IsFormatted(0), "new card is formatted");
    disk_bytes(path, PE_Memcard_Image(0));
    no_temp(path);
    printf("PASS %s: failed initial creation retries safely\n", case_name);
    checks++;
    reset_to(NULL);
    REQUIRE(unlink(path) == 0, "remove only this generated card");
}

static void memory_only(void)
{
    unsigned char data[128] = {0x37};
    case_name = "memory-only";
    reset_to("");
    audit_begin(FAULT_FLUSH);
    REQUIRE(PE_Memcard_IsFormatted(0), "memory-only card initializes");
    int fd = PE_Memcard_Open("bu00:A", 0x10203u);
    REQUIRE(fd >= 0 && PE_Memcard_Write(fd, data, sizeof data) == (int)sizeof data,
            "memory-only create/write needs no persistence");
    REQUIRE(PE_Memcard_Close(fd) == fd && PE_Memcard_Delete("bu00:A") &&
            PE_Memcard_Format("bu00:"), "memory-only delete/format needs no persistence");
    audit_end("");
    puts("PASS memory-only: no flush/sync/close/rename calls");
    checks++;
}

int main(int argc, char **argv)
{
    char directory[1024];
    REQUIRE(argc == 2 || argc == 3, "usage: test SCRATCH_PARENT [CASE]");
    int n = snprintf(directory, sizeof directory, "%s/memcard-sync-XXXXXX", argv[1]);
    REQUIRE(n >= 0 && (size_t)n < sizeof directory && mkdtemp(directory) != NULL,
            "create private synthetic-card directory");
    for (enum Fault fault = FAULT_FLUSH; fault <= FAULT_EINTR; fault++) {
        for (enum Operation op = OP_CREATE; op <= OP_FORMAT; op++) {
            char label[48];
            const char *prefix = fault == FAULT_FLUSH ? "flush" : fault == FAULT_SYNC ? "sync" : "eintr";
            n = snprintf(label, sizeof label, "%s-%s", prefix, operation_names[op]);
            REQUIRE(n >= 0 && (size_t)n < sizeof label, "case selector fits");
            if (argc == 2 || strcmp(argv[2], label) == 0) run_case(directory, op, fault);
        }
    }
    if (argc == 2 || strcmp(argv[2], "flush-missing") == 0) missing_card(directory, FAULT_FLUSH);
    if (argc == 2 || strcmp(argv[2], "sync-missing") == 0) missing_card(directory, FAULT_SYNC);
    if (argc == 2 || strcmp(argv[2], "memory-only") == 0) memory_only();
    REQUIRE(checks != 0, "requested test actually ran");
    REQUIRE(rmdir(directory) == 0, "remove only empty generated fixture directory");
    printf("PASS: %u synthetic POSIX memory-card persistence cases\n", checks);
    return 0;
}
