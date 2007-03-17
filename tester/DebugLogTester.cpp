

#include "test.h"
#include "libcc\DebugLog.h"
using namespace LibCC;



void DebugLogTest()
{
	LibCC::LogWindow x;
	x.Create("test.log", GetModuleHandle(NULL));
	x.Message("hi");
	{
		LogScopeMessage l("omg", &x);
		x.Message(L"hi");
	}
	x.Destroy();
}

