#pragma once
#include <string>
#include "core/plugin_interface.h"

namespace wasmh {

// NativePlugin loads a platform dynamic library (.so / .dll).
// It demonstrates the same stateless contract as WASM plugins:
// execute() receives data, computes, and returns output.
class NativePlugin : public IPlugin
{
public:
    NativePlugin(const std::string& path, uint32_t schema_version);
    ~NativePlugin() override;

    bool Initialize() override;
    void Shutdown() override;
    bool Execute(GameObject& obj, const std::string& action,
                 const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output) override;

private:
    std::string path_;
    uint32_t schema_version_;
    void* handle_ = nullptr;
};

class NativePluginFactory : public PluginFactory
{
public:
    std::unique_ptr<IPlugin> Create(const std::string& path, uint32_t schema_version) override;
};

} // namespace wasmh
