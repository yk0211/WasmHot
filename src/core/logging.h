#pragma once

#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <cstdint>

namespace wasmh {

// Configures spdlog with the project-wide format and rotation policy.
// Format: [time][pid][tid][function:file:line] message
// File:    <current_date>.log, rotating at 100MB with suffixes .1, .2, ...
void InitLogging(const std::string& program_name, const std::string log_level,
                 const std::string flush_log_level, std::size_t file_size,
                 std::size_t rotate_file_num);

}  // namespace wasmh

// Project-wide logging macros. They delegate to spdlog and automatically
// capture source location (file, function, line) via SPDLOG_* macros.
#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define FATAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

// Helper macros for generating unique variable names per call site.
#define LOG_DETAIL_CONCAT_IMPL(a, b) a##b
#define LOG_DETAIL_CONCAT(a, b) LOG_DETAIL_CONCAT_IMPL(a, b)
#define LOG_DETAIL_UNIQUE_NAME(prefix) LOG_DETAIL_CONCAT(prefix, __LINE__)

// Log once every N invocations.
#define LOG_EVERY_N(level, n, ...)                                           \
  do {                                                                       \
    static std::atomic<int64_t> LOG_DETAIL_UNIQUE_NAME(log_every_n_counter){ \
        0};                                                                  \
    auto count = LOG_DETAIL_UNIQUE_NAME(log_every_n_counter)                 \
                     .fetch_add(1, std::memory_order_relaxed);               \
    if ((count % (n)) == 0) {                                                \
      level(__VA_ARGS__);                                                    \
    }                                                                        \
  } while (0)

// Log only for the first N invocations.
#define LOG_FIRST_N(level, n, ...)                                           \
  do {                                                                       \
    static std::atomic<int64_t> LOG_DETAIL_UNIQUE_NAME(log_first_n_counter){ \
        0};                                                                  \
    auto count = LOG_DETAIL_UNIQUE_NAME(log_first_n_counter)                 \
                     .fetch_add(1, std::memory_order_relaxed);               \
    if (count < (n)) {                                                       \
      level(__VA_ARGS__);                                                    \
    }                                                                        \
  } while (0)

// Log at most once every N milliseconds.
#define LOG_EVERY_MS(level, ms, ...)                                          \
  do {                                                                        \
    static std::atomic<int64_t> LOG_DETAIL_UNIQUE_NAME(log_every_ms_last){0}; \
    auto now_ms = static_cast<int64_t>(                                       \
        std::chrono::duration_cast<std::chrono::milliseconds>(                \
            std::chrono::steady_clock::now().time_since_epoch())              \
            .count());                                                        \
    int64_t last = LOG_DETAIL_UNIQUE_NAME(log_every_ms_last)                  \
                       .load(std::memory_order_relaxed);                      \
    if (now_ms - last >= (ms) &&                                              \
        LOG_DETAIL_UNIQUE_NAME(log_every_ms_last)                             \
            .compare_exchange_weak(last, now_ms, std::memory_order_relaxed,   \
                                   std::memory_order_relaxed)) {              \
      level(__VA_ARGS__);                                                     \
    }                                                                         \
  } while (0)

// Log at most once every N seconds.
#define LOG_EVERY_SEC(level, sec, ...) \
  LOG_EVERY_MS(level, (sec) * 1000, __VA_ARGS__)

// Log only once.
#define LOG_ONCE(level, ...) LOG_FIRST_N(level, 1, __VA_ARGS__)

// Convenience macros for each log level.
#define TRACE_EVERY_N(n, ...) LOG_EVERY_N(TRACE, n, __VA_ARGS__)
#define DEBUG_EVERY_N(n, ...) LOG_EVERY_N(DEBUG, n, __VA_ARGS__)
#define INFO_EVERY_N(n, ...) LOG_EVERY_N(INFO, n, __VA_ARGS__)
#define WARN_EVERY_N(n, ...) LOG_EVERY_N(WARN, n, __VA_ARGS__)
#define ERROR_EVERY_N(n, ...) LOG_EVERY_N(ERROR, n, __VA_ARGS__)
#define FATAL_EVERY_N(n, ...) LOG_EVERY_N(FATAL, n, __VA_ARGS__)

#define TRACE_FIRST_N(n, ...) LOG_FIRST_N(TRACE, n, __VA_ARGS__)
#define DEBUG_FIRST_N(n, ...) LOG_FIRST_N(DEBUG, n, __VA_ARGS__)
#define INFO_FIRST_N(n, ...) LOG_FIRST_N(INFO, n, __VA_ARGS__)
#define WARN_FIRST_N(n, ...) LOG_FIRST_N(WARN, n, __VA_ARGS__)
#define ERROR_FIRST_N(n, ...) LOG_FIRST_N(ERROR, n, __VA_ARGS__)
#define FATAL_FIRST_N(n, ...) LOG_FIRST_N(FATAL, n, __VA_ARGS__)

#define TRACE_EVERY_MS(ms, ...) LOG_EVERY_MS(TRACE, ms, __VA_ARGS__)
#define DEBUG_EVERY_MS(ms, ...) LOG_EVERY_MS(DEBUG, ms, __VA_ARGS__)
#define INFO_EVERY_MS(ms, ...) LOG_EVERY_MS(INFO, ms, __VA_ARGS__)
#define WARN_EVERY_MS(ms, ...) LOG_EVERY_MS(WARN, ms, __VA_ARGS__)
#define ERROR_EVERY_MS(ms, ...) LOG_EVERY_MS(ERROR, ms, __VA_ARGS__)
#define FATAL_EVERY_MS(ms, ...) LOG_EVERY_MS(FATAL, ms, __VA_ARGS__)

#define TRACE_EVERY_SEC(sec, ...) LOG_EVERY_SEC(TRACE, sec, __VA_ARGS__)
#define DEBUG_EVERY_SEC(sec, ...) LOG_EVERY_SEC(DEBUG, sec, __VA_ARGS__)
#define INFO_EVERY_SEC(sec, ...) LOG_EVERY_SEC(INFO, sec, __VA_ARGS__)
#define WARN_EVERY_SEC(sec, ...) LOG_EVERY_SEC(WARN, sec, __VA_ARGS__)
#define ERROR_EVERY_SEC(sec, ...) LOG_EVERY_SEC(ERROR, sec, __VA_ARGS__)
#define FATAL_EVERY_SEC(sec, ...) LOG_EVERY_SEC(FATAL, sec, __VA_ARGS__)

#define TRACE_ONCE(...) LOG_ONCE(TRACE, __VA_ARGS__)
#define DEBUG_ONCE(...) LOG_ONCE(DEBUG, __VA_ARGS__)
#define INFO_ONCE(...) LOG_ONCE(INFO, __VA_ARGS__)
#define WARN_ONCE(...) LOG_ONCE(WARN, __VA_ARGS__)
#define ERROR_ONCE(...) LOG_ONCE(ERROR, __VA_ARGS__)
#define FATAL_ONCE(...) LOG_ONCE(FATAL, __VA_ARGS__)

// Only use this inside functions that return int32_t (or compatible type).
#define CHECK_WITH_TRACE_LOG(condition, ...) \
  do {                                       \
    if (!(condition)) {                      \
      TRACE(__VA_ARGS__);                    \
      return -1;                             \
    }                                        \
  } while (0)

#define CHECK_WITH_DEBUG_LOG(condition, ...) \
  do {                                       \
    if (!(condition)) {                      \
      DEBUG(__VA_ARGS__);                    \
      return -1;                             \
    }                                        \
  } while (0)

#define CHECK_WITH_INFO_LOG(condition, ...) \
  do {                                      \
    if (!(condition)) {                     \
      INFO(__VA_ARGS__);                    \
      return -1;                            \
    }                                       \
  } while (0)

#define CHECK_WITH_WARN_LOG(condition, ...) \
  do {                                      \
    if (!(condition)) {                     \
      WARN(__VA_ARGS__);                    \
      return -1;                            \
    }                                       \
  } while (0)

#define CHECK_WITH_ERROR_LOG(condition, ...) \
  do {                                       \
    if (!(condition)) {                      \
      ERROR(__VA_ARGS__);                    \
      return -1;                             \
    }                                        \
  } while (0)

#define CHECK_WITH_FATAL_LOG(condition, ...) \
  do {                                       \
    if (!(condition)) {                      \
      FATAL(__VA_ARGS__);                    \
      return -1;                             \
    }                                        \
  } while (0)
