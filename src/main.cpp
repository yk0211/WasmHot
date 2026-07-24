#include <jemalloc/jemalloc.h>
#include <spdlog/common.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

#include <CLI/CLI.hpp>
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "actor/actor_runtime.h"
#include "core/component_storage.h"
#include "core/gateway.h"
#include "core/logging.h"
#include "core/migration_engine.h"
#include "core/module_manager.h"
#include "core/object_registry.h"
#include "core/schema_manager.h"
#include "plugin/wasmedge_plugin.h"

const uint64_t g_interval_ms = 16ULL;

struct ServerConfig {
  std::string server_name;
  std::string listen_ip = "127.0.0.1";
  uint16_t listen_port = 8080;
  std::size_t thread_num = 1;
  std::string log_level = "info";
  std::string flush_log_level = "warn";
  std::size_t max_file_size = 100ULL * 1024 * 1024;
  std::size_t max_rotate_file_num = 10;
};

int32_t LoadServerConfig(const std::string& config_path, ServerConfig& server_config) {
  try {
    YAML::Node config = YAML::LoadFile(config_path);
    server_config.server_name = config["server_name"].as<std::string>();
    server_config.listen_ip = config["listen_ip"].as<std::string>();
    server_config.listen_port = config["listen_port"].as<uint16_t>();
    server_config.thread_num = config["thread_num"].as<std::size_t>();
    server_config.log_level = config["log_level"].as<std::string>();
    server_config.flush_log_level = config["flush_log_level"].as<std::string>();
    server_config.max_file_size = config["max_file_size"].as<std::size_t>();
    server_config.max_rotate_file_num = config["max_rotate_file_num"].as<std::size_t>();
  } catch (const YAML::BadFile& e) {
    std::fprintf(stderr, "Error: failed to open config file: %s\n", e.what());
    return -1;
  } catch (const YAML::ParserException& e) {
    std::fprintf(stderr, "Error: invalid YAML format: %s\n", e.what());
    return -1;
  } catch (const YAML::Exception& e) {
    std::fprintf(stderr, "Error: failed to read config field: %s\n", e.what());
    return -1;
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error: unexpected exception while reading config: %s\n", e.what());
    return -1;
  } catch (...) {
    std::fprintf(stderr, "Error: unknown exception while reading config\n");
    return -1;
  }

  return 0;
}

void StartActorTickLoop(asio::io_context& io, uint64_t interval_ms) {
  auto tick_timer = std::make_shared<asio::steady_timer>(io);
  auto schedule_tick = std::make_shared<std::function<void(const asio::error_code&)>>();
  *schedule_tick = [tick_timer, interval_ms, schedule_tick](const asio::error_code& ec) {
    if (ec)
      return;
    wasmh::ActorRuntime::Instance()->Tick(interval_ms);
    tick_timer->expires_after(std::chrono::milliseconds(interval_ms));
    tick_timer->async_wait(*schedule_tick);
  };
  tick_timer->expires_after(std::chrono::milliseconds(interval_ms));
  tick_timer->async_wait(*schedule_tick);
}

void DumpHeap() {
  static std::atomic<int32_t> num{0};
  const int32_t seq = num.fetch_add(1, std::memory_order_relaxed) + 1;
  const int32_t pid = getpid();
  const std::string filename = std::format("jeprof.{}.{}.heap", pid, seq);
  const char* filename_str = filename.data();
  const int32_t err = mallctl("prof.dump", nullptr, nullptr, (void*)&filename_str, sizeof(filename_str));
  if (err != 0) {
    ERROR("mallctl prof.dump failed, errno={}, filename={}", err, filename.data());
    return;
  }

  INFO("Heap dump saved to {}", filename.data());
}

void StartSignalWait(const std::shared_ptr<asio::signal_set>& signals) {
  signals->async_wait([signals](const asio::error_code& ec, int signal_number) {
    if (ec) {
      return;
    }
    if (signal_number == SIGUSR1) {
      INFO("SIGUSR1 received, starting dump heap");
      DumpHeap();
    } else if (signal_number == SIGHUP) {
      INFO("SIGHUP received, starting hot reload");
      if (!wasmh::ModuleManager::Instance()->HotReloadAll()) {
        WARN("Hot reload completed with failures");
      }
    }
    StartSignalWait(signals);
  });
}

void SetupSignalHandler(asio::io_context& io) {
  auto signals = std::make_shared<asio::signal_set>(io, SIGUSR1, SIGHUP);
  StartSignalWait(signals);
}

int32_t main(int32_t argc, char* argv[]) {
  try {
    std::string config_path;

    CLI::App app;
    app.add_option("--config", config_path, "Path to configuration file (yaml/yml)")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    ServerConfig cfg;
    if (LoadServerConfig(config_path, cfg) != 0) {
      std::fprintf(stderr, "Failed to load config: %s\n", config_path.c_str());
      return -1;
    }

    wasmh::InitLogging(cfg.server_name, cfg.log_level, cfg.flush_log_level, cfg.max_file_size, cfg.max_rotate_file_num);
    INFO("Init logging succeed, server_name={} log_level={} flush_log_level={} max_file_size={} max_rotate_file_num={}",
         cfg.server_name, cfg.log_level, cfg.flush_log_level, cfg.max_file_size, cfg.max_rotate_file_num);

    // Initialize singleton managers.
    wasmh::ModuleManager::Instance()->Initialize(std::make_unique<wasmh::WasmEdgePluginFactory>());

    asio::io_context io(static_cast<int>(cfg.thread_num));
    wasmh::ActorRuntime::Instance()->Initialize(io);
    CHECK_WITH_ERROR_LOG(wasmh::Gateway::Instance()->Initialize(io, cfg.listen_ip, cfg.listen_port) == 0,
                         "Failed to init gateway");
    INFO("Init gateway succeed, ip={} port={}", cfg.listen_ip, cfg.listen_port);

    StartActorTickLoop(io, g_interval_ms);
    SetupSignalHandler(io);

    std::vector<std::thread> threads;
    threads.reserve(cfg.thread_num);
    for (size_t i = 0; i < cfg.thread_num; ++i) {
      threads.emplace_back([&io]() { io.run(); });
    }
    for (auto& t : threads) {
      t.join();
    }
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Fatal error: %s\n", e.what());
    return -1;
  } catch (...) {
    std::fprintf(stderr, "Fatal error: unknown exception\n");
    return -1;
  }

  return 0;
}
