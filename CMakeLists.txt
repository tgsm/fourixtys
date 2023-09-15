cmake_minimum_required(VERSION 3.5)

project(n64emu CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-4")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(fmt REQUIRED)

option(N64_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers")

set(N64_FRONTEND "SDL2" CACHE STRING "The frontend n64emu will run on")
set_property(CACHE N64_FRONTEND PROPERTY STRINGS "SDL2" "Headless")

set(SOURCES
    src/common/bits.h
    src/common/defines.h
    src/common/logging.h
    src/common/types.h
    src/frontend/frontend.h
    src/gamepak.cpp
    src/gamepak.h
    src/main.cpp
    src/mmu.cpp
    src/mmu.h
    src/n64.cpp
    src/n64.h
    src/pi.cpp
    src/pi.h
    src/pif.cpp
    src/pif.h
    src/vi.cpp
    src/vi.h
    src/vr4300.cpp
    src/vr4300.h
)

if (${N64_FRONTEND} MATCHES "SDL2")
    set(SOURCES ${SOURCES} "src/frontend/sdl.cpp" "src/frontend/sdl.h")
else()
    set(SOURCES ${SOURCES} "src/frontend/headless.cpp" "src/frontend/headless.h")
endif()

add_executable(n64emu ${SOURCES})

target_include_directories(n64emu PRIVATE src)

target_compile_options(n64emu PRIVATE
    -Wall
    -Wextra
    -Wshadow

    -march=native
)

if (N64_ENABLE_SANITIZERS)
    target_compile_options(n64emu PRIVATE -fsanitize=undefined,address)
    target_link_libraries(n64emu asan ubsan)
endif()

if (${N64_FRONTEND} MATCHES "SDL2")
    find_package(SDL2 REQUIRED)
    target_compile_definitions(n64emu PRIVATE "N64_FRONTEND_SDL")
    target_link_libraries(n64emu SDL2)
endif()

target_link_libraries(n64emu fmt)
