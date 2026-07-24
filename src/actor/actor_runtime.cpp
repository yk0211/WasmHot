#include "actor/actor_runtime.h"

namespace wasmh {

void ActorRuntime::Initialize(asio::io_context& io) {
  io_ = &io;
}

void ActorRuntime::Spawn(std::shared_ptr<Actor> actor) {
  uint64_t id = actor->actor_id;
  std::unique_lock lock(actors_mutex_);
  actors_[id] = std::move(actor);
}

void ActorRuntime::Kill(uint64_t actor_id) {
  std::unique_lock lock(actors_mutex_);
  actors_.erase(actor_id);
}

void ActorRuntime::Tick(uint64_t now_ms) {
  std::vector<std::shared_ptr<Actor>> snapshot;
  {
    std::shared_lock lock(actors_mutex_);
    snapshot.reserve(actors_.size());
    for (const auto& [id, actor] : actors_) {
      (void)id;
      snapshot.push_back(actor);
    }
  }

  for (auto& actor : snapshot) {
    actor->ScheduleTick(now_ms);
  }
}

void ActorRuntime::SendMessage(uint64_t from_id, uint64_t to_id, const std::vector<uint8_t>& payload) {
  std::shared_ptr<Actor> actor;
  {
    std::shared_lock lock(actors_mutex_);
    auto it = actors_.find(to_id);
    if (it == actors_.end())
      return;
    actor = it->second;
  }
  actor->EnqueueMessage(from_id, payload);
}

}  // namespace wasmh
