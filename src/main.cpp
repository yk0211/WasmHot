#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include "core/object.h"
#include "core/component_storage.h"
#include "core/object_registry.h"
#include "core/schema_manager.h"
#include "core/migration_engine.h"
#include "core/module_manager.h"
#include "actor/actor_runtime.h"
#include "actor/battle_actor.h"
#include "actor/room_actor.h"
#include "network/gateway.h"
#include "plugin/wasmedge_plugin.h"

using namespace wasmh;

int main()
{
    // Schema management
    SchemaManager schema_manager;
    schema_manager.RegisterSchema({1, 1, {
        {"hp", FieldType::Int32, 0, 4},
        {"atk", FieldType::Int32, 4, 4}
    }, 8});
    schema_manager.RegisterSchema({2, 1, {
        {"hp", FieldType::Int32, 0, 4},
        {"atk", FieldType::Int32, 4, 4},
        {"def", FieldType::Int32, 8, 4}
    }, 12});

    // Migration engine
    MigrationEngine migration_engine;
    migration_engine.Register(1, 2, [](GameObject& obj, uint32_t from, uint32_t to) -> bool {
        (void)from;
        (void)to;
        auto* old_data = obj.GetComponent(1); // component type 1 = stats
        if (!old_data || old_data->size() != 8) return false;
        std::vector<uint8_t> new_data(12);
        std::copy(old_data->begin(), old_data->end(), new_data.begin());
        // set def = 0
        new_data[8] = 0; new_data[9] = 0; new_data[10] = 0; new_data[11] = 0;
        obj.SetComponent(1, std::move(new_data));
        return true;
    });

    // Component storage
    ComponentStorage storage;

    // Object registry
    ObjectRegistry registry(&storage);
    ObjectHeader player_header{1, 1, 1001};
    GameObject* player = registry.Create(player_header);

    // Set initial player stats (v1)
    std::vector<uint8_t> stats(8);
    int32_t hp = 100;
    int32_t atk = 20;
    std::memcpy(&stats[0], &hp, sizeof(hp));
    std::memcpy(&stats[4], &atk, sizeof(atk));
    player->SetComponent(1, std::move(stats));

    // Migrate to v2
    if (registry.Migrate(1001, 2, migration_engine))
    {
        std::cout << "Migrated player to schema v2\n";
    }

    // Module manager - WASM only
    auto plugin_factory = std::make_unique<WasmEdgePluginFactory>();
    ModuleManager modules(std::move(plugin_factory));
    modules.Load({"battle_rules", PluginType::WASM, "./test/battle_plugin.wasm", 2});

    // Actor runtime
    ActorRuntime runtime;
    ObjectHeader battle_header{2, 2, 2001};
    GameObject* battle = registry.Create(battle_header);
    runtime.Spawn(std::make_unique<BattleActor>(2001, battle, &modules));

    runtime.Tick(0);

    std::cout << "WasmHot server initialized\n";
    return 0;
}
