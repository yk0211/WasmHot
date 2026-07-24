#pragma once
#include <cstdint>

#include "actor/actor.h"

namespace wasmh {

// RoomActor manages a room/session. Room logic (matchmaking, broadcasting)
// can be partly delegated to WASM, but persistent room state stays in C++.
class RoomActor : public ActorWithObject {
 public:
  RoomActor(asio::io_context& io, uint64_t id, uint64_t object_id);

  void Tick(uint64_t now_ms) override;

 protected:
  void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) override;
};

}  // namespace wasmh
