#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "core/plugin_interface.h"

namespace wasmh {

class WasmEdgePlugin : public IPlugin {
 public:
  WasmEdgePlugin(std::string path, uint32_t schema_version);
  ~WasmEdgePlugin() override;

  bool Initialize() override;
  void Shutdown() override;
  bool Execute(GameObject& obj, const std::string& action, const std::vector<uint8_t>& input,
               std::vector<uint8_t>& output) override;
  [[nodiscard]] std::unique_ptr<IPlugin> Clone() const override;

 private:
  void ShutdownImpl();
  struct WasmEdgeState;
  std::unique_ptr<WasmEdgeState> state_;
  std::string path_;
  uint32_t schema_version_ = 0;
  bool initialized_ = false;
};

class WasmEdgePluginFactory : public PluginFactory {
 public:
  std::unique_ptr<IPlugin> Create(const std::string& path, uint32_t schema_version) override;
};

}  // namespace wasmh
