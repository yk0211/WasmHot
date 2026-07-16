#include "plugin/wasmtime_plugin.h"

#if WASM_HOT_USE_WASMTIME
#include <wasmtime.h>

namespace wasmh {

struct WasmtimePlugin::WasmtimeState
{
    wasm_engine_t* engine = nullptr;
    wasm_store_t* store = nullptr;
    wasm_module_t* module = nullptr;
    wasm_instance_t* instance = nullptr;
    wasm_memory_t* memory = nullptr;
};

WasmtimePlugin::WasmtimePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version),
      state_(std::make_unique<WasmtimeState>()) {}

WasmtimePlugin::~WasmtimePlugin()
{
    Shutdown();
}

bool WasmtimePlugin::Initialize()
{
    state_->engine = wasm_engine_new();
    if (!state_->engine) return false;

    state_->store = wasm_store_new(state_->engine);
    if (!state_->store) return false;

    // Load WASM binary from disk.
    FILE* file = nullptr;
#if defined(_WIN32)
    fopen_s(&file, path_.c_str(), "rb");
#else
    file = fopen(path_.c_str(), "rb");
#endif
    if (!file) return false;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::vector<uint8_t> wasm_bytes(static_cast<size_t>(size));
    fread(wasm_bytes.data(), 1, wasm_bytes.size(), file);
    fclose(file);

    wasm_byte_t binary{wasm_bytes.data(), wasm_bytes.size()};
    state_->module = wasm_module_new(state_->store, &binary);
    if (!state_->module) return false;

    wasm_extern_vec_t imports = WASM_EMPTY_VEC;
    state_->instance = wasm_instance_new(state_->store, state_->module, &imports, nullptr);
    if (!state_->instance) return false;

    // Find exported memory.
    wasm_extern_vec_t exports;
    wasm_instance_exports(state_->instance, &exports);
    for (size_t i = 0; i < exports.size; ++i)
    {
        if (wasm_extern_kind(exports.data[i]) == WASM_EXTERN_MEMORY)
        {
            state_->memory = wasm_extern_as_memory(exports.data[i]);
            break;
        }
    }

    initialized_ = state_->memory != nullptr;
    return initialized_;
}

void WasmtimePlugin::Shutdown()
{
    if (state_)
    {
        if (state_->instance) wasm_instance_delete(state_->instance);
        if (state_->module) wasm_module_delete(state_->module);
        if (state_->store) wasm_store_delete(state_->store);
        if (state_->engine) wasm_engine_delete(state_->engine);
        state_->engine = nullptr;
        state_->store = nullptr;
        state_->module = nullptr;
        state_->instance = nullptr;
        state_->memory = nullptr;
    }
    initialized_ = false;
}

bool WasmtimePlugin::Execute(GameObject& obj, const std::string& action,
                             const std::vector<uint8_t>& input,
                             std::vector<uint8_t>& output)
{
    if (!initialized_ || !state_ || !state_->memory) return false;

    // 1. Serialize object components as input.
    // 2. Copy input into WASM memory.
    // 3. Call exported execute(action_hash, input_ptr, input_len, output_ptr, output_cap).
    // 4. Read output from WASM memory and apply to component storage.
    (void)obj;
    (void)action;
    (void)input;
    (void)output;
    (void)schema_version_;
    return true;
}

} // namespace wasmh

#else // WASM_HOT_USE_WASMTIME

namespace wasmh {

WasmtimePlugin::WasmtimePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version) {}

WasmtimePlugin::~WasmtimePlugin()
{
    Shutdown();
}

bool WasmtimePlugin::Initialize()
{
    return false;
}

void WasmtimePlugin::Shutdown()
{
    initialized_ = false;
}

bool WasmtimePlugin::Execute(GameObject& obj, const std::string& action,
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

std::unique_ptr<IPlugin> WasmtimePluginFactory::Create(const std::string& path, uint32_t schema_version)
{
    return std::make_unique<WasmtimePlugin>(path, schema_version);
}

} // namespace wasmh
