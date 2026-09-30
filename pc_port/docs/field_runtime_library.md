# `libpe_field_runtime.a` consumer contract

The `pe_field_runtime` CMake target is the reusable native boundary for the
currently translated Parasite Eve runtime. It deliberately excludes the CLI
entry point and X11 adapter. Link it from another target with:

```cmake
target_link_libraries(my_consumer PRIVATE pe_field_runtime)
```

The target publishes the existing include roots and the established
`PE_PORT_HEADLESS`, `PE_PORT_FB_WIDTH=320`, and `PE_PORT_FB_HEIGHT=240`
definitions. Consumers may use the existing component headers under
`include/`, `platform/`, and `bootstrap/`. A consumer that reaches translated
code using the host trace must provide:

```c
void Trace_Direct(const char *event);
```

The library remains single-instance because retail globals and guest RAM are
process-global. The established host initialization order is:

```text
PE_RamInit
PE_Callback_Init
Bootstrap_Init
HostFB_Init
PE_Port_RunControlReset
```

`PE_RamDestroy` releases guest RAM. Disc ownership remains with the host:
open with `PE_Disc_Open`, publish with `PE_Disc_SetActive`, load the verified
guest executable with `PE_GuestImage_LoadExe`, adopt its retail overlay
authority with `PE_Globals_AdoptRetailImage`, and close with `PE_Disc_Close`.
The production CLI is the authoritative worked example.

This interface packages only behavior already present and tested. In
particular it does not provide a generic scene scheduler, fabricate m0360i,
write `persist[0] |= 4`, or claim a complete Day 1 field runtime. The current
strict frontier is `func_80081314_func_8007F0C8_cut`, immediately after the
authenticated libpress MDEC reset, table submissions, channel-1
`DecDCToutCallback` registration, 64-record pool initialization, and
stream-control setup, CD idle-wait, blocking CdlSetloc, and CdlReadS callback
registration.
