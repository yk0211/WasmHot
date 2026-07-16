#include "network/gateway.h"
#include <cstring>

namespace wasmh {

void Gateway::RegisterHandler(uint32_t msg_type, MessageHandler handler)
{
    handlers_[msg_type] = std::move(handler);
}

void Gateway::OnReceive(uint64_t session_id, uint64_t player_id, const std::vector<uint8_t>& msg)
{
    if (msg.size() < sizeof(uint32_t)) return;

    uint32_t msg_type = 0;
    std::memcpy(&msg_type, msg.data(), sizeof(uint32_t));

    auto it = handlers_.find(msg_type);
    if (it != handlers_.end())
    {
        it->second(session_id, player_id, msg);
    }
}

void Gateway::Send(uint64_t session_id, const std::vector<uint8_t>& msg)
{
    // Native network send would happen here.
    (void)session_id;
    (void)msg;
}

} // namespace wasmh
