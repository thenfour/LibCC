

#include "test.h"


extern bool RegistryTest();
extern bool StatusTest();
extern bool StreamTest();
extern bool StringTest();
extern bool PathMatchSpecTest();
extern bool FormatTest();
extern bool FormatBenchmark();
extern bool DebugLogTest();
extern bool BlobTest();

long g_TestCount;
long g_TestFailures;

int g_AssertCount;
int g_AssertPass;

/*
 - Positive Exponent
 - Negative Exponent
 - Zero
 - Powers of 2 (from negative exp to positive)
*/
LONG WINAPI CCUnhandledExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
  return EXCEPTION_EXECUTE_HANDLER;
}

template<typename Fn>
bool RunTest__(Fn f, const char* sz)
{
  bool r = false;

  g_AssertCount = 0;
  g_AssertPass = 0;

  g_TestCount ++;
  std::cout << "Running: " << sz << std::endl;
  OutputDebugString("Running: ");
  OutputDebugString(sz);
  OutputDebugString("\r\n");

  f();

  if(g_AssertCount == g_AssertPass)
  {
    std::cout << "  Total: PASS (100%)" << std::endl;
    OutputDebugString("    Total: PASS");
    OutputDebugString("\r\n");
  }
  else
  {
    g_TestFailures ++;
    std::cout << "  Total: FAIL (" << g_AssertPass << " of " << g_AssertCount << " passed)" << std::endl;
    OutputDebugString("    Total: FAIL: ");
    OutputDebugString("\r\n");
  }
  return r;
}

#define RunTest(x) RunTest__(x, #x)

int _tmain(int argc, _TCHAR* argv[])
{
  SetUnhandledExceptionFilter(CCUnhandledExceptionFilter);

  g_TestCount = 0;
  g_TestFailures = 0;

	//RunTest(BlobTest);
	//RunTest(DebugLogTest);
	//RunTest(FormatTest);
 // RunTest(FormatBenchmark);
 // //RunTest(RegistryTest); // careful with this of course.
 // RunTest(StatusTest);
 // RunTest(PathMatchSpecTest);
  RunTest(StringTest);

	return 0;
}

