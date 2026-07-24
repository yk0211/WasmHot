#pragma once
#include <cstdint>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/singleton.h"

namespace wasmh {

enum class FieldType : uint8_t {
  Int32 = 1,
  Int64 = 2,
  Float = 3,
  Double = 4,
  Bool = 5,
  String = 6,
  Bytes = 7,
  ObjectRef = 8
};

struct FieldDef {
  std::string name;
  FieldType type;
  uint32_t offset;
  uint32_t size;
};

struct Schema {
  uint32_t version = 0;
  uint32_t object_type = 0;
  std::vector<FieldDef> fields;
  uint32_t total_size = 0;
};

// SchemaManager knows every object type and every supported schema version.
// It is the source of truth for "what data shape is valid".
class SchemaManager : public Singleton<SchemaManager> {
  friend class Singleton<SchemaManager>;

 public:
  void RegisterSchema(const Schema& schema);
  const Schema* GetSchema(uint32_t object_type, uint32_t version) const;
  std::optional<uint32_t> GetLatestVersion(uint32_t object_type) const;

 private:
  SchemaManager() = default;

  // object_type -> (version -> Schema)
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, Schema>> schemas_;
  mutable std::shared_mutex mutex_;
};

}  // namespace wasmh
