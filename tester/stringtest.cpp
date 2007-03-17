

#include "test.h"
#include "libcc\stringutil.h"
#include <vector>
using namespace LibCC;


bool StringTest()
{
	{
		// ****  StringSplit
		std::wstring w = L",a,b,,aoeu,";
		std::wstring bw = L",";
		std::vector<std::wstring> v;
		StringSplit(w, bw, std::back_inserter(v));
		TestAssert(v.size() == 6);
		TestAssert(v[0].empty());
		TestAssert(v[1] == L"a");
		TestAssert(v[2] == L"b");
		TestAssert(v[3].empty());
		TestAssert(v[4] == L"aoeu");
		TestAssert(v[5].empty());

		v.clear();
		StringSplit(L",a,b,,aoeu,", bw, std::back_inserter(v));
		TestAssert(v.size() == 6);

		v.clear();
		StringSplit(w, L",", std::back_inserter(v));
		TestAssert(v.size() == 6);

		std::vector<std::string> va;
		StringSplit("---a---b------", "---", std::back_inserter(va));
		TestAssert(va.size() == 5);
		TestAssert(va[0].empty());
		TestAssert(va[1] == "a");
		TestAssert(va[2] == "b");
		TestAssert(va[3].empty());
		TestAssert(va[4].empty());

		v.clear();
		// using unicode chars
		w[0] = 9674;
		w[2] = 9674;
		w[4] = 9674;
		w[5] = 9674;
		w[10] = 9674;
		bw[0] = 9674;
		StringSplit(w, bw, std::back_inserter(v));
		TestAssert(v.size() == 6);
		TestAssert(v[0].empty());
		TestAssert(v[1] == L"a");
		TestAssert(v[2] == L"b");
		TestAssert(v[3].empty());
		TestAssert(v[4] == L"aoeu");
		TestAssert(v[5].empty());
		
		// other random tests
		v.clear();
		StringSplit(L"carl", L"a", std::back_inserter(v));
		TestAssert(v.size() == 2);
		TestAssert(v[0] == L"c");
		TestAssert(v[1] == L"rl");
		
		// other random tests
		v.clear();
		StringSplit(L"", L"", std::back_inserter(v));
		TestAssert(v.size() == 0);
		v.clear();
		StringSplit(L"", L"aoeu", std::back_inserter(v));
		TestAssert(v.size() == 0);
		v.clear();
		StringSplit(L"12345", L"", std::back_inserter(v));
		TestAssert(v.size() == 1);
		v.clear();
		StringSplit(L"12345", L"aoeu", std::back_inserter(v));
		TestAssert(v.size() == 1);
	}

	{ // *** StringJoin
		std::vector<std::string> va;
		va.push_back("tube");
		va.push_back("bowl");
		va.push_back("porcelain");
		std::string a = StringJoin(va.begin(), va.end(), "");
		TestAssert(a == "tubebowlporcelain");
		a = StringJoin(va.begin(), va.end(), "--");
		TestAssert(a == "tube--bowl--porcelain");

		va.clear();		
		va.push_back("tube");
		va.push_back("bowl");
		va.push_back("porcelain");
		va.push_back("");
		std::string b = ", ";
		a = StringJoin(va.begin(), va.end(), b);
		TestAssert(a == "tube, bowl, porcelain, ");
		
		va.clear();
		a = StringJoin(va.begin(), va.end(), b);
		TestAssert(a.empty());
		
		// unicode
		std::vector<std::wstring> vw;
		vw.push_back(L"-");
		vw.push_back(L"--");
		vw.push_back(L"blarg");
		vw[0][0] = 9674;
		vw[1][0] = 9674;
		vw[1][1] = 9674;
		std::wstring w = StringJoin(vw.begin(), vw.end(), L",");
		std::wstring compare = L"-,--,blarg";
		compare[0] = 9674;
		compare[2] = 9674;
		compare[3] = 9674;
		TestAssert(w == compare);
	}

	{ // **** StringTrim
		std::string a = StringTrim("", "");
		TestAssert(a.empty());
		
		a = StringTrim("aoeu", "aoeu");
		TestAssert(a.empty());
		
		a = StringTrim("aoeu", "uoea");
		TestAssert(a.empty());
		
		a = StringTrim("aoxeu", "uoea");
		TestAssert(a == "x");
		
		a = StringTrim("123x123x", "213");
		TestAssert(a == "x123x");
		
		a = StringTrim("x123x321", "213");
		TestAssert(a == "x123x");
		
		a = StringTrim("123x123x321", "213");
		TestAssert(a == "x123x");
		
		a = StringTrim("hi", "");
		TestAssert(a == "hi");
		
		std::wstring w1 = L"   \t omg\t    ";
		std::wstring w2 = L" \t";
		std::wstring w3 = StringTrim(w1, w2);
		TestAssert(w3 == L"omg");
	}
	
	{ // **** StringReplace
		std::string a1, a2, a3, a4;
		std::wstring w1, w2, w3, w4;
		
		a1 = "fredfred fredfred frefre fred";
		a2 = "fred";
		a3 = "sally";
		a4 = StringReplace(a1, a2, a3);
		TestAssert(a4 == "sallysally sallysally frefre sally");
		
		w1 = L"";
		w2 = L"";
		w3 = L"";
		w4 = StringReplace(w1, w2, w3);
		TestAssert(w1.empty());
		
		w1 = L"aoeu";
		w2 = L"";
		w3 = L"";
		w4 = StringReplace(w1, w2, w3);
		TestAssert(w1 == L"aoeu");
		
		w1 = L"aoeu";
		w2 = L"1234";
		w3 = L"";
		w4 = StringReplace(w1, w2, w3);
		TestAssert(w1 == L"aoeu");
		
		w1 = L"hi, fred";
		w2 = L"fred";
		w3 = L"sally";
		TestAssert(StringReplace(w1, w2, w3) == L"hi, sally");
		TestAssert(StringReplace(w1, w2, L"sally") == L"hi, sally");
		TestAssert(StringReplace(w1, L"fred", w3) == L"hi, sally");
		TestAssert(StringReplace(w1, L"fred", L"sally") == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", w2, w3) == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", w2, L"sally") == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", L"fred", w3) == L"hi, sally");
		TestAssert(StringReplace(L"hi, fred", L"fred", L"sally") == L"hi, sally");
	}
	
	{	// **** StringToUpper
		std::string a1;
		std::wstring w1;

		std::string srcA = "ABCDE123!@#abcdefghijklm˙·ÈÛÔ";
		std::wstring srcW = L"ABCDE123!@#abcdefghijklm˙·ÈÛÔ";
		std::basic_string<DWORD> srcX;
		ConvertString(srcW, srcX);
		
		std::string correctA = "ABCDE123!@#ABCDEFGHIJKLM⁄¡…”œ";
		std::wstring correctW = L"ABCDE123!@#ABCDEFGHIJKLM⁄¡…”œ";
		std::basic_string<DWORD> correctX;
		ConvertString(correctW, correctX);
		
		a1 = StringToUpper("ABCDE123!@#abcdefghijklm˙·ÈÛÔ");
		TestAssert(a1 == correctA);
		
		w1 = StringToUpper(L"ABCDE123!@#abcdefghijklm˙·ÈÛÔ");
		TestAssert(w1 == correctW);

		w1 = StringToUpper(srcW);
		TestAssert(w1 == correctW);
		
		a1 = StringToUpper(srcA);
		TestAssert(w1 == correctW);
		
		std::basic_string<DWORD> x1;
		x1 = StringToUpper(srcX);
		TestAssert(x1 == correctX);
	}
	
	{	// **** StringToLower
		std::string a1;
		std::wstring w1;

		std::string srcA = "aeousnt234@#$//¡»‘œÁAEXL>I<TTT";
		std::wstring srcW = L"aeousnt234@#$//¡»‘œÁAEXL>I<TTT";
		std::basic_string<DWORD> srcX;
		ConvertString(srcW, srcX);
		
		std::string correctA = "aeousnt234@#$//·ËÙÔÁaexl>i<ttt";
		std::wstring correctW = L"aeousnt234@#$//·ËÙÔÁaexl>i<ttt";
		std::basic_string<DWORD> correctX;
		ConvertString(correctW, correctX);
		
		a1 = StringToLower("aeousnt234@#$//¡»‘œÁAEXL>I<TTT");
		TestAssert(a1 == correctA);
		
		w1 = StringToLower(L"aeousnt234@#$//¡»‘œÁAEXL>I<TTT");
		TestAssert(w1 == correctW);

		w1 = StringToLower(srcW);
		TestAssert(w1 == correctW);
		
		a1 = StringToLower(srcA);
		TestAssert(w1 == correctW);
		
		std::basic_string<DWORD> x1;
		x1 = StringToLower(srcX);
		TestAssert(x1 == correctX);
	}

	{ // StringEquals
		std::string a1, a2;
		std::wstring w1, w2;

		a1 = "aoeu";
		a2 = "AOEU";
		TestAssert(!StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "AOEUx";
		TestAssert(!StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "aoeu";
		TestAssert(StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "aoeux";
		TestAssert(!StringEquals(a1, a2));

		a1 = "aoeu";
		a2 = "";
		TestAssert(!StringEquals(a1, a2));

		a1 = "";
		a2 = "";
		TestAssert(StringEquals(a1, a2));

		a1 = "";
		a2 = "a";
		TestAssert(!StringEquals(a1, a2));

		w1 = L"aoeu";
		w2 = L"aoeu";
		TestAssert(StringEquals(w1, w2));

		w1 = L"aoeu";
		w2 = L"aoeux";
		TestAssert(!StringEquals(w1, w2));

		a1 = "aoeu";
		TestAssert(StringEquals("aoeu", a1));

		a1 = "aoeu";
		TestAssert(StringEquals(a1, "aoeu"));

		TestAssert(StringEquals(L"HI THERE", L"HI THERE"));
	}
	
	{ // XStringEquals
		std::string a1;
		std::wstring w1;
		
		a1 = "aoeu";
		TestAssert(!XStringEquals(a1, "AOEU"));

		a1 = "aoeu";
		TestAssert(!XStringEquals(a1, "AOEUx"));

		w1 = L"aoeu";
		TestAssert(XStringEquals(w1, "aoeu"));

		w1 = L"aoeu";
		TestAssert(!XStringEquals(w1, "aoeux"));

		w1 = L"aoeu";
		TestAssert(!XStringEquals(w1, ""));

		w1 = L"";
		TestAssert(XStringEquals(w1, ""));

		a1 = "";
		TestAssert(!XStringEquals(a1, "a"));
	}
	
	{ // XStringContains
		TestAssert(XStringContains("aoeu", L'a'));
		TestAssert(!XStringContains("aoeu", L'x'));
		TestAssert(!XStringContains("aoeU", L'u'));
		TestAssert(XStringContains("aoeU", L'U'));
		TestAssert(!XStringContains("", L'a'));
		TestAssert(!XStringContains("aoeu", (char)0));
		TestAssert(!XStringContains("aoeu", (DWORD)0));
		TestAssert(!XStringContains("", (DWORD)0));
	}
	
	{ // XStringFindFirstOf
		std::string a1 = "1aoeu555555aoeu57";
		std::wstring w1 = L"1aoeu555555aoeu57";
		TestAssert(1 == XStringFindFirstOf(a1, "ueoa"));
		TestAssert(std::string::npos == XStringFindFirstOf(a1, "9342999"));
		TestAssert(5 == XStringFindFirstOf(a1, "5"));
		TestAssert(16 == XStringFindFirstOf(a1, "7"));
		TestAssert(16 == XStringFindFirstOf(w1, "7"));
		TestAssert(std::string::npos == XStringFindFirstOf(a1, ""));
		a1.clear();
		TestAssert(std::string::npos == XStringFindFirstOf(a1, "a"));
		TestAssert(std::string::npos == XStringFindFirstOf(a1, ""));
	}
	
	{ // XStringFindLastOf
		std::string a1 = "1aoeu555555aoeu57";
		std::wstring w1 = L"1aoeu555555aoeu57";
		TestAssert(14 == XStringFindLastOf(a1, "ueoa"));
		TestAssert(std::string::npos == XStringFindLastOf(a1, "9342999"));
		TestAssert(15 == XStringFindLastOf(a1, "5"));
		TestAssert(16 == XStringFindLastOf(a1, "7"));
		TestAssert(16 == XStringFindLastOf(w1, "7"));
		TestAssert(0 == XStringFindLastOf(w1, "1"));
		TestAssert(std::string::npos == XStringFindLastOf(a1, ""));
		a1.clear();
		TestAssert(std::string::npos == XStringFindLastOf(a1, "a"));
		TestAssert(std::string::npos == XStringFindLastOf(a1, ""));
	}

  std::string a;
  std::wstring w;
//
//  a = "omgomg";
//  a = StringReplace(a, "omg", "wtfu?");
//  TestAssert(a == "wtfu?wtfu?");
//
//  a = "";
//  a = StringReplace(a, "omg", "wtfu?");
//  TestAssert(a == "");
//
//  a = "zango";
//  a = StringReplace(a, "", "");
//  TestAssert(a == "zango");
//
//  w = StringReplace(L"omgomgomg", L"mg", L"8");
//  TestAssert(w == L"o8o8o8");
//
//  w = StringReplace(L"OmGoMG", L"omgomg", L"");
//  TestAssert(w == L"OmGoMG");
//
//  w = StringReplace(L"Michigan State University", L" ", L"");
//  TestAssert(w == L"MichiganStateUniversity");
//  
//
////	TestAssert(StringStartsWith("omgomg", "omg"));
//
//  a = Format().d<2>(1.1).Str();
//
//  a = "hi";
//  w = L"hi";
//  TestCStringT<char> ca("hi");
//  TestCStringT<wchar_t> cw(L"hi");

  // String equals = true
  //StringEquals("hi", "hi");
  //StringEquals("hi", L"hi");
  //StringEquals(L"hi", L"hi");

  //StringEquals(a, a);
  //StringEquals(a, L"hi");

  //StringEquals("hi", w);
  //StringEquals(w, w);

  //StringEquals(a, w);
  //StringEquals(w, a);

  //StringEquals(a, static_cast<const char*>(ca));
  //StringEquals(static_cast<const std::wstring&>(cw), a);

  //// StringI equals = true
  //StringEqualsI("hi", "hi");
  //StringEqualsI("hi", L"hi");
  //StringEqualsI(L"hi", L"hi");

  //StringEqualsI(a, a);
  //StringEqualsI(a, L"hi");

  //StringEqualsI("hi", w);
  //StringEqualsI(w, w);

  //StringEqualsI(a, w);
  //StringEqualsI(w, a);

  //StringEqualsI(a, static_cast<const char*>(ca));
  //StringEqualsI(static_cast<const std::wstring&>(cw), a);

	// convert encodings
	Blob<BYTE> b;
	TestAssert(SUCCEEDED(ConvertString(L"aoeu", b, CP_ACP)));

	// TEST UNICODE -> UTF8 -> UNICODE
	std::string a2;
	w = L"aoeu";
	w[0] = 9674;
	std::wstring w2 = w;// make a copy for comparing later
	TestAssert(SUCCEEDED(ToUTF8(w, a2)));
	// now that string in UTF8 is E2 97 8A 6F 65 75 (verified with Uedit32)
	TestAssert(a2.length() == 6);
	TestAssert((BYTE)a2[0] == 0xe2);
	TestAssert((BYTE)a2[1] == 0x97);
	TestAssert((BYTE)a2[2] == 0x8a);
	TestAssert(a2[3] == 0x6f);
	TestAssert(a2[4] == 0x65);
	TestAssert(a2[5] == 0x75);

	// convert back to unicode
	w = L"12312312";// clean the palette
	TestAssert(SUCCEEDED(ConvertString(a2, w, CP_UTF8)));
	TestAssert(w2 == w);

	// TODO: add tests for other MBCS encodings

  return true;
}


