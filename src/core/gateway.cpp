#include "core/gateway.h"

#include <spdlog/spdlog.h>

#include <asio/post.hpp>
#include <cstring>
#include <deque>

#include "logging.h"

namespace wasmh {

class Gateway::Session : public std::enable_shared_from_this<Session> {
 public:
  Session(asio::ip::tcp::socket socket, uint64_t id, Gateway* gateway)
      : socket_(std::move(socket)), session_id_(id), gateway_(gateway) {}

  void Start() { DoReadHeader(); }

  void Send(const std::vector<uint8_t>& msg) {
    auto self = shared_from_this();
    asio::post(socket_.get_executor(), [self, msg]() {
      self->send_queue_.push_back(msg);
      if (!self->send_in_progress_) {
        self->send_in_progress_ = true;
        self->DoWrite();
      }
    });
  }

  uint64_t Id() const { return session_id_; }

 private:
  void DoReadHeader() {
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(&length_, sizeof(length_)),
                     [self](const asio::error_code& ec, std::size_t /*bytes*/) {
                       if (ec) {
                         self->HandleError(ec, "read");
                         return;
                       }
                       if (self->length_ < sizeof(uint32_t)) {
                         self->gateway_->OnSessionClosed(self->session_id_);
                         return;
                       }
                       self->body_buf_.resize(self->length_);
                       self->DoReadBody();
                     });
  }

  void DoReadBody() {
    auto self = shared_from_this();
    asio::async_read(socket_, asio::buffer(body_buf_), [self](const asio::error_code& ec, std::size_t /*bytes*/) {
      if (ec) {
        self->HandleError(ec, "read");
        return;
      }
      self->gateway_->OnReceive(self->session_id_, 0, self->body_buf_);
      self->DoReadHeader();
    });
  }

  void DoWrite() {
    if (send_queue_.empty()) {
      send_in_progress_ = false;
      return;
    }

    auto msg = std::move(send_queue_.front());
    send_queue_.pop_front();

    std::vector<uint8_t> framed;
    framed.reserve(sizeof(uint32_t) + msg.size());
    const auto len = static_cast<uint32_t>(msg.size());
    framed.resize(sizeof(uint32_t));
    std::memcpy(framed.data(), &len, sizeof(uint32_t));
    framed.insert(framed.end(), msg.begin(), msg.end());

    auto self = shared_from_this();
    asio::async_write(socket_, asio::buffer(framed),
                      [self, framed = std::move(framed)](const asio::error_code& ec, std::size_t /*bytes*/) {
                        if (ec) {
                          self->HandleError(ec, "write");
                          return;
                        }
                        self->DoWrite();
                      });
  }

  void HandleError(const asio::error_code& ec, const char* op) {
    if (ec != asio::error::eof && ec != asio::error::operation_aborted) {
      WARN("Gateway {} error: {}", op, ec.message());
    }
    gateway_->OnSessionClosed(session_id_);
  }

  asio::ip::tcp::socket socket_;
  uint64_t session_id_;
  Gateway* gateway_;
  uint32_t length_ = 0;
  std::vector<uint8_t> body_buf_;
  std::deque<std::vector<uint8_t>> send_queue_;
  bool send_in_progress_ = false;
};

int32_t Gateway::Initialize(asio::io_context& io, const std::string& ip, uint16_t port) {
  io_ = &io;

  asio::error_code ec;
  auto address = asio::ip::make_address(ip, ec);
  if (ec) {
    WARN("Invalid listen address '{}', falling back to 0.0.0.0", ip);
    address = asio::ip::address_v4::any();
  }

  acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(io);
  acceptor_->open(address.is_v6() ? asio::ip::tcp::v6() : asio::ip::tcp::v4(), ec);
  if (!ec && address.is_v6()) {
    (void)acceptor_->set_option(asio::ip::v6_only(false), ec);
    if (ec) {
      WARN("Failed to disable IPV6_V6ONLY: {}", ec.message());
    }
  }
  acceptor_->bind(asio::ip::tcp::endpoint(address, port), ec);
  if (ec) {
    ERROR("Failed to bind {}:{}: {}", ip, port, ec.message());
    return -1;
  }
  acceptor_->listen(asio::ip::tcp::acceptor::max_listen_connections, ec);
  if (ec) {
    ERROR("Failed to listen on {}:{}: {}", ip, port, ec.message());
    return -1;
  }

  DoAccept();
  return 0;
}

void Gateway::DoAccept() {
  acceptor_->async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
    if (!ec) {
      const uint64_t id = next_session_id_.fetch_add(1, std::memory_order_relaxed);
      auto session = std::make_shared<Session>(std::move(socket), id, this);
      {
        std::unique_lock lock(sessions_mutex_);
        sessions_[id] = session;
      }
      session->Start();
    }
    DoAccept();
  });
}

void Gateway::RegisterHandler(uint32_t msg_type, MessageHandler handler) {
  std::unique_lock lock(handlers_mutex_);
  handlers_[msg_type] = std::move(handler);
}

void Gateway::OnReceive(uint64_t session_id, uint64_t player_id, const std::vector<uint8_t>& msg) {
  if (msg.size() < sizeof(uint32_t))
    return;

  uint32_t msg_type = 0;
  std::memcpy(&msg_type, msg.data(), sizeof(uint32_t));

  MessageHandler handler;
  {
    std::shared_lock lock(handlers_mutex_);
    auto it = handlers_.find(msg_type);
    if (it == handlers_.end())
      return;
    handler = it->second;
  }
  handler(session_id, player_id, msg);
}

void Gateway::Send(uint64_t session_id, const std::vector<uint8_t>& msg) {
  std::shared_ptr<Session> session;
  {
    std::shared_lock lock(sessions_mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end())
      return;
    session = it->second;
  }
  session->Send(msg);
}

void Gateway::OnSessionClosed(uint64_t session_id) {
  std::unique_lock lock(sessions_mutex_);
  sessions_.erase(session_id);
}

}  // namespace wasmh
