#include "actor/room_actor.h"

namespace wasmh {

RoomActor::RoomActor(asio::io_context& io, uint64_t id, uint64_t object_id)
    : ActorWithObject(io, object_id, {"room"}) {
  actor_id = id;
}

void RoomActor::Tick(uint64_t now_ms) {
  (void)now_ms;
  InvokeModule("room", "tick", {});
}

void RoomActor::HandleMessage(uint64_t sender_id,
                              const std::vector<uint8_t>& payload) {
  (void)sender_id;
  InvokeModule("room", "handle_message", payload);
}

}  // namespace wasmh
