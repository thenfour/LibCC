
#ifndef LIBCC_LOG_H
#define LIBCC_LOG_H

#include "format.hpp"
#include <Windows.h>

namespace cc {

  template<typename ...Targs>
  inline void log(const char *fmt_, Targs...args) {
    std::string userMsg = format(fmt_, args...);
    std::string logMsg = format("[---] %\r\n", userMsg);
    ::OutputDebugStringA(logMsg.c_str());
  }
}

#endif // LIBCC_LOG_H
