#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include "actor/actor.h"

namespace wasmh {

class ActorRuntime
{
public:
    void Spawn(std::unique_ptr<Actor> actor);
    void Kill(uint64_t actor_id);
    void Tick(uint64_t now_ms);
    void SendMessage(uint64_t from_id, uint64_t to_id, const std::vector<uint8_t>& payload);
    Actor* GetActor(uint64_t actor_id);

private:
    std::unordered_map<uint64_t, std::unique_ptr<Actor>> actors_;
};

} // namespace wasmh
