/* audio3: the harness VBlank clock (PE_PAD_SCRIPT_CLOCK=vblank, PE_SHOT_VSYNC)
 * reads PE_GPU_VSyncQuery(): it advances once per emulated VBlank, not once
 * per VSync() call.  "[FB] vsyncs" (HostFB_GetState) is the call count. */
static void test_HOSTFB_vblank_clock_vs_vsync_calls(void)
{
    int calls0, calls;
    uint32_t vb0;
    TEST("HOSTFB_vblank_clock_vs_vsync_calls");
    ResetTestState();
    HostFB_GetState(&calls0, NULL, NULL, NULL);
    vb0 = PE_GPU_VSyncQuery();
    for (int i = 0; i < 5; i++) (void)HostFB_VSync(-1);   /* counter queries */
    (void)HostFB_VSync(1);                                  /* scanline query */
    HostFB_GetState(&calls, NULL, NULL, NULL);
    ASSERT(calls == calls0 + 6, "every VSync() call is counted");
    ASSERT(PE_GPU_VSyncQuery() == vb0, "queries are not VBlanks");
    (void)HostFB_VSync(0);
    ASSERT(PE_GPU_VSyncQuery() == vb0 + 1u, "VSync(0) waits one VBlank");
    (void)HostFB_VSync(2);
    ASSERT(PE_GPU_VSyncQuery() == vb0 + 3u, "VSync(2) waits two VBlanks");
    HostFB_GetState(&calls, NULL, NULL, NULL);
    ASSERT(calls == calls0 + 8, "two more calls, three more VBlanks");
    PASS();
}
