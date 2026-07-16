cmake_minimum_required(VERSION 3.20)
project(WasmHot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WASM_HOT_USE_WASMTIME "Enable Wasmtime WASM runtime integration" OFF)

add_executable(WasmHotServer
    src/main.cpp
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
    src/plugin/native_plugin.cpp
    src/plugin/wasmtime_plugin.cpp
)

target_include_directories(WasmHotServer PRIVATE src)

if(WASM_HOT_USE_WASMTIME)
    find_package(wasmtime REQUIRED)
    target_link_libraries(WasmHotServer PRIVATE wasmtime)
    target_compile_definitions(WasmHotServer PRIVATE WASM_HOT_USE_WASMTIME=1)
endif()

if(UNIX)
    target_link_libraries(WasmHotServer PRIVATE dl pthread)
elseif(WIN32)
    target_link_libraries(WasmHotServer PRIVATE ws2_32)
endif()
