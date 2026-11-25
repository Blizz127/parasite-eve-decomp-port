/* Standalone screenshot-preservation tests. Only synthetic RGB and files under
 * a fresh mkdtemp directory are used. No window, disc, owner path or game runs.
 * fopen is interposed only while host_png.h is included; the wrapper delegates
 * actual writes to libc, including the C11 exclusive-create mode. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static FILE *shot_test_fopen(const char *path, const char *mode);
#define fopen shot_test_fopen
#include "host_png.h"
#undef fopen

enum Scenario {
    ORDINARY, READABLE_COLLISION, UNREADABLE_COLLISION, CREATE_RACE,
    WRITE_DENIED, EXISTING_SYMLINK, DANGLING_SYMLINK, SHORT_PATH
};
static enum Scenario scenario;
static char first_path[1200], target_path[1200];
static int injected, reads, writes, exclusive_writes, actual_read_denied, infra_error;
static const char sentinel[] = "preserve the earlier screenshot\n";
static const uint8_t pixels[6] = {0x31, 0x72, 0xB4, 0xC5, 0x86, 0x47};

static int make_sentinel(const char *path, mode_t permissions)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, permissions);
    ssize_t done;
    if (fd < 0) return 0;
    done = write(fd, sentinel, sizeof sentinel - 1u);
    return close(fd) == 0 && done == (ssize_t)(sizeof sentinel - 1u);
}

static int is_sentinel(const char *path)
{
    char bytes[sizeof sentinel];
    FILE *f = fopen(path, "rb");
    size_t n;
    if (!f) return 0;
    n = fread(bytes, 1, sizeof bytes, f);
    fclose(f);
    return n == sizeof sentinel - 1u && !memcmp(bytes, sentinel, n);
}

static void inject_collision(const char *path)
{
    if (injected) return;
    injected = 1;
    if (scenario == EXISTING_SYMLINK || scenario == DANGLING_SYMLINK) {
        if (scenario == EXISTING_SYMLINK && !make_sentinel(target_path, 0600))
            infra_error = 1;
        if (symlink(target_path, path) != 0) infra_error = 1;
    } else {
        mode_t permissions = scenario == UNREADABLE_COLLISION ? 0200 : 0600;
        if (!make_sentinel(path, permissions)) infra_error = 1;
        if (scenario == UNREADABLE_COLLISION) {
            FILE *probe;
            errno = 0;
            probe = fopen(path, "rb");
            actual_read_denied = !probe && errno == EACCES;
            if (probe) fclose(probe);
        }
    }
}

static FILE *shot_test_fopen(const char *path, const char *mode)
{
    int writing = mode[0] == 'w';
    if (!first_path[0]) {
        if (snprintf(first_path, sizeof first_path, "%s", path) >= (int)sizeof first_path)
            infra_error = 1;
    }
    if (writing) {
        writes++;
        if (strchr(mode, 'x')) exclusive_writes++;
    } else if (mode[0] == 'r') reads++;

    /* The competitor wins creation immediately before the first real write
     * attempt. With the old code, its earlier existence probe already returned
     * absent. This deterministic interleaving needs neither sleeps nor a race. */
    if (scenario == CREATE_RACE && writing && !injected)
        inject_collision(path);
    if ((scenario == READABLE_COLLISION || scenario == UNREADABLE_COLLISION ||
         scenario == EXISTING_SYMLINK || scenario == DANGLING_SYMLINK) && !injected)
        inject_collision(path);
    if (scenario == UNREADABLE_COLLISION && mode[0] == 'r' &&
        !strcmp(path, first_path)) {
        /* Also deterministic when this fixture runs as root. The actual
         * access denial is separately reported when ordinary permissions apply. */
        errno = EACCES;
        return NULL;
    }
    if (scenario == WRITE_DENIED && writing) {
        errno = EACCES;
        return NULL;
    }
    return fopen(path, mode);
}

static int is_png(const char *path)
{
    static const uint8_t sig[8] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
    uint8_t header[24];
    struct stat st;
    FILE *f = fopen(path, "rb");
    size_t n;
    if (!f) return 0;
    n = fread(header, 1, sizeof header, f);
    fclose(f);
    return n == sizeof header && !memcmp(header, sig, sizeof sig) &&
           !memcmp(header + 12, "IHDR", 4) && header[19] == 2 && header[23] == 1 &&
           stat(path, &st) == 0 && (size_t)st.st_size == HostPng_EncodedSize(2, 1);
}

static int run_case(enum Scenario which, const char *name)
{
    char directory[] = "/tmp/pe-hostshot-exclusive-XXXXXX";
    char output[1200], shotdir[1024], appdir[1100];
    struct stat st;
    int rc, ok, preserved = -1, distinct = -1, absent = -1;
    scenario = which;
    first_path[0] = target_path[0] = 0;
    injected = reads = writes = exclusive_writes = actual_read_denied = infra_error = 0;
    if (!mkdtemp(directory) || setenv("XDG_DATA_HOME", directory, 1) != 0 ||
        setenv("HOME", directory, 1) != 0) {
        printf("CASE %s INFRA_FAIL\n", name);
        return 0;
    }
    if (snprintf(target_path, sizeof target_path, "%s/protected-target", directory) >= (int)sizeof target_path)
        infra_error = 1;
    rc = HostShot_Save(pixels, 2, 1, output, which == SHORT_PATH ? 4u : sizeof output);
    if (which == UNREADABLE_COLLISION && first_path[0]) (void)chmod(first_path, 0600);
    ok = !infra_error;
    if (which == WRITE_DENIED || which == SHORT_PATH) {
        absent = first_path[0] && lstat(first_path, &st) != 0 && errno == ENOENT;
        ok = ok && rc == -1 && writes == 1 && absent;
    } else {
        ok = ok && rc == 0 && is_png(output);
        if (which != ORDINARY) {
            distinct = strcmp(output, first_path) != 0;
            if (which == EXISTING_SYMLINK || which == DANGLING_SYMLINK) {
                preserved = lstat(first_path, &st) == 0 && S_ISLNK(st.st_mode);
                if (which == EXISTING_SYMLINK) preserved = preserved && is_sentinel(target_path);
                else preserved = preserved && lstat(target_path, &st) != 0 && errno == ENOENT;
            } else preserved = is_sentinel(first_path);
            ok = ok && injected && distinct && preserved;
        }
    }
    printf("CASE %s %s rc=%d reads=%d writes=%d exclusive=%d preserved=%d distinct=%d absent=%d actual_read_denied=%d\n",
           name, ok ? "PASS" : "FAIL", rc, reads, writes, exclusive_writes,
           preserved, distinct, absent, actual_read_denied);

    /* Delete only paths this isolated case owns. unlink does not follow links. */
    if (rc == 0 && output[0] && strcmp(output, first_path)) (void)unlink(output);
    if (first_path[0]) (void)unlink(first_path);
    (void)unlink(target_path);
    if (HostShot_Dir(shotdir, sizeof shotdir) == 0) (void)rmdir(shotdir);
    if (snprintf(appdir, sizeof appdir, "%s/parasite-eve-port", directory) < (int)sizeof appdir)
        (void)rmdir(appdir);
    (void)rmdir(directory);
    return ok;
}

int main(void)
{
    static const char *names[] = {
        "ordinary_png", "readable_collision", "unreadable_existing",
        "create_between_probe_and_open", "write_access_denied",
        "existing_symlink", "dangling_symlink", "short_output_path"
    };
    unsigned passed = 0;
    for (unsigned i = 0; i < sizeof names / sizeof names[0]; i++)
        passed += (unsigned)run_case((enum Scenario)i, names[i]);
    printf("SUMMARY %u/%u PASS\n", passed, (unsigned)(sizeof names / sizeof names[0]));
    return passed == sizeof names / sizeof names[0] ? 0 : 1;
}
