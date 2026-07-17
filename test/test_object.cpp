#include "test_common.h"
#include "core/object.h"
#include "core/component_storage.h"
#include "core/object_registry.h"
#include "core/schema_manager.h"
#include "core/migration_engine.h"

int run_object_tests()
{
    using namespace wasmh;

    ComponentStorage storage;
    ObjectRegistry registry(&storage);
    ObjectHeader header{1, 1, 1};
    GameObject* obj = registry.Create(header);
    CHECK(obj != nullptr);
    CHECK(obj->header.object_id == 1);

    std::vector<uint8_t> data = {1, 2, 3, 4};
    obj->SetComponent(1, std::move(data));
    auto* comp = obj->GetComponent(1);
    CHECK(comp != nullptr);
    CHECK(comp->size() == 4);
    CHECK((*comp)[0] == 1);

    SchemaManager sm;
    sm.RegisterSchema({1, 1, {{"hp", FieldType::Int32, 0, 4}}, 4});
    auto* schema = sm.GetSchema(1, 1);
    CHECK(schema != nullptr);
    CHECK(schema->version == 1);

    auto latest = sm.GetLatestVersion(1);
    CHECK(latest.has_value());
    CHECK(latest.value() == 1);

    MigrationEngine me;
    me.Register(1, 2, [](GameObject& o, uint32_t from, uint32_t to) -> bool {
        (void)from; (void)to;
        auto* d = o.GetComponent(1);
        if (!d) return false;
        std::vector<uint8_t> nd(8);
        std::copy(d->begin(), d->end(), nd.begin());
        o.SetComponent(1, std::move(nd));
        return true;
    });

    CHECK(registry.Migrate(1, 2, me));
    auto* comp2 = obj->GetComponent(1);
    CHECK(comp2 != nullptr);
    CHECK(comp2->size() == 8);
    CHECK(obj->header.schema_version == 2);

    registry.Destroy(1);
    CHECK(registry.Get(1) == nullptr);

    return 0;
}
