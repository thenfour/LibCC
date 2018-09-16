
#define WIN32_LEAN_AND_MEAN

#include "test.h"
#include "libcc/winapi.hpp"

bool WinapiTest()
{
  TestAssert(LibCC::GetModuleFilenameW().size() > 0);
  TestAssert(LibCC::GetModuleFilenameW(GetModuleHandle(0)).size() > 0);
  std::string x;
  x = LibCC::GetModuleFilenameX<char>();
  TestAssert(x.size() > 0);
  x = LibCC::GetModuleFilenameX<char>(GetModuleHandle(0));
  TestAssert(x.size() > 0);

  return true;
}


