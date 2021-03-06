

#include "test.h"
//#include "libcc\parse.hpp"
//using namespace LibCC::Parse;

extern bool RegistryTest();
//extern bool StatusTest();
//extern bool StreamTest();
extern bool StringTest();
//extern bool PathMatchSpecTest();
extern bool FormatTest();
extern bool FormatBenchmark();
//extern bool ParseBenchmark();
extern void LogTest();
//extern bool BlobTest();
//extern bool AllocationTrackerTest();
extern bool StringCompilationTest();
//extern bool ParseTest();
extern bool WinapiTest();

int g_indent = 0;
std::list<TestState> g_runningTests;

template<typename Fn>
bool RunTest__(Fn f, const char* sz)
{
  bool r = false;
	g_indent ++;
  std::string indent("                         ", g_indent * 2);
  g_runningTests.push_back(TestState());
  TestState& state = g_runningTests.back();

  state.assertCount = 0;
  state.assertPass = 0;

  std::cout << indent.c_str() << "Running: " << sz << std::endl;
  OutputDebugString(indent.c_str());
  OutputDebugString("Running: ");
  OutputDebugString(sz);
  OutputDebugString("\r\n");

  f();

  if(state.assertCount == state.assertPass)
  {
    std::cout << indent.c_str() << "Total: PASS (100%)" << std::endl;
		OutputDebugString(indent.c_str());
    OutputDebugString("Total: PASS");
    OutputDebugString("\r\n");
  }
  else
  {
    std::cout << indent.c_str() << "Total: FAIL (" << state.assertPass << " of " << state.assertCount << " passed)" << std::endl;
		OutputDebugString(indent.c_str());
    OutputDebugString("Total: FAIL: ");
    OutputDebugString("\r\n");
  }
  g_runningTests.pop_back();
  -- g_indent;
  return r;
}

#define RunTest(x) RunTest__(x, #x)


void TestCollection()
{
	//RunTest(BlobTest);

	//RunTest(ParseTest);
	//RunTest(ParseBenchmark);

	//RunTest(LogTest);
	//RunTest(AllocationTrackerTest);

	RunTest(StringTest);
	RunTest(StringCompilationTest);
	RunTest(FormatTest);
	// RunTest(FormatBenchmark);

	//RunTest(RegistryTest); // careful with this of course.
	//RunTest(StatusTest);
	//RunTest(PathMatchSpecTest);
  RunTest(WinapiTest);
}

int _tmain(int argc, _TCHAR* argv[])
{
  //_CrtSetBreakAlloc(113);
  //SetUnhandledExceptionFilter(CCUnhandledExceptionFilter);
	RunTest(TestCollection);
	_CrtDumpMemoryLeaks();
	return 0;
}

