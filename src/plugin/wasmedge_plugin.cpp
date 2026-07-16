#include "plugin/wasmedge_plugin.h"

#if WASM_HOT_USE_WASMEDGE
#include <wasmedge/wasmedge.h>

namespace wasmh {

struct WasmEdgePlugin::WasmEdgeState
{
    WasmEdge_ConfigureContext* config = nullptr;
    WasmEdge_VMContext* vm = nullptr;
};

WasmEdgePlugin::WasmEdgePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version),
      state_(std::make_unique<WasmEdgeState>()) {}

WasmEdgePlugin::~WasmEdgePlugin()
{
    Shutdown();
}

bool WasmEdgePlugin::Initialize()
{
    state_->config = WasmEdge_ConfigureCreate();
    if (!state_->config) return false;

    state_->vm = WasmEdge_VMCreate(state_->config, nullptr);
    if (!state_->vm) return false;

    // Load, validate, and instantiate the WASM module once.
    WasmEdge_Result res = WasmEdge_VMLoadWasmFromFile(state_->vm, path_.c_str());
    if (!WasmEdge_ResultOK(res)) return false;

    res = WasmEdge_VMValidate(state_->vm);
    if (!WasmEdge_ResultOK(res)) return false;

    res = WasmEdge_VMInstantiate(state_->vm);
    if (!WasmEdge_ResultOK(res)) return false;

    initialized_ = true;
    return true;
}

void WasmEdgePlugin::Shutdown()
{
    if (state_)
    {
        if (state_->vm) WasmEdge_VMDelete(state_->vm);
        if (state_->config) WasmEdge_ConfigureDelete(state_->config);
        state_->vm = nullptr;
        state_->config = nullptr;
    }
    initialized_ = false;
}

bool WasmEdgePlugin::Execute(GameObject& obj, const std::string& action,
                             const std::vector<uint8_t>& input,
                             std::vector<uint8_t>& output)
{
    if (!initialized_ || !state_ || !state_->vm) return false;

    // Real implementation steps:
    // 1. Get the store and exported memory from the VM.
    // 2. Write serialized input data into WASM memory.
    // 3. Call the exported execute(action_hash, input_ptr, input_len, output_ptr, output_cap).
    // 4. Read output from WASM memory and apply it to the native component storage.
    (void)obj;
    (void)action;
    (void)input;
    (void)output;
    (void)schema_version_;
    return true;
}

} // namespace wasmh

#else // WASM_HOT_USE_WASMEDGE

namespace wasmh {

WasmEdgePlugin::WasmEdgePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version) {}

WasmEdgePlugin::~WasmEdgePlugin()
{
    Shutdown();
}

bool WasmEdgePlugin::Initialize()
{
    return false;
}

void WasmEdgePlugin::Shutdown()
{
    initialized_ = false;
}

bool WasmEdgePlugin::Execute(GameObject& obj, const std::string& action,
                             const std::vector<uint8_t>& input,
                             std::vector<uint8_t>& output)
{
    (void)obj;
    (void)action;
    (void)input;
    (void)output;
    (void)path_;
    (void)schema_version_;
    return false;
}

} // namespace wasmh

#endif

namespace wasmh {

std::unique_ptr<IPlugin> WasmEdgePluginFactory::Create(const std::string& path, uint32_t schema_version)
{
    return std::make_unique<WasmEdgePlugin>(path, schema_version);
}

} // namespace wasmh
