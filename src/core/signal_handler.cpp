#include "core/signal_handler.h"

#include <jemalloc/jemalloc.h>
#include <unistd.h>

#include <atomic>
#include <csignal>
#include <cstdio>
#include <format>
#include <memory>

#include "core/logging.h"

namespace wasmh {

namespace {

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
      DumpHeap();
    }
    StartSignalWait(signals);
  });
}

}  // namespace

void SetupSignalHandler(asio::io_context& io) {
  auto signals = std::make_shared<asio::signal_set>(io, SIGUSR1);
  StartSignalWait(signals);
}

}  // namespace wasmh
