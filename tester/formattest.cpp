

#include "test.h"
#include "libcc\stringutil.h"
#include "libcc\Log.h"

using namespace LibCC;


bool FormatTest()
{
	LibCC::FormatA x;
	x.gle();
	LibCC::FormatW a((HINSTANCE)0, 0);
	LibCC::FormatW b("omg");
	LibCC::FormatW c(L"omg");
	b.SetFormat("omg");
	b.SetFormat(L"omg");
	b.SetFormat(1);
	//b.SetFormat(a);
	//b.SetFormat(x);
	
	return true;
}