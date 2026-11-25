/* Exercise the compiled wrapper through real flush/register routing. */
static int CDWRAP_Quit(void) { return 1; }
static void test_CDWRAP_stopped_flush(void)
{
    TEST("CDWRAP_stopped_flush");
    for (int requested=0; requested<2; requested++) {
        CDINIT_SeedHost();
        PE_StoreU8(0x8009B558u,0xA5u);
        PE_StoreU32(0x8009B560u,0x80123456u);
        if (requested) PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
        else PE_Port_SetQuitPoll(CDWRAP_Quit);
        int result=func_8007FCFC(8u,0u);
        PE_Port_SetQuitPoll(NULL);
        ASSERT(result==0 && PE_Port_GetStopReason()==PE_PORT_STOP_HOST_QUIT &&
               PE_LoadU8(0x8009B558u)==0xA5u && PE_LoadU32(0x8009B560u)==0x80123456u,
               "stopped flush allowed command-wrapper state writes");
    }
    PASS();
}
