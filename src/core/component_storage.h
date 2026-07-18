#pragma once
#include <cstdint>
#include <shared_mutex>
#include <vector>
#include <unordered_map>
#include "core/object.h"
#include "core/schema_manager.h"

namespace wasmh {

// ComponentStorage owns all long-lived component bytes.
// It is managed by the Native C++ runtime, never by WASM logic.
class ComponentStorage
{
public:
    ComponentData* Get(uint64_t object_id, ComponentType type);
    const ComponentData* Get(uint64_t object_id, ComponentType type) const;
    void Set(uint64_t object_id, ComponentType type, ComponentData data);
    bool Set(uint64_t object_id, ComponentType type, const Schema& schema, ComponentData data);
    bool Remove(uint64_t object_id, ComponentType type);
    void RemoveAll(uint64_t object_id);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<uint64_t, std::unordered_map<ComponentType, ComponentData>> store_;
};

} // namespace wasmh
