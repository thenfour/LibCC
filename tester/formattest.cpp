

#include "test.h"
#include "libcc\Log.hpp"
#include "libcc\stringutil.hpp"
#include "resource.h"
using namespace LibCC;
#include "libcc\AllocationTracker.hpp"

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
	//{
	//	LibCC::QuickString<wchar_t> qs(L"");
	//	qs = LibCC::QuickString<wchar_t>(L"1");

	//	{
	//		LibCC::QuickString<wchar_t> qs2(L"1");
	//		qs2 = LibCC::QuickString<wchar_t>(L"");
	//		qs2.append(L"");
	//		qs2.append(L"2");
	//		qs2.push_back('3');
	//		qs2.push_back('4');
	//		qs2.push_back('5');
	//		qs2.push_back('6');
	//		qs2.push_back('7');
	//		qs2.push_back('8');
	//		qs2.push_back('9');
	//		qs = qs2;
	//	}

	//	qs = LibCC::QuickString<wchar_t>(L"1", 0);
	//	qs = LibCC::QuickString<wchar_t>(L"1", 1);
	//	qs = LibCC::QuickString<wchar_t>(L"1", 2);
	//}

	//{
	//	FormatW().d<2>(1.0).Str();
	//}
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

		// escapes and other chars
		a.SetFormat("a|b");
		TestAssert(a.Str() == "a\r\nb");

		a.SetFormat("a^^");
		TestAssert(a.Str() == "a^");
	}

	// arguments.
	{
		FormatW w;
		w = FormatW(L"%%%");
		TestAssert(w.Str() == L"%%%");

		w = FormatW(L"%%%").i(1).i(2).i(3);
		TestAssert(w.Str() == L"123");

		w = FormatW(L"%%").i(1).i(2).i(3).i(4);
		TestAssert(w.Str() == L"1234");

		w = FormatW(L"{");
		TestAssert(w.Str() == L"{");

		w = FormatW(L"{0").i(1);
		TestAssert(w.Str() == L"{01");

		w = FormatW(L"{0}");
		TestAssert(w.Str() == L"{0}");

		w = FormatW(L"1{0}2");
		TestAssert(w.Str() == L"1{0}2");

		w = FormatW(L"1{0}3").i(2);
		TestAssert(w.Str() == L"123");

		w = FormatW(L"1{0}3{1}").i(2).i(456);
		TestAssert(w.Str() == L"123456");

		w = FormatW(L"1{0}{1}").i(2).s(L"3456");
		TestAssert(w.Str() == L"123456");

		w = FormatW(L"1{0}{1}{1}").i(2).s(L"3456");
		TestAssert(w.Str() == L"1234563456");

		w = FormatW(L"1{1}{0}{0}").i(2).s(L"3456");
		TestAssert(w.Str() == L"1345622");

		w = FormatW(L"1{1}{0}{0}%%").i(2).s(L"3456");
		TestAssert(w.Str() == L"134562223456");

		w = FormatW(L"1{1}{0}{0}%%%").i(2).s(L"3456");
		TestAssert(w.Str() == L"134562223456%");

		w = FormatW(L"{10}").i(1).i(2).i(3).i(4).i(5).i(6).i(7).i(8).i(9).i(10).i(11);
		TestAssert(w.Str() == L"11");
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

	// u(number, base, width, paddingchar)
	{
		TestAssert(FormatW().ul(256).Str() == L"256");
		TestAssert(FormatW().ul(256, 16).Str() == L"100");
		TestAssert(FormatW().ul(256, 16, 8).Str() == L"00000100");
		TestAssert(FormatW().ul(256, 16, 8, '-').Str() == L"-----100");
		TestAssert(FormatW().ul<16>(256).Str() == L"100");
		FormatW w;
		
		w.Clear();
		w.ul<16, 8>(256);
		TestAssert(w.Str() == L"00000100");
		
		w.Clear();
		w.ul<16, 8, '-'>(256);
		TestAssert(w.Str() == L"-----100");

		w.Clear();
		w.ul<1>(256);
		TestAssert(w.Str() == L"");

		TestAssert(FormatW().ul(0).Str() == L"0");
		TestAssert(FormatW().ul(1).Str() == L"1");
		TestAssert(FormatW().ul(7).Str() == L"7");
		TestAssert(FormatW().ul(8).Str() == L"8");
		TestAssert(FormatW().ul(9).Str() == L"9");
		TestAssert(FormatW().ul(10).Str() == L"10");
		TestAssert(FormatW().ul(99).Str() == L"99");
		TestAssert(FormatW().ul(100).Str() == L"100");
		TestAssert(FormatW().ul(255).Str() == L"255");
		TestAssert(FormatW().ul(256).Str() == L"256");
		TestAssert(FormatW().ul(0x1000).Str() == L"4096");
		TestAssert(FormatW().ul(0x10000).Str() == L"65536");
		TestAssert(FormatW().ul(0xffffff).Str() == L"16777215");
		TestAssert(FormatW().ul(0xfffffff).Str() == L"268435455");
		TestAssert(FormatW().ul(0xffffffff).Str() == L"4294967295");

		TestAssert(FormatW().ul<2>(0).Str() == L"0");
		TestAssert(FormatW().ul<2>(1).Str() == L"1");
		TestAssert(FormatW().ul<2>(7).Str() == L"111");
		TestAssert(FormatW().ul<2>(8).Str() == L"1000");
		TestAssert(FormatW().ul<2>(9).Str() == L"1001");
		TestAssert(FormatW().ul<2>(10).Str() == L"1010");
		TestAssert(FormatW().ul<2>(99).Str() == L"1100011");
		TestAssert(FormatW().ul<2>(100).Str() == L"1100100");
		TestAssert(FormatW().ul<2>(255).Str() == L"11111111");
		TestAssert(FormatW().ul<2>(256).Str() == L"100000000");
		TestAssert(FormatW().ul<2>(0x1000).Str() == L"1000000000000");
		TestAssert(FormatW().ul<2>(0x10000).Str() == L"10000000000000000");
		TestAssert(FormatW().ul<2>(0xffffff).Str() == L"111111111111111111111111");
		TestAssert(FormatW().ul<2>(0xfffffff).Str() == L"1111111111111111111111111111");
		TestAssert(FormatW().ul<2>(0xffffffff).Str() == L"11111111111111111111111111111111");
	}

	{
		// during float operations, make sure to test 0.5. and other combinations of oddball shit
		std::wstring w;
		TestAssert((w = FormatW().d<2>(1.0).Str()) == L"1.0");
		TestAssert((w = FormatW().d<2>(1.12).Str()) == L"1.12");
		TestAssert((w = FormatW().d<2>(1.124).Str()) == L"1.12");
		TestAssert((w = FormatW().d<3>(1.124).Str()) == L"1.124");
		TestAssert((w = FormatW().d<3,2>(1.124).Str()) == L"01.124");
		TestAssert((w = FormatW().d<3,2,' '>(1.124).Str()) == L" 1.124");
		TestAssert((w = FormatW().d<3,50>(1.124).Str()) == L"00000000000000000000000000000000000000000000000001.124");
	}

	// long strings or lots of arguments.
	{
		// lots of args
		FormatW w;
		for(int i = 0; i < 100000; ++ i)
		{
			w.i(1);
		}
		TestAssert(w.Str() == std::wstring(100000, '1'));

		// long strings
		std::wstring formatstr = L"{1}";
		for(int i = 0; i < 100000; ++ i)
		{
			formatstr.push_back('.');
		}
		formatstr.append(L"{0}");

		std::wstring arg0(100000, L'-');
		std::wstring arg1(100000, L'!');
		
		w.SetFormat(formatstr);
		w.s(arg0);
		w.s(arg1);

		std::wstring shouldBe = std::wstring(100000, L'!') + std::wstring(100000, L'.') + std::wstring(100000, L'-');
		TestAssert(w.Str() == shouldBe);

		// lots of args + long strings. this string will be 200 megs - 10000 args of 10000 chars each * sizeof(wchar_t)
		formatstr = std::wstring(10000, L'%');
		std::wstring arg(10000, L'-');
		w.SetFormat(formatstr);
		for(int i = 0; i < 10000; ++ i)
		{
			w.s(arg);
		}
		shouldBe.clear();
		for(int i = 0; i < 10000; ++ i)
		{
			shouldBe.append(arg);
		}
		TestAssert(w.Str() == shouldBe);
	}

	return true;
}
