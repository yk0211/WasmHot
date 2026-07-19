#include <cstring>

#include "core/object.h"
#include "core/component_storage.h"

namespace wasmh {

namespace {
constexpr uint32_t kDeltaMagic = 0x4D534157; // "WASM" little-endian
} // namespace

void GameObject::ReadComponent(ComponentType type, ReadComponentCallback callback)
{
    if (!storage) {
        callback(nullptr);
        return;
    }
    storage->Read(header.object_id, type, std::move(callback));
}

void GameObject::ReadComponent(ComponentType type, ReadComponentCallback callback) const
{
    if (!storage) {
        callback(nullptr);
        return;
    }
    storage->Read(header.object_id, type, std::move(callback));
}

void GameObject::WriteComponent(ComponentType type, ComponentData data)
{
    if (storage)
    {
        storage->Write(header.object_id, type, std::move(data));
    }
}

bool GameObject::WriteComponent(ComponentType type, const Schema& schema, ComponentData data)
{
    if (!storage) return false;
    return storage->Write(header.object_id, type, schema, std::move(data));
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
    WriteComponent(type, std::move(data));
    return true;
}

} // namespace wasmh
