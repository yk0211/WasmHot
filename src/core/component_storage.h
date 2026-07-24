#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common/singleton.h"
#include "core/object.h"
#include "core/schema_manager.h"

namespace wasmh {

// ComponentStorage owns all long-lived component bytes. Each object has its own
// mutex. External callers must use the Read/Write callback APIs so that
// pointers never escape the locks.
class ComponentStorage : public Singleton<ComponentStorage> {
  friend class Singleton<ComponentStorage>;

 public:
  using ReadCallback = std::function<void(const ComponentData*)>;

  // Read the component under its object lock. The pointer is valid only for
  // the duration of the callback.
  void Read(uint64_t object_id, ComponentType type,
            const ReadCallback& callback) const;

  // Replace the component under its object lock.
  void Write(uint64_t object_id, ComponentType type, ComponentData data);
  bool Write(uint64_t object_id, ComponentType type, const Schema& schema,
             ComponentData data);

  bool Remove(uint64_t object_id, ComponentType type);
  void RemoveAll(uint64_t object_id);

 private:
  ComponentStorage() = default;

  struct ObjectComponents {
    std::mutex mutex;
    std::unordered_map<ComponentType, ComponentData> components;
  };

  mutable std::shared_mutex map_mutex_;
  std::unordered_map<uint64_t, std::unique_ptr<ObjectComponents>> store_;
};

}  // namespace wasmh
