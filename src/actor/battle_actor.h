#pragma once
#include <cstdint>
#include "actor/actor.h"

namespace wasmh {

// BattleActor represents a running battle. Native C++ owns the actor;
// WASM plugins implement battle rules, skill calculation, and AI decisions.
class BattleActor : public ActorWithObject
{
public:
    BattleActor(uint64_t id, GameObject* battle_obj, ModuleManager* modules);

    void Tick(uint64_t now_ms) override;
    void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) override;
};

} // namespace wasmh
