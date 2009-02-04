

#include "test.h"
#include "resource.h"
#include "libcc\parse.hpp"
#include "parse-743.hpp"
#include "parse-745.hpp"
#include "parse-746.hpp"
#include "parse-746b.hpp"
#include "parse-747.hpp"
#include "parse-748.hpp"
#include "libcc\timer.hpp"

#include <sstream>


// built in ParseResult class which holds all messages in memory.
struct ParseResultStdout :
	LibCC::Parse::ParseResult
{
	int traceIndentLevel;
	bool traceEnabled;

	ParseResultStdout() :
		traceIndentLevel(0),
		traceEnabled(false)
	{
	}
	virtual void IncrementTraceIndent() { ++traceIndentLevel; }
	virtual void DecrementTraceIndent() { --traceIndentLevel; }
	virtual void SetTraceEnabled(bool b) { traceEnabled = b; }
	virtual bool IsTraceEnabled() const { return traceEnabled; }

	virtual void Trace(const std::wstring& msg)
	{
		LibCC::FormatW x;
		for(int i = 0; i < traceIndentLevel; ++ i)
		{
			x(L"  ");
		}
		x(msg);
		x(L"\r\n");
		//std::wcout << x.CStr();
		OutputDebugStringW(x.CStr());
	}

	virtual void RenderMessage(const std::wstring& msg)
	{
		//std::wcout << msg;
		OutputDebugStringW(msg.c_str());
		OutputDebugStringW(L"\r\n");
	}
};


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


namespace LibCC
{
	namespace Parse
	{
		template<typename Toutput>
		struct AttributeParserT : ParserWithOutput<Toutput, AttributeParserT<Toutput> >
		{
			virtual std::wstring GetParserName() const { return L"AttributeParser"; }
			AttributeParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
				BackupOutput(input);
				output.Save(input, temp);
				return true;
			}
		};
		template<typename Toutput>
		AttributeParserT<Toutput> AttributeParser(const Toutput& output_)
		{
			return AttributeParserT<Toutput>(output_);
		}

		template<typename Toutput>
		struct ElementParserT : ParserWithOutput<Toutput, ElementParserT<Toutput> >
		{
			virtual std::wstring GetParserName() const { return L"ElementParser"; }
			ElementParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
							AttributeParser(VectorOutput(temp.m_attributes))
						)
						+ Or
						(
							FalseMsg(L"Error; error parsing simple closing tag", Str(L"/>")),
							(
								Char('>')
								+ ZeroOrMoreS(ElementParser(VectorOutput(temp.m_children)))
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
				BackupOutput(input);
				output.Save(input, temp);
				return true;
			}
		};

		template<typename Toutput>
		ElementParserT<Toutput> ElementParser(const Toutput& output_)
		{
			return ElementParserT<Toutput>(output_);
		}
	}
}

namespace LibCC_748
{
	namespace Parse
	{
		template<typename Toutput>
		struct AttributeParserT : ParserWithOutput<Toutput, AttributeParserT<Toutput> >
		{
			virtual std::wstring GetParserName() const { return L"AttributeParser"; }
			AttributeParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
				BackupOutput(input);
				output.Save(input, temp);
				return true;
			}
		};
		template<typename Toutput>
		AttributeParserT<Toutput> AttributeParser(const Toutput& output_)
		{
			return AttributeParserT<Toutput>(output_);
		}

		template<typename Toutput>
		struct ElementParserT : ParserWithOutput<Toutput, ElementParserT<Toutput> >
		{
			virtual std::wstring GetParserName() const { return L"ElementParser"; }
			ElementParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
							AttributeParser(VectorOutput(temp.m_attributes))
						)
						+ Or
						(
							FalseMsg(L"Error; error parsing simple closing tag", Str(L"/>")),
							(
								Char('>')
								+ ZeroOrMoreS(ElementParser(VectorOutput(temp.m_children)))
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
				BackupOutput(input);
				output.Save(input, temp);
				return true;
			}
		};

		template<typename Toutput>
		ElementParserT<Toutput> ElementParser(const Toutput& output_)
		{
			return ElementParserT<Toutput>(output_);
		}
	}
}

namespace LibCC_747
{
	namespace Parse
	{
		template<typename Toutput>
		struct AttributeParserT : ParserWithOutput<Toutput, AttributeParserT<Toutput> >
		{
			virtual std::wstring GetParserName() const { return L"AttributeParser"; }
			AttributeParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
		AttributeParserT<Toutput> AttributeParser(const Toutput& output_)
		{
			return AttributeParserT<Toutput>(output_);
		}

		template<typename Toutput>
		struct ElementParserT : ParserWithOutput<Toutput, ElementParserT<Toutput> >
		{
			virtual std::wstring GetParserName() const { return L"ElementParser"; }
			ElementParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
							AttributeParser(VectorOutput(temp.m_attributes))
						)
						+ Or
						(
							FalseMsg(L"Error; error parsing simple closing tag", Str(L"/>")),
							(
								Char('>')
								+ ZeroOrMoreS(ElementParser(VectorOutput(temp.m_children)))
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
		ElementParserT<Toutput> ElementParser(const Toutput& output_)
		{
			return ElementParserT<Toutput>(output_);
		}
	}
}


namespace LibCC_746
{
	namespace Parse
	{
		template<typename Toutput>
		struct AttributeParserT : ParserWithOutput<Toutput, AttributeParserT<Toutput> >
		{
			AttributeParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
		AttributeParserT<Toutput> AttributeParser(const Toutput& output_)
		{
			return AttributeParserT<Toutput>(output_);
		}

		template<typename Toutput>
		struct ElementParserT : ParserWithOutput<Toutput, ElementParserT<Toutput> >
		{
			ElementParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
							AttributeParser(VectorOutput(temp.m_attributes))
						)
						+ Or
						(
							FalseMsg(L"Error; error parsing simple closing tag", Str(L"/>")),
							(
								Char('>')
								+ ZeroOrMoreS(ElementParser(VectorOutput(temp.m_children)))
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
		ElementParserT<Toutput> ElementParser(const Toutput& output_)
		{
			return ElementParserT<Toutput>(output_);
		}
	}
}




namespace LibCC_746b
{
	namespace Parse
	{
		template<typename Toutput>
		struct AttributeParserT : ParserWithOutput<Toutput, AttributeParserT<Toutput> >
		{
			AttributeParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
		AttributeParserT<Toutput> AttributeParser(const Toutput& output_)
		{
			return AttributeParserT<Toutput>(output_);
		}

		template<typename Toutput>
		struct ElementParserT : ParserWithOutput<Toutput, ElementParserT<Toutput> >
		{
			ElementParserT(const Toutput& output_) : ParserWithOutput(output_) { }
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
							AttributeParser(VectorOutput(temp.m_attributes))
						)
						+ Or
						(
							FalseMsg(L"Error; error parsing simple closing tag", Str(L"/>")),
							(
								Char('>')
								+ ZeroOrMoreS(ElementParser(VectorOutput(temp.m_children)))
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
		ElementParserT<Toutput> ElementParser(const Toutput& output_)
		{
			return ElementParserT<Toutput>(output_);
		}
	}
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
	//{
	//	LibCC::QuickStringList<char> a;
	//	a.push_back("aoeusntahoesutnahoeusatoehusnatoheusnateohsanteohusantehusantehusanohusanotehusntoehusantehuasntoehusantoehusantoehu");
	//	{
	//		LibCC::QuickStringList<char> b(a);
	//	}
	//}

  LibCC::Timer t;
#ifdef _DEBUG
  const int MaxNum = 1;
#else
  const int MaxNum = 50;
#endif

	std::cout << std::endl << "LibCC::Parse benchmarks, " << MaxNum << " passes." << std::endl;

	//////////////////////////////////
	std::cout << std::endl << "Parse big typical file:" << std::endl;

	for(int overall = 0; overall < 1; ++ overall)
	{
		std::cout << std::endl;

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
			LibCC_746::Parse::ParseResultMem result;
			Element el;
			LibCC_746::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_746::Parse::Space() + LibCC_746::Parse::ElementParser(LibCC_746::Parse::RefOutput(el)) + LibCC_746::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "746 revision (optimized parserBase)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_746b::Parse::ParseResultMem result;
			Element el;
			LibCC_746b::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_746b::Parse::Space() + LibCC_746b::Parse::ElementParser(LibCC_746b::Parse::RefOutput(el)) + LibCC_746b::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "746b revision (QuickString passthrough)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_747::Parse::ParseResultMem result;
			Element el;
			LibCC_747::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_747::Parse::Space() + LibCC_747::Parse::ElementParser(LibCC_747::Parse::RefOutput(el)) + LibCC_747::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "747 revision (#define LIBCC_PARSE_TRACE_ENABLED)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_748::Parse::ParseResultMem result;
			Element el;
			LibCC_748::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_748::Parse::Space() + LibCC_748::Parse::ElementParser(LibCC_748::Parse::RefOutput(el)) + LibCC_748::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "748 revision (fewer output backups)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC::Parse::ParseResultMem result;
			//result.SetTraceEnabled(true);
			Element el;
			LibCC::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC::Parse::Space() + LibCC::Parse::ElementParser(LibCC::Parse::RefOutput(el)) + LibCC::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "HEAD revision");



		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC::Parse::ParseResultMem result;
			//result.SetTraceEnabled(true);
			Element el;
			LibCC::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC::Parse::Space() + LibCC::Parse::ElementParser(LibCC::Parse::RefOutput(el)) + LibCC::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "HEAD revision");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_748::Parse::ParseResultMem result;
			Element el;
			LibCC_748::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_748::Parse::Space() + LibCC_748::Parse::ElementParser(LibCC_748::Parse::RefOutput(el)) + LibCC_748::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "748 revision (fewer output backups)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_747::Parse::ParseResultMem result;
			Element el;
			LibCC_747::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_747::Parse::Space() + LibCC_747::Parse::ElementParser(LibCC_747::Parse::RefOutput(el)) + LibCC_747::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "747 revision (#define LIBCC_PARSE_TRACE_ENABLED)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_746b::Parse::ParseResultMem result;
			Element el;
			LibCC_746b::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_746b::Parse::Space() + LibCC_746b::Parse::ElementParser(LibCC_746b::Parse::RefOutput(el)) + LibCC_746b::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "746b revision (QuickString passthrough)");

		StartBenchmark(t);
		for(int n = 0; n < MaxNum; n ++)
		{
			LibCC_746::Parse::ParseResultMem result;
			Element el;
			LibCC_746::Parse::CScriptReader input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_746::Parse::Space() + LibCC_746::Parse::ElementParser(LibCC_746::Parse::RefOutput(el)) + LibCC_746::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "746 revision (optimized parserBase)");

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
			LibCC_743::Parse::ParseResultMem result;
			Element el;
			LibCC_743::Parse::CScriptReader740 input(LoadTextFileResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")));
			bool r = (*LibCC_743::Parse::Space() + ElementParser743(LibCC_743::Parse::RefOutput(el)) + LibCC_743::Parse::Eof()).ParseRetainingStateOnError(result, input);
			DoNotOptimize(el);
		}
		ReportBenchmark(t, "740 revision (original CScriptReader)");

	}

	return true;
}


