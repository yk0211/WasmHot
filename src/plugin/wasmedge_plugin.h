#pragma once
#include "core/plugin_interface.h"

namespace wasmh {

// WasmEdgePlugin loads a WASM module through the WasmEdge runtime.
// When WASM_HOT_USE_WASMEDGE is defined, real WasmEdge symbols are used;
// otherwise the class compiles as a stub that reports initialization failure.
class WasmEdgePlugin : public IPlugin
{
public:
    WasmEdgePlugin(const std::string& path, uint32_t schema_version);
    ~WasmEdgePlugin() override;

    bool Initialize() override;
    void Shutdown() override;
    bool Execute(GameObject& obj, const std::string& action,
                 const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output) override;

private:
    std::string path_;
    uint32_t schema_version_;
    bool initialized_ = false;

#if WASM_HOT_USE_WASMEDGE
    // Opaque handles owned by this plugin instance.
    struct WasmEdgeState;
    std::unique_ptr<WasmEdgeState> state_;
#endif
};

class WasmEdgePluginFactory : public PluginFactory
{
public:
    std::unique_ptr<IPlugin> Create(const std::string& path, uint32_t schema_version) override;
};

} // namespace wasmh
