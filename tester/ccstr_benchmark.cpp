

#include "test.h"
#include "libcc\Timer.h"
using namespace LibCC;

#include <sstream>


bool FormatBenchmark()
{
  LibCC::Timer t;
  char crap[100];
  const int MaxNum = 1000;

  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(itoa(n, crap, 16));
  }
  t.Tick();
  std::cout << "itoa    : " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(Format().ul<16>(n));
  }
  t.Tick();
  std::cout << "Format(templated): " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(Format().ul(n, 16));
  }
  t.Tick();
  std::cout << "Format(runtime): " << t.GetLastDelta() << std::endl;


  t.Tick();
  for(int n = 0; n < MaxNum; n ++)
  {
    DoNotOptimize(sprintf(crap, "%x", n));
  }
  t.Tick();
  std::cout << "sprintf(): " << t.GetLastDelta() << std::endl;


  t.Tick();
  std::string crap2;
  for(int n = 0; n < MaxNum; n ++)
  {
    std::stringstream ss;
    ss << std::hex << n;
    ss >> crap2;
    DoNotOptimize(crap2);
  }
  t.Tick();
  std::cout << "stringstream(): " << t.GetLastDelta() << std::endl;

  return true;
}


