# macOS parts of the sdl2 host backend (included from pc_port/cmake/sdl.cmake
# when APPLE).  docs/port/MACOS.md has the cross-build recipe (zig toolchains
# pc_port/cmake/zig-x86_64-macos.cmake / zig-aarch64-macos.cmake).
#
# - platform/host_pad.c is the Linux joystick API (<linux/joystick.h>) and is
#   not built.  host_window_sdl.c then provides the HostPad_* entry points
#   (PE_HOST_PAD_SDL_ONLY) over the SDL2 GameController backend
#   (host_pad_sdl.c), which the dev-menu combo (host_dev_menu.c) reads as on
#   Linux.
# - The only linked library is libSystem (dl, m and pthread are part of it).
#   SDL2 is found at run time: libSDL2-2.0.0.dylib next to the executable or
#   in ../Frameworks, the dyld search path, Homebrew/MacPorts, SDL2.framework,
#   or PE_SDL2_LIB.
get_target_property(_pe_mac_srcs parasite-eve-port SOURCES)
list(REMOVE_ITEM _pe_mac_srcs platform/host_pad.c)
set_property(TARGET parasite-eve-port PROPERTY SOURCES ${_pe_mac_srcs})
target_compile_definitions(parasite-eve-port PRIVATE PE_HOST_PAD_SDL_ONLY=1)
# game/decomp_hand/akao_tick_port.c references PE_AudioDriver_CallGuest as a
# weak symbol that only the test oracle defines (the port binary is native
# only and checks the address for NULL).  ELF resolves an undefined weak to 0;
# the Mach-O linker refuses it unless the symbol may be looked up at load
# time, where dyld binds a missing weak import to NULL.  zig's linker has no
# per-symbol -U, so this allows dynamic lookup; the built binary's only
# flat-lookup import is that one weak symbol (docs/port/MACOS.md shows the
# check).  Dropping this needs a non-weak hook in akao_tick_port.c.
target_link_options(parasite-eve-port PRIVATE "LINKER:-undefined,dynamic_lookup")
unset(_pe_mac_srcs)
