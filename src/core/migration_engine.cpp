#include "core/migration_engine.h"

namespace wasmh {

uint64_t MigrationEngine::MakeKey(uint32_t from, uint32_t to) {
  return (static_cast<uint64_t>(from) << 32) | static_cast<uint64_t>(to);
}

void MigrationEngine::Register(uint32_t from_version, uint32_t to_version,
                               MigrationFunc func) {
  std::unique_lock lock(mutex_);
  migrations_[MakeKey(from_version, to_version)] = std::move(func);
}

bool MigrationEngine::Migrate(GameObject& obj, uint32_t target_version) {
  uint32_t current = obj.header.schema_version;
  while (current != target_version) {
    uint32_t next = current < target_version ? current + 1 : current - 1;
    if (!MigrateStep(obj, current, next)) {
      return false;
    }
    current = next;
    obj.header.schema_version = current;
  }
  return true;
}

bool MigrationEngine::MigrateStep(GameObject& obj, uint32_t from, uint32_t to) {
  MigrationFunc func;
  {
    std::shared_lock lock(mutex_);
    auto it = migrations_.find(MakeKey(from, to));
    if (it == migrations_.end())
      return false;
    func = it->second;
  }
  return func(obj, from, to);
}

}  // namespace wasmh
