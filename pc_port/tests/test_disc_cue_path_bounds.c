/* Synthetic pathname checks of the private cue-name helper; no disc access. */
#include <errno.h>
#include <stdio.h>
#include <string.h>
static size_t opened_length;
static FILE *fixture_open(const char *path,const char *mode)
{
    (void)mode;opened_length=strlen(path);errno=ENOENT;return NULL;
}
#define fopen fixture_open
#include "../platform/pe_disc.c"
#undef fopen
int main(void)
{
    char path[4100];
    memset(path, 'a', sizeof(path));
    path[4093] = '.';
    path[4095] = '\0';
    if (PE_Disc_CheckCue(path) != -1) return 1;
    path[4093] = 'a';
    path[4091] = '.';
    if (PE_Disc_CheckCue(path) != 0 || opened_length != 4095) return 4;
    path[4095] = 'a';
    path[4099] = '\0';
    if (PE_Disc_CheckCue(path) != -1) return 2;
    if (PE_Disc_CheckCue("/tmp/pe-nonexistent-cue-fixture-239847.bin") != 0) return 3;
    puts("cue path bounds and absent cue controls pass");
    return 0;
}
