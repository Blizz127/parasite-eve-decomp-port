/* Isolated guest-RAM range check. The translation control allocates only the
 * host test RAM; MMIO aborts if reached. Not registered in the held build. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pe_guest_ram.h"
#include "pe_mmio.h"

int PE_MMIO_InShadowBlock(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void *PE_MMIO_Translate(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }

static int ranges(void)
{
    if (!PE_RangeIsRam(PE_RAM_BASE, 0) || !PE_RangeIsRam(PE_RAM_BASE, PE_RAM_SIZE) ||
        !PE_RangeIsRam(PE_RAM_END - 1u, 0) || !PE_RangeIsRam(PE_RAM_END - 1u, 1) ||
        PE_RangeIsRam(PE_RAM_END, 0) || PE_RangeIsRam(PE_RAM_END, 1) ||
        PE_RangeIsRam(PE_RAM_END - 1u, 2) || PE_RangeIsRam(PE_RAM_BASE - 1u, 1) ||
        PE_RangeIsRam(PE_RAM_BASE, (size_t)PE_RAM_SIZE + 1u)) abort();
    if (PE_RangeIsRam(PE_RAM_BASE, SIZE_MAX) ||
        PE_RangeIsRam(PE_RAM_END - 1u, SIZE_MAX - 1u)) {
        puts("oversized guest RAM range was accepted");
        return 1;
    }
    puts("valid, boundary and oversized guest RAM ranges handled");
    return 0;
}

static int translation(void)
{
    PE_RamInit();
    uint8_t *first = PE_Translate(PE_RAM_BASE, PE_RAM_SIZE);
    uint8_t *last = PE_Translate(PE_RAM_END - 1u, 1);
    if (last != first + PE_RAM_SIZE - 1u || PE_Translate(0, 1) != first ||
        PE_Translate(0xA01FFFFFu, 1) != last) abort();
    pid_t pid = fork();
    if (pid < 0) abort();
    if (pid == 0) {
        struct rlimit limit = {0, 0};
        if (setrlimit(RLIMIT_CORE, &limit) != 0) _exit(2);
        (void)PE_Translate(PE_RAM_BASE, SIZE_MAX);
        _exit(0);
    }
    int status;
    if (waitpid(pid, &status, 0) != pid) abort();
    PE_RamDestroy();
    if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGABRT) {
        printf("oversized translation was not rejected: status=%d\n", status);
        return 1;
    }
    puts("oversized guest translation aborted; valid edges translated");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) abort();
    if (!strcmp(argv[1], "ranges")) return ranges();
    if (!strcmp(argv[1], "translation")) return translation();
    abort();
}
