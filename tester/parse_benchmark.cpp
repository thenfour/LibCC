
#define _SECURE_SCL 0

#include "test.h"
#include "resource.h"
#include "libcc\parse.hpp"
#include "parse-743.hpp"
#include "parse-745.hpp"
#include "libcc\timer.hpp"

#include <sstream>


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
struct AttributeParserHEADT : LibCC::Parse::ParserWithOutput<Toutput, AttributeParserHEADT<Toutput> >
{
	AttributeParserHEADT(const Toutput& output_) : ParserWithOutput(output_) { }
	bool Parse(LibCC::Parse::ParseResult& result, LibCC::Parse::ScriptReader& input)
	{
		Attribute temp;
		LibCC::Parse::Parser identifier = LibCC::Parse::OneOrMore(LibCC::Parse::CharRangeI('a', 'z', LibCC::Parse::CharToStringOutput(temp.name)) | LibCC::Parse::CharRange('0', '9', LibCC::Parse::CharToStringOutput(temp.name)));
		LibCC::Parse::Parser p = identifier + LibCC::Parse::FalseMsg(L"Error; missing equal sign", LibCC::Parse::Char('='))
			+ LibCC::Parse::Or
			(
				LibCC::Parse::CInteger2(temp.intValue),
				LibCC::Parse::StringParser(temp.stringValue)
			);
		if(!p.ParseRetainingStateOnError(result, input))
			return false;
		output.Save(input, temp);
		return true;
	}
};
template<typename Toutput>
AttributeParserHEADT<Toutput> AttributeParserHEAD(const Toutput& output_)
{
	return AttributeParserHEADT<Toutput>(output_);
}

template<typename Toutput>
struct ElementParserHEADT : LibCC::Parse::ParserWithOutput<Toutput, ElementParserHEADT<Toutput> >
{
	ElementParserHEADT(const Toutput& output_) : ParserWithOutput(output_) { }
	bool Parse(LibCC::Parse::ParseResult& result, LibCC::Parse::ScriptReader& input)
	{
		Element temp;

		LibCC::Parse::Parser openingIdentifier = LibCC::Parse::FalseMsg(L"Error; error parsing opening tag name",
			LibCC::Parse::OneOrMore(LibCC::Parse::CharRangeI('a', 'z', LibCC::Parse::CharToStringOutput(temp.openingName)) | LibCC::Parse::CharRange('0', '9', LibCC::Parse::CharToStringOutput(temp.openingName))));

		LibCC::Parse::Parser closingIdentifier = LibCC::Parse::FalseMsg(L"Error; error parsing closing tag name",
			LibCC::Parse::OneOrMore(LibCC::Parse::CharRangeI('a', 'z', LibCC::Parse::CharToStringOutput(temp.closingName)) | LibCC::Parse::CharRange('0', '9', LibCC::Parse::CharToStringOutput(temp.closingName))));

		LibCC::Parse::Parser p =
			LibCC::Parse::FalseMsg(L"Error; error parsing element.",
			(
				LibCC::Parse::Char('<')
				>> openingIdentifier
				+ LibCC::Parse::ZeroOrMoreS
				(
					AttributeParserHEAD(LibCC::Parse::VectorOutput(temp.m_attributes))
				)
				+ LibCC::Parse::Or
				(
					LibCC::Parse::FalseMsg(L"Error; error parsing simple closing tag", LibCC::Parse::Str(L"/>")),
					(
						LibCC::Parse::Char('>')
						+ LibCC::Parse::ZeroOrMoreS(ElementParserHEAD(LibCC::Parse::VectorOutput(temp.m_children)))
						+ LibCC::Parse::FalseMsg(L"Error; error parsing closing tag",
						(
							LibCC::Parse::Str(L"</")
							>> closingIdentifier
							>> LibCC::Parse::Char('>')
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
ElementParserHEADT<Toutput> ElementParserHEAD(const Toutput& output_)
{
	return ElementParserHEADT<Toutput>(output_);
}





template<typename Toutput>
struct AttributeParser745T : LibCC_745::Parse::ParserWithOutput<Toutput, AttributeParser745T<Toutput> >
{
	AttributeParser745T(const Toutput& output_) : ParserWithOutput(output_) { }
	bool Parse(LibCC_745::Parse::ParseResult& result, LibCC_745::Parse::ScriptReader& input)
	{
		Attribute temp;
		LibCC_745::Parse::Parser identifier = LibCC_745::Parse::OneOrMore(LibCC_745::Parse::CharRangeI('a', 'z', LibCC_745::Parse::CharToStringOutput(temp.name)) | LibCC_745::Parse::CharRange('0', '9', LibCC_745::Parse::CharToStringOutput(temp.name)));
		LibCC_745::Parse::Parser p = identifier + LibCC_745::Parse::FalseMsg(L"Error; missing equal sign", LibCC_745::Parse::Char('='))
			+ LibCC_745::Parse::Or
			(
				LibCC_745::Parse::CInteger2(temp.intValue),
				LibCC_745::Parse::StringParser(temp.stringValue)
			);
		if(!p.ParseRetainingStateOnError(result, input))
			return false;
		output.Save(input, temp);
		return true;
	}
};
template<typename Toutput>
AttributeParser745T<Toutput> AttributeParser745(const Toutput& output_)
{
	return AttributeParser745T<Toutput>(output_);
}

template<typename Toutput>
struct ElementParser745T : LibCC_745::Parse::ParserWithOutput<Toutput, ElementParser745T<Toutput> >
{
	ElementParser745T(const Toutput& output_) : ParserWithOutput(output_) { }
	bool Parse(LibCC_745::Parse::ParseResult& result, LibCC_745::Parse::ScriptReader& input)
	{
		Element temp;

		LibCC_745::Parse::Parser openingIdentifier = LibCC_745::Parse::FalseMsg(L"Error; error parsing opening tag name",
			LibCC_745::Parse::OneOrMore(LibCC_745::Parse::CharRangeI('a', 'z', LibCC_745::Parse::CharToStringOutput(temp.openingName)) | LibCC_745::Parse::CharRange('0', '9', LibCC_745::Parse::CharToStringOutput(temp.openingName))));

		LibCC_745::Parse::Parser closingIdentifier = LibCC_745::Parse::FalseMsg(L"Error; error parsing closing tag name",
			LibCC_745::Parse::OneOrMore(LibCC_745::Parse::CharRangeI('a', 'z', LibCC_745::Parse::CharToStringOutput(temp.closingName)) | LibCC_745::Parse::CharRange('0', '9', LibCC_745::Parse::CharToStringOutput(temp.closingName))));

		LibCC_745::Parse::Parser p =
			LibCC_745::Parse::FalseMsg(L"Error; error parsing element.",
			(
				LibCC_745::Parse::Char('<')
				>> openingIdentifier
				+ LibCC_745::Parse::ZeroOrMoreS
				(
					AttributeParser745(LibCC_745::Parse::VectorOutput(temp.m_attributes))
				)
				+ LibCC_745::Parse::Or
				(
					LibCC_745::Parse::FalseMsg(L"Error; error parsing simple closing tag", LibCC_745::Parse::Str(L"/>")),
					(
						LibCC_745::Parse::Char('>')
						+ LibCC_745::Parse::ZeroOrMoreS(ElementParser745(LibCC_745::Parse::VectorOutput(temp.m_children)))
						+ LibCC_745::Parse::FalseMsg(L"Error; error parsing closing tag",
						(
							LibCC_745::Parse::Str(L"</")
							>> closingIdentifier
							>> LibCC_745::Parse::Char('>')
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
ElementParser745T<Toutput> ElementParser745(const Toutput& output_)
{
	return ElementParser745T<Toutput>(output_);
}





struct AttributeParser743 : LibCC_743::Parse::ParserWithOutput<Attribute, AttributeParser743>
{
	AttributeParser743(const LibCC_743::Parse::OutputPtr<Attribute>& output_) { output.Assign(output_); }
	bool Parse(LibCC_743::Parse::ParseResult& result, LibCC_743::Parse::ScriptReader& input)
	{
		Attribute temp;
		LibCC_743::Parse::Parser identifier = LibCC_743::Parse::OneOrMore(LibCC_743::Parse::CharRangeI('a', 'z', LibCC_743::Parse::CharToStringOutput(temp.name)) | LibCC_743::Parse::CharRange('0', '9', LibCC_743::Parse::CharToStringOutput(temp.name)));
		LibCC_743::Parse::Parser p = identifier + LibCC_743::Parse::FalseMsg(L"Error; missing equal sign", LibCC_743::Parse::Char('='))
			+ LibCC_743::Parse::Or
			(
				LibCC_743::Parse::CInteger2(temp.intValue),
				LibCC_743::Parse::StringParser(temp.stringValue)
			);
		if(!p.ParseRetainingStateOnError(result, input))
			return false;
		output->Save(input, temp);
		return true;
	}
};

struct ElementParser743 : LibCC_743::Parse::ParserWithOutput<Element, ElementParser743>
{
	ElementParser743(const LibCC_743::Parse::OutputPtr<Element>& output_) { output.Assign(output_); }
	bool Parse(LibCC_743::Parse::ParseResult& result, LibCC_743::Parse::ScriptReader& input)
	{
		Element temp;

		LibCC_743::Parse::Parser openingIdentifier = LibCC_743::Parse::FalseMsg(L"Error; error parsing opening tag name",
			LibCC_743::Parse::OneOrMore(LibCC_743::Parse::CharRangeI('a', 'z', LibCC_743::Parse::CharToStringOutput(temp.openingName)) | LibCC_743::Parse::CharRange('0', '9', LibCC_743::Parse::CharToStringOutput(temp.openingName))));

		LibCC_743::Parse::Parser closingIdentifier = LibCC_743::Parse::FalseMsg(L"Error; error parsing closing tag name",
			LibCC_743::Parse::OneOrMore(LibCC_743::Parse::CharRangeI('a', 'z', LibCC_743::Parse::CharToStringOutput(temp.closingName)) | LibCC_743::Parse::CharRange('0', '9', LibCC_743::Parse::CharToStringOutput(temp.closingName))));

		LibCC_743::Parse::Parser p =
			LibCC_743::Parse::FalseMsg(L"Error; error parsing element.",
			(
				LibCC_743::Parse::Char('<')
				>> openingIdentifier
				+ LibCC_743::Parse::ZeroOrMoreS
				(
					AttributeParser743(LibCC_743::Parse::VectorOutput(temp.m_attributes))
				)
				+ LibCC_743::Parse::Or
				(
					LibCC_743::Parse::FalseMsg(L"Error; error parsing simple closing tag", LibCC_743::Parse::Str(L"/>")),
					(
						LibCC_743::Parse::Char('>')
						+ LibCC_743::Parse::ZeroOrMoreS(ElementParser743(LibCC_743::Parse::VectorOutput(temp.m_children)))
						+ LibCC_743::Parse::FalseMsg(L"Error; error parsing closing tag",
						(
							LibCC_743::Parse::Str(L"</")
							>> closingIdentifier
							>> LibCC_743::Parse::Char('>')
						))
					)
				)
			));
		if(!p.ParseRetainingStateOnError(result, input))
			return false;
		output->Save(input, temp);
		return true;
	}
};



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
  const int MaxNum = 5;
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
	// after switching to using parserRoot instead of parserbase (to eliminate all the allocations),
	// between 1.14 and 1.16. WEIRD. Oh i see. The previous version is now going at 1.68 seconds - presumably because of the new compiler version.
	std::cout << std::endl << "Parse big typical file:" << std::endl;

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC_743::Parse::ParseResultMem result;
		Element el;
		LibCC_743::Parse::CScriptReader740 input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC_743::Parse::Space() + ElementParser743(LibCC_743::Parse::RefOutput(el)) + LibCC_743::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "740 revision (original CScriptReader)");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC_743::Parse::ParseResultMem result;
		Element el;
		LibCC_743::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC_743::Parse::Space() + ElementParser743(LibCC_743::Parse::RefOutput(el)) + LibCC_743::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "743 revision (optimized cursor)");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC_745::Parse::ParseResultMem result;
		Element el;
		LibCC_745::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC_745::Parse::Space() + ElementParser745(LibCC_745::Parse::RefOutput(el)) + LibCC_745::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "745 revision (optimized outputs)");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC::Parse::ParseResultMem result;
		Element el;
		LibCC::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC::Parse::Space() + ElementParserHEAD(LibCC::Parse::RefOutput(el)) + LibCC::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "HEAD revision (optimized parserBase)");





	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC_743::Parse::ParseResultMem result;
		Element el;
		LibCC_743::Parse::CScriptReader740 input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC_743::Parse::Space() + ElementParser743(LibCC_743::Parse::RefOutput(el)) + LibCC_743::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "740 revision (original CScriptReader)");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC_743::Parse::ParseResultMem result;
		Element el;
		LibCC_743::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC_743::Parse::Space() + ElementParser743(LibCC_743::Parse::RefOutput(el)) + LibCC_743::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "743 revision (optimized cursor)");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC_745::Parse::ParseResultMem result;
		Element el;
		LibCC_745::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC_745::Parse::Space() + ElementParser745(LibCC_745::Parse::RefOutput(el)) + LibCC_745::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "745 revision (optimized outputs)");

	StartBenchmark(t);
	for(int n = 0; n < MaxNum; n ++)
  {
		LibCC::Parse::ParseResultMem result;
		Element el;
		LibCC::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
		bool r = (*LibCC::Parse::Space() + ElementParserHEAD(LibCC::Parse::RefOutput(el)) + LibCC::Parse::Eof()).ParseRetainingStateOnError(result, input);
		DoNotOptimize(el);
  }
	ReportBenchmark(t, "HEAD revision (optimized parserBase)");



	return true;
}


