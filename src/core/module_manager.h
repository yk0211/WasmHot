#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "core/plugin_interface.h"

namespace wasmh {

enum class PluginType : uint8_t
{
    Native = 1,
    WASM = 2,
    Lua = 3
};

struct ModuleConfig
{
    std::string name;
    PluginType type;
    std::string path;
    uint32_t schema_version;
};

// ModuleManager owns loaded logic modules. It is the hot-update entry point:
// unload old version -> load new version -> migrate affected objects.
class ModuleManager
{
public:
    explicit ModuleManager(std::unique_ptr<PluginFactory> factory);

    bool Load(const ModuleConfig& config);
    void Unload(const std::string& name);
    IPlugin* Get(const std::string& name) const;
    uint32_t GetLoadedSchemaVersion(const std::string& name) const;

private:
    std::unique_ptr<PluginFactory> factory_;
    std::unordered_map<std::string, std::unique_ptr<IPlugin>> modules_;
    std::unordered_map<std::string, uint32_t> schema_versions_;
};

} // namespace wasmh
