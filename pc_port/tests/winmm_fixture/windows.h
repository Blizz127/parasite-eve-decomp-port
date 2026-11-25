#ifndef PE_FAKE_WINMM_WINDOWS_H
#define PE_FAKE_WINMM_WINDOWS_H
#include <stddef.h>
#include <stdint.h>
#define WINAPI
#define FALSE 0
#define THREAD_PRIORITY_TIME_CRITICAL 15
#define WAIT_OBJECT_0 0u
#define WAIT_TIMEOUT 258u
typedef void *HANDLE;
typedef void *LPVOID;
typedef char *LPSTR;
typedef uint32_t DWORD;
typedef uintptr_t DWORD_PTR;
typedef int32_t LONG;
typedef struct { int unused; } CRITICAL_SECTION;
HANDLE CreateEventA(void *,int,int,const char *);
HANDLE CreateThread(void *,size_t,DWORD (*)(LPVOID),LPVOID,DWORD,DWORD *);
DWORD GetLastError(void);
int CloseHandle(HANDLE);
int SetEvent(HANDLE);
int SetThreadPriority(HANDLE,int);
DWORD WaitForSingleObject(HANDLE,DWORD);
LONG InterlockedExchange(volatile LONG *,LONG);
void InitializeCriticalSection(CRITICAL_SECTION *);
void DeleteCriticalSection(CRITICAL_SECTION *);
void EnterCriticalSection(CRITICAL_SECTION *);
void LeaveCriticalSection(CRITICAL_SECTION *);
#endif
