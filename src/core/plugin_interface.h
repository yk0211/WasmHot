#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/object.h"

namespace wasmh {

// IPlugin is the interface for all logic plugins: native .so, WASM, or Lua.
// A plugin must be stateless regarding persistent data. It receives a
// GameObject snapshot, computes, and returns output bytes.
class IPlugin {
 public:
  virtual ~IPlugin() = default;
  virtual bool Initialize() = 0;
  virtual void Shutdown() = 0;
  virtual bool Execute(GameObject& obj, const std::string& action,
                       const std::vector<uint8_t>& input,
                       std::vector<uint8_t>& output) = 0;

  // Create an independent, initialized copy of this plugin instance.
  // Each actor receives its own clone so that no runtime state is shared.
  [[nodiscard]] virtual std::unique_ptr<IPlugin> Clone() const = 0;
};

// PluginFactory creates plugin instances based on their backend.
class PluginFactory {
 public:
  virtual ~PluginFactory() = default;
  virtual std::unique_ptr<IPlugin> Create(const std::string& path,
                                          uint32_t schema_version) = 0;
};

}  // namespace wasmh
