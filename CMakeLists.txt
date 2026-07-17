cmake_minimum_required(VERSION 3.20)
project(WasmHot)

set(CMAKE_C_COMPILER clang-17)
set(CMAKE_CXX_COMPILER clang++-17)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(WasmEdge QUIET)

add_library(WasmHotLib STATIC
    src/core/object.cpp
    src/core/component_storage.cpp
    src/core/schema_manager.cpp
    src/core/migration_engine.cpp
    src/core/object_registry.cpp
    src/core/module_manager.cpp
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
    target_compile_definitions(WasmHotLib PUBLIC WASM_HOT_USE_WASMEDGE=1)
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
        target_compile_definitions(WasmHotLib PUBLIC WASM_HOT_USE_WASMEDGE=1)
        message(STATUS "Using WasmEdge fallback: ${WASMEDGE_LIBRARY}")
    else()
        message(FATAL_ERROR "WasmEdge not found. Please set WASMEDGE_ROOT or install WasmEdge.")
    endif()
endif()
target_link_libraries(WasmHotLib PUBLIC dl pthread)

# Main server executable
add_executable(WasmHotServer src/main.cpp)
target_link_libraries(WasmHotServer PRIVATE WasmHotLib)

# Tests
enable_testing()
add_subdirectory(test)
