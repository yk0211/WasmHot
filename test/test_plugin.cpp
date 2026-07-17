#include "test_common.h"
#include "plugin/wasmedge_plugin.h"
#include "core/module_manager.h"
#include "core/object.h"
#include <string>

int run_plugin_tests()
{
    using namespace wasmh;

    WasmEdgePluginFactory factory;
    ModuleManager manager(std::make_unique<WasmEdgePluginFactory>());
    CHECK(manager.Get("nonexistent") == nullptr);
    CHECK(manager.GetLoadedSchemaVersion("nonexistent") == 0);

    WasmEdgePlugin wp("nonexistent.wasm", 1);
    CHECK(!wp.Initialize());

    // 测试真实 WASM 加载与执行
    {
        WasmEdgePlugin real_plugin(BATTLE_PLUGIN_WASM, 1);
        CHECK(real_plugin.Initialize());

        GameObject obj;
        obj.header = {1, 1, 1};

        std::vector<uint8_t> input = {1, 2, 3, 4, 5};

        // test_action 成功，输出格式为 "input: 1,2,3,4,5"
        std::vector<uint8_t> output;
        CHECK(real_plugin.Execute(obj, "test_action", input, output));
        std::string result(output.begin(), output.end());
        CHECK(result == "input: 1,2,3,4,5");

        // 未知 action 失败
        std::vector<uint8_t> output2;
        CHECK(!real_plugin.Execute(obj, "unknown_action", input, output2));
    }

    // 通过 ModuleManager 加载
    {
        ModuleManager modules(std::make_unique<WasmEdgePluginFactory>());
        CHECK(modules.Load({"battle_rules", PluginType::WASM, BATTLE_PLUGIN_WASM, 2}));
        auto* plugin = modules.Get("battle_rules");
        CHECK(plugin != nullptr);

        GameObject obj;
        obj.header = {2, 2, 2001};

        std::vector<uint8_t> input = {10, 20, 30};
        std::vector<uint8_t> output;
        CHECK(plugin->Execute(obj, "test_action", input, output));
        std::string result(output.begin(), output.end());
        CHECK(result == "input: 10,20,30");
    }

    return 0;
}
