enable_testing()

# 检测 wasm 编译器（优先查找 wasi-sdk 提供的 clang）
find_program(WASM_CLANG wasm32-wasi-clang PATHS /opt/wasi-sdk/bin NO_DEFAULT_PATH)
if(NOT WASM_CLANG)
    find_program(WASM_CLANG clang)
endif()
find_program(WASM_EMCC emcc)

set(WASM_COMPILER "")
if(WASM_CLANG)
    set(WASM_COMPILER ${WASM_CLANG})
elseif(WASM_EMCC)
    set(WASM_COMPILER ${WASM_EMCC})
endif()

if(WASM_COMPILER)
    if(WASM_CLANG)
        set(WASM_FLAGS "-O2" "-Wl,--no-entry" "-Wl,--export=execute" "-fno-exceptions" "-nostartfiles")
    else()
        set(WASM_FLAGS "-O2" "-s" "WASM=1" "-s" "SIDE_MODULE=1" "-s" "EXPORTED_FUNCTIONS='[\"_execute\"]'")
    endif()

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/battle_plugin.wasm
        COMMAND ${WASM_COMPILER} ${WASM_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/battle_plugin.wasm ${CMAKE_CURRENT_SOURCE_DIR}/battle_plugin.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/battle_plugin.cpp
        COMMENT "Compiling battle_plugin.cpp to WASM"
    )
    add_custom_target(wasm_plugins ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/battle_plugin.wasm)
else()
    message(STATUS "No WASM compiler found (wasi-sdk/clang or emcc). Skipping WASM plugin build.")
endif()

# 单元测试可执行文件
add_executable(WasmHotTests
    test_main.cpp
    test_object.cpp
    test_plugin.cpp
)

target_include_directories(WasmHotTests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(WasmHotTests PRIVATE WasmHotLib)

if(WASM_COMPILER)
    add_dependencies(WasmHotTests wasm_plugins)
endif()

target_compile_definitions(WasmHotTests PRIVATE
    BATTLE_PLUGIN_WASM="${CMAKE_CURRENT_BINARY_DIR}/battle_plugin.wasm"
)

# CTest
add_test(NAME WasmHotUnitTests COMMAND WasmHotTests)
