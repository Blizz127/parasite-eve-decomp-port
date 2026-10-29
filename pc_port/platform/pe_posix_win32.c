/*
 * Windows-only definitions behind pe_posix_win32.h (cmake/windows.cmake adds
 * this file to pe_field_runtime on WIN32 only), plus a one-line crash report:
 * on Linux src/port_main.c prints "[HOST] fatal <signal>: backtrace" from its
 * signal handlers, which are no-ops on Windows, so an unhandled exception
 * gets the same kind of line here before the default action runs.
 */
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00   /* GetSystemTimePreciseAsFileTime: Windows 8+ */
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <direct.h>
#include <errno.h>
#include <io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int pe_win32_clock_gettime(int clock_id, struct timespec *ts)
{
    if (!ts) { errno = EINVAL; return -1; }
    if (clock_id == CLOCK_MONOTONIC) {
        static LARGE_INTEGER freq;   /* fixed at boot; a racing first call stores the same value */
        LARGE_INTEGER now;
        if (!freq.QuadPart && (!QueryPerformanceFrequency(&freq) || freq.QuadPart <= 0)) {
            freq.QuadPart = 0;
            errno = EINVAL;
            return -1;
        }
        QueryPerformanceCounter(&now);
        ts->tv_sec = (time_t)(now.QuadPart / freq.QuadPart);
        ts->tv_nsec = (long)(((now.QuadPart % freq.QuadPart) * INT64_C(1000000000)) / freq.QuadPart);
        return 0;
    }
    if (clock_id == CLOCK_REALTIME) {
        FILETIME ft;
        ULARGE_INTEGER t;
        GetSystemTimePreciseAsFileTime(&ft);
        t.LowPart = ft.dwLowDateTime;
        t.HighPart = ft.dwHighDateTime;
        t.QuadPart -= UINT64_C(116444736000000000);   /* 1601-01-01 -> 1970-01-01 in 100 ns */
        ts->tv_sec = (time_t)(t.QuadPart / UINT64_C(10000000));
        ts->tv_nsec = (long)((t.QuadPart % UINT64_C(10000000)) * 100u);
        return 0;
    }
    errno = EINVAL;
    return -1;
}

int pe_win32_mkdir(const char *path)
{
    return _mkdir(path);
}

int pe_win32_backtrace(void **frames, int max)
{
    if (!frames || max <= 0) return 0;
    if (max > 62) max = 62;   /* RtlCaptureStackBackTrace limit before Vista SP1 */
    return (int)CaptureStackBackTrace(1, (DWORD)max, frames, NULL);
}

/* "  #i 0x<addr> <module>+0x<offset>" per frame: the offset symbolizes with
 * llvm-symbolizer against the PDB that zig writes next to the .exe. */
static void write_frame(int fd, int i, const void *addr)
{
    char line[400], path[MAX_PATH];
    HMODULE mod = NULL;
    const char *name = "?";
    uintptr_t base = 0;
    int n;
    if (addr && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                   GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   (LPCSTR)addr, &mod) && mod) {
        DWORD k = GetModuleFileNameA(mod, path, (DWORD)sizeof(path));
        if (k > 0 && k < sizeof(path)) {
            const char *s = strrchr(path, '\\');
            name = s ? s + 1 : path;
        }
        base = (uintptr_t)mod;
    }
    if (base)
        n = snprintf(line, sizeof(line), "  #%d 0x%016llx %s+0x%llx\n", i,
                     (unsigned long long)(uintptr_t)addr, name,
                     (unsigned long long)((uintptr_t)addr - base));
    else
        n = snprintf(line, sizeof(line), "  #%d 0x%016llx\n", i,
                     (unsigned long long)(uintptr_t)addr);
    if (n > 0) (void)_write(fd, line, (unsigned)(n < (int)sizeof(line) ? n : (int)sizeof(line) - 1));
}

void pe_win32_backtrace_symbols_fd(void *const *frames, int n, int fd)
{
    for (int i = 0; frames && i < n; i++) write_frame(fd, i, frames[i]);
}

static LONG WINAPI crash_filter(EXCEPTION_POINTERS *ep)
{
    static volatile LONG once;
    char line[256];
    int n;
    void *frames[48];
    int count;
    if (InterlockedExchange(&once, 1)) return EXCEPTION_CONTINUE_SEARCH;
    if (ep && ep->ExceptionRecord) {
        const EXCEPTION_RECORD *r = ep->ExceptionRecord;
        n = snprintf(line, sizeof(line), "[HOST] fatal exception 0x%08lX at 0x%016llx",
                     (unsigned long)r->ExceptionCode,
                     (unsigned long long)(uintptr_t)r->ExceptionAddress);
        if (n > 0) (void)_write(2, line, (unsigned)n);
        if (r->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && r->NumberParameters >= 2) {
            n = snprintf(line, sizeof(line), " (%s 0x%016llx)",
                         r->ExceptionInformation[0] == 0 ? "read" :
                         r->ExceptionInformation[0] == 8 ? "execute" : "write",
                         (unsigned long long)r->ExceptionInformation[1]);
            if (n > 0) (void)_write(2, line, (unsigned)n);
        }
        (void)_write(2, ": backtrace\n", 12);
        write_frame(2, 0, r->ExceptionAddress);
#if defined(_M_X64) || defined(__x86_64__)
        /* A call through a NULL pointer faults at 0 with the caller's
         * return address still at [rsp]. */
        if (ep->ContextRecord && !IsBadReadPtr((const void *)(uintptr_t)ep->ContextRecord->Rsp, 8)) {
            (void)_write(2, "  [rsp]:\n", 9);
            write_frame(2, 0, *(void *const *)(uintptr_t)ep->ContextRecord->Rsp);
        }
#endif
    }
    count = (int)CaptureStackBackTrace(0, 48, frames, NULL);
    pe_win32_backtrace_symbols_fd(frames, count, 2);
    return EXCEPTION_CONTINUE_SEARCH;   /* then the default action, as on Linux */
}

__attribute__((constructor)) static void pe_win32_prelude(void)
{
    SetUnhandledExceptionFilter(crash_filter);
}
#endif /* _WIN32 */
