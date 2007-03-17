
#pragma once

#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <list>

//using namespace LibCC;

struct TestState
{
	int assertCount;
	int assertPass;
};

extern int g_indent;
extern std::list<TestState> g_runningTests;


// stolen from varargs.h
#define cc_va_start(ap) ap = (va_list)&va_alist
#define cc_va_end(ap) ap = (va_list)0



template<typename T>
void DoNotOptimize(const T& arg)
{
  TCHAR x[2];
  wsprintf(x, _T(""), arg);
}


inline bool TestAssert__(bool b, const char* sz, const char* file, int line)
{
	for(std::list<TestState>::iterator it = g_runningTests.begin(); it != g_runningTests.end(); ++ it)
	{
		it->assertCount ++;
	}
  std::string indent("  ", g_indent ++);
  if(!b)
  {
    std::cout << indent.c_str() << "    FAILED: " << sz << "(line:" << line << ", " << file << ")" << std::endl;
    OutputDebugString(indent.c_str());
    OutputDebugString("    FAILED: ");
    OutputDebugString(sz);
    OutputDebugString("\r\n");
  }
  else
  {
		for(std::list<TestState>::iterator it = g_runningTests.begin(); it != g_runningTests.end(); ++ it)
		{
			it->assertPass ++;
		}
    std::cout << indent.c_str() << "    PASS: " << sz << std::endl;
    OutputDebugString(indent.c_str());
    OutputDebugString("    PASS: ");
    OutputDebugString(sz);
    OutputDebugString("\r\n");
  }

  return b;
}

#define TestAssert(x) TestAssert__((x), #x, __FILE__, __LINE__)
