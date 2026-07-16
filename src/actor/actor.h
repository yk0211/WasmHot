#pragma once
#include <cstdint>
#include <vector>
#include <memory>

namespace wasmh {

// Actor is the unit of scheduling in the Native C++ runtime.
// WASM logic is invoked inside Tick/HandleMessage, but the actor lifecycle
// (creation, destruction, message queue, timers) is managed by C++.
class Actor
{
public:
    uint64_t actor_id = 0;
    virtual ~Actor() = default;
    virtual void Tick(uint64_t now_ms) = 0;
    virtual void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) = 0;
};

} // namespace wasmh
