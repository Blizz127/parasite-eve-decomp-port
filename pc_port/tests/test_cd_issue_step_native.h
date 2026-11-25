/* Device-disabled Stop/Pause must close the active host stream even on stop. */
static void test_CDISSUE_native_stream_stop(void)
{
    TEST("CDISSUE_native_stream_stop");
    const int commands[]={8,9,264};
    for(unsigned i=0;i<3;i++)for(int stopped=0;stopped<2;stopped++){
        CDINIT_SeedHost();
        PE_StoreU32(0x80150000u,0x00000200u);
        PE_StrFeed_Open(0x80150000u,0u);
        ASSERT(PE_StrFeed_Active(),"host stream fixture did not open");
        if(stopped)PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
        int result=func_80080DC4(commands[i],0x80150000u,0u);
        ASSERT(result==1 && !PE_StrFeed_Active(),"Stop/Pause did not close host stream");
        ASSERT(PE_Port_GetStopReason()==(stopped?PE_PORT_STOP_HOST_QUIT:PE_PORT_STOP_NONE),
               "Stop/Pause changed existing stop reason");
        ASSERT(PE_LoadU32(0x800A3608u)==0u,"host Stop/Pause unexpectedly queued a command");
    }
    PASS();
}
