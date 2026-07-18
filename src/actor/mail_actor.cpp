#include "actor/mail_actor.h"

namespace wasmh {

MailActor::MailActor(uint64_t id, GameObject* mail_obj, ModuleManager* modules)
    : ActorWithObject(mail_obj, modules)
{
    actor_id = id;
}

void MailActor::Tick(uint64_t now_ms)
{
    (void)now_ms;
    InvokeModule("economy", "tick", {});
}

void MailActor::HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload)
{
    (void)sender_id;
    InvokeModule("economy", "handle_message", payload);
}

} // namespace wasmh
