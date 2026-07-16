cmake_minimum_required(VERSION 3.20)
project(WasmHot)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WASM_HOT_USE_WASMEDGE "Enable WasmEdge WASM runtime integration" OFF)

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
    src/plugin/wasmedge_plugin.cpp
)

target_include_directories(WasmHotServer PRIVATE src)

if(WASM_HOT_USE_WASMEDGE)
    find_package(WasmEdge REQUIRED)
    # Adjust the target name if your WasmEdge installation uses a different exported target.
    target_link_libraries(WasmHotServer PRIVATE wasmedge)
    target_compile_definitions(WasmHotServer PRIVATE WASM_HOT_USE_WASMEDGE=1)
endif()

if(UNIX)
    target_link_libraries(WasmHotServer PRIVATE dl pthread)
elseif(WIN32)
    target_link_libraries(WasmHotServer PRIVATE ws2_32)
endif()
