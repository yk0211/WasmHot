#pragma once
#include <memory>
#include <mutex>

#include "noncopyable.h"

namespace wasmh {

template <typename T>
class Singleton : public NonCopyable {
 public:
  static T* Instance() {
    static std::once_flag flag;
    std::call_once(flag, [] { instance_.reset(new T); });

    return instance_.get();
  }

 private:
  static std::shared_ptr<T> instance_;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::instance_;

}  // namespace wasmh
