#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include "core/schema_manager.h"

namespace wasmh {

using ComponentType = uint32_t;
using ComponentData = std::vector<uint8_t>;

struct ObjectHeader {
  uint32_t schema_version;
  uint32_t object_type;
  uint64_t object_id;
};

class ComponentStorage;  // forward declaration

// GameObject is a lightweight view. It holds identity (header) and delegates
// component data access to ComponentStorage. Logic plugins receive this view
// and are not allowed to own long-lived state.
class GameObject {
 public:
  using ReadComponentCallback = std::function<void(const ComponentData*)>;

  ObjectHeader header{};
  ComponentStorage* storage = nullptr;

  GameObject() = default;
  GameObject(ObjectHeader h, ComponentStorage* s) : header(h), storage(s) {}

  // Read the component under its object lock. The pointer is valid only for
  // the duration of the callback.
  void ReadComponent(ComponentType type, const ReadComponentCallback& callback) const;

  // Replace the component under its object lock.
  void WriteComponent(ComponentType type, ComponentData data) const;
  [[nodiscard]] bool WriteComponent(ComponentType type, const Schema& schema, ComponentData data) const;

  [[nodiscard]] bool RemoveComponent(ComponentType type) const;
  [[nodiscard]] bool ApplyOutputDelta(const std::vector<uint8_t>& output) const;
};

}  // namespace wasmh
