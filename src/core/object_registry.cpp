#include "core/object_registry.h"

namespace wasmh {

ObjectRegistry::ObjectRegistry(ComponentStorage* storage) : storage_(storage) {}

GameObject* ObjectRegistry::Create(const ObjectHeader& header)
{
    auto obj = std::make_unique<GameObject>(header, storage_);
    GameObject* ptr = obj.get();
    objects_[header.object_id] = std::move(obj);
    return ptr;
}

GameObject* ObjectRegistry::Get(uint64_t object_id)
{
    auto it = objects_.find(object_id);
    return it != objects_.end() ? it->second.get() : nullptr;
}

void ObjectRegistry::Destroy(uint64_t object_id)
{
    objects_.erase(object_id);
    if (storage_) storage_->RemoveAll(object_id);
}

bool ObjectRegistry::Migrate(uint64_t object_id, uint32_t target_version, MigrationEngine& migration)
{
    GameObject* obj = Get(object_id);
    if (!obj) return false;
    return migration.Migrate(*obj, target_version);
}

} // namespace wasmh
