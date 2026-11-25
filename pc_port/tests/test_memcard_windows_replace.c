/* Synthetic Windows replacement fault check, run only with the test headers
 * and an explicitly provided scratch parent. It creates a new private temp
 * directory per run and never resolves the game's card path. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "win32_card_stub/windows.h"
int fixture_rename(const char *source, const char *destination);
#define rename fixture_rename
#include "../platform/pe_memcard.c"
#undef rename

static int force_failure, replace_calls;
int fixture_rename(const char *source, const char *destination)
{
    if (force_failure) { errno = EACCES; return -1; }
    return rename(source, destination);
}
int MoveFileExA(const char *source, const char *destination, unsigned long flags)
{
    if (flags != MOVEFILE_REPLACE_EXISTING) abort();
    replace_calls++;
    if (force_failure) return 0;
    return rename(source, destination) == 0;
}
unsigned long GetLastError(void) { return 5u; }

int main(int argc, char **argv)
{
    if (argc != 3) abort();
    char directory[1024];
    int n = snprintf(directory, sizeof directory, "%s/card-XXXXXX", argv[1]);
    if (n < 0 || (size_t)n >= sizeof directory || !mkdtemp(directory)) abort();
    McPort p = {0};
    n = snprintf(p.path, sizeof p.path, "%s/card.mcd", directory);
    if (n < 0 || (size_t)n >= sizeof p.path) abort();
    FILE *f = fopen(p.path, "wb");
    if (!f) abort();
    for (size_t i = 0; i < PE_MC_IMAGE_SIZE; i++) if (fputc(0xa1, f) == EOF) abort();
    if (fclose(f)) abort();
    p.present = 1;
    memset(p.image, 0xb2, sizeof p.image);
    force_failure = strcmp(argv[2], "failure") == 0;
    if (!force_failure && strcmp(argv[2], "success") != 0) abort();
    int result = save_image(&p);
    f = fopen(p.path, "rb");
    if (!f) { puts("destination missing after synthetic replacement"); fflush(stdout); abort(); }
    for (size_t i = 0; i < PE_MC_IMAGE_SIZE; i++)
        if (fgetc(f) != (force_failure ? 0xa1 : 0xb2)) abort();
    if (fgetc(f) != EOF || fclose(f)) abort();
    if (result != !force_failure) abort();
    printf("synthetic replacement: failure=%d result=%d old-or-new intact; replace_calls=%d\n",
           force_failure, result, replace_calls);
    return 0;
}
