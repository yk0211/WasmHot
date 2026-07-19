#include "core/object_registry.h"
#include "core/component_storage.h"
#include "core/migration_engine.h"

namespace wasmh {

ObjectRegistry::ObjectRegistry() : storage_(ComponentStorage::Instance()) {}

uint64_t ObjectRegistry::Create(const ObjectHeader& header)
{
    std::unique_lock lock(map_mutex_);
    auto entry = std::make_unique<ObjectEntry>();
    entry->header = header;
    objects_[header.object_id] = std::move(entry);
    return header.object_id;
}

void ObjectRegistry::Get(uint64_t object_id, ObjectCallback callback) const
{
    std::shared_lock lock(map_mutex_);
    auto it = objects_.find(object_id);
    if (it == objects_.end()) {
        callback(nullptr);
        return;
    }

    ObjectHeader header;
    {
        std::lock_guard obj_lock(it->second->mutex);
        header = it->second->header;
    }
    lock.unlock();

    GameObject view(header, storage_);
    callback(&view);
}

void ObjectRegistry::Destroy(uint64_t object_id)
{
    {
        std::unique_lock lock(map_mutex_);
        auto it = objects_.find(object_id);
        if (it == objects_.end()) return;

        {
            // Synchronize with any thread currently using this object view
            // before erasing the entry.
            std::lock_guard obj_lock(it->second->mutex);
        }
        objects_.erase(it);
    }

    if (storage_) {
        storage_->RemoveAll(object_id);
    }
}

bool ObjectRegistry::Migrate(uint64_t object_id, uint32_t target_version)
{
    std::shared_lock lock(map_mutex_);
    auto it = objects_.find(object_id);
    if (it == objects_.end()) return false;

    std::lock_guard obj_lock(it->second->mutex);
    GameObject view(it->second->header, storage_);
    bool ok = MigrationEngine::Instance()->Migrate(view, target_version);
    if (ok) {
        it->second->header = view.header;
    }
    return ok;
}

} // namespace wasmh
