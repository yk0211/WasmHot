#include "plugin/wasmedge_plugin.h"

#include <city.h>
#include <wasmedge/wasmedge.h>

#include "core/component_storage.h"

namespace {
struct HostContext {
  wasmh::ComponentStorage* storage = nullptr;
  uint64_t object_id = 0;
};

thread_local HostContext g_host_ctx;

WasmEdge_Result HostGetCurrentObjectId(void*,
                                       const WasmEdge_CallingFrameContext*,
                                       const WasmEdge_Value*,
                                       WasmEdge_Value* out) {
  out[0] = WasmEdge_ValueGenI64(static_cast<int64_t>(g_host_ctx.object_id));
  return WasmEdge_Result_Success;
}

WasmEdge_Result HostGetComponent(void*,
                                 const WasmEdge_CallingFrameContext* frame,
                                 const WasmEdge_Value* in,
                                 WasmEdge_Value* out) {
  uint64_t object_id = static_cast<uint64_t>(WasmEdge_ValueGetI64(in[0]));
  uint32_t type = static_cast<uint32_t>(WasmEdge_ValueGetI32(in[1]));
  uint32_t buf_ptr = static_cast<uint32_t>(WasmEdge_ValueGetI32(in[2]));
  uint32_t buf_cap = static_cast<uint32_t>(WasmEdge_ValueGetI32(in[3]));

  if (!g_host_ctx.storage) {
    out[0] = WasmEdge_ValueGenI32(-1);
    return WasmEdge_Result_Success;
  }

  g_host_ctx.storage->Read(
      object_id, type,
      [frame, buf_ptr, buf_cap, out](const wasmh::ComponentData* data) {
        if (!data) {
          out[0] = WasmEdge_ValueGenI32(0);
          return;
        }

        uint32_t len = static_cast<uint32_t>(data->size());
        if (len > buf_cap) {
          out[0] = WasmEdge_ValueGenI32(-2);
          return;
        }

        if (len > 0) {
          WasmEdge_MemoryInstanceContext* memory =
              WasmEdge_CallingFrameGetMemoryInstance(frame, 0);
          if (!memory) {
            out[0] = WasmEdge_ValueGenI32(-3);
            return;
          }
          auto res = WasmEdge_MemoryInstanceSetData(memory, data->data(),
                                                    buf_ptr, len);
          if (!WasmEdge_ResultOK(res)) {
            out[0] = WasmEdge_ValueGenI32(-3);
            return;
          }
        }

        out[0] = WasmEdge_ValueGenI32(static_cast<int32_t>(len));
      });
  return WasmEdge_Result_Success;
}

WasmEdge_Result HostSetComponent(void*,
                                 const WasmEdge_CallingFrameContext* frame,
                                 const WasmEdge_Value* in,
                                 WasmEdge_Value* out) {
  uint64_t object_id = static_cast<uint64_t>(WasmEdge_ValueGetI64(in[0]));
  uint32_t type = static_cast<uint32_t>(WasmEdge_ValueGetI32(in[1]));
  uint32_t buf_ptr = static_cast<uint32_t>(WasmEdge_ValueGetI32(in[2]));
  uint32_t len = static_cast<uint32_t>(WasmEdge_ValueGetI32(in[3]));

  if (!g_host_ctx.storage) {
    out[0] = WasmEdge_ValueGenI32(0);
    return WasmEdge_Result_Success;
  }

  wasmh::ComponentData data(len);
  if (len > 0) {
    WasmEdge_MemoryInstanceContext* memory =
        WasmEdge_CallingFrameGetMemoryInstance(frame, 0);
    if (!memory) {
      out[0] = WasmEdge_ValueGenI32(0);
      return WasmEdge_Result_Success;
    }
    auto res =
        WasmEdge_MemoryInstanceGetData(memory, data.data(), buf_ptr, len);
    if (!WasmEdge_ResultOK(res)) {
      out[0] = WasmEdge_ValueGenI32(0);
      return WasmEdge_Result_Success;
    }
  }

  g_host_ctx.storage->Write(object_id, type, std::move(data));
  out[0] = WasmEdge_ValueGenI32(1);
  return WasmEdge_Result_Success;
}

WasmEdge_ModuleInstanceContext* CreateHostModule() {
  auto module_name = WasmEdge_StringCreateByCString("host");
  auto* module = WasmEdge_ModuleInstanceCreate(module_name);
  WasmEdge_StringDelete(module_name);
  if (!module)
    return nullptr;

  WasmEdge_ValType get_id_out[] = {WasmEdge_ValTypeGenI64()};
  auto* get_id_type = WasmEdge_FunctionTypeCreate(nullptr, 0, get_id_out, 1);
  auto* get_id_func = WasmEdge_FunctionInstanceCreate(
      get_id_type, HostGetCurrentObjectId, nullptr, 0);
  WasmEdge_FunctionTypeDelete(get_id_type);
  auto get_id_name = WasmEdge_StringCreateByCString("get_current_object_id");
  WasmEdge_ModuleInstanceAddFunction(module, get_id_name, get_id_func);
  WasmEdge_StringDelete(get_id_name);

  WasmEdge_ValType comp_in[] = {
      WasmEdge_ValTypeGenI64(), WasmEdge_ValTypeGenI32(),
      WasmEdge_ValTypeGenI32(), WasmEdge_ValTypeGenI32()};
  WasmEdge_ValType comp_out[] = {WasmEdge_ValTypeGenI32()};

  auto* get_type = WasmEdge_FunctionTypeCreate(comp_in, 4, comp_out, 1);
  auto* get_func =
      WasmEdge_FunctionInstanceCreate(get_type, HostGetComponent, nullptr, 0);
  WasmEdge_FunctionTypeDelete(get_type);
  auto get_name = WasmEdge_StringCreateByCString("get_component");
  WasmEdge_ModuleInstanceAddFunction(module, get_name, get_func);
  WasmEdge_StringDelete(get_name);

  auto* set_type = WasmEdge_FunctionTypeCreate(comp_in, 4, comp_out, 1);
  auto* set_func =
      WasmEdge_FunctionInstanceCreate(set_type, HostSetComponent, nullptr, 0);
  WasmEdge_FunctionTypeDelete(set_type);
  auto set_name = WasmEdge_StringCreateByCString("set_component");
  WasmEdge_ModuleInstanceAddFunction(module, set_name, set_func);
  WasmEdge_StringDelete(set_name);

  return module;
}

}  // namespace

namespace wasmh {

struct WasmEdgePlugin::WasmEdgeState {
  WasmEdge_VMContext* vm = nullptr;
  WasmEdge_MemoryInstanceContext* memory = nullptr;
  WasmEdge_ModuleInstanceContext* host_module = nullptr;
};

WasmEdgePlugin::WasmEdgePlugin(const std::string& path, uint32_t schema_version)
    : path_(path), schema_version_(schema_version) {}

WasmEdgePlugin::~WasmEdgePlugin() {
  Shutdown();
}

bool WasmEdgePlugin::Initialize() {
  if (initialized_)
    return true;

  state_ = std::make_unique<WasmEdgeState>();
  state_->vm = WasmEdge_VMCreate(nullptr, nullptr);
  if (!state_->vm) {
    state_.reset();
    return false;
  }

  state_->host_module = CreateHostModule();
  if (!state_->host_module) {
    WasmEdge_VMDelete(state_->vm);
    state_.reset();
    return false;
  }

  auto res = WasmEdge_VMLoadWasmFromFile(state_->vm, path_.c_str());
  if (!WasmEdge_ResultOK(res)) {
    WasmEdge_ModuleInstanceDelete(state_->host_module);
    WasmEdge_VMDelete(state_->vm);
    state_.reset();
    return false;
  }

  res = WasmEdge_VMValidate(state_->vm);
  if (!WasmEdge_ResultOK(res)) {
    WasmEdge_ModuleInstanceDelete(state_->host_module);
    WasmEdge_VMDelete(state_->vm);
    state_.reset();
    return false;
  }

  res = WasmEdge_VMRegisterModuleFromImport(state_->vm, state_->host_module);
  if (!WasmEdge_ResultOK(res)) {
    WasmEdge_ModuleInstanceDelete(state_->host_module);
    WasmEdge_VMDelete(state_->vm);
    state_.reset();
    return false;
  }

  res = WasmEdge_VMInstantiate(state_->vm);
  if (!WasmEdge_ResultOK(res)) {
    WasmEdge_ModuleInstanceDelete(state_->host_module);
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

void WasmEdgePlugin::Shutdown() {
  if (state_) {
    if (state_->vm) {
      WasmEdge_VMDelete(state_->vm);
      state_->vm = nullptr;
    }
    if (state_->host_module) {
      WasmEdge_ModuleInstanceDelete(state_->host_module);
      state_->host_module = nullptr;
    }
    state_->memory = nullptr;
  }
  initialized_ = false;
}

bool WasmEdgePlugin::Execute(GameObject& obj, const std::string& action,
                             const std::vector<uint8_t>& input,
                             std::vector<uint8_t>& output) {
  (void)schema_version_;

  if (!initialized_ || !state_ || !state_->vm)
    return false;

  g_host_ctx.storage = obj.storage;
  g_host_ctx.object_id = obj.header.object_id;

  const uint32_t input_offset = 0;
  const uint32_t output_offset = 4096;
  const uint32_t output_cap = 4096;

  if (state_->memory && !input.empty()) {
    if (input.size() > output_offset)
      return false;
    auto res = WasmEdge_MemoryInstanceSetData(state_->memory, input.data(),
                                              input_offset, input.size());
    if (!WasmEdge_ResultOK(res))
      return false;
  }

  auto func_name = WasmEdge_StringCreateByCString("execute");
  int64_t action_hash =
      static_cast<int64_t>(CityHash64(action.data(), action.size()));
  WasmEdge_Value params[5] = {
      WasmEdge_ValueGenI64(action_hash),
      WasmEdge_ValueGenI32(static_cast<int32_t>(input_offset)),
      WasmEdge_ValueGenI32(static_cast<int32_t>(input.size())),
      WasmEdge_ValueGenI32(static_cast<int32_t>(output_offset)),
      WasmEdge_ValueGenI32(static_cast<int32_t>(output_cap))};
  WasmEdge_Value ret;
  auto res = WasmEdge_VMExecute(state_->vm, func_name, params, 5, &ret, 1);
  WasmEdge_StringDelete(func_name);

  if (!WasmEdge_ResultOK(res))
    return false;

  int32_t ret_len = WasmEdge_ValueGetI32(ret);
  if (ret_len < 0)
    return false;

  if (ret_len > 0) {
    if (ret_len > static_cast<int32_t>(output_cap))
      return false;
    output.resize(ret_len);
    if (state_->memory) {
      auto res2 = WasmEdge_MemoryInstanceGetData(state_->memory, output.data(),
                                                 output_offset, ret_len);
      if (!WasmEdge_ResultOK(res2))
        return false;
    } else {
      output.clear();
    }
  } else {
    output.clear();
  }

  return true;
}

std::unique_ptr<IPlugin> WasmEdgePlugin::Clone() const {
  auto cloned = std::make_unique<WasmEdgePlugin>(path_, schema_version_);
  if (!cloned->Initialize()) {
    return nullptr;
  }
  return cloned;
}

std::unique_ptr<IPlugin> WasmEdgePluginFactory::Create(
    const std::string& path, uint32_t schema_version) {
  return std::make_unique<WasmEdgePlugin>(path, schema_version);
}

}  // namespace wasmh
