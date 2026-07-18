#pragma once
#include "core/plugin_interface.h"

namespace wasmh {
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

    struct WasmEdgeState;
    std::unique_ptr<WasmEdgeState> state_;
};

class WasmEdgePluginFactory : public PluginFactory
{
public:
    std::unique_ptr<IPlugin> Create(const std::string& path, uint32_t schema_version) override;
};

} // namespace wasmh
