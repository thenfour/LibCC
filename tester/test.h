
#pragma once

#define _SECURE_SCL 0
#define LIBCC_PARSE_TRACE_ENABLED 0

#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <list>
#include "libcc\stringutil.hpp"

//using namespace LibCC;

struct TestState
{
	int assertCount;
	int assertPass;
};

//extern int g_indent;
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
  std::string indent("  ", g_runningTests.size());
  if(!b)
  {
    std::cout << indent.c_str() << "FAILED: " << sz << "(line:" << line << ", " << file << ")" << std::endl;
    OutputDebugString(indent.c_str());
    OutputDebugString("FAILED: ");
    OutputDebugString(sz);
    OutputDebugString("\r\n");
  }
  else
  {
		for(std::list<TestState>::iterator it = g_runningTests.begin(); it != g_runningTests.end(); ++ it)
		{
			it->assertPass ++;
		}
    std::cout << indent.c_str() << "PASS: " << sz << std::endl;
    OutputDebugString(indent.c_str());
    OutputDebugString("PASS: ");
    OutputDebugString(sz);
    OutputDebugString("\r\n");
  }

  return b;
}

template<typename Char>
inline void TestMessage(const std::basic_string<Char>& msg)
{
  std::wstring indent(L"  ", g_runningTests.size());
	std::wstring msgW;
	LibCC::StringConvert(msg, msgW);

	std::wcout << indent.c_str() << msgW << std::endl;
  OutputDebugStringW(indent.c_str());
	OutputDebugStringW(msgW.c_str());
  OutputDebugStringW(L"\r\n");
}

#define TestAssert(x) TestAssert__((x), #x, __FILE__, __LINE__)
