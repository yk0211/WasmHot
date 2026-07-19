#include "actor/mail_actor.h"

namespace wasmh {

MailActor::MailActor(asio::io_context& io, uint64_t id, uint64_t object_id)
    : ActorWithObject(io, object_id, {"economy"})
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
