#include "plugin/wasmedge_plugin.h"
#include <wasmedge/wasmedge.h>

namespace {
uint32_t djb2(const std::string& str) {
    uint32_t hash = 5381;
    for (unsigned char c : str) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}
} // namespace

namespace wasmh {

struct WasmEdgePlugin::WasmEdgeState
{
    WasmEdge_VMContext* vm = nullptr;
    WasmEdge_MemoryInstanceContext* memory = nullptr;
};

WasmEdgePlugin::WasmEdgePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version) {}

WasmEdgePlugin::~WasmEdgePlugin()
{
    Shutdown();
}

bool WasmEdgePlugin::Initialize()
{
    if (initialized_) return true;

    state_ = std::make_unique<WasmEdgeState>();
    state_->vm = WasmEdge_VMCreate(nullptr, nullptr);
    if (!state_->vm) {
        state_.reset();
        return false;
    }

    auto res = WasmEdge_VMLoadWasmFromFile(state_->vm, path_.c_str());
    if (!WasmEdge_ResultOK(res)) {
        WasmEdge_VMDelete(state_->vm);
        state_.reset();
        return false;
    }

    res = WasmEdge_VMValidate(state_->vm);
    if (!WasmEdge_ResultOK(res)) {
        WasmEdge_VMDelete(state_->vm);
        state_.reset();
        return false;
    }

    res = WasmEdge_VMInstantiate(state_->vm);
    if (!WasmEdge_ResultOK(res)) {
        WasmEdge_VMDelete(state_->vm);
        state_.reset();
        return false;
    }

    auto* module = WasmEdge_VMGetActiveModule(state_->vm);
    if (module) {
        auto mem_name = WasmEdge_StringCreateByCString("memory");
        state_->memory = WasmEdge_ModuleInstanceFindMemory(module, mem_name);
        WasmEdge_StringDelete(mem_name);
    }

    initialized_ = true;
    return true;
}

void WasmEdgePlugin::Shutdown()
{
    if (state_) {
        if (state_->vm) {
            WasmEdge_VMDelete(state_->vm);
            state_->vm = nullptr;
        }
        state_->memory = nullptr;
    }
    initialized_ = false;
}

bool WasmEdgePlugin::Execute(GameObject& obj, const std::string& action,
                             const std::vector<uint8_t>& input,
                             std::vector<uint8_t>& output)
{
    (void)obj;
    (void)schema_version_;

    if (!initialized_ || !state_ || !state_->vm) return false;

    const uint32_t input_offset = 0;
    const uint32_t output_offset = 4096;
    const uint32_t output_cap = 4096;

    if (state_->memory && !input.empty()) {
        if (input.size() > output_offset) return false;
        auto res = WasmEdge_MemoryInstanceSetData(
            state_->memory, input.data(), input_offset, input.size());
        if (!WasmEdge_ResultOK(res)) return false;
    }

    auto func_name = WasmEdge_StringCreateByCString("execute");
    int32_t action_hash = static_cast<int32_t>(djb2(action));
    WasmEdge_Value params[5] = {
        WasmEdge_ValueGenI32(action_hash),
        WasmEdge_ValueGenI32(static_cast<int32_t>(input_offset)),
        WasmEdge_ValueGenI32(static_cast<int32_t>(input.size())),
        WasmEdge_ValueGenI32(static_cast<int32_t>(output_offset)),
        WasmEdge_ValueGenI32(static_cast<int32_t>(output_cap))
    };
    WasmEdge_Value ret;
    auto res = WasmEdge_VMExecute(state_->vm, func_name, params, 5, &ret, 1);
    WasmEdge_StringDelete(func_name);

    if (!WasmEdge_ResultOK(res)) return false;

    int32_t ret_len = WasmEdge_ValueGetI32(ret);
    if (ret_len < 0) return false;

    if (ret_len > 0) {
        if (ret_len > static_cast<int32_t>(output_cap)) return false;
        output.resize(ret_len);
        if (state_->memory) {
            auto res2 = WasmEdge_MemoryInstanceGetData(
                state_->memory, output.data(), output_offset, ret_len);
            if (!WasmEdge_ResultOK(res2)) return false;
        } else {
            output.clear();
        }
    } else {
        output.clear();
    }

    return true;
}

std::unique_ptr<IPlugin> WasmEdgePluginFactory::Create(const std::string& path, uint32_t schema_version)
{
    return std::make_unique<WasmEdgePlugin>(path, schema_version);
}

} // namespace wasmh
