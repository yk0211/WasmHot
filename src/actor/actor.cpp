#include "actor/actor.h"
#include "core/module_manager.h"
#include "core/object.h"
#include "core/object_registry.h"
#include "core/plugin_interface.h"
#include <asio/post.hpp>

namespace wasmh {

Actor::Actor(asio::io_context& io)
    : strand_(asio::make_strand(io))
{
}

void Actor::EnqueueMessage(uint64_t sender_id, const std::vector<uint8_t>& payload)
{
    std::lock_guard lock(mailbox_mutex_);
    const bool was_empty = mailbox_.empty();
    mailbox_.push_back({sender_id, payload});
    if (was_empty) {
        asio::post(strand_, [self = shared_from_this()]() { self->ProcessMessages(); });
    }
}

void Actor::ProcessMessages()
{
    std::deque<ActorMessage> local_messages;
    {
        std::lock_guard lock(mailbox_mutex_);
        local_messages.swap(mailbox_);
    }
    for (const auto& msg : local_messages) {
        HandleMessage(msg.sender_id, msg.payload);
    }
}

void Actor::ScheduleTick(uint64_t now_ms)
{
    asio::post(strand_, [self = shared_from_this(), now_ms]() {
        self->ProcessMessages();
        self->Tick(now_ms);
    });
}

ActorWithObject::ActorWithObject(asio::io_context& io, uint64_t object_id,
                                 const std::vector<std::string>& module_names)
    : Actor(io), object_id_(object_id)
{
    auto* modules = ModuleManager::Instance();
    for (const auto& name : module_names) {
        auto plugin = modules->Get(name);
        if (!plugin) continue;
        auto cloned = plugin->Clone();
        if (!cloned) continue;
        plugins_[name] = std::shared_ptr<IPlugin>(cloned.release());
    }
}

void ActorWithObject::InvokeModule(const std::string& module_name,
                                   const std::string& action,
                                   const std::vector<uint8_t>& input)
{
    auto it = plugins_.find(module_name);
    if (it == plugins_.end() || !it->second) return;

    std::shared_ptr<IPlugin> plugin = it->second;
    ObjectRegistry::Instance()->Get(object_id_, [plugin, action, input](GameObject* obj) {
        if (!obj) return;

        std::vector<uint8_t> output;
        if (!plugin->Execute(*obj, action, input, output)) return;

        obj->ApplyOutputDelta(output);
    });
}

} // namespace wasmh
