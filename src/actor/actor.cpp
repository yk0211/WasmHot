#include "actor/actor.h"
#include "core/object.h"
#include "core/module_manager.h"
#include "core/plugin_interface.h"

namespace wasmh {

ActorWithObject::ActorWithObject(GameObject* obj, ModuleManager* modules)
    : target_object_(obj), modules_(modules)
{
}

void ActorWithObject::InvokeModule(const std::string& module_name,
                                   const std::string& action,
                                   const std::vector<uint8_t>& input)
{
    if (!modules_ || !target_object_) return;

    auto plugin = modules_->Get(module_name);
    if (!plugin) return;

    std::vector<uint8_t> output;
    if (!plugin->Execute(*target_object_, action, input, output)) return;

    target_object_->ApplyOutputDelta(output);
}

} // namespace wasmh
