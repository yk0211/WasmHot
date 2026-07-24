#pragma once
#include <asio/io_context.hpp>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "actor/actor.h"
#include "common/singleton.h"

namespace wasmh {

class ActorRuntime : public Singleton<ActorRuntime> {
  friend class Singleton<ActorRuntime>;

 public:
  void Initialize(asio::io_context& io);

  void Spawn(std::shared_ptr<Actor> actor);
  void Kill(uint64_t actor_id);

  // Drives all actors: each actor schedules its pending messages (in order)
  // followed by its periodic Tick on the actor's private strand.
  void Tick(uint64_t now_ms);

  // Sends a message to the target actor by enqueueing it into its mailbox.
  // This is non-blocking and returns immediately; it is safe to call from
  // any thread (e.g., the network I/O thread).
  void SendMessage(uint64_t from_id, uint64_t to_id, const std::vector<uint8_t>& payload);

  asio::io_context* GetIoContext() const { return io_; }

 private:
  ActorRuntime() = default;

  asio::io_context* io_ = nullptr;
  std::unordered_map<uint64_t, std::shared_ptr<Actor>> actors_;
  mutable std::shared_mutex actors_mutex_;
};

}  // namespace wasmh
