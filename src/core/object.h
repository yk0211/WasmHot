#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "core/schema_manager.h"

namespace wasmh {

using ComponentType = uint32_t;
using ComponentData = std::vector<uint8_t>;

struct ObjectHeader
{
    uint32_t schema_version;
    uint32_t object_type;
    uint64_t object_id;
};

class ComponentStorage; // forward declaration

// GameObject is a lightweight view. It holds identity (header) and delegates
// component data access to ComponentStorage. Logic plugins receive this view
// and are not allowed to own long-lived state.
class GameObject
{
public:
    ObjectHeader header;
    ComponentStorage* storage = nullptr;

    GameObject() = default;
    GameObject(ObjectHeader h, ComponentStorage* s) : header(h), storage(s) {}

    ComponentData* GetComponent(ComponentType type);
    const ComponentData* GetComponent(ComponentType type) const;
    void SetComponent(ComponentType type, ComponentData data);
    bool SetComponent(ComponentType type, const Schema& schema, ComponentData data);
    bool RemoveComponent(ComponentType type);
    bool ApplyOutputDelta(const std::vector<uint8_t>& output);
};

} // namespace wasmh
