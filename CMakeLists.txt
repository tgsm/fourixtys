cmake_minimum_required(VERSION 3.5)

project(fourixtys CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-4")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(fmt REQUIRED)

option(FOURIXTYS_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers")

set(FOURIXTYS_FRONTEND "SDL2" CACHE STRING "The frontend fourixtys will run on")
set_property(CACHE FOURIXTYS_FRONTEND PROPERTY STRINGS "SDL2" "Headless")

set(SOURCES
    src/common/bits.h
    src/common/defines.h
    src/common/logging.h
    src/common/types.h
    src/frontend/frontend.h
    src/cop0.cpp
    src/cop0.h
    src/cop1.cpp
    src/cop1.h
    src/gamepak.cpp
    src/gamepak.h
    src/main.cpp
    src/mi.cpp
    src/mi.h
    src/mmu.cpp
    src/mmu.h
    src/n64.cpp
    src/n64.h
    src/pi.cpp
    src/pi.h
    src/pif.cpp
    src/pif.h
    src/si.cpp
    src/si.h
    src/vi.cpp
    src/vi.h
    src/vr4300.cpp
    src/vr4300.h
)

if (${FOURIXTYS_FRONTEND} MATCHES "SDL2")
    set(SOURCES ${SOURCES} "src/frontend/sdl.cpp" "src/frontend/sdl.h")
else()
    set(SOURCES ${SOURCES} "src/frontend/headless.cpp" "src/frontend/headless.h")
endif()

add_executable(fourixtys ${SOURCES})

target_include_directories(fourixtys PRIVATE src)

target_compile_options(fourixtys PRIVATE
    -Wall
    -Wextra
    -Wshadow

    -march=native
)

if (FOURIXTYS_ENABLE_SANITIZERS)
    target_compile_options(fourixtys PRIVATE -fsanitize=undefined,address)
    target_link_libraries(fourixtys asan ubsan)
endif()

if (${FOURIXTYS_FRONTEND} MATCHES "SDL2")
    find_package(SDL2 REQUIRED)
    target_compile_definitions(fourixtys PRIVATE "FOURIXTYS_FRONTEND_SDL")
    target_link_libraries(fourixtys SDL2)
endif()

target_link_libraries(fourixtys fmt)
