#include <cstring>

#include "core/object.h"
#include "core/component_storage.h"

namespace wasmh {

namespace {
constexpr uint32_t kDeltaMagic = 0x4D534157; // "WASM" little-endian
} // namespace

ComponentData* GameObject::GetComponent(ComponentType type)
{
    return storage ? storage->Get(header.object_id, type) : nullptr;
}

const ComponentData* GameObject::GetComponent(ComponentType type) const
{
    return storage ? storage->Get(header.object_id, type) : nullptr;
}

void GameObject::SetComponent(ComponentType type, ComponentData data)
{
    if (storage)
    {
        storage->Set(header.object_id, type, std::move(data));
    }
}

bool GameObject::SetComponent(ComponentType type, const Schema& schema, ComponentData data)
{
    if (!storage) return false;
    return storage->Set(header.object_id, type, schema, std::move(data));
}

bool GameObject::RemoveComponent(ComponentType type)
{
    return storage ? storage->Remove(header.object_id, type) : false;
}

bool GameObject::ApplyOutputDelta(const std::vector<uint8_t>& output)
{
    if (output.size() < sizeof(uint32_t) * 2) return false;

    uint32_t magic = 0;
    uint32_t type = 0;
    std::memcpy(&magic, output.data(), sizeof(magic));
    std::memcpy(&type, output.data() + sizeof(magic), sizeof(type));
    if (magic != kDeltaMagic) return false;

    ComponentData data(output.begin() + sizeof(uint32_t) * 2, output.end());
    SetComponent(type, std::move(data));
    return true;
}

} // namespace wasmh
