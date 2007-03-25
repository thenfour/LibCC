

#include "test.h"
#include "libcc\Log.h"
#include "libcc\stringutil.h"
#include "resource.h"
using namespace LibCC;

void FormatTestA(FormatA a)
{
	TestAssert(a.Str() == "hi carl");
}

void FormatTestB(FormatA& a)
{
	TestAssert(a.Str() == "hi carl");
}

void FormatTestC(const FormatA& a)
{
	TestAssert(a.Str() == "hi carl");
}

bool FormatTest()
{
	//LibCC::FormatA a;
	//LibCC::FormatW w;
	//FormatX<DWORD> x;
	//FormatX<__int64> y;

	// constructors
	{
		// default constructor
		FormatA a;
		TestAssert(a.Str().empty());

		// copy constructor
		a.SetFormat("hi %");
		a("carl");
		FormatA b(a);
		TestAssert(b.Str() == "hi carl");

		// char* constructor
		FormatA c("hi %");
		c("carl");
		TestAssert(c.Str() == "hi carl");

		// string constructor
		std::string s("hi %");
		FormatA d(s);
		d("carl");
		TestAssert(d.Str() == "hi carl");

		// XChar* constructor
		FormatA e(L"hi %");
		e("carl");
		TestAssert(e.Str() == "hi carl");

		// Xstring constructor
		std::wstring ws(L"hi %");
		FormatA f(ws);
		f("carl");
		TestAssert(f.Str() == "hi carl");

		// HINSTANCE constructor
		FormatW g(GetModuleHandle(0), IDS_HI);
		g(L"carl");
		TestAssert(g.Str() == L"hi carl");

		FormatA h(IDS_HI);
		h(L"carl");
		TestAssert(h.Str() == "hi carl");
	}

	// code scenarios of passing into other functions. the point of this is mostly to make sure that there are no compiler warnings / errors for doing typical stuff.
	// another point of this is if you uncomment things here, the compiler should NOT allow you to do certain things, such as implicitly
	// convert from const char* to FormatA.
	{
		// should not compile!
		//FormatTestA("hi carl");
		//FormatTestB("hi carl");
		//FormatTestC("hi carl");

		//FormatTestA(L"hi carl");
		//FormatTestB(L"hi carl");
		//FormatTestC(L"hi carl");

		FormatA a("hi carl");
		FormatTestA(a);
		FormatTestB(a);
		FormatTestC(a);

		const FormatA b("hi carl");
		FormatTestA(b);
		//FormatTestB(b); it's const! should not compile
		FormatTestC(b);
	}

	// SetFormat
	{
		FormatA a("omg");
		a.SetFormat("hi %");
		a("carl");
		TestAssert(a.Str() == "hi carl");

		a.SetFormat((char*)0);
		a("carl");
		TestAssert(a.Str() == "carl");

		a.SetFormat((wchar_t*)0);
		a("carl");
		TestAssert(a.Str() == "carl");

		a.SetFormat(L"bye %");
		a("jake");
		TestAssert(a.Str() == "bye jake");

		std::string sa = "hi %";
		a.SetFormat(sa);
		a("carl");
		TestAssert(a.Str() == "hi carl");

		std::wstring sw = L"bye %";
		a.SetFormat(sw);
		a("jake");
		TestAssert(a.Str() == "bye jake");

		a.SetFormat(GetModuleHandle(0), IDS_HI);
		a("carl");
		TestAssert(a.Str() == "hi carl");

		a.SetFormat("clear...");
		a.SetFormat(IDS_HI);
		a("carl");
		TestAssert(a.Str() == "hi carl");
	}

	// p()
	{
		char* c = (char*)0x01;
		wchar_t* w = (wchar_t*)0x01;
		DWORD* dw = (DWORD*)0x01;
		FormatA a;

		a.Clear();
		a.p(c);
		TestAssert(a.Str() == "0x00000001");

		a.Clear();
		a.p(w);
		TestAssert(a.Str() == "0x00000001");

		a.Clear();
		a.p(dw);
		TestAssert(a.Str() == "0x00000001");
	}

	// c(count)
	{
		FormatW w;

		w.SetFormat("yo ");
		w.c((char)'a', 15);
		TestAssert(w.Str() == L"yo aaaaaaaaaaaaaaa");

		w.SetFormat("yo ");
		w.c((char)'a', 1);
		TestAssert(w.Str() == L"yo a");

		w.SetFormat("yo ");
		w.c((char)'a', 0);
		TestAssert(w.Str() == L"yo ");

		w.SetFormat("yo ");
		w.c((wchar_t)'a', 15);
		TestAssert(w.Str() == L"yo aaaaaaaaaaaaaaa");

		w.SetFormat("yo ");
		w.c((wchar_t)'a', 1);
		TestAssert(w.Str() == L"yo a");

		w.SetFormat("yo ");
		w.c((wchar_t)'a', 0);
		TestAssert(w.Str() == L"yo ");
	}

	// s(maxlen) - templated versions
	{
		FormatW fw;
		std::string a = "Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::wstring w = L"Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::basic_string<DWORD> x;
		XLastDitchStringCopy("Lorem ipsum dolor sit amet, consectetur adipisicing", x);

		// from foreign char* 
		fw.Clear();
		fw.s("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"Lorem ipsum dolor sit amet, consectetur adipisicing");

		fw.Clear();
		fw.s<0>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.s<1>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"L");

		fw.Clear();
		fw.s<100000>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"Lorem ipsum dolor sit amet, consectetur adipisicing");

		// from foreign string
		fw.Clear();
		fw.s(a);
		TestAssert(fw.Str() == w);

		fw.Clear();
		fw.s<0>(a);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.s<1>(x);
		TestAssert(fw.Str() == L"L");

		fw.Clear();
		fw.s<100000>(x);
		TestAssert(fw.Str() == L"Lorem ipsum dolor sit amet, consectetur adipisicing");

		// from native char*
		fw.Clear();
		fw.s(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == w);

		fw.Clear();
		fw.s<6>(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"Lorem ");

		fw.Clear();
		fw.s<100000>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == w);

		// from native string
		fw.Clear();
		fw.s(w);
		TestAssert(fw.Str() == w);

		fw.Clear();
		fw.s<6>(w);
		TestAssert(fw.Str() == L"Lorem ");

		fw.Clear();
		fw.s<100000>(w);
		TestAssert(fw.Str() == w);
	}

	// s(maxlen) - runtime versions
	{
		FormatW fw;
		std::string a = "Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::wstring w = L"Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::basic_string<DWORD> x;
		XLastDitchStringCopy("Lorem ipsum dolor sit amet, consectetur adipisicing", x);

		// from foreign char* 
		fw.Clear();
		fw.s("Lorem ipsum dolor sit amet, consectetur adipisicing", 0);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.s("Lorem ipsum dolor sit amet, consectetur adipisicing", 1);
		TestAssert(fw.Str() == L"L");

		fw.Clear();
		fw.s("Lorem ipsum dolor sit amet, consectetur adipisicing", 100000);
		TestAssert(fw.Str() == L"Lorem ipsum dolor sit amet, consectetur adipisicing");

		// from foreign string
		fw.Clear();
		fw.s(a, 0);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.s(x, 1);
		TestAssert(fw.Str() == L"L");

		fw.Clear();
		fw.s(x, 100000);
		TestAssert(fw.Str() == L"Lorem ipsum dolor sit amet, consectetur adipisicing");

		// from native char*
		fw.Clear();
		fw.s(L"Lorem ipsum dolor sit amet, consectetur adipisicing", 6);
		TestAssert(fw.Str() == L"Lorem ");

		fw.Clear();
		fw.s("Lorem ipsum dolor sit amet, consectetur adipisicing", 100000);
		TestAssert(fw.Str() == w);

		// from native string
		fw.Clear();
		fw.s(w, 6);
		TestAssert(fw.Str() == L"Lorem ");

		fw.Clear();
		fw.s(w, 100000);
		TestAssert(fw.Str() == w);
	}


	// qs(maxlen) - templated versions
	{
		FormatW fw;
		std::string a = "Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::wstring w = L"Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::wstring qw = L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"";
		std::basic_string<DWORD> x;
		XLastDitchStringCopy("Lorem ipsum dolor sit amet, consectetur adipisicing", x);

		// from foreign char* 
		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"");

		fw.Clear();
		fw.qs<0>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs<1>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs<2>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs<3>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs<100000>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"");

		// from foreign string
		fw.Clear();
		fw.qs(a);
		TestAssert(fw.Str() == qw);

		fw.Clear();
		fw.qs<0>(a);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs<1>(x);
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs<2>(x);
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs<3>(x);
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs<100000>(x);
		TestAssert(fw.Str() == L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"");

		// from native char*
		fw.Clear();
		fw.qs(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == qw);

		fw.Clear();
		fw.qs<0>(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs<1>(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs<2>(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs<3>(L"Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs<100000>("Lorem ipsum dolor sit amet, consectetur adipisicing");
		TestAssert(fw.Str() == qw);

		// from native string
		fw.Clear();
		fw.qs(w);
		TestAssert(fw.Str() == qw);

		fw.Clear();
		fw.qs<0>(w);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs<1>(w);
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs<2>(w);
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs<3>(w);
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs<100000>(w);
		TestAssert(fw.Str() == qw);
	}

	// qs(maxlen) - runtime versions
	{
		FormatW fw;
		std::string a = "Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::wstring w = L"Lorem ipsum dolor sit amet, consectetur adipisicing";
		std::wstring qw = L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"";
		std::basic_string<DWORD> x;
		XLastDitchStringCopy("Lorem ipsum dolor sit amet, consectetur adipisicing", x);

		// from foreign char* 
		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing", 0);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing", 1);
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing", 2);
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing", 3);
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing", 100000);
		TestAssert(fw.Str() == L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"");

		// from foreign string
		fw.Clear();
		fw.qs(a, 0);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs(x, 1);
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs(x, 2);
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs(x, 3);
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs(x, 100000);
		TestAssert(fw.Str() == L"\"Lorem ipsum dolor sit amet, consectetur adipisicing\"");

		// from native char*
		fw.Clear();
		fw.qs(L"Lorem ipsum dolor sit amet, consectetur adipisicing", 0);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs(L"Lorem ipsum dolor sit amet, consectetur adipisicing", 1);
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs(L"Lorem ipsum dolor sit amet, consectetur adipisicing", 2);
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs(L"Lorem ipsum dolor sit amet, consectetur adipisicing", 3);
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs("Lorem ipsum dolor sit amet, consectetur adipisicing", 100000);
		TestAssert(fw.Str() == qw);

		// from native string
		fw.Clear();
		fw.qs(w, 0);
		TestAssert(fw.Str() == L"");

		fw.Clear();
		fw.qs(w, 1);
		TestAssert(fw.Str() == L"\"");

		fw.Clear();
		fw.qs(w, 2);
		TestAssert(fw.Str() == L"\"\"");

		fw.Clear();
		fw.qs(w, 3);
		TestAssert(fw.Str() == L"\"L\"");

		fw.Clear();
		fw.qs(w, 6);
		TestAssert(fw.Str() == L"\"Lore\"");

		fw.Clear();
		fw.qs(w, 100000);
		TestAssert(fw.Str() == qw);
	}


	return true;
}
