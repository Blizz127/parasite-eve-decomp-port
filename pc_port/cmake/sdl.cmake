# Host backend selection for parasite-eve-port (PE_HOST_BACKEND).
# Included once from pc_port/CMakeLists.txt, after the parasite-eve-port
# target is complete (it edits that target's SOURCES).
#
#   x11   default on Linux/BSD: platform/host_window.c (X11) and
#         platform/host_audio.c (PulseAudio).  This file then changes nothing:
#         the default build is the same as without it.
#   sdl2  platform/host_window_sdl.c and platform/host_audio_sdl.c: window,
#         keyboard and audio through SDL2, dlopened at run time (no link-time
#         SDL dependency, no SDL headers needed).  Controllers stay on
#         host_pad.c / host_pad_sdl.c and the dev menu on host_dev_menu.c.
#
# APPLE always builds sdl2 (pc_port/cmake/macos.cmake adds the macOS parts).
# WIN32 keeps its own backend (host_window_win32.c); sdl2 is not wired there.
# docs/port/SDL_BACKEND.md and docs/port/MACOS.md describe both.
set(PE_HOST_BACKEND "x11" CACHE STRING "Host window/audio backend of parasite-eve-port: x11 (default) or sdl2; APPLE always uses sdl2")
set_property(CACHE PE_HOST_BACKEND PROPERTY STRINGS x11 sdl2)
if(NOT PE_HOST_BACKEND MATCHES "^(x11|sdl2)$")
    message(FATAL_ERROR "PE_HOST_BACKEND must be x11 or sdl2 (got '${PE_HOST_BACKEND}')")
endif()
if(APPLE)
    if(NOT PE_HOST_BACKEND STREQUAL "sdl2")
        message(STATUS "pe: APPLE builds the sdl2 host backend (PE_HOST_BACKEND=${PE_HOST_BACKEND} does not apply)")
    endif()
elseif(WIN32)
    if(PE_HOST_BACKEND STREQUAL "sdl2")
        message(FATAL_ERROR "PE_HOST_BACKEND=sdl2 is not wired for Windows builds (host_window_win32.c is the Windows backend)")
    endif()
    return()
elseif(PE_HOST_BACKEND STREQUAL "x11")
    return()
endif()

get_target_property(_pe_host_srcs parasite-eve-port SOURCES)
list(REMOVE_ITEM _pe_host_srcs platform/host_window.c platform/host_audio.c)
list(APPEND _pe_host_srcs platform/host_window_sdl.c platform/host_audio_sdl.c)
set_property(TARGET parasite-eve-port PROPERTY SOURCES ${_pe_host_srcs})
if(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/macos.cmake)
endif()
get_target_property(_pe_host_srcs parasite-eve-port SOURCES)
message(STATUS "pe: host backend sdl2 (SDL2 dlopened at run time): ${_pe_host_srcs}")
unset(_pe_host_srcs)
