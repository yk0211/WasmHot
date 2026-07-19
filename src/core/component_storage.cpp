#include "core/component_storage.h"

namespace wasmh {

void ComponentStorage::Read(uint64_t object_id, ComponentType type, ReadCallback callback) const
{
    std::shared_lock map_lock(map_mutex_);
    auto it = store_.find(object_id);
    if (it == store_.end()) {
        callback(nullptr);
        return;
    }

    std::lock_guard obj_lock(it->second->mutex);
    auto jt = it->second->components.find(type);
    callback(jt != it->second->components.end() ? &jt->second : nullptr);
}

void ComponentStorage::Write(uint64_t object_id, ComponentType type, ComponentData data)
{
    std::unique_ptr<ObjectComponents> new_obj;

    std::shared_lock map_lock(map_mutex_);
    auto it = store_.find(object_id);
    if (it == store_.end()) {
        map_lock.unlock();
        new_obj = std::make_unique<ObjectComponents>();
        std::unique_lock unique_map_lock(map_mutex_);
        auto& slot = store_[object_id];
        if (!slot) slot = std::move(new_obj);
        it = store_.find(object_id);
    }

    std::lock_guard obj_lock(it->second->mutex);
    it->second->components[type] = std::move(data);
}

bool ComponentStorage::Write(uint64_t object_id, ComponentType type, const Schema& schema, ComponentData data)
{
    if (data.size() != schema.total_size) return false;
    Write(object_id, type, std::move(data));
    return true;
}

bool ComponentStorage::Remove(uint64_t object_id, ComponentType type)
{
    std::shared_lock map_lock(map_mutex_);
    auto it = store_.find(object_id);
    if (it == store_.end()) return false;

    std::lock_guard obj_lock(it->second->mutex);
    return it->second->components.erase(type) > 0;
}

void ComponentStorage::RemoveAll(uint64_t object_id)
{
    std::unique_lock map_lock(map_mutex_);
    auto it = store_.find(object_id);
    if (it == store_.end()) return;

    {
        // Synchronize with any thread currently operating on this object's
        // components before erasing the object.
        std::lock_guard obj_lock(it->second->mutex);
    }
    store_.erase(it);
}

} // namespace wasmh
