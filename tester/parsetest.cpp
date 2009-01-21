

#include "test.h"
#include "libcc\timer.hpp"
#include "libcc\parse.hpp"


using namespace LibCC::Parse;

bool ParseTest()
{
	{
		LibCC::Timer t;
		int out;
		t.Tick();
		for(int i = 0; i < 1000; ++i)
		{
			CInteger2(out).ParseSimple(L"1010101b");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"0123456");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"123456789");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"0xabcdef");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"-1010101b");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"-0123456");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"-123456789");
			DoNotOptimize(i);
			CInteger2(out).ParseSimple(L"-0xabcdef");
			DoNotOptimize(i);
		}
		t.Tick();
		std::cout << "CInteger2 (8000 various integers separately)    : " << t.GetLastDelta() << std::endl;

		std::wstring script;
		for(int i = 0; i < 2000; ++ i)
		{
			script.append(LibCC::FormatW(L"%b 0% % 0x% ").i<2>(rand()).i<8>(rand()).i<10>(rand()).i<16>(rand()).Str());
		}

		t.Tick();
		std::vector<int> outv;
		(*(CInteger(VectorOutput(outv)) + *Space())).ParseSimple(script);
		t.Tick();
		std::cout << "CInteger2 (8000 integers in a script, whitespace separated)    : " << t.GetLastDelta() << std::endl;
	}

	double d;
	SignedRationalParserT<double>(RefOutput(d), 12).ParseSimple(L"-1.2e3");
  // template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
	std::wstring sd = LibCC::FormatW().d<10, 1, '0', true, 12>(d).Str();

	//CScriptReader reader(L"a");
	//ScriptResult result;
	//int i;
	//wchar_t wch;
	//std::vector<wchar_t> vwch;
	//std::wstring ws;
	//vwch.push_back('b');
	//ScriptCursor cur = reader.GetCursorCopy();
	//(*Char(0, InserterOutput<wchar_t>(vwch, std::back_inserter(vwch)))).ParseRetainingStateOnError(result, reader);

	//reader.SetCursor(cur);
	//(*Char(0, BackInserterOutput<wchar_t>(vwch))).ParseRetainingStateOnError(result, reader);

	//reader.SetCursor(cur);
	//(*Char(0, VectorOutput(vwch))).ParseRetainingStateOnError(result, reader);

	//reader.SetCursor(cur);
	//(*Char(0, CharToStringOutput(ws))).ParseRetainingStateOnError(result, reader);

  return true;
}
