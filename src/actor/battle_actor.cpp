#include "actor/battle_actor.h"

namespace wasmh {

BattleActor::BattleActor(uint64_t id, GameObject* battle_obj, ModuleManager* modules)
    : ActorWithObject(battle_obj, modules)
{
    actor_id = id;
}

void BattleActor::Tick(uint64_t now_ms)
{
    (void)now_ms;
    // Stateless tick computation in WASM: input battle state, output new state delta.
    std::vector<uint8_t> tick_context;
    InvokeModule("battle_rules", "tick", tick_context);
    InvokeModule("ai_decision", "decide", tick_context);
}

void BattleActor::HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload)
{
    (void)sender_id;
    InvokeModule("battle_rules", "handle_message", payload);
}

} // namespace wasmh
