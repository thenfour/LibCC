
#pragma once

#include <windows.h>
#include <iostream>
#include <tchar.h>

//using namespace LibCC;

extern int g_AssertCount;
extern int g_AssertPass;


// stolen from varargs.h
#define cc_va_start(ap) ap = (va_list)&va_alist
#define cc_va_end(ap) ap = (va_list)0



template<typename T>
void DoNotOptimize(const T& arg)
{
  TCHAR x[2];
  wsprintf(x, _T(""), arg);
}


inline bool TestAssert__(bool b, const char* sz)
{
  g_AssertCount ++;
  if(!b)
  {
    std::cout << "    FAILED: " << sz << std::endl;
    OutputDebugString("    FAILED: ");
    OutputDebugString(sz);
    OutputDebugString("\r\n");
  }
  else
  {
    g_AssertPass ++;
    std::cout << "    PASS: " << sz << std::endl;
    OutputDebugString("    PASS: ");
    OutputDebugString(sz);
    OutputDebugString("\r\n");
  }

  return b;
}

#define TestAssert(x) TestAssert__((x), #x)
