#include "actor/room_actor.h"

namespace wasmh {

RoomActor::RoomActor(uint64_t id, GameObject* room_obj, ModuleManager* modules)
    : room_object_(room_obj), modules_(modules)
{
    actor_id = id;
}

void RoomActor::Tick(uint64_t now_ms)
{
    (void)now_ms;
    if (!modules_) return;
    IPlugin* plugin = modules_->Get("room");
    if (!plugin) return;

    std::vector<uint8_t> output;
    plugin->Execute(*room_object_, "tick", {}, output);
}

void RoomActor::HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload)
{
    (void)sender_id;
    if (!modules_) return;
    IPlugin* plugin = modules_->Get("room");
    if (!plugin) return;

    std::vector<uint8_t> output;
    plugin->Execute(*room_object_, "handle_message", payload, output);
}

} // namespace wasmh
