#pragma once
#include <cstdint>
#include "actor/actor.h"
#include "core/object.h"
#include "core/module_manager.h"

namespace wasmh {

// BattleActor represents a running battle. Native C++ owns the actor;
// WASM plugins implement battle rules, skill calculation, and AI decisions.
class BattleActor : public Actor
{
public:
    BattleActor(uint64_t id, GameObject* battle_obj, ModuleManager* modules);

    void Tick(uint64_t now_ms) override;
    void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) override;

private:
    GameObject* battle_object_;
    ModuleManager* modules_;

    void InvokeModule(const std::string& module_name, const std::string& action,
                      const std::vector<uint8_t>& input);
};

} // namespace wasmh
