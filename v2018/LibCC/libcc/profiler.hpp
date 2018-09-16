
#ifndef LIBCC_PROFILER_H
#define LIBCC_PROFILER_H

#include "format.hpp"
#include "log.hpp"
#include "winutils.hpp"
#include <map>

namespace cc {
  namespace profiler {
    std::map<std::string, qptimer> tasks;
    std::string currentTaskName;
    inline static void enterTask(const std::string& n) {
      tasks[n].start();
      currentTaskName = n;
    }
    inline static void leaveTask() {
      tasks[currentTaskName].pause();
    }
    inline static void leaveTask(const std::string& n) {
      tasks[n].pause();
    }
    inline static void report() {
      for (auto& p : tasks) {
        log("% = %", p.first, toString(p.second.elapsedSeconds()));
      }
    }
  };
}

#endif // LIBCC_WINUTIL_H
