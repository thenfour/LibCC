

#include "test.h"
#include "libcc\timer.hpp"
#include "libcc\parse.hpp"


using namespace LibCC::Parse;


// holds all messages in memory.
		struct MyParseResult :
			ParseResult
		{
			int traceIndentLevel;
			bool traceEnabled;

			MyParseResult() :
				traceIndentLevel(0),
				traceEnabled(false)
			{
			}
			virtual void IncrementTraceIndent() { ++traceIndentLevel; }
			virtual void DecrementTraceIndent() { --traceIndentLevel; }
			virtual void SetTraceEnabled(bool b) { traceEnabled = b; }
			virtual bool IsTraceEnabled() const { return traceEnabled; }

			// parse debugging trace messages
			virtual void Trace(const std::wstring& msg)
			{
				std::wstring x;
				for(int i = 0; i < traceIndentLevel; ++ i)
				{
					x.append(L"  ");
				}
				x.append(msg);
				
				TestMessage(msg);
			}

			// parser evaluation messages
			virtual void ParserMessage(const std::wstring& msg)
			{
				TestMessage(msg);
			}
		};


struct Expression : public ParserWithOutput<int, Expression>
{
	Expression(const OutputPtr<int>& output_) { output.Assign(output_); }
	bool Parse(ParseResult& result, ScriptReader& input)
	{
		std::vector<int> operands;
		std::vector<wchar_t> operators;

		Parser operandParser = (Char('(') + Expression(VectorOutput(operands)) + Char(')')) | CInteger(VectorOutput(operands));
		Parser p = operandParser + *(CharOf(L"+-*/", VectorOutput(operators)) + operandParser);

		if(!p.ParseRetainingStateOnError(result, input))
			return false;

		std::vector<int>::iterator itOperand = operands.begin();
		std::vector<wchar_t>::iterator itOperator = operators.begin();
		int temp = *itOperand;
		while(itOperator != operators.end())
		{
			++ itOperand;
			switch(*itOperator)
			{
			case '+':
				temp += *itOperand;
				break;
			case '-':
				temp -= *itOperand;
				break;
			case '/':
				temp /= *itOperand;
				break;
			case '*':
				temp *= *itOperand;
				break;
			}
			++ itOperator;
		}
		output->Save(temp);
		return true;
	}
};

int ParseExpression(const std::wstring& exp)
{
	int ret = 0;
	Expression(RefOutput(ret)).ParseSimple(exp);
	return ret;
}


 struct AnyChar :
    public ParserWithOutput<wchar_t, AnyChar>
 {
    AnyChar() { output.Assign(NullOutput<wchar_t>().NewClone()); }
    AnyChar(const OutputPtr<wchar_t>& out) { output = out; }
 
    virtual bool Parse(ParseResult& result, ScriptReader& input)
    {
        if(input.IsEOF())
            return false;// could not parse a char FAIL
 
        wchar_t ch = input.CurrentChar();
        output->Save(ch);// store the character we parsed.
        input.Advance();// tell the script to point to the next character.
        return true;// parse success.
    }
 };

bool ParseTest()
{
	wchar_t ch = 0;
	std::wstring str;
	bool r = false;
	Parser p;
	wchar_t ch1, ch2;
	std::wstring str1, str2;

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

	p = Char(RefOutput(ch));
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

	p = CharI(RefOutput(ch));
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

	// CInteger
	// CInteger2
	// CSignedInteger
	// CUnsignedInteger
	// UnsignedIntegerParserT
	// UnsignedIntegerParser
	// UIntegerBin
	// UIntegerOct
	// UIntegerDec
	// UIntegerHex
	// SIntegerBin
	// SIntegerOct
	// SIntegerDec
	// SIntegerHex

	// UnsignedRationalParserT
	// SignedRationalParserT
	// Rational
	// Rational2

  return true;
}
