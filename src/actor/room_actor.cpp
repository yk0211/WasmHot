#include "actor/room_actor.h"

#include <cstring>

namespace wasmh {

RoomActor::RoomActor(asio::io_context& io, uint64_t id, uint64_t object_id) : ActorWithObject(io, object_id, {"room"}) {
  actor_id = id;
}

void RoomActor::Tick(uint64_t interval_ms) {
  std::vector<uint8_t> tick_input(sizeof(interval_ms));
  std::memcpy(tick_input.data(), &interval_ms, sizeof(interval_ms));
  InvokeModule("room", "tick", tick_input);
}

void RoomActor::HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) {
  (void)sender_id;
  InvokeModule("room", "handle_message", payload);
}

}  // namespace wasmh
