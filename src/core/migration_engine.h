#pragma once
#include <cstdint>
#include <functional>
#include <shared_mutex>
#include <unordered_map>

#include "common/singleton.h"
#include "core/object.h"

namespace wasmh {

// MigrationFunc rewrites a GameObject's components from one schema version to
// another. It must be pure data transformation: no side effects, no business
// logic.
using MigrationFunc = std::function<bool(GameObject& obj, uint32_t from_version, uint32_t to_version)>;

class MigrationEngine : public Singleton<MigrationEngine> {
  friend class Singleton<MigrationEngine>;

 public:
  void Register(uint32_t from_version, uint32_t to_version, MigrationFunc func);
  bool Migrate(GameObject& obj, uint32_t target_version);

 private:
  MigrationEngine() = default;

  std::unordered_map<uint64_t, MigrationFunc> migrations_;
  mutable std::shared_mutex mutex_;

  static uint64_t MakeKey(uint32_t from, uint32_t to);
  bool MigrateStep(GameObject& obj, uint32_t from, uint32_t to);
};

}  // namespace wasmh
