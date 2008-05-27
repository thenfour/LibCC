

#include "test.h"

extern bool RegistryTest();
extern bool StatusTest();
extern bool StreamTest();
extern bool StringTest();
extern bool PathMatchSpecTest();
extern bool FormatTest();
extern bool FormatBenchmark();
extern void LogTest();
extern bool BlobTest();
extern bool AllocationTrackerTest();


LONG WINAPI CCUnhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
  return EXCEPTION_EXECUTE_HANDLER;
}

int g_indent = 0;
std::list<TestState> g_runningTests;

template<typename Fn>
bool RunTest__(Fn f, const char* sz)
{
  bool r = false;
	g_indent ++;
  std::string indent("  ", g_indent);
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
	//RunTest(LogTest);
	RunTest(AllocationTrackerTest);
	RunTest(FormatTest);
  RunTest(FormatBenchmark);
	//RunTest(RegistryTest); // careful with this of course.
  //RunTest(StatusTest);
  //RunTest(PathMatchSpecTest);
  //RunTest(StringTest);
}

int _tmain(int argc, _TCHAR* argv[])
{
  SetUnhandledExceptionFilter(CCUnhandledExceptionFilter);
	RunTest(TestCollection);
	return 0;
}

