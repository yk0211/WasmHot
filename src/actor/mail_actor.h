#pragma once
#include <cstdint>
#include "actor/actor.h"
#include "core/object.h"
#include "core/module_manager.h"

namespace wasmh {

// MailActor handles asynchronous mail/economy operations.
// Economy rules may be implemented in WASM; mail storage is native.
class MailActor : public Actor
{
public:
    MailActor(uint64_t id, GameObject* mail_obj, ModuleManager* modules);

    void Tick(uint64_t now_ms) override;
    void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) override;

private:
    GameObject* mail_object_;
    ModuleManager* modules_;
};

} // namespace wasmh
