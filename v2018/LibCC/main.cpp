

#include "libcc/format.hpp"
#include "libcc/winutils.hpp"
#include "libcc\profiler.hpp"

int main()
{
  cc::profiler::enterTask("ohay");
  Sleep(100);
  cc::profiler::enterTask("jack");
  Sleep(100);
  cc::profiler::leaveTask();
  cc::profiler::enterTask("black");
  Sleep(100);
  cc::profiler::report();
  return 0;
}

