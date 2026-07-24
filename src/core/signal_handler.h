#pragma once

#include <asio.hpp>

namespace wasmh {

// Registers asynchronous signal handlers with the given io_context.
// Currently handles SIGUSR1 by dumping a jemalloc heap profile.
void SetupSignalHandler(asio::io_context& io);

}  // namespace wasmh
