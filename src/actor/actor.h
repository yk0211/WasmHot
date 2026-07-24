#pragma once
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace wasmh {

class IPlugin;
class ModuleManager;
class ObjectRegistry;

struct ActorMessage {
  uint64_t sender_id = 0;
  std::vector<uint8_t> payload;
};

// Actor is the unit of concurrency and state encapsulation. Actors communicate
// exclusively through messages; no external code may invoke an actor's logic
// directly. Each actor owns a private mailbox and processes messages in order
// on its own strand.
class Actor : public std::enable_shared_from_this<Actor> {
 public:
  explicit Actor(asio::io_context& io);
  virtual ~Actor() = default;

  uint64_t actor_id = 0;

  // Enqueue a message into the actor's private mailbox. This is the only way
  // external code (or other actors) should communicate with this actor.
  void EnqueueMessage(uint64_t sender_id, const std::vector<uint8_t>& payload);

  // Schedule the actor's next tick. The tick (and pending messages) will run
  // sequentially on the actor's private strand.
  void ScheduleTick(uint64_t now_ms);

  auto& GetStrand() { return strand_; }

 protected:
  // Tick is called periodically by the runtime. It is the actor's
  // opportunity to perform scheduled work (e.g., timers, heartbeats).
  virtual void Tick(uint64_t interval_ms) = 0;

  // HandleMessage is invoked sequentially, one message at a time, by
  // ProcessMessages. Subclasses implement the actual message handling logic.
  virtual void HandleMessage(uint64_t sender_id, const std::vector<uint8_t>& payload) = 0;

  asio::strand<asio::io_context::executor_type> strand_;

 private:
  void ProcessMessages();

  std::mutex mailbox_mutex_;
  std::deque<ActorMessage> mailbox_;
};

// ActorWithObject provides common WASM module invocation for actors that
// operate on a single GameObject. Each actor owns its own plugin instances so
// that no runtime state is shared across actors. The actor does not keep a raw
// pointer to the GameObject; it looks up a temporary view through
// ObjectRegistry on demand.
class ActorWithObject : public Actor {
 protected:
  ActorWithObject(asio::io_context& io, uint64_t object_id, const std::vector<std::string>& module_names);

  void InvokeModule(const std::string& module_name, const std::string& action, const std::vector<uint8_t>& input);

  uint64_t object_id_ = 0;
  std::unordered_map<std::string, std::shared_ptr<IPlugin>> plugins_;
};

}  // namespace wasmh
