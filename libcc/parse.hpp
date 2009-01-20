/*
  LibCC
  Parse Module
  (c) 2009 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC
	Official source code: http://svn.winprog.org/personal/carl/LibCC

	Original version: Jan 19, 2009

	== License:

  All software on this site is provided 'as-is', without any express or
  implied warranty, by its respective authors and owners. In no event will
  the authors be held liable for any damages arising from the use of this
  software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in
  a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/
/*

TODO:
- properly handle overflow on numbers?
- floating point number parser
- better diagnostic stuff (output a sample input string or EBNF string, set breakpoints in the parse process)
- other ScriptReaders that:
	 - could work on binary, not just text?
	 - provide more options and don't force you to use C++-style comments

OPERATORS
*          zero or more
+ (unary)  one or more
!          not (never advances cursor)
-          zero or one
+          sequence
&&         sequence (AND)
||         logical OR

*/

#pragma once

#include "stringutil.hpp"
#include <vector>


namespace LibCC
{
	namespace Parse
	{

		class ScriptCursor
		{
		public:
			ScriptCursor() :
				line(1),
				column(1),
				pos(0)
			{
			}

			int line;
			int column;
			int pos;
		};

		// feeds a string into the parser, and converts all newlines into \n
		class ScriptReader
		{
		public:
			ScriptReader()
			{
			}

			ScriptReader(const std::wstring& s) :
				m_script(s)
			{
				AdvancePastComments();
			}

			wchar_t CurrentChar() const
			{
				return m_script[m_cursor.pos];
			}

			void Advance()
			{
				InternalAdvance();
				AdvancePastComments();
			}

			bool IsEOF() const
			{
				return m_cursor.pos >= (int)m_script.size();
			}

			int GetLine() const
			{
				return m_cursor.line;
			}

			int GetColumn() const
			{
				return m_cursor.column;
			}

			ScriptCursor GetCursorCopy() const
			{
				return m_cursor;
			}

			void SetCursor(const ScriptCursor& c)
			{
				m_cursor = c;
			}

		private:
			bool CursorStartsWith(const std::wstring& s)
			{
				return m_cursor.pos == m_script.find(s, m_cursor.pos);
			}
			void AdvancePastComments()
			{
				for(;;)
				{
					if(CursorStartsWith(L"//"))
					{
						// advance until a newline.
						int oldLine = m_cursor.line;
						while((!IsEOF()) && (m_cursor.line == oldLine))
						{
							InternalAdvance();
						}
					}
					else if(CursorStartsWith(L"/*"))
					{
						while(!CursorStartsWith(L"*/") && !IsEOF())
						{
							InternalAdvance();
						}
						// go past the terminator
						InternalAdvance();
						InternalAdvance();
					}
					else
					{
						break;
					}
				}
			}

			void InternalAdvance()
			{
				if(IsEOF())
					return;

				if(CurrentChar() == L'\n')
				{
					m_cursor.line ++;
					m_cursor.column = 1;
					m_cursor.pos ++;

					if(CurrentChar() == L'\r')// skip a \r character following \n.
					{
						m_cursor.pos ++;
					}
				}
				else if(CurrentChar() == L'\r')
				{
					m_cursor.line ++;
					m_cursor.column = 1;
					m_cursor.pos ++;

					if(CurrentChar() == L'\n')// skip a \n character following \r.
					{
						m_cursor.pos ++;
					}
				}
				else
				{
					m_cursor.column ++;
					m_cursor.pos ++;
				}
			}

			ScriptCursor m_cursor;
			std::wstring m_script;
		};

		struct ScriptResult
		{
			std::vector<std::wstring> messages;
			void Message(const std::wstring& msg)
			{
				messages.push_back(msg);
			}
		};

		struct ParserBase
		{
			bool ParseRetainingStateOnError(ScriptResult& result, ScriptReader& input)
			{
				ScriptCursor oldCursor = input.GetCursorCopy();
				SaveOutputState();
				bool ret = Parse(result, input);
				if(!ret)
				{
					RestoreOutputState();
					input.SetCursor(oldCursor);
					return false;
				}
				return true;
			}

			virtual ParserBase* NewClone() const = 0;

			virtual void SaveOutputState() = 0;
			virtual void RestoreOutputState() = 0;

			virtual bool Parse(ScriptResult& result, ScriptReader& input) = 0;

			virtual std::wstring Dump(int indentLevel)
			{
				return std::wstring(indentLevel, ' ') + L"(unnamed)\r\n";
			}
		};

		struct ParserPtr
		{
			struct ParserBase* p;

			ParserPtr() : p(0)
			{
			}
			ParserPtr(const ParserPtr& rhs) : p(0)
			{
				Assign(rhs);
			}
			explicit ParserPtr(ParserBase* rhs) : p(0)
			{
				Assign(rhs);
			}

			~ParserPtr()
			{
				Release();
			}

			bool IsEmpty() const { return p == 0; }
			bool IsValid() const { return p != 0; }

			void Assign(const ParserPtr& rhs)
			{
				Release();
				if(rhs.IsValid())
					p = rhs.p->NewClone();// copying creates a clone.
			}

			void Assign(ParserBase* rhs)
			{
				Release();
				p = rhs;
			}

			void Release()
			{
				delete p;
				p = 0;
			}

			ParserBase* operator ->()
			{
				return p;
			}

			const ParserBase* operator ->() const
			{
				return p;
			}
		};


		struct Passthrough : public ParserBase
		{
		private:
			ParserPtr m_child;

		public:
			Passthrough()
			{
			}

			Passthrough(const Passthrough& rhs)
			{
				if(rhs.m_child.IsEmpty())
					return;
				m_child.Assign(rhs.m_child->NewClone());
			}

			Passthrough(const ParserBase& rhs)
			{
				m_child.Assign(rhs.NewClone());
			}

			virtual ParserBase* NewClone() const { return new Passthrough(*this); }

			virtual void SaveOutputState()
			{
				if(m_child.IsEmpty())
					return;
				m_child->SaveOutputState();
			}

			virtual void RestoreOutputState()
			{
				if(m_child.IsEmpty())
					return;
				m_child->RestoreOutputState();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(m_child.IsEmpty())
					return true;
				return m_child->ParseRetainingStateOnError(result, input);
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Passthrough\r\n";
				ret += std::wstring(indentLevel, ' ') + L"{\r\n";
				if(m_child.IsValid())
					ret += m_child->Dump(indentLevel + 1);
				ret += std::wstring(indentLevel, ' ') + L"}\r\n";
				return ret;
			}
		};

		typedef Passthrough Parser;

		template<int MinimumOccurrences>
		struct Occurrences : public ParserBase
		{
		private:
			ParserPtr m_child;

		public:
			Occurrences()
			{
			}

			Occurrences(const Occurrences<MinimumOccurrences>& rhs)
			{
				if(rhs.m_child.IsEmpty())
					return;
				m_child.Assign(rhs.m_child->NewClone());
			}

			Occurrences(const ParserBase& rhs)
			{
				m_child.Assign(rhs.NewClone());
			}

			virtual ParserBase* NewClone() const { return new Occurrences<MinimumOccurrences>(*this); }

			virtual void SaveOutputState() { if(m_child.IsValid()) m_child->SaveOutputState(); }
			virtual void RestoreOutputState() { if(m_child.IsValid()) m_child->RestoreOutputState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(m_child.IsEmpty())
					return true;
				
				int count = 0;
				
				while(true)
				{
					int oldCursor = input.GetCursorCopy().pos;
					if(!m_child->ParseRetainingStateOnError(result, input))
					{
						if(count < MinimumOccurrences)
						{
							//result.Message(L"Not enough occurrences.");
							return false;
						}
						else
						{
							return true;
						}
					}
					count ++;
					// if it succeeded but the cursor is in the same place, then we're headed for an endless loop.
					if(oldCursor == input.GetCursorCopy().pos)
					{
						__asm int 3;// TODO: some kind of cool error handling.
					}
				}
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Occurrences<>\r\n";
				ret += std::wstring(indentLevel, ' ') + L"{\r\n";
				if(m_child.IsValid())
					ret += m_child->Dump(indentLevel + 1);
				ret += std::wstring(indentLevel, ' ') + L"}\r\n";
				return ret;
			}
		};

		typedef Occurrences<0> ZeroOrMore;
		typedef Occurrences<1> OneOrMore;

		ZeroOrMore operator * (const ParserBase& lhs)
		{
			return ZeroOrMore(lhs);
		}

		OneOrMore operator + (const ParserBase& lhs)
		{
			return OneOrMore(lhs);
		}

		struct Not : public ParserBase
		{
		private:
			ParserPtr m_child;

		public:
			Not()
			{
			}

			Not(const Not& rhs)
			{
				if(rhs.m_child.IsEmpty())
					return;
				m_child.Assign(rhs.m_child->NewClone());
			}

			Not(const ParserBase& rhs)
			{
				m_child.Assign(rhs.NewClone());
			}

			virtual ParserBase* NewClone() const { return new Not(*this); }

			virtual void SaveOutputState() { if(m_child.IsValid()) m_child->SaveOutputState(); }
			virtual void RestoreOutputState() { if(m_child.IsValid()) m_child->RestoreOutputState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(m_child.IsEmpty())
					return true;

				bool r = m_child->ParseRetainingStateOnError(result, input);
				return !r;// <-- this is the NOT.
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Not\r\n";
				ret += std::wstring(indentLevel, ' ') + L"{\r\n";
				if(m_child.IsValid())
					ret += m_child->Dump(indentLevel + 1);
				ret += std::wstring(indentLevel, ' ') + L"}\r\n";
				return ret;
			}
		};

		Not operator ! (const ParserBase& lhs)
		{
			return Not(lhs);
		}

		struct Optional : public ParserBase
		{
		private:
			ParserPtr m_child;

		public:
			Optional()
			{
			}

			Optional(const Optional& rhs)
			{
				if(rhs.m_child.IsEmpty())
					return;
				m_child.Assign(rhs.m_child->NewClone());
			}

			Optional(const ParserBase& rhs)
			{
				m_child.Assign(rhs.NewClone());
			}

			virtual ParserBase* NewClone() const { return new Optional(*this); }

			virtual void SaveOutputState() { if(m_child.IsValid()) m_child->SaveOutputState(); }
			virtual void RestoreOutputState() { if(m_child.IsValid()) m_child->RestoreOutputState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(m_child.IsEmpty())
					return true;

				m_child->ParseRetainingStateOnError(result, input);// ignore result because we succeed whether it was there or not. state is automatically retained.
				return true;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Optional\r\n";
				ret += std::wstring(indentLevel, ' ') + L"{\r\n";
				if(m_child.IsValid())
					ret += m_child->Dump(indentLevel + 1);
				ret += std::wstring(indentLevel, ' ') + L"}\r\n";
				return ret;
			}
		};

		Optional operator - (const ParserBase& lhs)
		{
			return Optional(lhs);
		}

		template<bool TCaseSensitive>
		struct CharT : public ParserBase
		{
			wchar_t match;

			wchar_t chDummy;
			wchar_t& chOutput;
			wchar_t chOutputBackup;

			std::wstring strDummy;
			std::wstring& strOutput;
			std::wstring strOutputBackup;

			CharT() :
				match(0),
				chOutput(chDummy),
				strOutput(strDummy)
			{
			}

			CharT(wchar_t match_) :
				match(match_),
				chOutput(chDummy),
				strOutput(strDummy)
			{
			}

			CharT(wchar_t match_, wchar_t& out) :
				match(match_),
				chOutput(out),
				strOutput(strDummy)
			{
			}

			CharT(wchar_t match_, std::wstring& out) :
				match(match_),
				chOutput(chDummy),
				strOutput(out)
			{
			}

			virtual ParserBase* NewClone() const { return new CharT<TCaseSensitive>(*this); }

			virtual void SaveOutputState()
			{
				chOutputBackup = chOutput;
				strOutputBackup = strOutput;
			}
			virtual void RestoreOutputState()
			{
				chOutput = chOutputBackup;
				strOutput = strOutputBackup;
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
				{
					//result.Message(L"Unexpected end of file searching for character ...");
					return false;
				}

				wchar_t ch = input.CurrentChar();
				chOutput = ch;
				strOutput.push_back(ch);
				input.Advance();

				if(match == 0)
				{
					return true;
				}

				if(match == ch)
					return true;

				if(!TCaseSensitive)
				{
					if(CharToLower(ch) == CharToLower(match))
						return true;
				}
				//result.Message(L"Unexpected character");
				return false;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				if(match == 0)
					ret += std::wstring(indentLevel, ' ') + L"Char(any)\r\n";
				else
					ret += std::wstring(indentLevel, ' ') + std::wstring(L"Char('") + std::wstring(1, match) + std::wstring(L"')\r\n");
				return ret;
			}
		};

		typedef CharT<true> Char;
		typedef CharT<false> CharI;

		template<bool TCaseSensitive>
		struct StrT : public ParserBase
		{
			std::wstring dummy;
			std::wstring& output;
			std::wstring outputBackup;
			std::wstring match;

			StrT(const std::wstring& match, std::wstring& out) :
				output(out),
				match(match)
			{
			}

			StrT() :
				output(dummy)
			{
			}

			StrT(const std::wstring& match) :
				output(dummy),
				match(match)
			{
			}

			virtual ParserBase* NewClone() const { return new StrT<TCaseSensitive>(*this); }
			virtual void SaveOutputState() { outputBackup = output; }
			virtual void RestoreOutputState() { output = outputBackup; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
					return match.empty();

				for(std::wstring::const_iterator it = match.begin(); it != match.end(); ++ it)
				{
					if(input.IsEOF())
					{
						//result.Message(L"Unexpected end of file searching for string ...");
						return false;
					}

					wchar_t ch = input.CurrentChar();
					input.Advance();

					if(TCaseSensitive && (ch != *it))
					{
						//result.Message(L"Unexpected character searching for string ...");
						return false;
					}
					if(!TCaseSensitive && (CharToLower(ch) == CharToLower(*it)))
					{
						return false;
					}
				}

				output = match;
				return true;// made it all the way
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Str(" + match + L")\r\n";
				return ret;
			}
		};

		typedef StrT<true> Str;
		typedef StrT<false> StrI;

		struct Sequence : public ParserBase
		{
			ParserPtr lhs;
			ParserPtr rhs;

			Sequence(const ParserBase& _lhs, const ParserBase& _rhs)
			{
				lhs.Assign(_lhs.NewClone());
				rhs.Assign(_rhs.NewClone());
			}

			virtual ParserBase* NewClone() const { return new Sequence(*this); }

			virtual void SaveOutputState()
			{
				lhs->SaveOutputState();
				rhs->SaveOutputState();
			}

			virtual void RestoreOutputState()
			{
				lhs->RestoreOutputState();
				rhs->RestoreOutputState();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(!lhs->ParseRetainingStateOnError(result, input))
					return false;
				if(!rhs->ParseRetainingStateOnError(result, input))
					return false;
				return true;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Sequence\r\n";
				ret += std::wstring(indentLevel, ' ') + L"{\r\n";
				if(lhs.IsValid())
					ret += lhs->Dump(indentLevel + 1);
				if(rhs.IsValid())
					ret += rhs->Dump(indentLevel + 1);
				ret += std::wstring(indentLevel, ' ') + L"}\r\n";
				return ret;
			}
		};

		Sequence operator + (const ParserBase& lhs, const ParserBase& rhs)
		{
			return Sequence(lhs, rhs);
		}

		// same as operator+(), but in certain contexts this mkes more sense. like (!Char('}') && CharRange('etc'))
		Sequence operator && (const ParserBase& lhs, const ParserBase& rhs)
		{
			return Sequence(lhs, rhs);
		}

		struct Or : public ParserBase
		{
			ParserPtr lhs;
			ParserPtr rhs;

			Or(const ParserBase& _lhs, const ParserBase& _rhs)
			{
				lhs.Assign(_lhs.NewClone());
				rhs.Assign(_rhs.NewClone());
			}

			virtual ParserBase* NewClone() const { return new Or(*this); }

			virtual void SaveOutputState()
			{
				lhs->SaveOutputState();
				rhs->SaveOutputState();
			}

			virtual void RestoreOutputState()
			{
				lhs->RestoreOutputState();
				rhs->RestoreOutputState();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(lhs->ParseRetainingStateOnError(result, input))
					return true;
				if(rhs->ParseRetainingStateOnError(result, input))
					return true;
				return false;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Or\r\n";
				ret += std::wstring(indentLevel, ' ') + L"{\r\n";
				if(lhs.IsValid())
					ret += lhs->Dump(indentLevel + 1);
				if(rhs.IsValid())
					ret += rhs->Dump(indentLevel + 1);
				ret += std::wstring(indentLevel, ' ') + L"}\r\n";
				return ret;
			}
		};

		Or operator || (const ParserBase& lhs, const ParserBase& rhs)
		{
			return Or(lhs, rhs);
		}

		struct Eof : public ParserBase
		{
			Eof()
			{
			}

			virtual ParserBase* NewClone() const { return new Eof(); }

			virtual void SaveOutputState() { }
			virtual void RestoreOutputState() { }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return input.IsEOF();
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Eof\r\n";
				return ret;
			}
		};

		// implements some basic stuff so that custom parsers are less typing.
		// the derived class needs to have 2 constructors:
		// 1) Derived(OutputArg& output) : ParserWithoutput<OutputArg, Derived>(output) { }
		// 1) Derived(const Derived& rhs) : ParserWithoutput<OutputArg, Derived>(rhs.output) { }
		// then just implement Parse().
		template<typename OutputArgT, typename DerivedT>
		struct ParserWithOutput : public ParserBase
		{
			ParserWithOutput(OutputArgT& arg) : output(arg) { }
			OutputArgT& output;
			OutputArgT outputBackup;
			virtual ParserBase* NewClone() const
			{
				return new DerivedT(*((DerivedT*)(this)));// create a new derived class using copy ctor
			}
			virtual void SaveOutputState()
			{
				outputBackup = output;
			}
			virtual void RestoreOutputState()
			{
				output = outputBackup;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"ParserWithOutput(generic)\r\n";
				return ret;
			}
		};

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// matches exactly 1 whitespace char
		struct Space : public ParserBase
		{
			std::wstring chars;

			Space()
			{
				chars = L"\r\n \t";
				chars.push_back(0x0085);// NEL (control character next line)
				chars.push_back(0x00A0);// NBSP (NO-BREAK SPACE)
				chars.push_back(0x1680);// OGHAM SPACE MARK
				chars.push_back(0x180E);// MONGOLIAN VOWEL SEPARATOR
				chars.push_back(0x2000);// EN QUAD
				chars.push_back(0x2001);// EM QUAD
				chars.push_back(0x2002);// EN SPACE
				chars.push_back(0x2003);// EM SPACE
				chars.push_back(0x2004);// THREE-PER-EM SPACE
				chars.push_back(0x2005);// FOUR-PER-EM SPACE
				chars.push_back(0x2006);// SIX-PER-EM SPACE
				chars.push_back(0x2007);// FIGURE SPACE
				chars.push_back(0x2008);// PUNCTUATION SPACE
				chars.push_back(0x2009);// THIN SPACE
				chars.push_back(0x200A);// HAIR SPACE
				chars.push_back(0x2028);// LS (LINE SEPARATOR)
				chars.push_back(0x2029);// PS (PARAGRAPH SEPARATOR)
				chars.push_back(0x202F);// NNBSP (NARROW NO-BREAK SPACE)
				chars.push_back(0x205F);// MMSP (MEDIUM MATHEMATICAL SPACE)
				chars.push_back(0x3000);// IDEOGRAPHIC SPACE
				chars.push_back(0xFEFF);// ZERO WIDTH NO-BREAK SPACE
			}

			virtual ParserBase* NewClone() const { return new Space(); }
			virtual void SaveOutputState() { }
			virtual void RestoreOutputState() { }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
					if(input.IsEOF() || (std::wstring::npos == chars.find(input.CurrentChar())))
					{
						// non-matching char
						return false;
					}

					input.Advance();
					return true;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Space\r\n";
				return ret;
			}
		};

		template<bool TCaseSensitive>
		struct CharOfT : public ParserBase
		{
			std::wstring chars;

			wchar_t chDummy;
			wchar_t& chOutput;
			wchar_t chOutputBackup;

			std::wstring strDummy;
			std::wstring& strOutput;
			std::wstring strOutputBackup;

			CharOfT(const std::wstring& chars_) :
				chars(chars_),
				chOutput(chDummy),
				strOutput(strDummy)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
			}

			CharOfT(const std::wstring& chars_, wchar_t& output_) :
				chars(chars_),
				chOutput(output_),
				strOutput(strDummy)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
			}

			// this appends the char to the output. for convenience.
			CharOfT(const std::wstring& chars_, std::wstring& output_) :
				chars(chars_),
				chOutput(chDummy),
				strOutput(output_)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
			}

			virtual ParserBase* NewClone() const
			{
				return new CharOfT<TCaseSensitive>(*this);
			}
			virtual void SaveOutputState()
			{
				chOutputBackup = chOutput;
				strOutputBackup = strOutput;
			}
			virtual void RestoreOutputState()
			{
				chOutput = chOutputBackup;
				strOutput = strOutputBackup;
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
				{
					return false;
				}
				wchar_t ch = input.CurrentChar();
				if(TCaseSensitive && (std::wstring::npos == chars.find(ch)))
				{
					return false;
				}
				if(!TCaseSensitive && (std::wstring::npos == chars.find(CharToLower(ch))))
				{
					return false;
				}

				chOutput = ch;
				strOutput.push_back(ch);
				input.Advance();
				return true;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"CharOf(\"" + chars + L"\")\r\n";
				return ret;
			}
		};

		typedef CharOfT<true> CharOf;
		typedef CharOfT<false> CharOfI;

		template<bool TCaseSensitive>
		struct CharRangeT : public ParserBase
		{
			wchar_t lower;
			wchar_t upper;

			wchar_t chDummy;
			wchar_t& chOutput;
			wchar_t chOutputBackup;

			std::wstring strDummy;
			std::wstring& strOutput;
			std::wstring strOutputBackup;

			CharRangeT(wchar_t lower_, wchar_t upper_) :
				lower(lower_),
				upper(upper_),
				chOutput(chDummy),
				strOutput(strDummy)
			{
				if(!TCaseSensitive)
				{
					lower = CharToLower(lower);
					upper = CharToLower(upper);
				}
			}

			CharRangeT(wchar_t lower_, wchar_t upper_, wchar_t& output_) :
				lower(lower_),
				upper(upper_),
				chOutput(output_),
				strOutput(strDummy)
			{
				if(!TCaseSensitive)
				{
					lower = CharToLower(lower);
					upper = CharToLower(upper);
				}
			}

			// this appends the char to the output. for convenience.
			CharRangeT(wchar_t lower_, wchar_t upper_, std::wstring& output_) :
				lower(lower_),
				upper(upper_),
				chOutput(chDummy),
				strOutput(output_)
			{
				if(!TCaseSensitive)
				{
					lower = CharToLower(lower);
					upper = CharToLower(upper);
				}
			}

			virtual ParserBase* NewClone() const
			{
				return new CharRangeT<TCaseSensitive>(*this);
			}
			virtual void SaveOutputState()
			{
				chOutputBackup = chOutput;
				strOutputBackup = strOutput;
			}
			virtual void RestoreOutputState()
			{
				chOutput = chOutputBackup;
				strOutput = strOutputBackup;
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
				{
					return false;
				}
				wchar_t ch = input.CurrentChar();
				if(TCaseSensitive && ((ch < lower) || (ch > upper)))
				{
					return false;
				}
				if(!TCaseSensitive)
				{
					wchar_t chL = CharToLower(ch);
					if((ch < lower) || (ch > upper))
						return false;
				}

				chOutput = ch;
				strOutput.push_back(ch);
				input.Advance();
				return true;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"CharRange('" + std::wstring(1, lower) + L"'-'" + std::wstring(1,upper) + L"')\r\n";
				return ret;
			}
		};

		typedef CharRangeT<true> CharRange;
		typedef CharRangeT<false> CharRangeI;

		struct Eol : public ParserBase
		{
			Eol()
			{
			}

			virtual ParserBase* NewClone() const { return new Eol(); }

			virtual void SaveOutputState() { }
			virtual void RestoreOutputState() { }

			// basically we match \r, \n, or \r\n
			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())// count EOF as EOL
					return true;
				return (*(Str(L"\r\n") || Char('\r') || Char('\n'))).ParseRetainingStateOnError(result, input);
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Eol\r\n";
				return ret;
			}
		};

		// parses C++-style string escape sequences
		struct StringEscapeParser :
			public ParserWithOutput<std::wstring, StringEscapeParser>
		{
			StringEscapeParser(std::wstring& output) : ParserWithOutput<std::wstring, StringEscapeParser>(output) { }
			StringEscapeParser(StringEscapeParser& rhs) : ParserWithOutput<std::wstring, StringEscapeParser>(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"StringEscapeParser\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t escapeChar = 0;
				if(!(Char('\\') + Char(0, escapeChar)).ParseRetainingStateOnError(result, input))
					return false;
				switch(escapeChar)
				{
				case 'r':
					output.push_back('\r');
					return true;
				case 'n':
					output.push_back('\n');
					return true;
				case 't':
					output.push_back('\t');
					return true;
				case '\"':
					output.push_back('\"');
					return true;
				case '\\':
					output.push_back('\\');
					return true;
				case '\'':
					output.push_back('\'');
					return true;
				// TODO: handle other escape sequences
				default:
					// unrecognized escape
					return false;
				}
			}
		};

		// helps parsing javascript-style strings and unquoted strings
		struct StringParser :
			public ParserWithOutput<std::wstring, StringParser>
		{
			StringParser(std::wstring& output) : ParserWithOutput<std::wstring, StringParser>(output) { }
			StringParser(StringParser& rhs) : ParserWithOutput<std::wstring, StringParser>(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"StringParser\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				Parser noQuotes = +(!Space() && Char(0, output));
				Parser singleQuotes = Char('\'') + *(!CharOf(L"\'\r\n") && (StringEscapeParser(output) || Char(0, output))) + Char('\'');
				Parser doubleQuotes = Char('\"') + *(!CharOf(L"\"\r\n") && (StringEscapeParser(output) || Char(0, output))) + Char('\"');
				Parser p = singleQuotes || doubleQuotes || noQuotes;
				bool ret = p.ParseRetainingStateOnError(result, input);
				return ret;
			}
		};

		template<typename IntT>
		struct UnsignedIntegerParserT :
			public ParserBase
		{
			std::wstring digits;
			int base;

			IntT outputDummy;
			IntT& output;
			IntT outputBackup;

			UnsignedIntegerParserT() :
				output(outputDummy),
				base(10)
			{
				InitDigits();
			}

			UnsignedIntegerParserT(int base_) :
				output(outputDummy),
				base(base_)
			{
				InitDigits();
			}

			UnsignedIntegerParserT(IntT& output_) :
				output(output_),
				base(10)
			{
				InitDigits();
			}

			UnsignedIntegerParserT(int base_, IntT& output_) :
				output(output_),
				base(base_)
			{
				InitDigits();
			}

			void InitDigits()
			{
				digits = std::wstring(L"0123456789abcdefghijklmnopqrstuvwxyz", base);
			}

			virtual ParserBase* NewClone() const
			{
				return new UnsignedIntegerParserT<IntT>(*this);
			}
			virtual void SaveOutputState()
			{
				outputBackup = output;
			}
			virtual void RestoreOutputState()
			{
				output = outputBackup;
			}

			virtual std::wstring Dump(int indentLevel)
			{
				return std::wstring(indentLevel, ' ') + L"UnsignedIntegerParser\r\n";
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				std::wstring outputStr;

				Parser p = +(CharOfI(digits, outputStr));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;// no digits at all. maybe whitespace? or a non-digit char?

				// convert from outputStr to integer
				output = 0;
				for(std::wstring::const_iterator it = outputStr.begin(); it != outputStr.end(); ++ it)
				{
					wchar_t ch = CharToLower(*it);
					int digit;
					if(ch >= '0' && ch <= '9')
					{
						digit = ch - '0';
					}
					if(ch >= 'a' && ch <= 'z')
					{
						digit = 10 + ch - 'a';
					}
					output *= base;
					output += digit;
				}
				return true;
			}
		};

		template<typename T>
		UnsignedIntegerParserT<T> UnsignedIntegerParser(int base, T& output)
		{
			return UnsignedIntegerParserT<T>(base, output);
		}

		template<typename T>
		UnsignedIntegerParserT<T> UnsignedIntegerParser(T& output)
		{
			return UnsignedIntegerParserT<T>(output);
		}

		// UIntegerHexT (signed 16-bit integer parser with prefix of 0x)
		template<typename IntT>
		struct UIntegerHexT :
			public ParserWithOutput<IntT, UIntegerHexT<IntT> >
		{
			UIntegerHexT(IntT& output) : ParserWithOutput<IntT, UIntegerHexT<IntT> >(output) { }
			UIntegerHexT(UIntegerHexT<IntT>& rhs) : ParserWithOutput<IntT, UIntegerHexT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerHexT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (Str(L"0x") + UnsignedIntegerParser(16, output)).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename T>
		UIntegerHexT<T> UIntegerHex(T& output)
		{
			return UIntegerHexT<T>(output);
		}

		// UIntegerOct (signed 8-bit integer parser with prefix of 0)
		template<typename IntT>
		struct UIntegerOctT :
			public ParserWithOutput<IntT, UIntegerOctT<IntT> >
		{
			UIntegerOctT(IntT& output) : ParserWithOutput<IntT, UIntegerOctT<IntT> >(output) { }
			UIntegerOctT(UIntegerOctT<IntT>& rhs) : ParserWithOutput<IntT, UIntegerOctT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerOctT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (Char('0') + UnsignedIntegerParser(8, output)).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename T>
		UIntegerOctT<T> UIntegerOct(T& output)
		{
			return UIntegerOctT<T>(output);
		}

		// UIntegerDecT (signed 10-bit integer parser)
		template<typename IntT>
		struct UIntegerDecT :
			public ParserWithOutput<IntT, UIntegerDecT<IntT> >
		{
			UIntegerDecT(IntT& output) : ParserWithOutput<IntT, UIntegerDecT<IntT> >(output) { }
			UIntegerDecT(UIntegerDecT<IntT>& rhs) : ParserWithOutput<IntT, UIntegerDecT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerDecT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (!Char('0') + UnsignedIntegerParser(10, output) + !Char('b')).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename T>
		UIntegerDecT<T> UIntegerDec(T& output)
		{
			return UIntegerDecT<T>(output);
		}

		// UIntegerBinT (signed 2-bit integer parser with suffix of 'b')
		template<typename IntT>
		struct UIntegerBinT :
			public ParserWithOutput<IntT, UIntegerBinT<IntT> >
		{
			UIntegerBinT(IntT& output) : ParserWithOutput<IntT, UIntegerBinT<IntT> >(output) { }
			UIntegerBinT(UIntegerBinT<IntT>& rhs) : ParserWithOutput<IntT, UIntegerBinT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerBinT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (!Char('0') + UnsignedIntegerParser(2, output) + Char('b')).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename T>
		UIntegerBinT<T> UIntegerBin(T& output)
		{
			return UIntegerBinT<T>(output);
		}

		// UIntegerHexT (signed 16-bit integer parser with prefix of 0x)
		template<typename IntT>
		struct IntegerHexT :
			public ParserWithOutput<IntT, IntegerHexT<IntT> >
		{
			IntegerHexT(IntT& output) : ParserWithOutput<IntT, IntegerHexT<IntT> >(output) { }
			IntegerHexT(IntegerHexT<IntT>& rhs) : ParserWithOutput<IntT, IntegerHexT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"IntegerHexT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				const int base = 16;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + Str(L"0x") + UnsignedIntegerParserT<IntT>(base, output));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				if(sign == '-')
					output = -output;
				return true;
			}
		};

		template<typename T>
		IntegerHexT<T> IntegerHex(T& output)
		{
			return IntegerHexT<T>(output);
		}

		// IntegerOctT (signed 8-bit integer parser with prefix of 0)
		template<typename IntT>
		struct IntegerOctT :
			public ParserWithOutput<IntT, IntegerOctT<IntT> >
		{
			IntegerOctT(IntT& output) : ParserWithOutput<IntT, IntegerOctT<IntT> >(output) { }
			IntegerOctT(IntegerOctT<IntT>& rhs) : ParserWithOutput<IntT, IntegerOctT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"IntegerOctT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				const int base = 8;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + Char('0') + UnsignedIntegerParserT<IntT>(base, output));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				if(sign == '-')
					output = -output;
				return true;
			}
		};

		template<typename T>
		IntegerOctT<T> IntegerOct(T& output)
		{
			return IntegerOctT<T>(output);
		}

		// IntegerBinT (signed 2-bit integer parser with suffix of 'b')
		template<typename IntT>
		struct IntegerBinT :
			public ParserWithOutput<IntT, IntegerBinT<IntT> >
		{
			IntegerBinT(IntT& output) : ParserWithOutput<IntT, IntegerBinT<IntT> >(output) { }
			IntegerBinT(IntegerBinT<IntT>& rhs) : ParserWithOutput<IntT, IntegerBinT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"IntegerBinT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				const int base = 2;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + UnsignedIntegerParserT<IntT>(base, output) + Char('b'));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				if(sign == '-')
					output = -output;
				return true;
			}
		};

		template<typename T>
		IntegerBinT<T> IntegerBin(T& output)
		{
			return IntegerBinT<T>(output);
		}

		// IntegerDecT (signed 10-bit integer parser)
		template<typename IntT>
		struct IntegerDecT :
			public ParserWithOutput<IntT, IntegerDecT<IntT> >
		{
			IntegerDecT(IntT& output) : ParserWithOutput<IntT, IntegerDecT<IntT> >(output) { }
			IntegerDecT(IntegerDecT<IntT>& rhs) : ParserWithOutput<IntT, IntegerDecT<IntT> >(rhs.output) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"IntegerDecT\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				const int base = 10;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + !Char('0') + UnsignedIntegerParserT<IntT>(base, output) + !Char('b'));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				if(sign == '-')
					output = -output;
				return true;
			}
		};

		template<typename T>
		IntegerDecT<T> IntegerDec(T& output)
		{
			return IntegerDecT<T>(output);
		}

		// even more high-level.
		template<typename IntT>
		Parser CSignedInteger(IntT& output)
		{
			return IntegerDec(output) || IntegerOct(output) || IntegerBin(output) || IntegerHex(output);
		}

		template<typename IntT>
		Parser CUnsignedInteger(IntT& output)
		{
			return UIntegerDec(output) || UIntegerOct(output) || UIntegerBin(output) || UIntegerHex(output);
		}

		template<typename IntT>
		Parser CInteger(IntT& output)
		{
			return CUnsignedInteger(output) || CSignedInteger(output);
		}
	}
}
