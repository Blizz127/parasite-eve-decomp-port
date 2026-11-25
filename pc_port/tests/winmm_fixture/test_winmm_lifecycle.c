/* Synthetic WinMM open/close lifecycle (fake Win32/WinMM API; the fake writer
 * thread never runs).  No audio device, user files or saves.
 * Since 2026-11-09 it asserts the fixed contract (shared with host_audio.c):
 * a failed open does not later report success, a reopen after close honours a
 * new PE_AUDIO_WAV, and a confirmed writer join releases everything once.
 * test_winmm_timeout_contract.c covers the unconfirmed-join case. */
#define _POSIX_C_SOURCE 200809L
#ifndef _WIN32
#define _WIN32 1
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../platform/host_audio_winmm.c"
static int thread_token,event_token,device_token;
static DWORD wait_result;
static unsigned wait_seen,thread_close,device_reset,device_close,event_close,mutex_delete;
HANDLE CreateEventA(void *a,int b,int c,const char *d) { (void)a;(void)b;(void)c;(void)d;return &event_token; }
HANDLE CreateThread(void *a,size_t b,DWORD (*fn)(LPVOID),LPVOID arg,DWORD f,DWORD *id) { (void)a;(void)b;(void)fn;(void)arg;(void)f;(void)id;return &thread_token; }
DWORD GetLastError(void) { return 0; }
int CloseHandle(HANDLE h) { if(h==&thread_token) thread_close++;else if(h==&event_token) event_close++;else abort();return 1; }
int SetEvent(HANDLE h) { if(h!=&event_token) abort();return 1; }
int SetThreadPriority(HANDLE h,int p) { (void)h;(void)p;return 1; }
DWORD WaitForSingleObject(HANDLE h,DWORD ms) { if(h!=&thread_token || ms!=5000) abort();wait_seen++;return wait_result; }
LONG InterlockedExchange(volatile LONG *p,LONG v) { LONG old=*p;*p=v;return old; }
void InitializeCriticalSection(CRITICAL_SECTION *p) { (void)p; }
void DeleteCriticalSection(CRITICAL_SECTION *p) { (void)p;mutex_delete++; }
void EnterCriticalSection(CRITICAL_SECTION *p) { (void)p; }
void LeaveCriticalSection(CRITICAL_SECTION *p) { (void)p; }
MMRESULT waveOutOpen(HWAVEOUT *out,unsigned d,const WAVEFORMATEX *f,DWORD_PTR c,DWORD_PTR x,DWORD y) { (void)d;(void)f;(void)c;(void)x;(void)y;*out=&device_token;return MMSYSERR_NOERROR; }
unsigned waveOutGetNumDevs(void) { return 1; }
MMRESULT waveOutPrepareHeader(HWAVEOUT h,WAVEHDR *w,unsigned n) { (void)h;(void)w;(void)n;return 0; }
MMRESULT waveOutUnprepareHeader(HWAVEOUT h,WAVEHDR *w,unsigned n) { (void)h;(void)w;(void)n;return 0; }
MMRESULT waveOutWrite(HWAVEOUT h,WAVEHDR *w,unsigned n) { (void)h;(void)w;(void)n;return 0; }
MMRESULT waveOutReset(HWAVEOUT h) { if(h!=&device_token) abort();device_reset++;return 0; }
MMRESULT waveOutClose(HWAVEOUT h) { if(h!=&device_token) abort();device_close++;return 0; }
static void fixture_reset(void) {
    memset(&D,0,sizeof D);g_open=0;g_device=0;g_device_stuck=0;g_wav_frames=0;g_wav_last_patch=0;
    wait_seen=thread_close=device_reset=device_close=event_close=mutex_delete=0;
}
static int fake_device_close(DWORD wait_status) {
    fixture_reset();unsetenv("PE_AUDIO_WAV");wait_result=wait_status;
    if(!HostAudio_Open(1) || !HostAudio_DeviceOpen()) return 0;
    int allocated=D.nbufs;HostAudio_Close();
    if(wait_seen!=1 || thread_close!=1 || device_reset!=1 || device_close!=1 || event_close!=1 || mutex_delete!=1) return 0;
    for(int i=0;i<allocated;i++) if(D.mem[i]) return 0;
    return 1;
}
static int file_exists(const char *path) {
    FILE *f=fopen(path,"rb");if(!f) return 0;fclose(f);return 1;
}
int main(void) {
    /* CTest runs in the build directory; no pre-existing scratch path needed. */
    char dir[]="winmm-lifecycle-XXXXXX",a[256],b[256];
    const int16_t frame[2]={12,-34};int rc=1,have_a=0,have_b=0;
    if(!mkdtemp(dir)) return 1;
    if(snprintf(a,sizeof a,"%s/a.wav",dir)>=(int)sizeof a || snprintf(b,sizeof b,"%s/b.wav",dir)>=(int)sizeof b) goto done;
    /* 1. Nothing to open: both calls fail (previously the second reported success). */
    fixture_reset();unsetenv("PE_AUDIO_WAV");
    if(HostAudio_Open(0)!=0) { fputs("case 1: open without sink succeeded\n",stderr);goto done; }
    if(HostAudio_Open(0)!=0) { fputs("case 1: second open without sink reported success\n",stderr);goto done; }
    /* 2. Close then reopen with a new WAV path writes the new file. */
    fixture_reset();
    if(setenv("PE_AUDIO_WAV",a,1)!=0 || HostAudio_Open(0)!=1) goto done;
    have_a=1;HostAudio_Submit(frame,1);HostAudio_Close();
    if(setenv("PE_AUDIO_WAV",b,1)!=0 || HostAudio_Open(0)!=1) goto done;
    have_b=file_exists(b);
    if(!have_b) { fputs("case 2: reopen after close ignored the new PE_AUDIO_WAV\n",stderr);goto done; }
    if(g_wav_frames!=0) { fputs("case 2: WAV frame count carried into the new file\n",stderr);goto done; }
    HostAudio_Close();unsetenv("PE_AUDIO_WAV");
    /* 3. A confirmed writer join releases everything exactly once, and the
     *    device can be opened again afterwards. */
    if(!fake_device_close(WAIT_OBJECT_0)) { fputs("case 3: cleanup after a confirmed join\n",stderr);goto done; }
    if(g_device_stuck || HostAudio_Open(1)!=1 || !HostAudio_DeviceOpen()) { fputs("case 3: device reopen after a clean close\n",stderr);goto done; }
    /* 4. Per-session device state does not leak into the next session. */
    HostAudio_Submit(frame,1);
    if(D.in_frames!=1) { fputs("case 4: submit not counted\n",stderr);goto done; }
    HostAudio_Close();
    if(HostAudio_Open(1)!=1 || D.in_frames!=0 || D.n!=0 || D.r!=0 || D.w!=0) {
        fprintf(stderr,"case 4: stale session state after reopen (in=%llu n=%u)\n",(unsigned long long)D.in_frames,(unsigned)D.n);goto done;
    }
    HostAudio_Close();
    rc=0;
done:
    HostAudio_Close();unsetenv("PE_AUDIO_WAV");
    if(have_a && unlink(a)!=0) rc=1;
    if(have_b && unlink(b)!=0) rc=1;
    if(rmdir(dir)!=0) rc=1;
    if(rc) { fputs("synthetic WinMM lifecycle failed\n",stderr);return rc; }
    puts("WinMM lifecycle: failed-open, reopen-with-new-WAV, confirmed-join cleanup and fresh-session state pass");return 0;
}
