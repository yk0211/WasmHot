#include "actor/battle_actor.h"

namespace wasmh {

BattleActor::BattleActor(uint64_t id, GameObject* battle_obj, ModuleManager* modules)
    : battle_object_(battle_obj), modules_(modules)
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

void BattleActor::InvokeModule(const std::string& module_name, const std::string& action,
                               const std::vector<uint8_t>& input)
{
    if (!modules_) return;
    IPlugin* plugin = modules_->Get(module_name);
    if (!plugin) return;

    std::vector<uint8_t> output;
    plugin->Execute(*battle_object_, action, input, output);
    // Native runtime applies output to component storage; WASM does not keep state.
}

} // namespace wasmh
