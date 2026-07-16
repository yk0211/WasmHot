#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "core/object.h"

namespace wasmh {

// ComponentStorage owns all long-lived component bytes.
// It is managed by the Native C++ runtime, never by WASM logic.
class ComponentStorage
{
public:
    ComponentData* Get(uint64_t object_id, ComponentType type);
    const ComponentData* Get(uint64_t object_id, ComponentType type) const;
    void Set(uint64_t object_id, ComponentType type, ComponentData data);
    bool Remove(uint64_t object_id, ComponentType type);
    void RemoveAll(uint64_t object_id);

private:
    std::unordered_map<uint64_t, std::unordered_map<ComponentType, ComponentData>> store_;
};

} // namespace wasmh
