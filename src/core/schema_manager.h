#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace wasmh {

enum class FieldType : uint32_t
{
    Int32 = 1,
    Int64 = 2,
    Float = 3,
    Double = 4,
    Bool = 5,
    String = 6,
    Bytes = 7,
    ObjectRef = 8
};

struct FieldDef
{
    std::string name;
    FieldType type;
    uint32_t offset;
    uint32_t size;
};

struct Schema
{
    uint32_t version;
    uint32_t object_type;
    std::vector<FieldDef> fields;
    uint32_t total_size;
};

// SchemaManager knows every object type and every supported schema version.
// It is the source of truth for "what data shape is valid".
class SchemaManager
{
public:
    void RegisterSchema(const Schema& schema);
    const Schema* GetSchema(uint32_t object_type, uint32_t version) const;
    std::optional<uint32_t> GetLatestVersion(uint32_t object_type) const;

private:
    // object_type -> (version -> Schema)
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, Schema>> schemas_;
};

} // namespace wasmh
