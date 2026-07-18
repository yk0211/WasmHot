#pragma once
#include <shared_mutex>
#include <string>
#include <memory>
#include <unordered_map>
#include "core/plugin_interface.h"

namespace wasmh {

struct ModuleConfig
{
    std::string name;
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
    bool HotReload(const ModuleConfig& config);
    void Rollback(const std::string& name);
    void Unload(const std::string& name);
    std::shared_ptr<IPlugin> Get(const std::string& name) const;
    uint32_t GetLoadedSchemaVersion(const std::string& name) const;

private:
    std::unique_ptr<PluginFactory> factory_;
    std::unordered_map<std::string, std::shared_ptr<IPlugin>> modules_;
    std::unordered_map<std::string, std::shared_ptr<IPlugin>> previous_modules_;
    std::unordered_map<std::string, ModuleConfig> configs_;
    std::unordered_map<std::string, uint32_t> schema_versions_;
    mutable std::shared_mutex mutex_;
};

} // namespace wasmh
