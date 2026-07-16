#pragma once
#include "core/plugin_interface.h"

namespace wasmh {

// WasmtimePlugin loads a WASM module through the Wasmtime runtime.
// When WASM_HOT_USE_WASMTIME is defined, real wasmtime symbols are used;
// otherwise the class compiles as a stub that reports initialization failure.
class WasmtimePlugin : public IPlugin
{
public:
    WasmtimePlugin(const std::string& path, uint32_t schema_version);
    ~WasmtimePlugin() override;

    bool Initialize() override;
    void Shutdown() override;
    bool Execute(GameObject& obj, const std::string& action,
                 const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output) override;

private:
    std::string path_;
    uint32_t schema_version_;
    bool initialized_ = false;

#if WASM_HOT_USE_WASMTIME
    // Opaque handles owned by this plugin instance.
    struct WasmtimeState;
    std::unique_ptr<WasmtimeState> state_;
#endif
};

class WasmtimePluginFactory : public PluginFactory
{
public:
    std::unique_ptr<IPlugin> Create(const std::string& path, uint32_t schema_version) override;
};

} // namespace wasmh
