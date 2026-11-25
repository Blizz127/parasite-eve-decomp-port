/* Actual SDL polling path with an in-process fake function table; no SDL or devices. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../platform/host_pad_sdl.c"
static uint8_t buttons[256];
static unsigned reads[256], total_reads;
static int count, token;
static void fake_update(void) { }
static int fake_num_joysticks(void) { return 0; }
static int fake_attached(void *p) { if(p!=&token) abort();return 1; }
static int fake_button_count(void *p) { if(p!=&token) abort();return count; }
static int fake_zero(void *p) { if(p!=&token) abort();return 0; }
static uint8_t fake_button(void *p,int i) {
    if(p!=&token || i<0 || i>=count || i>=256) abort();
    reads[i]++;total_reads++;return buttons[i];
}
static void reset_reads(void) { memset(reads,0,sizeof reads);total_reads=0; }
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"failed line %d\n",__LINE__);return 1; } } while(0)
int main(void) {
    memset(&S,0,sizeof S);S.state=1;
    for(int i=0;i<HPS_MAX_DEVICES;i++) S.dev[i].id=-1;
    S.GCUpdate=fake_update;S.JSUpdate=fake_update;S.NumJoysticks=fake_num_joysticks;
    S.JSAttached=fake_attached;S.JSNumButtons=fake_button_count;
    S.JSNumAxes=fake_zero;S.JSNumHats=fake_zero;S.JSButton=fake_button;
    S.dev[0].id=7;S.dev[0].js=&token;S.dev[0].input_seen=1;
    HostPadSDL_RawMapDefaults(&S.raw);
    CHECK(HostPadSDL_RawMapParse(&S.raw,"32,255,31,3,4,5,-1,-1,6,7")==0);
    count=256;buttons[31]=buttons[32]=buttons[255]=1;HostPadSDL_Poll();
    CHECK(HostPadSDL_Held()==(HOST_PAD_CROSS|HOST_PAD_CIRCLE|HOST_PAD_SQUARE));
    CHECK(reads[31]==1 && reads[32]==1 && reads[255]==1);
    reset_reads();count=32;HostPadSDL_Poll();
    CHECK(HostPadSDL_Held()==HOST_PAD_SQUARE && !reads[32] && !reads[255]);
    reset_reads();count=300;HostPadSDL_Poll();
    CHECK(HostPadSDL_Held()==(HOST_PAD_CROSS|HOST_PAD_CIRCLE|HOST_PAD_SQUARE));
    CHECK(reads[255]==1);
    reset_reads();S.raw.square=32;HostPadSDL_Poll();
    CHECK(HostPadSDL_Held()==(HOST_PAD_CROSS|HOST_PAD_CIRCLE|HOST_PAD_SQUARE));
    CHECK(reads[32]==1); /* duplicate map is polled just once */
    reset_reads();count=-1;HostPadSDL_Poll();
    CHECK(HostPadSDL_Held()==0 && total_reads==0);
    puts("synthetic SDL raw high-index polling controls pass");return 0;
}
