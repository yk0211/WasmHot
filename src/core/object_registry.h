#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "common/singleton.h"
#include "core/migration_engine.h"
#include "core/object.h"

namespace wasmh {

class ComponentStorage;

// ObjectRegistry creates and tracks GameObject views by schema version.
// It is the bridge between the actor runtime and the component storage.
// All public access is callback-based so that no internal GameObject pointer
// escapes the registry locks.
class ObjectRegistry : public Singleton<ObjectRegistry> {
  friend class Singleton<ObjectRegistry>;

 public:
  using ObjectCallback = std::function<void(GameObject*)>;

  // Create a new object and return its id. The header is the authoritative
  // identity stored in the registry; callers use Get() to obtain a view.
  uint64_t Create(const ObjectHeader& header);

  // Invoke callback with a temporary GameObject view for the requested object.
  // If the object does not exist, callback(nullptr) is called.
  void Get(uint64_t object_id, const ObjectCallback& callback) const;

  void Destroy(uint64_t object_id);

  bool Migrate(uint64_t object_id, uint32_t target_version);

 private:
  ObjectRegistry();

  struct ObjectEntry {
    std::mutex mutex;
    ObjectHeader header{};
  };

  ComponentStorage* storage_;
  mutable std::shared_mutex map_mutex_;
  std::unordered_map<uint64_t, std::unique_ptr<ObjectEntry>> objects_;
};

}  // namespace wasmh
