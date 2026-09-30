#include "pe_vsync.h"
#include "retail_vsync_cases.h"
typedef struct {uint32_t next,end,repeats;int failed;} TestVSyncTrace;
static uint32_t test_vsync_event(TestVSyncTrace *t,uint32_t kind,uint32_t address,uint32_t value,uint32_t extra)
{
    if(t->next>=t->end)goto fail;
    const uint32_t *e=DAY1_vsync_events[t->next];
    if(e[1]!=kind || e[2]!=address || (kind && (e[3]!=value || e[4]!=extra)))goto fail;
    uint32_t result=e[3];
    if(++t->repeats==e[0]){t->next++;t->repeats=0;}
    return result;
 fail:
    if(!t->failed)fprintf(stderr,"VSync event%u kind%u address%X value%X\n",t->next,kind,address,value);
    t->failed=1;PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
}
static uint32_t test_vsync_read(void *p,pe_addr_t a){return test_vsync_event(p,0,a,0,0);}
static void test_vsync_write(void *p,pe_addr_t a,uint32_t v){(void)test_vsync_event(p,1,a,v,0);}
static void test_vsync_bios(void *p,uint32_t table,uint32_t service,uint32_t a,uint32_t b)
{(void)test_vsync_event(p,2,(table<<8)|service,a,table==0xC0u?b:0);}
static void test_DAY1_host_vsync_contract(void)
{
    TEST("DAY1_host_vsync_contract");
    ResetTestState();HostFB_Init();
    /* Negative modes return the absolute VBlank counter (monotonic). */
    uint32_t a=HostFB_VSync(-1),b=HostFB_VSync(-1);
    ASSERT(b==a+1u,"negative-mode VBlank counter is not monotonic");
    /* A waiting mode samples the scanline timer at entry and re-latches the
     * baseline after the wait, so an immediately following query reads zero. */
    ASSERT(HostFB_VSync(0)==0u,"mode-0 entry delta when baseline current");
    ASSERT(HostFB_VSync(1)==0u,"mode-1 delta right after a wait");
    /* A mode-1 query is non-consuming: it does not re-latch the baseline, so
     * successive queries accumulate the modeled per-call scanline advance. */
    ASSERT(HostFB_VSync(1)==1u,"mode-1 query is non-consuming");
    ASSERT(HostFB_VSync(1)==2u,"mode-1 queries accumulate");
    /* Counter advanced once for each of the five calls after b. */
    ASSERT(HostFB_VSync(-1)-b==5u,"counter did not advance once per call");
    PASS();
}

static void test_DAY1_vsync(void)
{
    TEST("DAY1_vsync");
    for(unsigned i=0;i<sizeof(DAY1_vsync_cases)/sizeof(DAY1_vsync_cases[0]);i++) {
        ResetTestState();
        TestVSyncTrace trace={DAY1_vsync_cases[i].first,DAY1_vsync_cases[i].end,0,0};
        PeVSyncClock clock={&trace,test_vsync_read,test_vsync_write,test_vsync_bios};
        uint32_t result=PE_RetailVSync(DAY1_vsync_cases[i].mode,&clock);
        if(trace.failed || result!=DAY1_vsync_cases[i].result)fprintf(stderr,"VSync case%u mode%d result%X/%X\n",i,DAY1_vsync_cases[i].mode,result,DAY1_vsync_cases[i].result);
        ASSERT(!trace.failed && trace.next==trace.end && !trace.repeats,"VSync device/BIOS transcript differs from original");
        ASSERT(result==DAY1_vsync_cases[i].result,"VSync return differs from original");
        ASSERT(!PE_Port_ShouldStop()&&!g_stub_order_count,"VSync stopped or hit stub");
    }
    PASS();
}
