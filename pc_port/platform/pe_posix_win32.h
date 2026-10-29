/*
 * Windows-only forced-include prelude.  cmake/windows.cmake compiles every
 * pc_port target with `-include pe_posix_win32.h` on WIN32; no Linux build
 * ever sees this file.
 *
 * It supplies the few POSIX interfaces that shared runtime files call and
 * that the MinGW-w64 headers shipped with zig do not declare (zig has no
 * winpthreads, so <time.h> has no clock_gettime), so those files compile
 * unchanged:
 *   clock_gettime(CLOCK_MONOTONIC | CLOCK_REALTIME, ts)
 *       QueryPerformanceCounter / GetSystemTimePreciseAsFileTime
 *   mkdir(path, mode)                 _mkdir(path); the mode is ignored
 *   backtrace / backtrace_symbols_fd  CaptureStackBackTrace; addresses only
 *       (<execinfo.h> itself is an empty generated header in the build tree)
 * The definitions are in pe_posix_win32.c.  This header includes no system
 * header on purpose: <windows.h> macros must not reach the translated
 * retail code.
 */
#ifndef PE_POSIX_WIN32_H
#define PE_POSIX_WIN32_H
#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

struct timespec;

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

int pe_win32_clock_gettime(int clock_id, struct timespec *ts);
int pe_win32_mkdir(const char *path);
int pe_win32_backtrace(void **frames, int max);
void pe_win32_backtrace_symbols_fd(void *const *frames, int n, int fd);

#define clock_gettime(clock_id, ts) pe_win32_clock_gettime((clock_id), (ts))
/* POSIX mkdir(path, mode) and the one-argument CRT form both land here. */
#define mkdir(path, ...) pe_win32_mkdir(path)
#define backtrace(frames, max) pe_win32_backtrace((frames), (max))
#define backtrace_symbols_fd(frames, n, fd) pe_win32_backtrace_symbols_fd((frames), (n), (fd))

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */
#endif /* PE_POSIX_WIN32_H */
