#include "actor/battle_actor.h"

namespace wasmh {

BattleActor::BattleActor(asio::io_context& io, uint64_t id, uint64_t object_id)
    : ActorWithObject(io, object_id, {"battle_rules", "ai_decision"})
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
