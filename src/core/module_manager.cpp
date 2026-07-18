#include <mutex>

#include "core/module_manager.h"

namespace wasmh {

ModuleManager::ModuleManager(std::unique_ptr<PluginFactory> factory)
    : factory_(std::move(factory)) {}

bool ModuleManager::Load(const ModuleConfig& config)
{
    auto plugin = factory_->Create(config.path, config.schema_version);
    if (!plugin) return false;
    if (!plugin->Initialize()) return false;

    std::unique_lock lock(mutex_);
    modules_[config.name] = std::move(plugin);
    configs_[config.name] = config;
    schema_versions_[config.name] = config.schema_version;
    return true;
}

bool ModuleManager::HotReload(const ModuleConfig& config)
{
    auto plugin = factory_->Create(config.path, config.schema_version);
    if (!plugin) return false;
    if (!plugin->Initialize()) return false;

    std::unique_lock lock(mutex_);
    auto it = modules_.find(config.name);
    if (it != modules_.end()) {
        previous_modules_[config.name] = std::move(it->second);
    }
    modules_[config.name] = std::move(plugin);
    configs_[config.name] = config;
    schema_versions_[config.name] = config.schema_version;
    return true;
}

void ModuleManager::Rollback(const std::string& name)
{
    std::unique_lock lock(mutex_);
    auto it = previous_modules_.find(name);
    if (it == previous_modules_.end()) return;
    modules_[name] = std::move(it->second);
    previous_modules_.erase(it);

    auto cit = configs_.find(name);
    if (cit != configs_.end()) {
        schema_versions_[name] = cit->second.schema_version;
    }
}

void ModuleManager::Unload(const std::string& name)
{
    std::unique_lock lock(mutex_);
    auto it = modules_.find(name);
    if (it != modules_.end()) {
        it->second->Shutdown();
        modules_.erase(it);
    }
    previous_modules_.erase(name);
    configs_.erase(name);
    schema_versions_.erase(name);
}

std::shared_ptr<IPlugin> ModuleManager::Get(const std::string& name) const
{
    std::shared_lock lock(mutex_);
    auto it = modules_.find(name);
    return it != modules_.end() ? it->second : nullptr;
}

uint32_t ModuleManager::GetLoadedSchemaVersion(const std::string& name) const
{
    std::shared_lock lock(mutex_);
    auto it = schema_versions_.find(name);
    return it != schema_versions_.end() ? it->second : 0;
}

} // namespace wasmh
