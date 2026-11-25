/* Isolated reverse-translation check. Only fresh host test RAM and the
 * backend's scratchpad are used; MMIO aborts if reached. Not registered in
 * the held production build configuration. */
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "pe_guest_ram.h"
#include "pe_mmio.h"

int PE_MMIO_InShadowBlock(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void *PE_MMIO_Translate(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }

static void expect_invalid(const void *pointer)
{
    pid_t pid = fork();
    if (pid < 0) abort();
    if (pid == 0) {
        struct rlimit limit = {0, 0};
        if (setrlimit(RLIMIT_CORE, &limit) != 0) _exit(2);
        (void)PE_HostToGuest(pointer);
        _exit(0);
    }
    int status;
    if (waitpid(pid, &status, 0) != pid || !WIFSIGNALED(status) ||
        WTERMSIG(status) != SIGABRT) abort();
}

int main(void)
{
    PE_RamInit();
    uint8_t *ram = PE_Translate(PE_RAM_BASE, PE_RAM_SIZE);
    uint8_t *scratch = PE_Translate(PE_SCRATCHPAD_BASE, PE_SCRATCHPAD_SIZE);
    if (PE_HostToGuest(NULL) != 0 || PE_HostToGuest(ram) != PE_RAM_BASE ||
        PE_HostToGuest(ram + PE_RAM_SIZE - 1u) != PE_RAM_END - 1u ||
        PE_HostToGuest(scratch) != PE_SCRATCHPAD_BASE ||
        PE_HostToGuest(scratch + PE_SCRATCHPAD_SIZE - 1u) !=
            PE_SCRATCHPAD_BASE + PE_SCRATCHPAD_SIZE - 1u) abort();
    int foreign = 0;
    expect_invalid(scratch + PE_SCRATCHPAD_SIZE);
    expect_invalid(&foreign);
    PE_RamDestroy();
    puts("null, RAM, scratchpad and invalid host-pointer controls pass");
    return 0;
}
