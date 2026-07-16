#include "actor/actor_runtime.h"

namespace wasmh {

void ActorRuntime::Spawn(std::unique_ptr<Actor> actor)
{
    uint64_t id = actor->actor_id;
    actors_[id] = std::move(actor);
}

void ActorRuntime::Kill(uint64_t actor_id)
{
    actors_.erase(actor_id);
}

void ActorRuntime::Tick(uint64_t now_ms)
{
    for (auto& [id, actor] : actors_)
    {
        (void)id;
        actor->Tick(now_ms);
    }
}

void ActorRuntime::SendMessage(uint64_t from_id, uint64_t to_id, const std::vector<uint8_t>& payload)
{
    auto it = actors_.find(to_id);
    if (it != actors_.end())
    {
        it->second->HandleMessage(from_id, payload);
    }
}

Actor* ActorRuntime::GetActor(uint64_t actor_id)
{
    auto it = actors_.find(actor_id);
    return it != actors_.end() ? it->second.get() : nullptr;
}

} // namespace wasmh
