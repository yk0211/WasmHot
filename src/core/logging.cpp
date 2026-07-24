#include "core/logging.h"

#include <spdlog/common.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>

namespace wasmh {

constexpr std::size_t g_min_file_size = 100ULL * 1024 * 1024;
const std::size_t g_max_file_num = 1000;

static void AtExitFlush() {
  spdlog::shutdown();
}

static std::string GetLogFilename() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y%m%d");
  return oss.str() + ".log";
}

class DateRotatingFileSink : public spdlog::sinks::base_sink<std::mutex> {
 public:
  DateRotatingFileSink(std::string base_filename, std::size_t max_size, std::size_t max_files)
      : base_filename_(std::move(base_filename)),
        max_size_(std::max(max_size, g_min_file_size)),
        max_files_(std::min(max_files, g_max_file_num)) {
    file_helper_.open(base_filename_);
    current_size_ = file_helper_.size();
  }

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    this->formatter_->format(msg, formatted);

    CheckDateAndRotate();

    auto new_size = current_size_ + formatted.size();
    if (new_size > max_size_) {
      file_helper_.flush();
      if (file_helper_.size() > 0) {
        Rotate();
        new_size = formatted.size();
      }
    }
    file_helper_.write(formatted);
    current_size_ = new_size;
  }

  void flush_() override { file_helper_.flush(); }

 private:
  static std::string CalcFilename(const std::string& filename, std::size_t index) {
    if (index == 0)
      return filename;
    return filename + "." + std::to_string(index);
  }

  void CheckDateAndRotate() {
    std::string new_filename = GetLogFilename();
    if (new_filename == base_filename_)
      return;

    file_helper_.flush();
    file_helper_.close();
    base_filename_ = new_filename;
    file_helper_.open(base_filename_);
    current_size_ = file_helper_.size();
  }

  void Rotate() {
    file_helper_.close();
    for (auto i = max_files_; i > 0; --i) {
      auto src = CalcFilename(base_filename_, i - 1);
      if (!std::filesystem::exists(src))
        continue;
      auto target = CalcFilename(base_filename_, i);
      std::filesystem::remove(target);
      std::filesystem::rename(src, target);
    }
    file_helper_.open(base_filename_);
    current_size_ = 0;
  }

 private:
  std::string base_filename_;
  std::size_t max_size_;
  std::size_t max_files_;
  std::size_t current_size_ = 0;
  spdlog::details::file_helper file_helper_;
};

void InitLogging(const std::string& logger_name, const std::string& log_level, const std::string& flush_log_level,
                 std::size_t file_size, std::size_t rotate_file_num) {
  std::string filename = GetLogFilename();
  auto file_sink = std::make_shared<DateRotatingFileSink>(filename, file_size, rotate_file_num);
  file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%P][%t][%!:%s:%#] %v");

  auto logger = std::make_shared<spdlog::logger>(logger_name, spdlog::sinks_init_list{file_sink});
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::from_str(log_level));
  spdlog::flush_on(spdlog::level::from_str(flush_log_level));
  std::atexit(AtExitFlush);
}

}  // namespace wasmh
