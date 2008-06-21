

#include "test.h"
#include "libcc\stringutil.h"
#include <vector>
using namespace LibCC;


bool StringCompilationTest()
{
		std::string a1, a2;
		std::wstring w1, w2;
		std::basic_string<int> x1, x2;

		// just test that shit compiles without ambiguities and warnings

		StringEquals(a1.c_str(), a1.c_str());
		StringEquals(a1.c_str(), w1.c_str());
		StringEquals(a1.c_str(), x1.c_str());
		StringEquals(w1.c_str(), a1.c_str());
		StringEquals(w1.c_str(), w1.c_str());
		StringEquals(w1.c_str(), x1.c_str());
		StringEquals(x1.c_str(), a1.c_str());
		StringEquals(x1.c_str(), w1.c_str());
		StringEquals(x1.c_str(), x1.c_str());

		StringEquals(a1.c_str(), a1);
		StringEquals(a1.c_str(), w1);
		StringEquals(a1.c_str(), x1);
		StringEquals(w1.c_str(), a1);
		StringEquals(w1.c_str(), w1);
		StringEquals(w1.c_str(), x1);
		StringEquals(x1.c_str(), a1);
		StringEquals(x1.c_str(), w1);
		StringEquals(x1.c_str(), x1);

		StringEquals(a1, a1.c_str());
		StringEquals(a1, w1.c_str());
		StringEquals(a1, x1.c_str());
		StringEquals(w1, a1.c_str());
		StringEquals(w1, w1.c_str());
		StringEquals(w1, x1.c_str());
		StringEquals(x1, a1.c_str());
		StringEquals(x1, w1.c_str());
		StringEquals(x1, x1.c_str());

		StringEquals(a1, a1);
		StringEquals(a1, w1);
		StringEquals(a1, x1);
		StringEquals(w1, a1);
		StringEquals(w1, w1);
		StringEquals(w1, x1);
		StringEquals(x1, a1);
		StringEquals(x1, w1);
		StringEquals(x1, x1);

		StringFindLastOf(a1.c_str(), a1.c_str());
		StringFindLastOf(a1.c_str(), w1.c_str());
		StringFindLastOf(a1.c_str(), x1.c_str());
		StringFindLastOf(w1.c_str(), a1.c_str());
		StringFindLastOf(w1.c_str(), w1.c_str());
		StringFindLastOf(w1.c_str(), x1.c_str());
		StringFindLastOf(x1.c_str(), a1.c_str());
		StringFindLastOf(x1.c_str(), w1.c_str());
		StringFindLastOf(x1.c_str(), x1.c_str());

		StringFindLastOf(a1.c_str(), a1);
		StringFindLastOf(a1.c_str(), w1);
		StringFindLastOf(a1.c_str(), x1);
		StringFindLastOf(w1.c_str(), a1);
		StringFindLastOf(w1.c_str(), w1);
		StringFindLastOf(w1.c_str(), x1);
		StringFindLastOf(x1.c_str(), a1);
		StringFindLastOf(x1.c_str(), w1);
		StringFindLastOf(x1.c_str(), x1);

		StringFindLastOf(a1, a1.c_str());
		StringFindLastOf(a1, w1.c_str());
		StringFindLastOf(a1, x1.c_str());
		StringFindLastOf(w1, a1.c_str());
		StringFindLastOf(w1, w1.c_str());
		StringFindLastOf(w1, x1.c_str());
		StringFindLastOf(x1, a1.c_str());
		StringFindLastOf(x1, w1.c_str());
		StringFindLastOf(x1, x1.c_str());

		StringFindLastOf(a1, a1);
		StringFindLastOf(a1, w1);
		StringFindLastOf(a1, x1);
		StringFindLastOf(w1, a1);
		StringFindLastOf(w1, w1);
		StringFindLastOf(w1, x1);
		StringFindLastOf(x1, a1);
		StringFindLastOf(x1, w1);
		StringFindLastOf(x1, x1);

		StringFindFirstOf(a1.c_str(), a1.c_str());
		StringFindFirstOf(a1.c_str(), w1.c_str());
		StringFindFirstOf(a1.c_str(), x1.c_str());
		StringFindFirstOf(w1.c_str(), a1.c_str());
		StringFindFirstOf(w1.c_str(), w1.c_str());
		StringFindFirstOf(w1.c_str(), x1.c_str());
		StringFindFirstOf(x1.c_str(), a1.c_str());
		StringFindFirstOf(x1.c_str(), w1.c_str());
		StringFindFirstOf(x1.c_str(), x1.c_str());

		StringFindFirstOf(a1.c_str(), a1);
		StringFindFirstOf(a1.c_str(), w1);
		StringFindFirstOf(a1.c_str(), x1);
		StringFindFirstOf(w1.c_str(), a1);
		StringFindFirstOf(w1.c_str(), w1);
		StringFindFirstOf(w1.c_str(), x1);
		StringFindFirstOf(x1.c_str(), a1);
		StringFindFirstOf(x1.c_str(), w1);
		StringFindFirstOf(x1.c_str(), x1);

		StringFindFirstOf(a1, a1.c_str());
		StringFindFirstOf(a1, w1.c_str());
		StringFindFirstOf(a1, x1.c_str());
		StringFindFirstOf(w1, a1.c_str());
		StringFindFirstOf(w1, w1.c_str());
		StringFindFirstOf(w1, x1.c_str());
		StringFindFirstOf(x1, a1.c_str());
		StringFindFirstOf(x1, w1.c_str());
		StringFindFirstOf(x1, x1.c_str());

		StringFindFirstOf(a1, a1);
		StringFindFirstOf(a1, w1);
		StringFindFirstOf(a1, x1);
		StringFindFirstOf(w1, a1);
		StringFindFirstOf(w1, w1);
		StringFindFirstOf(w1, x1);
		StringFindFirstOf(x1, a1);
		StringFindFirstOf(x1, w1);
		StringFindFirstOf(x1, x1);

		StringContains(a1.c_str(), a1[0]);
		StringContains(a1.c_str(), w1[0]);
		StringContains(a1.c_str(), x1[0]);
		StringContains(w1.c_str(), a1[0]);
		StringContains(w1.c_str(), w1[0]);
		StringContains(w1.c_str(), x1[0]);
		StringContains(x1.c_str(), a1[0]);
		StringContains(x1.c_str(), w1[0]);
		StringContains(x1.c_str(), x1[0]);

		StringContains(a1, a1[0]);
		StringContains(a1, w1[0]);
		StringContains(a1, x1[0]);
		StringContains(w1, a1[0]);
		StringContains(w1, w1[0]);
		StringContains(w1, x1[0]);
		StringContains(x1, a1[0]);
		StringContains(x1, w1[0]);
		StringContains(x1, x1[0]);

		StringConvert(a1.c_str(), a1);
		StringConvert(a1.c_str(), w1);
		StringConvert(a1.c_str(), x1);
		StringConvert(w1.c_str(), a1);
		StringConvert(w1.c_str(), w1);
		StringConvert(w1.c_str(), x1);
		StringConvert(x1.c_str(), a1);
		StringConvert(x1.c_str(), w1);
		StringConvert(x1.c_str(), x1);
		StringConvert(a1, a1);
		StringConvert(a1, w1);
		StringConvert(a1, x1);
		StringConvert(w1, a1);
		StringConvert(w1, w1);
		StringConvert(w1, x1);
		StringConvert(x1, a1);
		StringConvert(x1, w1);
		StringConvert(x1, x1);

		std::vector<std::string> va;
		std::vector<std::wstring> vw;
		std::vector<std::basic_string<__int32> > vx;

		StringSplitByString(a1.c_str(), a1.c_str(), std::back_inserter(va));
		StringSplitByString(a1.c_str(), w1.c_str(), std::back_inserter(va));
		StringSplitByString(a1.c_str(), x1.c_str(), std::back_inserter(va));
		StringSplitByString(w1.c_str(), a1.c_str(), std::back_inserter(vw));
		StringSplitByString(w1.c_str(), w1.c_str(), std::back_inserter(vw));
		StringSplitByString(w1.c_str(), x1.c_str(), std::back_inserter(vw));
		StringSplitByString(x1.c_str(), a1.c_str(), std::back_inserter(vx));
		StringSplitByString(x1.c_str(), w1.c_str(), std::back_inserter(vx));
		StringSplitByString(x1.c_str(), x1.c_str(), std::back_inserter(vx));

		StringSplitByString(a1.c_str(), a1, std::back_inserter(va));
		StringSplitByString(a1.c_str(), w1, std::back_inserter(va));
		StringSplitByString(a1.c_str(), x1, std::back_inserter(va));
		StringSplitByString(w1.c_str(), a1, std::back_inserter(vw));
		StringSplitByString(w1.c_str(), w1, std::back_inserter(vw));
		StringSplitByString(w1.c_str(), x1, std::back_inserter(vw));
		StringSplitByString(x1.c_str(), a1, std::back_inserter(vx));
		StringSplitByString(x1.c_str(), w1, std::back_inserter(vx));
		StringSplitByString(x1.c_str(), x1, std::back_inserter(vx));

		StringSplitByString(a1, a1.c_str(), std::back_inserter(va));
		StringSplitByString(a1, w1.c_str(), std::back_inserter(va));
		StringSplitByString(a1, x1.c_str(), std::back_inserter(va));
		StringSplitByString(w1, a1.c_str(), std::back_inserter(vw));
		StringSplitByString(w1, w1.c_str(), std::back_inserter(vw));
		StringSplitByString(w1, x1.c_str(), std::back_inserter(vw));
		StringSplitByString(x1, a1.c_str(), std::back_inserter(vx));
		StringSplitByString(x1, w1.c_str(), std::back_inserter(vx));
		StringSplitByString(x1, x1.c_str(), std::back_inserter(vx));

		StringSplitByString(a1, a1, std::back_inserter(va));
		StringSplitByString(a1, w1, std::back_inserter(va));
		StringSplitByString(a1, x1, std::back_inserter(va));
		StringSplitByString(w1, a1, std::back_inserter(vw));
		StringSplitByString(w1, w1, std::back_inserter(vw));
		StringSplitByString(w1, x1, std::back_inserter(vw));
		StringSplitByString(x1, a1, std::back_inserter(vx));
		StringSplitByString(x1, w1, std::back_inserter(vx));
		StringSplitByString(x1, x1, std::back_inserter(vx));

		a1 = StringJoin(va.begin(), va.end(), a1);
		a1 = StringJoin<char>(va.begin(), va.end(), w1);
		a1 = StringJoin<char>(va.begin(), va.end(), x1);
		a1 = StringJoin(va.begin(), va.end(), a1.c_str());
		a1 = StringJoin<char>(va.begin(), va.end(), w1.c_str());
		a1 = StringJoin<char>(va.begin(), va.end(), x1.c_str());

		w1 = StringJoin<wchar_t>(va.begin(), va.end(), a1);
		w1 = StringJoin(va.begin(), va.end(), w1);
		w1 = StringJoin<wchar_t>(va.begin(), va.end(), x1);
		w1 = StringJoin<wchar_t>(va.begin(), va.end(), a1.c_str());
		w1 = StringJoin(va.begin(), va.end(), w1.c_str());
		w1 = StringJoin<wchar_t>(va.begin(), va.end(), x1.c_str());

		x1 = StringJoin<__int32>(va.begin(), va.end(), a1);
		x1 = StringJoin<__int32>(va.begin(), va.end(), w1);
		x1 = StringJoin(va.begin(), va.end(), x1);
		x1 = StringJoin<__int32>(va.begin(), va.end(), a1.c_str());
		x1 = StringJoin<__int32>(va.begin(), va.end(), w1.c_str());
		x1 = StringJoin(va.begin(), va.end(), x1.c_str());

		a1 = StringTrim(a1, a1);
		a1 = StringTrim(a1, w1);
		a1 = StringTrim(a1, x1);
		w1 = StringTrim(w1, a1);
		w1 = StringTrim(w1, w1);
		w1 = StringTrim(w1, x1);
		x1 = StringTrim(x1, a1);
		x1 = StringTrim(x1, w1);
		x1 = StringTrim(x1, x1);

		a1 = StringTrim(a1, a1.c_str());
		a1 = StringTrim(a1, w1.c_str());
		a1 = StringTrim(a1, x1.c_str());
		w1 = StringTrim(w1, a1.c_str());
		w1 = StringTrim(w1, w1.c_str());
		w1 = StringTrim(w1, x1.c_str());
		x1 = StringTrim(x1, a1.c_str());
		x1 = StringTrim(x1, w1.c_str());
		x1 = StringTrim(x1, x1.c_str());

		a1 = StringTrim(a1.c_str(), a1);
		a1 = StringTrim(a1.c_str(), w1);
		a1 = StringTrim(a1.c_str(), x1);
		w1 = StringTrim(w1.c_str(), a1);
		w1 = StringTrim(w1.c_str(), w1);
		w1 = StringTrim(w1.c_str(), x1);
		x1 = StringTrim(x1.c_str(), a1);
		x1 = StringTrim(x1.c_str(), w1);
		x1 = StringTrim(x1.c_str(), x1);

		a1 = StringTrim(a1.c_str(), a1.c_str());
		a1 = StringTrim(a1.c_str(), w1.c_str());
		a1 = StringTrim(a1.c_str(), x1.c_str());
		w1 = StringTrim(w1.c_str(), a1.c_str());
		w1 = StringTrim(w1.c_str(), w1.c_str());
		w1 = StringTrim(w1.c_str(), x1.c_str());
		x1 = StringTrim(x1.c_str(), a1.c_str());
		x1 = StringTrim(x1.c_str(), w1.c_str());
		x1 = StringTrim(x1.c_str(), x1.c_str());

		return true;
}


