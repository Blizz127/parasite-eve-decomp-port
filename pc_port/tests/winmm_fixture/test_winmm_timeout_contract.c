/* Regression for the fixed unconfirmed-join contract in host_audio_winmm.c.
 * This fake thread never runs; it records cleanup calls only. */
#define main existing_observation_fixture_main
#include "test_winmm_lifecycle.c"
#undef main
#define FAKE_WAIT_FAILED 0xffffffffu
int main(int argc,char **argv) {
    unsigned scenario=argc>1?(unsigned)strtoul(argv[1],NULL,10):0;
    if(scenario>2) return 2;
    fixture_reset();unsetenv("PE_AUDIO_WAV");
    wait_result=scenario==1?FAKE_WAIT_FAILED:WAIT_TIMEOUT;
    if(!HostAudio_Open(1) || !HostAudio_DeviceOpen()) return 3;
    int allocated=D.nbufs;
    HostAudio_Close();
    if(scenario==2) {
        if(HostAudio_Open(1)!=0) {
            fputs("contract failure: reopening reported success while teardown is unconfirmed\n",stderr);
            return 1;
        }
    } else {
        if(wait_seen!=1 || thread_close || device_reset || device_close || event_close || mutex_delete ||
           !D.thread || !D.done_event || !D.wo) {
            fprintf(stderr,"contract failure after %s: wait=%u thread_close=%u reset=%u device_close=%u event_close=%u mutex_delete=%u\n",
                    scenario==0?"WAIT_TIMEOUT":"WAIT_FAILED",wait_seen,thread_close,device_reset,device_close,event_close,mutex_delete);
            return 1;
        }
        for(int i=0;i<allocated;i++) if(!D.mem[i]) {
            fprintf(stderr,"contract failure: buffer %d released before join\n",i);return 1;
        }
    }
    puts("fake unsignaled-wait safety contract passes");return 0;
}
