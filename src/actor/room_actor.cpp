#include "actor/room_actor.h"

namespace wasmh {

RoomActor::RoomActor(uint64_t id, GameObject* room_obj, ModuleManager* modules)
    : ActorWithObject(room_obj, modules)
{
    actor_id = id;
}

void RoomActor::Tick(uint64_t now_ms)
{
    (void)now_ms;
    InvokeModule("room", "tick", {});
}

void RoomActor::HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload)
{
    (void)sender_id;
    InvokeModule("room", "handle_message", payload);
}

} // namespace wasmh
