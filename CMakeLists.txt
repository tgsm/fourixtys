cmake_minimum_required(VERSION 3.5)

project(n64emu CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-4")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

option(N64_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers")

find_package(fmt REQUIRED)

set(SOURCES
    src/frontend/frontend.h
    src/frontend/headless.cpp
    src/frontend/headless.h
    src/main.cpp
)

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

target_link_libraries(n64emu fmt)
