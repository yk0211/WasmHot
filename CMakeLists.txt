cmake_minimum_required(VERSION 3.20)
project(WasmHot)

set(CMAKE_C_COMPILER clang-21)
set(CMAKE_CXX_COMPILER clang++-21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_Declare(
    cityhash
    GIT_REPOSITORY https://github.com/google/cityhash.git
    GIT_TAG        f5dc54147fcce12cefd16548c8e760d68ac04226
)

FetchContent_GetProperties(cityhash)
if(NOT cityhash_POPULATED)
    FetchContent_Populate(cityhash)
endif()

set(CITYHASH_SOURCE_DIR ${cityhash_SOURCE_DIR})
set(CITYHASH_BUILD_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cityhash-build)

file(WRITE ${CITYHASH_BUILD_DIR}/config.h
"#ifndef CITYHASH_CONFIG_H_\n"
"#define CITYHASH_CONFIG_H_\n"
"#define STDC_HEADERS 1\n"
"#define HAVE_STDINT_H 1\n"
"#define HAVE_STDLIB_H 1\n"
"#define HAVE_SYS_TYPES_H 1\n"
"#define HAVE_ENDIAN_H 1\n"
"#define HAVE_BYTESWAP_H 1\n"
"#define HAVE_SYS_MMAN_H 1\n"
"#define HAVE_UNISTD_H 1\n"
"#define HAVE_INTTYPES_H 1\n"
"/* #undef WORDS_BIGENDIAN */\n"
"#define HAVE_PEROR 1\n"
"#define HAVE___INT128 1\n"
"#define HAVE_DECL_BSWAP_64 1\n"
"#define HAVE_DECL___BUILTIN_EXPECT 1\n"
"#define HAVE_DECL___BUILTIN_CTZLL 1\n"
"#define SIZEOF_VOID_P 8\n"
"#define SIZEOF_LONG_INT 8\n"
"#endif\n"
)

add_library(cityhash STATIC
    ${CITYHASH_SOURCE_DIR}/src/city.cc
    ${CITYHASH_SOURCE_DIR}/src/city.h
    ${CITYHASH_SOURCE_DIR}/src/citycrc.h
)

target_include_directories(cityhash PUBLIC
    ${CITYHASH_SOURCE_DIR}/src
    ${CITYHASH_BUILD_DIR}
)

find_package(WasmEdge QUIET)

add_library(WasmHotLib STATIC
    src/core/object.cpp
    src/core/component_storage.cpp
    src/core/schema_manager.cpp
    src/core/migration_engine.cpp
    src/core/object_registry.cpp
    src/core/module_manager.cpp
    src/actor/actor.cpp
    src/actor/actor_runtime.cpp
    src/actor/battle_actor.cpp
    src/actor/room_actor.cpp
    src/actor/mail_actor.cpp
    src/network/gateway.cpp
    src/plugin/wasmedge_plugin.cpp
)

target_include_directories(WasmHotLib PUBLIC src)

if(WasmEdge_FOUND)
    target_link_libraries(WasmHotLib PUBLIC wasmedge)
else()
    find_path(WASMEDGE_INCLUDE_DIR NAMES wasmedge/wasmedge.h
        PATHS
            $ENV{WASMEDGE_INCLUDE_DIR}
            $ENV{WASMEDGE_ROOT}/include
            $ENV{HOME}/.wasmedge/include
            /usr/local/include
            /usr/include
    )
    find_library(WASMEDGE_LIBRARY NAMES wasmedge
        PATHS
            $ENV{WASMEDGE_LIB_DIR}
            $ENV{WASMEDGE_ROOT}/lib
            $ENV{HOME}/.wasmedge/lib
            /usr/local/lib
            /usr/lib
    )
    if(WASMEDGE_INCLUDE_DIR AND WASMEDGE_LIBRARY)
        target_include_directories(WasmHotLib PUBLIC ${WASMEDGE_INCLUDE_DIR})
        target_link_libraries(WasmHotLib PUBLIC ${WASMEDGE_LIBRARY})
        message(STATUS "Using WasmEdge fallback: ${WASMEDGE_LIBRARY}")
    else()
        message(FATAL_ERROR "WasmEdge not found. Please set WASMEDGE_ROOT or install WasmEdge.")
    endif()
endif()
target_link_libraries(WasmHotLib PUBLIC cityhash dl pthread)

add_executable(WasmHotServer src/main.cpp)
target_link_libraries(WasmHotServer PRIVATE WasmHotLib)
