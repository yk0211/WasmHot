#include "plugin/native_plugin.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace wasmh {

NativePlugin::NativePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version) {}

NativePlugin::~NativePlugin()
{
    Shutdown();
}

bool NativePlugin::Initialize()
{
#if defined(_WIN32)
    handle_ = static_cast<void*>(LoadLibraryA(path_.c_str()));
#else
    handle_ = dlopen(path_.c_str(), RTLD_LAZY);
#endif
    return handle_ != nullptr;
}

void NativePlugin::Shutdown()
{
    if (handle_)
    {
#if defined(_WIN32)
        FreeLibrary(static_cast<HMODULE>(handle_));
#else
        dlclose(handle_);
#endif
        handle_ = nullptr;
    }
}

bool NativePlugin::Execute(GameObject& obj, const std::string& action,
                           const std::vector<uint8_t>& input,
                           std::vector<uint8_t>& output)
{
    // In a real implementation this would resolve platform symbols such as
    // plugin_execute(object_id, action_hash, input, input_len, output, output_cap).
    // For the architecture demonstration we keep it as a stateless no-op.
    (void)obj;
    (void)action;
    (void)input;
    (void)output;
    (void)schema_version_;
    return true;
}

std::unique_ptr<IPlugin> NativePluginFactory::Create(const std::string& path, uint32_t schema_version)
{
    return std::make_unique<NativePlugin>(path, schema_version);
}

} // namespace wasmh
