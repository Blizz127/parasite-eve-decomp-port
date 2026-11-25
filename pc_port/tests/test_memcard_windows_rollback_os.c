/* Synthetic Win32 card-write rollback check. All files are created below the
 * supplied scratch parent; this test never resolves an owner card path. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/pe_memcard.c"

static void read_image(const char *path, uint8_t out[PE_MC_IMAGE_SIZE])
{
    FILE *f = fopen(path, "rb");
    if (!f || fread(out, 1, PE_MC_IMAGE_SIZE, f) != PE_MC_IMAGE_SIZE ||
        fgetc(f) != EOF || fclose(f)) abort();
}
int main(int argc, char **argv)
{
    if (argc != 2) abort();
    char dir[MAX_PATH], path[MAX_PATH];
    if (!GetTempFileNameA(argv[1], "pcr", 0, dir) ||
        !DeleteFileA(dir) || !CreateDirectoryA(dir, NULL)) abort();
    int n = snprintf(path, sizeof path, "%s\\card.mcd", dir);
    if (n < 0 || (size_t)n >= sizeof path) abort();
    static uint8_t image[PE_MC_IMAGE_SIZE], before[PE_MC_IMAGE_SIZE], after[PE_MC_IMAGE_SIZE];
    uint8_t old_data[128], new_data[128], readback[128];
    memset(old_data, 0x51, sizeof old_data);
    memset(new_data, 0x92, sizeof new_data);
    PE_Memcard_FormatImage(image);
    FILE *f = fopen(path, "wb");
    if (!f || fwrite(image, 1, sizeof image, f) != sizeof image || fclose(f)) abort();
    PE_Memcard_SetPath(0, path);
    int fd = PE_Memcard_Open("bu00:TEST", (1u << 16) | 0x200u | 3u);
    if (fd < 0 || PE_Memcard_Write(fd, old_data, sizeof old_data) != sizeof old_data ||
        PE_Memcard_Lseek(fd, 0, 0) != 0) abort();
    read_image(path, before);
    HANDLE lock = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (lock == INVALID_HANDLE_VALUE) abort();
    int result = PE_Memcard_Write(fd, new_data, sizeof new_data);
    if (!CloseHandle(lock) || result != -1 || PE_Memcard_Lseek(fd, 0, 1) != 0) abort();
    if (PE_Memcard_Read(fd, readback, sizeof readback) != sizeof readback ||
        memcmp(readback, old_data, sizeof readback)) abort();
    read_image(path, after);
    if (memcmp(before, after, sizeof before) || memcmp(before, PE_Memcard_Image(0), sizeof before)) abort();
    if (PE_Memcard_Close(fd) != fd) abort();
    puts("synthetic Win32 card write failure: disk, cache and position retained");
    return 0;
}
