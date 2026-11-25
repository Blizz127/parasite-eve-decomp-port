/* Synthetic Win32 replacement integration check. Run only in an isolated
 * test environment: all files are created below a supplied scratch parent.
 * This does not open a game or an owner memory card. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_memcard.c"

int main(int argc, char **argv)
{
    if (argc != 3) abort();
    char dir[MAX_PATH], path[MAX_PATH];
    if (!GetTempFileNameA(argv[2], "pct", 0, dir) ||
        !DeleteFileA(dir) || !CreateDirectoryA(dir, NULL)) abort();
    int n = snprintf(path, sizeof path, "%s\\card.mcd", dir);
    if (n < 0 || (size_t)n >= sizeof path) abort();
    int creating = strcmp(argv[1], "create") == 0;
    FILE *f;
    if (!creating) {
        f = fopen(path, "wb");
        if (!f) abort();
        for (size_t i = 0; i < PE_MC_IMAGE_SIZE; i++) if (fputc(0xa1, f) == EOF) abort();
        if (fclose(f)) abort();
    }
    McPort p = {0};
    p.present = 1;
    strcpy(p.path, path);
    memset(p.image, 0xb2, sizeof p.image);
    int failure = strcmp(argv[1], "failure") == 0;
    if (!failure && !creating && strcmp(argv[1], "success") != 0) abort();
    HANDLE lock = INVALID_HANDLE_VALUE;
    if (failure) {
        lock = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (lock == INVALID_HANDLE_VALUE) abort();
    }
    int result = save_image(&p);
    if (failure && !CloseHandle(lock)) abort();
    if (result != !failure) abort();
    f = fopen(path, "rb");
    if (!f) abort();
    for (size_t i = 0; i < PE_MC_IMAGE_SIZE; i++)
        if (fgetc(f) != (failure ? 0xa1 : 0xb2)) abort();
    if (fgetc(f) != EOF || fclose(f)) abort();
    printf("synthetic Win32 replacement create=%d failure=%d result=%d image intact\n", creating, failure, result);
    return 0;
}
