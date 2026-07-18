#include <mutex>

#include "core/object_registry.h"

namespace wasmh {

ObjectRegistry::ObjectRegistry(ComponentStorage* storage) : storage_(storage) {}

GameObject* ObjectRegistry::Create(const ObjectHeader& header)
{
    std::unique_lock lock(mutex_);
    auto obj = std::make_unique<GameObject>(header, storage_);
    GameObject* ptr = obj.get();
    objects_[header.object_id] = std::move(obj);
    return ptr;
}

GameObject* ObjectRegistry::Get(uint64_t object_id)
{
    std::shared_lock lock(mutex_);
    auto it = objects_.find(object_id);
    return it != objects_.end() ? it->second.get() : nullptr;
}

void ObjectRegistry::Destroy(uint64_t object_id)
{
    std::unique_lock lock(mutex_);
    objects_.erase(object_id);
    if (storage_) storage_->RemoveAll(object_id);
}

bool ObjectRegistry::Migrate(uint64_t object_id, uint32_t target_version, MigrationEngine& migration)
{
    std::shared_lock lock(mutex_);
    auto it = objects_.find(object_id);
    if (it == objects_.end()) return false;
    return migration.Migrate(*it->second, target_version);
}

} // namespace wasmh
