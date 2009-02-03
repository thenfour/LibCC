

#include "test.h"
#include "libcc\timer.hpp"
#include "libcc\parse.hpp"


using namespace LibCC::Parse;


bool ParseTest()
{
	wchar_t ch = 0;
	std::wstring str;
	bool r = false;
	Parser p;
	wchar_t ch1, ch2;
	std::wstring str1, str2;

	ParseResultMem resM;

	{
		resM.parseMessages.clear();
		bool r = FalseMsg(L"Error parsing the banana", ParseMsg(L"")).ParseRetainingStateOnError(resM, CScriptReader(L""));

		int a = 0;
	}

	p =
		(
			FalseMsg(L"Error parsing the banana", Str(L"banana"))
			| FalseMsg(L"Error parsing an accordion", Str(L"accordion"))
		);
	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"ba")));// parser will get 2 chars into banana, but only 1 char into accordion.
	TestAssert(resM.parseMessages.size() > 0);
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Error parsing the banana"));
	resM.parseMessages.clear();

	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"ac")));
	TestAssert(resM.parseMessages.size() > 0);
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Error parsing an accordion"));
	resM.parseMessages.clear();

	// the point is that I want to be able to see error messages here for a ZeroOrMore thing.
	p = ZeroOrMoreS
		(
			FalseMsg(L"Missing 'w'", CharI('w'))
			+ FalseMsg(L"Missing 't'", CharI('t'))
			+ FalseMsg(L"Missing 'f'", CharI('f'))
		) + (Eof() | ParseMsg(L"Expected: end of file."));

	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"w")));
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Missing 't'"));
	resM.parseMessages.clear();

	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"l")));
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Missing 'w'"));
	resM.parseMessages.clear();

	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"wtfwtx")));
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Missing 'f'"));
	resM.parseMessages.clear();

	p =
		FalseMsg(L"Missing '['", Char(L'['))
		+ ZeroOrMoreS
		(
			Not(Char(']')) +
			(
				FalseMsg(L"Failed to parse lol", StrI(L"lol"))
				| FalseMsg(L"Failed to parse wtf", StrI(L"wtf"))
			)
		)
		+ FalseMsg(L"Missing ']'", Char(L']'));

	TestAssert(p.ParseRetainingStateOnError(resM, CScriptReader(L"[]")));
	resM.parseMessages.clear();

	TestAssert(p.ParseRetainingStateOnError(resM, CScriptReader(L"[wtf]")));
	resM.parseMessages.clear();

	TestAssert(p.ParseRetainingStateOnError(resM, CScriptReader(L"[lol ]")));
	resM.parseMessages.clear();

	TestAssert(p.ParseRetainingStateOnError(resM, CScriptReader(L"[lol wtf]")));
	resM.parseMessages.clear();

	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"[l]")));
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Failed to parse lol"));
	resM.parseMessages.clear();

	TestAssert(!p.ParseRetainingStateOnError(resM, CScriptReader(L"[lol 8]")));
	if(resM.parseMessages.size() > 0)
		TestAssert(std::wstring::npos != resM.parseMessages[0].find(L"Failed to parse wtf"));// technically this is not defined. if this test fails well no biggie. just make sure the messages make sense.
	resM.parseMessages.clear();






	// basic tests to make sure it compiles correctly,
	// and to test basic parser operation.

	// CharOf
	p = CharOf(L"aB ", ch);
	TestAssert(p.ParseSimple(L"ax"));
	TestAssert(ch == 'a');
	TestAssert(p.ParseSimple(L"Bx"));
	TestAssert(ch == 'B');
	TestAssert(p.ParseSimple(L" x"));
	TestAssert(ch == ' ');
	TestAssert(!p.ParseSimple(L"x "));
	TestAssert(!p.ParseSimple(L"xB"));
	TestAssert(!p.ParseSimple(L"xa"));
	TestAssert(!p.ParseSimple(L""));

	p = CharOf(L"", ch);
	TestAssert(!p.ParseSimple(L""));

	p = CharOf(L"bA", RefOutput(ch));
	TestAssert(p.ParseSimple(L"bx"));
	TestAssert(ch == 'b');

	p = CharOf(L"bA");
	TestAssert(p.ParseSimple(L"bx"));
	TestAssert(!p.ParseSimple(L"x"));

	p = CharOf(L"x", str);
	TestAssert(p.ParseSimple(L"xxx"));
	TestAssert(str == L"x");

	// CharOfI
	p = CharOfI(L"a", ch);
	TestAssert(p.ParseSimple(L"Ax"));
	TestAssert(ch == 'A');
	TestAssert(p.ParseSimple(L"ax"));
	TestAssert(ch == 'a');
	TestAssert(!p.ParseSimple(L"x "));
	TestAssert(!p.ParseSimple(L""));

	p = CharOfI(L"", ch);
	TestAssert(!p.ParseSimple(L""));

	p = CharOfI(L"bA", RefOutput(ch));
	TestAssert(p.ParseSimple(L"Bx"));
	TestAssert(ch == 'B');

	p = CharOfI(L"BA");
	TestAssert(p.ParseSimple(L"bx"));
	TestAssert(!p.ParseSimple(L"x"));

	// Char
	p = Char();
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(!p.ParseSimple(L""));

	p = Char(L'x');
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(p.ParseSimple(L"xa"));
	TestAssert(!p.ParseSimple(L"ax"));
	TestAssert(!p.ParseSimple(L"X"));
	TestAssert(!p.ParseSimple(L""));

	p = AnyChar(RefOutput(ch));
	ch = 0;
	TestAssert(p.ParseSimple(L"x"));

	p = Char('x', RefOutput(ch));
	ch = 0;
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(ch == L'x');

	p = Char('x', ch);
	ch = 0;
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(ch == L'x');

	p = Char('x', str);
	str = L"abc";
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(str == L"abcx");

	// CharI
	p = CharI();
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(!p.ParseSimple(L""));

	p = CharI(L'x');
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(!p.ParseSimple(L"ax"));
	TestAssert(p.ParseSimple(L"X"));
	TestAssert(!p.ParseSimple(L""));

	p = AnyCharI(RefOutput(ch));
	ch = 0;
	TestAssert(p.ParseSimple(L"x"));

	p = CharI('x', RefOutput(ch));
	ch = 0;
	TestAssert(p.ParseSimple(L"X"));
	TestAssert(ch == L'X');

	p = CharI('X', ch);
	ch = 0;
	TestAssert(p.ParseSimple(L"x"));
	TestAssert(ch == L'x');

	p = CharI('X', str);
	str = L"abc";
	TestAssert(p.ParseSimple(L"X"));
	TestAssert(str == L"abcX");

	// Eof
	p = Eof();
	TestAssert(p.ParseSimple(L""));
	TestAssert(!p.ParseSimple(L"\n"));
	TestAssert(!p.ParseSimple(L"X"));

	// Eol
	p = Eol();
	TestAssert(p.ParseSimple(L""));// end of file is counted as a newline.
	TestAssert(!p.ParseSimple(L" \n"));
	TestAssert(p.ParseSimple(L"\n"));
	TestAssert(p.ParseSimple(L"\r"));
	TestAssert(p.ParseSimple(L"\r\n"));
	TestAssert(!p.ParseSimple(L"x"));

	// Space
	p = Space();
	TestAssert(!p.ParseSimple(L""));
	TestAssert(p.ParseSimple(L" \n"));
	TestAssert(p.ParseSimple(L"\n"));
	TestAssert(!p.ParseSimple(L"x"));

	// CharRange
	p = CharRange('b', 'b');
	TestAssert(!p.ParseSimple(L"a"));
	TestAssert(p.ParseSimple(L"b"));
	TestAssert(p.ParseSimple(L"ba"));
	TestAssert(!p.ParseSimple(L"ab"));
	TestAssert(!p.ParseSimple(L"B"));
	TestAssert(!p.ParseSimple(L"c"));

	p = CharRange('b', 'y', RefOutput(ch));
	TestAssert(!p.ParseSimple(L"a"));
	ch = 0;
	TestAssert(p.ParseSimple(L"b"));
	TestAssert(ch == 'b');
	ch = 0;
	TestAssert(p.ParseSimple(L"m"));
	TestAssert(ch == 'm');

	p = CharRange('a', 'z', ch);
	ch = 0;
	TestAssert(p.ParseSimple(L"b"));
	TestAssert(ch == 'b');

	p = CharRange('a', 'z', str);
	str = L"x";
	TestAssert(p.ParseSimple(L"bbbbb"));
	TestAssert(str == L"xb");

	// CharRangeI
	p = CharRangeI('B', 'B');
	TestAssert(!p.ParseSimple(L"a"));
	TestAssert(p.ParseSimple(L"b"));
	TestAssert(p.ParseSimple(L"ba"));
	TestAssert(!p.ParseSimple(L"ab"));
	TestAssert(p.ParseSimple(L"B"));
	TestAssert(!p.ParseSimple(L"c"));

	p = CharRangeI('B', 'y', RefOutput(ch));
	TestAssert(!p.ParseSimple(L"a"));
	ch = 0;
	TestAssert(p.ParseSimple(L"b"));
	TestAssert(ch == 'b');
	ch = 0;
	TestAssert(p.ParseSimple(L"m"));
	TestAssert(ch == 'm');

	p = CharRangeI('a', 'Z', ch);
	ch = 0;
	TestAssert(p.ParseSimple(L"b"));
	TestAssert(ch == 'b');

	p = CharRangeI('A', 'Z', str);
	str = L"x";
	TestAssert(p.ParseSimple(L"bbbbb"));
	TestAssert(str == L"xb");

	p = CharRangeI('a', 'z', str);
	str = L"x";
	TestAssert(p.ParseSimple(L"BBBBBB"));
	TestAssert(str == L"xB");

	// Str
	p = Str(L"");
	TestAssert(p.ParseSimple(L""));
	TestAssert(p.ParseSimple(L"1"));

	p = Str(L"abc", RefOutput(str));
	TestAssert(!p.ParseSimple(L"ab"));
	str.clear();
	TestAssert(p.ParseSimple(L"abc"));
	TestAssert(str == L"abc");
	TestAssert(!p.ParseSimple(L"Abc"));
	str.clear();
	TestAssert(p.ParseSimple(L"abcd"));
	TestAssert(str == L"abc");

	p = Str(L"abc", str);
	TestAssert(!p.ParseSimple(L"ab"));
	str.clear();
	TestAssert(p.ParseSimple(L"abc"));
	TestAssert(str == L"abc");
	str.clear();
	TestAssert(p.ParseSimple(L"abcd"));
	TestAssert(str == L"abc");

	// StrI
	p = StrI(L"");
	TestAssert(p.ParseSimple(L""));
	TestAssert(p.ParseSimple(L"1"));

	p = StrI(L"abc", RefOutput(str));
	TestAssert(!p.ParseSimple(L"ab"));
	str.clear();
	TestAssert(p.ParseSimple(L"abc"));
	TestAssert(str == L"abc");
	str.clear();
	TestAssert(p.ParseSimple(L"abcd"));
	TestAssert(str == L"abc");

	p = StrI(L"abcd", str);
	TestAssert(!p.ParseSimple(L"abc"));
	str.clear();
	TestAssert(p.ParseSimple(L"abcd"));
	TestAssert(str == L"abcd");
	str.clear();
	TestAssert(p.ParseSimple(L"AbCd"));
	TestAssert(str == L"AbCd");
	str.clear();
	TestAssert(p.ParseSimple(L"abcde"));
	TestAssert(str == L"abcd");

	p = StrI(L"AbCd", str);
	TestAssert(!p.ParseSimple(L"AbC"));
	str.clear();
	TestAssert(p.ParseSimple(L"abcd"));
	TestAssert(str == L"abcd");
	str.clear();
	TestAssert(p.ParseSimple(L"AbCd"));
	TestAssert(str == L"AbCd");
	str.clear();
	TestAssert(p.ParseSimple(L"abcde"));
	TestAssert(str == L"abcd");

	// Sequence
	p = Sequence<false>(Char('a', ch1), Char('b', ch2));
	TestAssert(!p.ParseSimple(L""));
	TestAssert(!p.ParseSimple(L"a"));
	TestAssert(!p.ParseSimple(L"b"));
	TestAssert(!p.ParseSimple(L"a b"));

	ch1 = ch2 = 0;
	TestAssert(p.ParseSimple(L"abc"));
	TestAssert(ch1 == 'a' && ch2 == 'b');

	p = Sequence<true>(Char('a', ch1), Char('b', ch2));
	TestAssert(!p.ParseSimple(L""));
	TestAssert(!p.ParseSimple(L"a"));
	TestAssert(!p.ParseSimple(L"b"));

	ch1 = ch2 = 0;
	TestAssert(p.ParseSimple(L"abc"));
	TestAssert(ch1 == 'a' && ch2 == 'b');

	ch1 = ch2 = 0;
	TestAssert(p.ParseSimple(L"a bc"));
	TestAssert(ch1 == 'a' && ch2 == 'b');

	// Or
	p = Or(Str(L"abcd", str1), Str(L"abcde", str2));
	TestAssert(!p.ParseSimple(L""));
	str1 = str2 = L"";
	TestAssert(p.ParseSimple(L"abcde"));
	TestAssert(str1 == L"abcd");// lhs gets priority.
	TestAssert(str2 == L"");

	// Optional
	p = Optional(Str(L"abcd", str1));
	TestAssert(p.ParseSimple(L""));
	str1 = L"";
	TestAssert(p.ParseSimple(L"abcde"));
	TestAssert(str1 == L"abcd");

	// Occurrences
	p = Occurrences<0, false>(CharI('a', str));
	TestAssert(p.ParseSimple(L""));
	str = L"";
	TestAssert(p.ParseSimple(L"a"));
	TestAssert(str == L"a");
	str = L"";
	TestAssert(p.ParseSimple(L" a"));
	TestAssert(str == L"");
	str = L"";
	TestAssert(p.ParseSimple(L"a a"));
	TestAssert(str == L"a");
	str = L"";
	TestAssert(p.ParseSimple(L"aaaAAAaaa"));
	TestAssert(str == L"aaaAAAaaa");

	p = Occurrences<0, true>(CharI('a', str));
	TestAssert(p.ParseSimple(L""));
	str = L"";
	TestAssert(p.ParseSimple(L"a"));
	TestAssert(str == L"a");
	str = L"";
	TestAssert(p.ParseSimple(L" a"));
	TestAssert(str == L"");
	str = L"";
	TestAssert(p.ParseSimple(L"a a"));
	TestAssert(str == L"aa");
	str = L"";
	TestAssert(p.ParseSimple(L"a a a A A A a a a"));
	TestAssert(str == L"aaaAAAaaa");

	// Not
	p = Not(Eof());
	TestAssert(p.ParseSimple(L"a"));
	TestAssert(!p.ParseSimple(L""));

	p = Sequence<false>(Not(Passthrough(CharOfI(L"ab12", ch))), Char(0, ch));
	ch = 0;
	TestAssert(p.ParseSimple(L"c"));
	TestAssert(ch == 'c');

	// retaining a var.
	p =
		ZeroOrMore
		(
			(Str(L"1", str1) + Char('.'))
			| (Str(L"2", str2) + Char('.'))
			| (Str(L"3", str2) + Char('.'))
			| (Str(L"4", str2) + Char('.'))
		);
	str1 = str2 = L"";
	p.ParseSimple(L"1.2.");
	TestAssert(str1 == L"1");
	TestAssert(str2 == L"2");
	str1 = str2 = L"";
	p.ParseSimple(L"2.1.");
	TestAssert(str1 == L"1");
	TestAssert(str2 == L"2");

	// StringParser
	p = StringParser(str);
	TestAssert(!p.ParseSimple(L""));
	
	// double quotes
	str = L"";
	TestAssert(p.ParseSimple(L"\"abc\""));
	TestAssert(str == L"abc");
	
	str = L"";
	TestAssert(p.ParseSimple(L"\"abc\"b"));
	TestAssert(str == L"abc");
	
	str = L"";
	TestAssert(p.ParseSimple(L"\"\""));
	TestAssert(str == L"");
	
	TestAssert(!p.ParseSimple(L"\"a\n\"b"));
	
	str = L"";
	TestAssert(p.ParseSimple(L"\"a'b\""));
	TestAssert(str == L"a'b");
	
	str = L"";
	TestAssert(p.ParseSimple(L"\"abc\\n\""));
	TestAssert(str == L"abc\n");
	
	TestAssert(!p.ParseSimple(L"\"a\\\""));

	// single quotes
	str = L"";
	TestAssert(p.ParseSimple(L"'abc'"));
	TestAssert(str == L"abc");
	
	str = L"";
	TestAssert(p.ParseSimple(L"'abc''b...."));
	TestAssert(str == L"abc");
	
	TestAssert(!p.ParseSimple(L"'a\n'"));
	
	str = L"";
	TestAssert(p.ParseSimple(L"'a\\n'"));
	TestAssert(str == L"a\n");
	
	TestAssert(!p.ParseSimple(L"'abc\""));
	TestAssert(!p.ParseSimple(L"\"abc'"));
	TestAssert(!p.ParseSimple(L"'a"));
	TestAssert(!p.ParseSimple(L"\"a"));

	// UnsignedIntegerParser
	unsigned int i;
	unsigned __int64 ui64 = 0;

	TestAssert(!UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L"-1"));
	TestAssert(!UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L""));
	TestAssert(!UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L" "));

	// base 2
	i = 1;
	TestAssert(UnsignedIntegerParser<unsigned int>(2, RefOutput(i)).ParseSimple(L"0"));
	TestAssert(i == 0);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(2, RefOutput(i)).ParseSimple(L"1"));
	TestAssert(i == 1);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(2, RefOutput(i)).ParseSimple(L"11"));
	TestAssert(i == 3);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(2, RefOutput(ui64)).ParseSimple(L"101010101010"));
	TestAssert(ui64 == 2730);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(2, RefOutput(ui64)).ParseSimple(L"1111111111111111111111111111111111111111111111111111111111111111"));
	TestAssert(ui64 == 18446744073709551615);

	TestAssert(!UnsignedIntegerParser<unsigned int>(2, RefOutput(i)).ParseSimple(L"2"));

	// base 8
	i = 1;
	TestAssert(UnsignedIntegerParser<unsigned int>(8, RefOutput(i)).ParseSimple(L"0"));
	TestAssert(i == 0);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(8, RefOutput(i)).ParseSimple(L"1"));
	TestAssert(i == 1);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(8, RefOutput(i)).ParseSimple(L"77"));
	TestAssert(i == 63);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(8, RefOutput(ui64)).ParseSimple(L"1234567"));
	TestAssert(ui64 == 01234567);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(8, RefOutput(ui64)).ParseSimple(L"1777777777777777777777"));
	TestAssert(ui64 == 18446744073709551615);

	TestAssert(!UnsignedIntegerParser<unsigned int>(8, RefOutput(i)).ParseSimple(L"8"));

	// base 10
	i = 1;
	TestAssert(UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L"0"));
	TestAssert(i == 0);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L"1"));
	TestAssert(i == 1);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L"99"));
	TestAssert(i == 99);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(10, RefOutput(ui64)).ParseSimple(L"123456789"));
	TestAssert(ui64 == 123456789);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(10, RefOutput(ui64)).ParseSimple(L"18446744073709551615"));
	TestAssert(ui64 == 18446744073709551615);

	TestAssert(!UnsignedIntegerParser<unsigned int>(10, RefOutput(i)).ParseSimple(L"a"));

	// base 16
	i = 1;
	TestAssert(UnsignedIntegerParser<unsigned int>(16, RefOutput(i)).ParseSimple(L"0"));
	TestAssert(i == 0);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(16, RefOutput(i)).ParseSimple(L"f"));
	TestAssert(i == 15);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(16, RefOutput(i)).ParseSimple(L"F"));
	TestAssert(i == 15);

	i = 0;
	TestAssert(UnsignedIntegerParser<unsigned int>(16, RefOutput(i)).ParseSimple(L"FF"));
	TestAssert(i == 255);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(16, RefOutput(ui64)).ParseSimple(L"123456789ABCDEF"));
	TestAssert(ui64 == 81985529216486895);

	ui64 = 0;
	TestAssert(UnsignedIntegerParser<unsigned __int64>(16, RefOutput(ui64)).ParseSimple(L"ffffFFFFffffFFFF"));
	TestAssert(ui64 == 18446744073709551615);

	TestAssert(!UnsignedIntegerParser<unsigned int>(16, RefOutput(i)).ParseSimple(L"g"));

	// UIntegerBin
	TestAssert(!UIntegerBin<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"101010101010"));
	TestAssert(!UIntegerBin<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"-101b"));

	ui64 = 1;
	TestAssert(UIntegerBin<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"0b"));
	TestAssert(ui64 == 0);

	ui64 = 0;
	TestAssert(UIntegerBin<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"101010101010b"));
	TestAssert(ui64 == 2730);

	// UIntegerOct
	TestAssert(!UIntegerOct<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"1234567"));
	TestAssert(!UIntegerOct<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"0x1234567"));
	TestAssert(!UIntegerOct<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"-01234567"));

	ui64 = 1;
	TestAssert(UIntegerOct<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"00"));
	TestAssert(ui64 == 0);

	ui64 = 0;
	TestAssert(UIntegerOct<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"01234567"));
	TestAssert(ui64 == 01234567);

	// UIntegerDec
	TestAssert(!UIntegerDec<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"-1234"));

	ui64 = 1;
	TestAssert(UIntegerDec<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"0"));
	TestAssert(ui64 == 0);

	ui64 = 0;
	TestAssert(UIntegerDec<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"01234567890"));
	TestAssert(ui64 == 1234567890);

	// UIntegerHex
	TestAssert(!UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"-0xabcd"));
	TestAssert(!UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"0"));
	TestAssert(!UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"1234567"));
	TestAssert(!UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"01234567"));
	TestAssert(!UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"1234567b"));

	ui64 = 1;
	TestAssert(UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"0x0"));
	TestAssert(ui64 == 0);

	ui64 = 0;
	TestAssert(UIntegerHex<unsigned __int64>(RefOutput(ui64)).ParseSimple(L"0x0123456789abcdEF"));
	TestAssert(ui64 == 0x0123456789abcdef);


	// SIntegerBin
	__int64 si64;

	TestAssert(!SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"-"));
	TestAssert(!SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"101010101010"));

	si64 = 1;
	TestAssert(SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"0b"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"101010101010b"));
	TestAssert(si64 == 2730);

	si64 = 1;
	TestAssert(SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"-0b"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"-101010101010b"));
	TestAssert(si64 == -2730);

	si64 = 0;
	TestAssert(SIntegerBin<__int64>(RefOutput(si64)).ParseSimple(L"+101010101010b"));
	TestAssert(si64 == 2730);

	// SIntegerOct
	TestAssert(!SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"-"));
	TestAssert(!SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"0"));
	TestAssert(!SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"1234567"));
	TestAssert(!SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"0x1234567"));

	si64 = 1;
	TestAssert(SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"00"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"01234567"));
	TestAssert(si64 == 01234567);

	si64 = 1;
	TestAssert(SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"-00"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"-01234567"));
	TestAssert(si64 == -01234567);

	si64 = 0;
	TestAssert(SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"+01234567"));
	TestAssert(si64 == 01234567);

	// SIntegerDec
	TestAssert(!SIntegerOct<__int64>(RefOutput(si64)).ParseSimple(L"-"));

	si64 = 1;
	TestAssert(SIntegerDec<__int64>(RefOutput(si64)).ParseSimple(L"0"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerDec<__int64>(RefOutput(si64)).ParseSimple(L"01234567890"));
	TestAssert(si64 == 1234567890);

	si64 = 0;
	TestAssert(SIntegerDec<__int64>(RefOutput(si64)).ParseSimple(L"-01234567890"));
	TestAssert(si64 == -1234567890);

	si64 = 0;
	TestAssert(SIntegerDec<__int64>(RefOutput(si64)).ParseSimple(L"+01234567890"));
	TestAssert(si64 == 1234567890);

	// SIntegerHex
	TestAssert(!SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"-"));
	TestAssert(!SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"0"));
	TestAssert(!SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"0x"));
	TestAssert(!SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"1234567"));
	TestAssert(!SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"01234567"));
	TestAssert(!SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"1234567b"));

	si64 = 1;
	TestAssert(SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"0x0"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"0x0123456789abcdef"));
	TestAssert(si64 == 0x0123456789abcdef);

	si64 = 1;
	TestAssert(SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"-0x0"));
	TestAssert(si64 == 0);

	si64 = 0;
	TestAssert(SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"-0x0123456789abcdef"));
	TestAssert(si64 == -0x0123456789abcdef);

	si64 = 0;
	TestAssert(SIntegerHex<__int64>(RefOutput(si64)).ParseSimple(L"+0x0123456789abcdef"));
	TestAssert(si64 == 0x0123456789abcdef);

	// CUnsignedInteger
	si64 = 0;
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L""));

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"012b"));// this will work; octal will fail; binary will fail, and decimal will pick it up.
	TestAssert(si64 == 12);

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"9a"));// this will work; parsing stops at a after base 10 is deduced
	TestAssert(si64 == 9);

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"078"));// this will work; parsing stops at 8 after octal is deduced.
	TestAssert(si64 == 07);

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"0x123g"));// this will work; parsing stops at g
	TestAssert(si64 == 0x123);

	si64 = 1;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"0"));
	TestAssert(si64 == 0);

	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+0"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-0"));

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"011b"));
	TestAssert(si64 == 3);

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"013"));
	TestAssert(si64 == 11);

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"13"));
	TestAssert(si64 == 13);

	si64 = 0;
	TestAssert(CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"0xafff"));
	TestAssert(si64 == 0xafff);

	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+011b"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+013"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+13"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+0xafff"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-011b"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-013"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-13"));
	TestAssert(!CUnsignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-0xafff"));


	// CSignedInteger
	si64 = 0;
	TestAssert(!CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L""));

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"012b"));// this will work; octal will fail; binary will fail, and decimal will pick it up.
	TestAssert(si64 == 12);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"9a"));// this will work; parsing stops at a after base 10 is deduced
	TestAssert(si64 == 9);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"078"));// this will work; parsing stops at 8 after octal is deduced.
	TestAssert(si64 == 07);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"0x123g"));// this will work; parsing stops at g
	TestAssert(si64 == 0x123);

	si64 = 1;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"0"));
	TestAssert(si64 == 0);

	si64 = 1;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+0"));
	TestAssert(si64 == 0);

	si64 = 1;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-0"));
	TestAssert(si64 == 0);


	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"011b"));
	TestAssert(si64 == 3);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"013"));
	TestAssert(si64 == 11);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"13"));
	TestAssert(si64 == 13);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"0xafff"));
	TestAssert(si64 == 0xafff);


	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+011b"));
	TestAssert(si64 == 3);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+013"));
	TestAssert(si64 == 11);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+13"));
	TestAssert(si64 == 13);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"+0xafff"));
	TestAssert(si64 == 0xafff);


	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-011b"));
	TestAssert(si64 == -3);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-013"));
	TestAssert(si64 == -11);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-13"));
	TestAssert(si64 == -13);

	si64 = 0;
	TestAssert(CSignedInteger<__int64>(RefOutput(si64)).ParseSimple(L"-0xafff"));
	TestAssert(si64 == -0xafff);


	// CInteger
	// CInteger2

	// UnsignedRationalParserT
	// SignedRationalParserT
	// Rational
	// Rational2

	// output

	// debugging & error messaging
  return true;
}
