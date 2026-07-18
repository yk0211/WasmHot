#pragma once
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include "core/object.h"
#include "core/component_storage.h"
#include "core/migration_engine.h"

namespace wasmh {

// ObjectRegistry creates and tracks GameObject views by schema version.
// It is the bridge between the actor runtime and the component storage.
class ObjectRegistry
{
public:
    explicit ObjectRegistry(ComponentStorage* storage);

    GameObject* Create(const ObjectHeader& header);
    GameObject* Get(uint64_t object_id);
    void Destroy(uint64_t object_id);
    bool Migrate(uint64_t object_id, uint32_t target_version, MigrationEngine& migration);

private:
    ComponentStorage* storage_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<uint64_t, std::unique_ptr<GameObject>> objects_;
};

} // namespace wasmh
