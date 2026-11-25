/* Synthetic WAV-only lifecycle; never opens an audio device. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "host_audio.h"
/* The headless fixture must never request a device or load SDL. */
void *HostSDL_Library(const char **source) { (void)source; abort(); }
static uint32_t le32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static int check_wav(const char *path,const int16_t *samples,unsigned frames) {
    unsigned char data[128]; FILE *f=fopen(path,"rb"); if(!f) return 0;
    size_t n=fread(data,1,sizeof data,f); int failed=ferror(f);
    if(fclose(f)!=0 || failed) return 0;
    if(n!=44u+frames*4u || memcmp(data,"RIFF",4) || memcmp(data+36,"data",4) ||
       le32(data+4)!=36u+frames*4u || le32(data+40)!=frames*4u) return 0;
    for(unsigned i=0;i<frames*2u;i++) {
        uint16_t sample=(uint16_t)samples[i];
        if(data[44u+i*2u]!=(uint8_t)sample || data[45u+i*2u]!=(uint8_t)(sample>>8)) return 0;
    }
    return 1;
}
int main(void) {
    char dir[]="build/codex-audio-reopen-20261108/case-XXXXXX",a[256],b[256],bad[256];
    const int16_t first[]={1,-2,300,-400}, second[]={42,-43};
    int rc=1,created_a=0,created_b=0;
    if(!mkdtemp(dir)) return 1;
    if(snprintf(a,sizeof a,"%s/a.wav",dir)>=(int)sizeof a || snprintf(b,sizeof b,"%s/b.wav",dir)>=(int)sizeof b ||
       snprintf(bad,sizeof bad,"%s/missing/fail.wav",dir)>=(int)sizeof bad) goto cleanup;
    unsetenv("PE_AUDIO_WAV");
    if(HostAudio_Open(0)!=0 || HostAudio_Open(0)!=0) { fputs("no-sink open returned success\n",stderr);goto cleanup; }
    if(setenv("PE_AUDIO_WAV",bad,1)!=0 || HostAudio_Open(0)!=0 || HostAudio_Open(0)!=0) goto cleanup;
    if(setenv("PE_AUDIO_WAV",a,1)!=0 || HostAudio_Open(0)!=1) goto cleanup;
    created_a=1;
    if(HostAudio_Open(0)!=1) goto cleanup;
    HostAudio_Submit(first,2);HostAudio_Close();
    if(!check_wav(a,first,2)) { fputs("first WAV mismatch\n",stderr);goto cleanup; }
    if(setenv("PE_AUDIO_WAV",b,1)!=0 || HostAudio_Open(0)!=1) goto cleanup;
    created_b=1;HostAudio_Submit(second,1);HostAudio_Close();
    if(!check_wav(b,second,1) || !check_wav(a,first,2)) { fputs("reopened WAV mismatch\n",stderr);goto cleanup; }
    HostAudio_Close();rc=0;
cleanup:
    HostAudio_Close();unsetenv("PE_AUDIO_WAV");
    if(created_b && unlink(b)!=0) rc=1;
    if(created_a && unlink(a)!=0) rc=1;
    if(rmdir(dir)!=0) rc=1;
    if(rc) return rc;
    puts("synthetic headless audio open/close/reopen controls pass");return 0;
}
