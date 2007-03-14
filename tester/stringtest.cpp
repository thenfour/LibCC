

#include "test.h"
#include "libcc\stringutil.h"
using namespace LibCC;

template<typename Char>
class TestCStringT
{
public:
  TestCStringT()
  {
  }
  TestCStringT(const TestCStringT& s) :
    str(s.str)
  {
  }
  TestCStringT(const Char* s) :
    str(s)
  {
  }
  TestCStringT(const std::basic_string<Char>& s) :
    str(s)
  {
  }

  operator const Char* () const { return str.c_str(); }
  operator const std::basic_string<Char>& () const { return str; }

  std::basic_string<Char> str;
};



bool StringTest()
{
  std::string a;
  std::wstring w;

  a = "omgomg";
  a = StringReplace(a.c_str(), "omg", L"wtf");

  a = Format().d<2>(1.1).Str();

  a = "hi";
  w = L"hi";
  TestCStringT<char> ca("hi");
  TestCStringT<wchar_t> cw(L"hi");

  // String equals = true
  StringEquals("hi", "hi");
  StringEquals("hi", L"hi");
  StringEquals(L"hi", L"hi");

  StringEquals(a, a);
  StringEquals(a, L"hi");

  StringEquals("hi", w);
  StringEquals(w, w);

  StringEquals(a, w);
  StringEquals(w, a);

  StringEquals(a, static_cast<const char*>(ca));
  StringEquals(static_cast<const std::wstring&>(cw), a);

  // StringI equals = true
  StringEqualsI("hi", "hi");
  StringEqualsI("hi", L"hi");
  StringEqualsI(L"hi", L"hi");

  StringEqualsI(a, a);
  StringEqualsI(a, L"hi");

  StringEqualsI("hi", w);
  StringEqualsI(w, w);

  StringEqualsI(a, w);
  StringEqualsI(w, a);

  StringEqualsI(a, static_cast<const char*>(ca));
  StringEqualsI(static_cast<const std::wstring&>(cw), a);

	// convert encodings
	Blob<BYTE> b;
	TestAssert(SUCCEEDED(ToMBCS(L"aoeu", b, CP_ACP)));

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
	TestAssert(SUCCEEDED(ToUnicode(a2, w, CP_UTF8)));
	TestAssert(w2 == w);

	// TODO: add tests for other MBCS encodings

  return true;
}


