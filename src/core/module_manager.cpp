#include "core/module_manager.h"

namespace wasmh {

ModuleManager::ModuleManager(std::unique_ptr<PluginFactory> factory)
    : factory_(std::move(factory)) {}

bool ModuleManager::Load(const ModuleConfig& config)
{
    auto plugin = factory_->Create(config.path, config.schema_version);
    if (!plugin) return false;
    if (!plugin->Initialize()) return false;

    modules_[config.name] = std::move(plugin);
    schema_versions_[config.name] = config.schema_version;
    return true;
}

void ModuleManager::Unload(const std::string& name)
{
    auto it = modules_.find(name);
    if (it != modules_.end())
    {
        it->second->Shutdown();
        modules_.erase(it);
    }
    schema_versions_.erase(name);
}

IPlugin* ModuleManager::Get(const std::string& name) const
{
    auto it = modules_.find(name);
    return it != modules_.end() ? it->second.get() : nullptr;
}

uint32_t ModuleManager::GetLoadedSchemaVersion(const std::string& name) const
{
    auto it = schema_versions_.find(name);
    return it != schema_versions_.end() ? it->second : 0;
}

} // namespace wasmh
