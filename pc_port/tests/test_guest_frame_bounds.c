/* Synthetic guest-frame bounds checks; no game data or production runtime. */
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
uint32_t PE_MMIO_Load(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void PE_MMIO_Store(pe_addr_t a, size_t n, uint32_t v) { (void)a; (void)n; (void)v; abort(); }

static void expect_abort(void (*fn)(void))
{
    pid_t pid = fork();
    if (pid < 0) abort();
    if (pid == 0) {
        struct rlimit limit = {0, 0};
        if (setrlimit(RLIMIT_CORE, &limit)) _exit(2);
        fn();
        _exit(0);
    }
    int status;
    if (waitpid(pid, &status, 0) != pid || !WIFSIGNALED(status) ||
        WTERMSIG(status) != SIGABRT) abort();
}
static void overflow_enter(void) { (void)PE_GuestFrame_Enter(UINT32_MAX); }
static void overflow_leave(void) { PE_GuestFrame_Leave(PE_GUEST_FRAME_HI, UINT32_MAX); }
static void oversized_leave(void) { PE_GuestFrame_Leave(PE_GUEST_FRAME_HI, 8); }
static void oversized_enter(void) { (void)PE_GuestFrame_Enter(PE_GUEST_FRAME_HI - PE_GUEST_FRAME_LO + 1); }
static void wrong_inner_size(void)
{
    (void)PE_GuestFrame_Enter(16);
    pe_addr_t inner = PE_GuestFrame_Enter(8);
    PE_GuestFrame_Leave(inner, 16); /* within arena, but consumes the outer frame */
}
static void wrong_aligned_size(void)
{
    pe_addr_t inner = PE_GuestFrame_Enter(9);
    PE_GuestFrame_Leave(inner, 10); /* same aligned size, different request */
}
int main(int argc, char **argv)
{
    (void)argv;
    if (argc > 1) { overflow_enter(); return 0; }
    pe_addr_t a = PE_GuestFrame_Enter(9), b = PE_GuestFrame_Enter(8);
    if (a != PE_GUEST_FRAME_HI - 16 || b != a - 8) abort();
    pe_addr_t zero = PE_GuestFrame_Enter(0);
    if (zero != b) abort();
    PE_GuestFrame_Leave(zero, 0);
    PE_GuestFrame_Leave(b, 8);
    PE_GuestFrame_Leave(a, 9);
    if (PE_GuestFrame_Enter(0) != PE_GUEST_FRAME_HI) abort();
    PE_GuestFrame_Leave(PE_GUEST_FRAME_HI, 0);
    pe_addr_t full = PE_GuestFrame_Enter(PE_GUEST_FRAME_HI - PE_GUEST_FRAME_LO);
    if (full != PE_GUEST_FRAME_LO) abort();
    PE_GuestFrame_Leave(full, PE_GUEST_FRAME_HI - PE_GUEST_FRAME_LO);
    expect_abort(overflow_enter);
    expect_abort(overflow_leave);
    expect_abort(oversized_leave);
    expect_abort(oversized_enter);
    expect_abort(wrong_inner_size);
    expect_abort(wrong_aligned_size);
    puts("nested alignment and frame boundary checks pass");
    return 0;
}
