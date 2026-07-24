#pragma once
#include <array>
#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/singleton.h"

namespace wasmh {

// Gateway abstracts network communication. It is built on top of ASIO and
// provides a minimal asynchronous TCP server. Messages are framed as:
//   [length: uint32_t][msg_type: uint32_t][payload: length-4 bytes]
// The handler receives a vector that begins with the 4-byte msg_type.
class Gateway : public Singleton<Gateway> {
  friend class Singleton<Gateway>;

 public:
  using MessageHandler = std::function<void(uint64_t session_id, uint64_t player_id, const std::vector<uint8_t>& msg)>;

  int32_t Initialize(asio::io_context& io, const std::string& ip, uint16_t port);

  void RegisterHandler(uint32_t msg_type, MessageHandler handler);
  void OnReceive(uint64_t session_id, uint64_t player_id, const std::vector<uint8_t>& msg);
  void Send(uint64_t session_id, const std::vector<uint8_t>& msg);

  asio::io_context* GetIoContext() const { return io_; }

 private:
  Gateway() = default;

  class Session;
  void DoAccept();
  void OnSessionClosed(uint64_t session_id);

  asio::io_context* io_ = nullptr;
  std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
  std::unordered_map<uint32_t, MessageHandler> handlers_;
  mutable std::shared_mutex handlers_mutex_;
  std::unordered_map<uint64_t, std::shared_ptr<Session>> sessions_;
  mutable std::shared_mutex sessions_mutex_;
  std::atomic<uint64_t> next_session_id_ = 1;
};

}  // namespace wasmh
