

#include "test.h"
#include "libcc\Log.h"
using namespace LibCC;



void LogTest()
{
	LibCC::Log x;
	x.Create("test.log", GetModuleHandle(NULL), true, true, true);
	x.Message("hi");
	{
		LogScopeMessage l("omg", &x);
		x.Message(L"hi");
	}
	x.Destroy();
}

