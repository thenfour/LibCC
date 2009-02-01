

#include "test.h"
#include "resource.h"
#include "libcc\parse.hpp"
#include "libcc\timer.hpp"

#include <sstream>

using namespace LibCC::Parse;

extern void StartBenchmark(LibCC::Timer& t);
extern void ReportBenchmark(LibCC::Timer& t, const std::string& name);

/*
	For testing purposes, create a simple parser which demonstrates:
	x Or
	x Sequence
	x Optional
	x Char
	x Str
	- Not
	x Spaces
	x CInteger
	- FalseMsg

	<element numericAttribute=1 stringAttribute="hi there" />
*/
struct Attribute
{
	std::wstring name;
	std::wstring stringValue;
	int intValue;
};

struct Element
{
	std::wstring openingName;
	std::wstring closingName;
	std::vector<Element> m_children;
	std::vector<Attribute> m_attributes;
};


template<typename Toutput>
struct AttributeParser740T : ParserWithOutput<Toutput, AttributeParser740T<Toutput> >
{
	AttributeParser740T(const Toutput& output_) : ParserWithOutput(output_) { }
	bool Parse(ParseResult& result, ScriptReader& input)
	{
		Attribute temp;
		Parser identifier = OneOrMore(CharRangeI('a', 'z', CharToStringOutput(temp.name)) | CharRange('0', '9', CharToStringOutput(temp.name)));
		Parser p = identifier + FalseMsg(L"Error; missing equal sign", Char('='))
			+ Or
			(
				CInteger2(temp.intValue),
				StringParser(temp.stringValue)
			);
		if(!p.ParseRetainingStateOnError(result, input))
			return false;
		output.Save(input, temp);
		return true;
	}
};
template<typename Toutput>
AttributeParser740T<Toutput> AttributeParser740(const Toutput& output_)
{
	return AttributeParser740T<Toutput>(output_);
}

template<typename Toutput>
struct ElementParser740T : ParserWithOutput<Toutput, ElementParser740T<Toutput> >
{
	ElementParser740T(const Toutput& output_) : ParserWithOutput(output_) { }
	bool Parse(ParseResult& result, ScriptReader& input)
	{
		Element temp;

		Parser openingIdentifier = FalseMsg(L"Error; error parsing opening tag name",
			OneOrMore(CharRangeI('a', 'z', CharToStringOutput(temp.openingName)) | CharRange('0', '9', CharToStringOutput(temp.openingName))));

		Parser closingIdentifier = FalseMsg(L"Error; error parsing closing tag name",
			OneOrMore(CharRangeI('a', 'z', CharToStringOutput(temp.closingName)) | CharRange('0', '9', CharToStringOutput(temp.closingName))));

		Parser p =
			FalseMsg(L"Error; error parsing element.",
			(
				Char('<')
				>> openingIdentifier
				+ ZeroOrMoreS
				(
					AttributeParser740(VectorOutput(temp.m_attributes))
				)
				+ Or
				(
					FalseMsg(L"Error; error parsing simple closing tag", Str(L"/>")),
					(
						Char('>')
						+ ZeroOrMoreS(ElementParser740(VectorOutput(temp.m_children)))
						+ FalseMsg(L"Error; error parsing closing tag",
						(
							Str(L"</")
							>> closingIdentifier
							>> Char('>')
						))
					)
				)
			));
		if(!p.ParseRetainingStateOnError(result, input))
			return false;
		output.Save(input, temp);
		return true;
	}
};

template<typename Toutput>
ElementParser740T<Toutput> ElementParser740(const Toutput& output_)
{
	return ElementParser740T<Toutput>(output_);
}



inline std::wstring LoadTextFileResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
    HRSRC hrsrc=FindResource(hInstance, szResName, szResType);
    if(!hrsrc) return L"";
    HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
    DWORD sz = SizeofResource(hInstance, hrsrc);
    void* ptr1 = LockResource(hg1);

		// assume the encoding is ASCII.
		std::string a((const char*)ptr1, sz);
		return LibCC::ToUTF16(a);
} 

bool ParseBenchmark()
{
  LibCC::Timer t;
#ifdef _DEBUG
  const int MaxNum = 10;
#else
  const int MaxNum = 50;
#endif

	std::cout << std::endl << "LibCC::Parse benchmarks, " << MaxNum << " passes." << std::endl;

	//////////////////////////////////
	// for version 740, 50 passes, i get results between 4.89 and 5.02 seconds.
	// after optimizing AdvancePastComments()
	// between 1.04 and 1.11 seconds. wow, huge improvement there.
	// now changing output to templated.
	// between  0.78 and 0.83
	std::cout << std::endl << "Parse big typical file:" << std::endl;
	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		ParseResultMem result;
		Element el;
		CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*Space() + ElementParser740(RefOutput(el)) + Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "Result");

	return true;
}


