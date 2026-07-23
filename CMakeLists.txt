cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER clang-21)
set(CMAKE_CXX_COMPILER clang++-21)

project(WasmHot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Reproducible build settings
set(CMAKE_C_COMPILER_LAUNCHER "")
set(CMAKE_CXX_COMPILER_LAUNCHER "")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type for reproducible builds" FORCE)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -frandom-seed=repro")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frandom-seed=repro")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffile-prefix-map=${CMAKE_BINARY_DIR}=.")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffile-prefix-map=${CMAKE_BINARY_DIR}=.")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdebug-prefix-map=${CMAKE_BINARY_DIR}=.")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdebug-prefix-map=${CMAKE_BINARY_DIR}=.")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--build-id=sha1")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--build-id=sha1")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--build-id=sha1")

# Deterministic static archives
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> Drc <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Drc <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -D <TARGET>")
set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -D <TARGET>")

include(FetchContent)
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.1
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        8806a6803cde7054c3049d3666d3ec36786568c5  # asio-1-38-2
)
FetchContent_MakeAvailable(asio)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.6.2
)
FetchContent_MakeAvailable(cli11)

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

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cityhash_config.h.in
    ${CITYHASH_BUILD_DIR}/config.h
    COPYONLY
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

# jemalloc 5.3.1
include(ExternalProject)
find_program(MAKE_EXECUTABLE NAMES make gmake REQUIRED)

set(JEMALLOC_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/jemalloc-install)

ExternalProject_Add(
    jemalloc_project
    URL https://github.com/jemalloc/jemalloc/releases/download/5.3.1/jemalloc-5.3.1.tar.bz2
    URL_HASH SHA256=3826bc80232f22ed5c4662f3034f799ca316e819103bdc7bb99018a421706f92
    PREFIX         ${CMAKE_CURRENT_BINARY_DIR}/jemalloc
    INSTALL_DIR    ${JEMALLOC_INSTALL_DIR}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --disable-shared --enable-static --disable-doc
    BUILD_COMMAND     ${MAKE_EXECUTABLE} -j4
    INSTALL_COMMAND   ${MAKE_EXECUTABLE} install
    BUILD_IN_SOURCE   1
    LOG_CONFIGURE 1
    LOG_BUILD     1
    LOG_INSTALL   1
)

add_library(jemalloc INTERFACE)
add_dependencies(jemalloc jemalloc_project)
target_link_libraries(jemalloc INTERFACE ${JEMALLOC_INSTALL_DIR}/lib/libjemalloc.a)
target_include_directories(jemalloc INTERFACE ${JEMALLOC_INSTALL_DIR}/include)

find_package(WasmEdge QUIET)

add_library(WasmHotLib STATIC
    src/core/object.cpp
    src/core/component_storage.cpp
    src/core/schema_manager.cpp
    src/core/migration_engine.cpp
    src/core/object_registry.cpp
    src/core/module_manager.cpp
    src/core/logging.cpp
    src/actor/actor.cpp
    src/actor/actor_runtime.cpp
    src/actor/battle_actor.cpp
    src/actor/room_actor.cpp
    src/actor/mail_actor.cpp
    src/core/gateway.cpp
    src/plugin/wasmedge_plugin.cpp
)

target_include_directories(WasmHotLib PUBLIC src)
target_include_directories(WasmHotLib PUBLIC ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(WasmHotLib PUBLIC ASIO_STANDALONE)

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
target_link_libraries(WasmHotLib PUBLIC cityhash dl pthread spdlog::spdlog yaml-cpp::yaml-cpp CLI11::CLI11 jemalloc)

add_executable(WasmHotServer src/main.cpp)
target_link_libraries(WasmHotServer PRIVATE WasmHotLib)

# -----------------------------------------------------------------------------
# Build-type-specific compile and link options.
# -----------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build: enabling AddressSanitizer and GDB-friendly flags")
    target_compile_options(WasmHotLib PUBLIC
        -O0
        -g3
        -fsanitize=address
        -fno-omit-frame-pointer
  
    )
    target_link_options(WasmHotServer PRIVATE
        -fsanitize=address
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "RelWithDebInfo build: enabling optimizations with debug info and lld linker")
    target_compile_options(WasmHotLib PUBLIC
        -O2
        -g
        -fno-omit-frame-pointer
    )
    target_link_options(WasmHotServer PRIVATE
        -fuse-ld=lld
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build: enabling O3, ThinLTO, dead-code elimination and stack protection")
    target_compile_options(WasmHotLib PUBLIC
        -O3
        -flto=thin
        -ffunction-sections
        -fdata-sections
        -fno-omit-frame-pointer
        -fstack-protector-strong
    )
    target_link_options(WasmHotServer PRIVATE
        -flto=thin
        -Wl,--gc-sections
        -Wl,--icf=all
        -Wl,-z,relro,-z,now
    )
endif()
