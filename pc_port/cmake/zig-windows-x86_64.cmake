# CMake toolchain: Windows x64 cross build of pc_port with zig cc (clang),
# MinGW-w64 ABI against the UCRT.  No MinGW gcc, no downloads: zig ships the
# MinGW-w64 headers and import libraries.
#
#   export ZIG_GLOBAL_CACHE_DIR=... ZIG_LOCAL_CACHE_DIR=...   # zig's caches
#   cmake -S pc_port -B build/pcbuild-win64 \
#         -DCMAKE_TOOLCHAIN_FILE=pc_port/cmake/zig-windows-x86_64.cmake \
#         -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
#   cmake --build build/pcbuild-win64 --target parasite-eve-port
#
# The Windows build settings themselves live in cmake/windows.cmake, which
# CMakeLists.txt includes for every WIN32 build.  docs/port/WINDOWS.md.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")

set(PE_ZIG_TARGET "x86_64-windows-gnu" CACHE STRING "zig target triple")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES PE_ZIG_TARGET)
# CMAKE_C_FLAGS also reach every link line, so this is the only place the
# target is named.
set(CMAKE_C_FLAGS_INIT "-target ${PE_ZIG_TARGET}")

# Never pick up the build host's libraries or headers for the target.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
