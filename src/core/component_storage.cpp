#include "core/component_storage.h"

namespace wasmh {

ComponentData* ComponentStorage::Get(uint64_t object_id, ComponentType type)
{
    auto it = store_.find(object_id);
    if (it == store_.end()) return nullptr;
    auto jt = it->second.find(type);
    return jt != it->second.end() ? &jt->second : nullptr;
}

const ComponentData* ComponentStorage::Get(uint64_t object_id, ComponentType type) const
{
    auto it = store_.find(object_id);
    if (it == store_.end()) return nullptr;
    auto jt = it->second.find(type);
    return jt != it->second.end() ? &jt->second : nullptr;
}

void ComponentStorage::Set(uint64_t object_id, ComponentType type, ComponentData data)
{
    store_[object_id][type] = std::move(data);
}

bool ComponentStorage::Remove(uint64_t object_id, ComponentType type)
{
    auto it = store_.find(object_id);
    if (it == store_.end()) return false;
    return it->second.erase(type) > 0;
}

void ComponentStorage::RemoveAll(uint64_t object_id)
{
    store_.erase(object_id);
}

} // namespace wasmh
