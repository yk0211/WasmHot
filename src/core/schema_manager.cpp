#include "core/schema_manager.h"

namespace wasmh {

void SchemaManager::RegisterSchema(const Schema& schema)
{
    schemas_[schema.object_type][schema.version] = schema;
}

const Schema* SchemaManager::GetSchema(uint32_t object_type, uint32_t version) const
{
    auto it = schemas_.find(object_type);
    if (it == schemas_.end()) return nullptr;
    auto jt = it->second.find(version);
    return jt != it->second.end() ? &jt->second : nullptr;
}

std::optional<uint32_t> SchemaManager::GetLatestVersion(uint32_t object_type) const
{
    auto it = schemas_.find(object_type);
    if (it == schemas_.end()) return std::nullopt;

    uint32_t latest = 0;
    for (const auto& [version, schema] : it->second)
    {
        (void)schema;
        if (version > latest) latest = version;
    }
    return latest > 0 ? std::optional<uint32_t>(latest) : std::nullopt;
}

} // namespace wasmh
