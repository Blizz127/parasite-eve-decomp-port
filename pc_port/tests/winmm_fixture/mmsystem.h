#ifndef PE_FAKE_WINMM_MMSYSTEM_H
#define PE_FAKE_WINMM_MMSYSTEM_H
#include "windows.h"
typedef void *HWAVEOUT;
typedef unsigned MMRESULT;
typedef struct { uint16_t wFormatTag,nChannels; uint32_t nSamplesPerSec,nAvgBytesPerSec; uint16_t nBlockAlign,wBitsPerSample,cbSize; } WAVEFORMATEX;
typedef struct { LPSTR lpData; DWORD dwBufferLength,dwBytesRecorded; DWORD_PTR dwUser; DWORD dwFlags,dwLoops; void *lpNext; DWORD_PTR reserved; } WAVEHDR;
#define WAVE_FORMAT_PCM 1u
#define WAVE_MAPPER ((unsigned)-1)
#define CALLBACK_EVENT 0x50000u
#define MMSYSERR_NOERROR 0u
#define WHDR_PREPARED 2u
#define WHDR_INQUEUE 16u
MMRESULT waveOutOpen(HWAVEOUT *,unsigned,const WAVEFORMATEX *,DWORD_PTR,DWORD_PTR,DWORD);
unsigned waveOutGetNumDevs(void);
MMRESULT waveOutPrepareHeader(HWAVEOUT,WAVEHDR *,unsigned);
MMRESULT waveOutUnprepareHeader(HWAVEOUT,WAVEHDR *,unsigned);
MMRESULT waveOutWrite(HWAVEOUT,WAVEHDR *,unsigned);
MMRESULT waveOutReset(HWAVEOUT);
MMRESULT waveOutClose(HWAVEOUT);
#endif
