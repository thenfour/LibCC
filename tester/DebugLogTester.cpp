

#include "test.h"
#include "libcc\Log.h"
using namespace LibCC;

//namespace LibCC
//{
//	LibCC::Log* g_pLog = 0;
//}

void LogTest()
{
	LibCC::Log x;
	x.Create("test.log", GetModuleHandle(NULL), true, true, true);
	x.Message("hi");
	{
		LogScopeMessage l("omg", &x);
		LibCC::g_pLog = new LibCC::Log("test2.log", GetModuleHandle(NULL));
		{
			LogScopeMessage m("mmmmmm");
		}
		delete LibCC::g_pLog;
		x.Message(L"hi");
	}
}

