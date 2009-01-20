

#include "test.h"
#include "libcc\parse.hpp"


using namespace LibCC::Parse;

struct Attribute
{
	std::wstring lhs;
	std::wstring rhs;
};

struct Element
{
	std::wstring tagName;
	std::vector<Attribute> attributes;
};

struct ElementList
{
	std::vector<Element> elements;
};

//struct AttributeParser :
//	public ParserWithOutput<std::vector<Attribute>, AttributeParser>
//{
//	AttributeParser(std::vector<Attribute>& output) : ParserWithOutput<std::vector<Attribute>, AttributeParser>(output) { }
//	AttributeParser(AttributeParser& rhs) : ParserWithOutput<std::vector<Attribute>, AttributeParser>(rhs.output) { }
//	virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"AttributeParser\r\n"; }
//
//	virtual bool Parse(ScriptResult& result, ScriptReader& input)
//	{
//		output.push_back(Attribute());
//		//Parser attributeWithoutEquals = ;
//		//Parser attributeWithEquals = ;
//		return true;
//	}
//};
//
//struct TagParser : public ParserWithOutput<Element, TagParser>
//{
//	TagParser(Element& output) : ParserWithOutput<Element, TagParser>(output) { }
//	TagParser(TagParser& rhs) : ParserWithOutput<Element, TagParser>(rhs.output) { }
//	virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"TagParser\r\n"; }
//
//	virtual bool Parse(ScriptResult& result, ScriptReader& input)
//	{
//		return true;
//	}
//};
//
//struct ElementParser : public ParserWithOutput<ElementList, ElementParser>
//{
//	ElementParser(ElementList& output) : ParserWithOutput<ElementList, ElementParser>(output) { }
//	ElementParser(ElementParser& rhs) : ParserWithOutput<ElementList, ElementParser>(rhs.output) { }
//	virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"ElementParser\r\n"; }
//
//	virtual bool Parse(ScriptResult& result, ScriptReader& input)
//	{
//		output.elements.push_back(Element());
//		Parser p =
//			Char('<')
//			+ TagParser(output.elements.back())
//			+ (*(+Space() + AttributeParser(output.elements.back().attributes)))
//			+ *Space()
//			+ Str(L"/>");
//		return p.ParseRetainingStateOnError(result, input);
//	}
//};
//

bool ParseTest()
{
	//ScriptReader reader(L" <hi/> <lol /> <omg a b/>");
	//ScriptResult result;
	//ElementList els;
	//Parser p = *Space() + (*(ElementParser(els) + *Space()));
	//std::wstring d = p.Dump(0);
	//std::wcout << d;
	//p.ParseRetainingStateOnError(result, reader);
	//TestAssert(els.elements.size() == 3);
	//if(els.elements.size() > 0)
	//{
	//	TestAssert(els.elements[0].tagName == L"hi");
	//	TestAssert(els.elements[0].attributes.size() == 0);
	//}
	//if(els.elements.size() > 1)
	//{
	//	TestAssert(els.elements[1].tagName == L"lol");
	//	TestAssert(els.elements[1].attributes.size() == 0);
	//}
	//if(els.elements.size() > 2)
	//{
	//	TestAssert(els.elements[2].tagName == L"omg");
	//	TestAssert(els.elements[2].attributes.size() == 2);
	//}




	//ScriptReader reader(L"    g   \'re\' \"at O'connor\" y e a h!");
	//ScriptResult result;
	//ElementList els;
	//std::wstring output;
	//Parser p = *(*Space() + StringParser(output) + *Space());
	//p.ParseRetainingStateOnError(result, reader);



	CScriptReader reader(L"54 0x36 066 110110b   -54 -0x36 -066 -110110b   ");
	//CScriptReader reader(L"1111b 0xff 0777 -15");
	ScriptResult result;
	std::vector<int> r;
	(*(CInteger(VectorOutput(r)) + *Space())).ParseRetainingStateOnError(result, reader);



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

	int i;
	CInteger(i).ParseSimple(L"0x68");

  return true;
}
