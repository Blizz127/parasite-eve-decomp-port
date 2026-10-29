# Shared body of the zig macOS cross toolchains (zig-x86_64-macos.cmake,
# zig-aarch64-macos.cmake): set PE_ZIG_MACOS_ARCH (x86_64 | aarch64), then
# include this file.  docs/port/MACOS.md has the full recipe.
#
# zig ships the macOS libc headers and the libSystem link stub, so neither
# an Apple SDK nor Xcode is needed; the result links only libSystem.  The
# compiler/archiver are the small wrapper scripts next to this file, which run
# "${PE_ZIG:-zig}" (set PE_ZIG when zig is not on PATH).  Keep zig's caches
# where you want them with ZIG_GLOBAL_CACHE_DIR / ZIG_LOCAL_CACHE_DIR.
if(NOT PE_ZIG_MACOS_ARCH MATCHES "^(x86_64|aarch64)$")
    message(FATAL_ERROR "PE_ZIG_MACOS_ARCH must be x86_64 or aarch64 (got '${PE_ZIG_MACOS_ARCH}')")
endif()
set(CMAKE_SYSTEM_NAME Darwin)
if(PE_ZIG_MACOS_ARCH STREQUAL "aarch64")
    set(CMAKE_SYSTEM_PROCESSOR arm64)
else()
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
endif()
set(CMAKE_C_COMPILER "${CMAKE_CURRENT_LIST_DIR}/cc-${PE_ZIG_MACOS_ARCH}-macos")
set(CMAKE_AR "${CMAKE_CURRENT_LIST_DIR}/ar" CACHE FILEPATH "zig ar" FORCE)
set(CMAKE_RANLIB "${CMAKE_CURRENT_LIST_DIR}/ranlib" CACHE FILEPATH "zig ranlib" FORCE)
# The target triple carries the architecture; no SDK, no -arch/-isysroot.
set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "zig: the -target triple selects the architecture" FORCE)
set(CMAKE_OSX_SYSROOT "" CACHE STRING "zig: no Apple SDK" FORCE)
# Never pick up the build host's libraries, headers or CMake packages.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
