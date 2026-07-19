#pragma once

#include <spdlog/spdlog.h>

namespace wasmh
{

// Configures spdlog with the project-wide format and rotation policy.
// Format: [time][pid][tid][function:file:line] message
// File:    <current_date>.log, rotating at 100MB with suffixes .1, .2, ...
void InitLogging(const std::string &program_name, const std::string log_level, const std::string flush_log_level,
                 std::size_t file_size, std::size_t rotate_file_num);

} // namespace wasmh

// Project-wide logging macros. They delegate to spdlog and automatically
// capture source location (file, function, line) via SPDLOG_* macros.
#define TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define FATAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

// Checks the condition; if false, logs a message at the specified level and returns -1.
// Only use this inside functions that return int32_t (or compatible type).
#define CHECK_WITH_TRACE_LOG(condition, ...) \
    do { \
        if (!(condition)) { \
            TRACE(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)

#define CHECK_WITH_DEBUG_LOG(condition, ...) \
    do { \
        if (!(condition)) { \
            DEBUG(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)

#define CHECK_WITH_INFO_LOG(condition, ...) \
    do { \
        if (!(condition)) { \
            INFO(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)

#define CHECK_WITH_WARN_LOG(condition, ...) \
    do { \
        if (!(condition)) { \
            WARN(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)

#define CHECK_WITH_ERROR_LOG(condition, ...) \
    do { \
        if (!(condition)) { \
            ERROR(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)

#define CHECK_WITH_FATAL_LOG(condition, ...) \
    do { \
        if (!(condition)) { \
            FATAL(__VA_ARGS__); \
            return -1; \
        } \
    } while (0)
