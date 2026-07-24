#include "core/object.h"

#include <cstring>

#include "core/component_storage.h"

namespace wasmh {

constexpr uint32_t g_delta_magic = 0x4D534157;  // "WASM" little-endian

void GameObject::ReadComponent(ComponentType type, const ReadComponentCallback& callback) const {
  if (storage == nullptr) {
    callback(nullptr);
    return;
  }
  storage->Read(header.object_id, type, callback);
}

void GameObject::WriteComponent(ComponentType type, ComponentData data) const {
  if (storage != nullptr) {
    storage->Write(header.object_id, type, std::move(data));
  }
}

bool GameObject::WriteComponent(ComponentType type, const Schema& schema, ComponentData data) const {
  if (storage == nullptr)
    return false;
  return storage->Write(header.object_id, type, schema, std::move(data));
}

bool GameObject::RemoveComponent(ComponentType type) const {
  return (storage != nullptr) ? storage->Remove(header.object_id, type) : false;
}

bool GameObject::ApplyOutputDelta(const std::vector<uint8_t>& output) const {
  if (output.size() < sizeof(uint32_t) * 2)
    return false;

  uint32_t magic = 0;
  uint32_t type = 0;
  std::memcpy(&magic, output.data(), sizeof(magic));
  std::memcpy(&type, output.data() + sizeof(magic), sizeof(type));
  if (magic != g_delta_magic)
    return false;

  ComponentData data(output.begin() + sizeof(uint32_t) * 2, output.end());
  WriteComponent(type, std::move(data));
  return true;
}

}  // namespace wasmh
