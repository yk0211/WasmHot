#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace wasmh {

class GameObject;
class ModuleManager;

// Actor is the unit of scheduling in the Native C++ runtime.
// WASM logic is invoked inside Tick/HandleMessage, but the actor lifecycle
// (creation, destruction, message queue, timers) is managed by C++.
class Actor
{
public:
    uint64_t actor_id = 0;
    virtual ~Actor() = default;
    virtual void Tick(uint64_t now_ms) = 0;
    virtual void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) = 0;
};

// ActorWithObject provides common WASM module invocation for actors that operate
// on a single GameObject. This eliminates the repeated lookup/execute/apply
// pattern in every actor implementation.
class ActorWithObject : public Actor
{
protected:
    ActorWithObject(GameObject* obj, ModuleManager* modules);

    void InvokeModule(const std::string& module_name, const std::string& action,
                      const std::vector<uint8_t>& input);

    GameObject* target_object_ = nullptr;
    ModuleManager* modules_ = nullptr;
};

} // namespace wasmh
