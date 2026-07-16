#include "actor/mail_actor.h"

namespace wasmh {

MailActor::MailActor(uint64_t id, GameObject* mail_obj, ModuleManager* modules)
    : mail_object_(mail_obj), modules_(modules)
{
    actor_id = id;
}

void MailActor::Tick(uint64_t now_ms)
{
    (void)now_ms;
    if (!modules_) return;
    if (IPlugin* plugin = modules_->Get("economy"))
    {
        std::vector<uint8_t> output;
        plugin->Execute(*mail_object_, "tick", {}, output);
    }
}

void MailActor::HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload)
{
    (void)sender_id;
    if (!modules_) return;
    if (IPlugin* plugin = modules_->Get("economy"))
    {
        std::vector<uint8_t> output;
        plugin->Execute(*mail_object_, "handle_message", payload, output);
    }
}

} // namespace wasmh
