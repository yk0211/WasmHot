#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <unordered_map>

namespace wasmh {

// Gateway abstracts network communication. It is purely native C++:
// sockets, sessions, encryption, and routing happen here.
class Gateway
{
public:
    using MessageHandler = std::function<void(uint64_t session_id, uint64_t player_id,
                                               const std::vector<uint8_t>& msg)>;

    void RegisterHandler(uint32_t msg_type, MessageHandler handler);
    void OnReceive(uint64_t session_id, uint64_t player_id, const std::vector<uint8_t>& msg);
    void Send(uint64_t session_id, const std::vector<uint8_t>& msg);

private:
    std::unordered_map<uint32_t, MessageHandler> handlers_;
};

} // namespace wasmh
