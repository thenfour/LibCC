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
- convenience parsing of numbers
- other ScriptReaders that:
	 - could work on binary, not just text?
	 - provide more options and don't force you to use C++-style comments

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

		struct Char : public ParserBase
		{
			wchar_t dummy;
			wchar_t& output;
			wchar_t outputBackup;
			wchar_t match;

			Char(wchar_t match, wchar_t& out) :
				dummy(0),
				output(out),
				match(match)
			{
			}

			Char() :
				dummy(0),
				output(dummy),
				match(0)
			{
			}

			Char(wchar_t match) :
				dummy(0),
				output(dummy),
				match(match)
			{
			}

			virtual ParserBase* NewClone() const { return new Char(*this); }

			virtual void SaveOutputState() { outputBackup = output; }

			virtual void RestoreOutputState() { output = outputBackup; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
				{
					//result.Message(L"Unexpected end of file searching for character ...");
					return false;
				}

				output = input.CurrentChar();
				input.Advance();

				if(match == 0)
				{
					return true;
				}

				if(match != output)
				{
					//result.Message(L"Unexpected character");
					return false;
				}
				else
				{
					return true;
				}
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


		struct Str : public ParserBase
		{
			std::wstring dummy;
			std::wstring& output;
			std::wstring outputBackup;
			std::wstring match;

			Str(const std::wstring& match, std::wstring& out) :
				output(out),
				match(match)
			{
			}

			Str() :
				output(dummy)
			{
			}

			Str(const std::wstring& match) :
				output(dummy),
				match(match)
			{
			}

			virtual ParserBase* NewClone() const { return new Str(*this); }
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

					if(ch != *it)
					{
						//result.Message(L"Unexpected character searching for string ...");
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
		struct SpaceChar : public ParserBase
		{
			std::wstring chars;

			SpaceChar()
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

			virtual ParserBase* NewClone() const { return new SpaceChar(); }
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
				ret += std::wstring(indentLevel, ' ') + L"SpaceChar\r\n";
				return ret;
			}
		};

		template<int minimumOccurrences>
		struct CharOf : public ParserBase
		{
			std::wstring chars;
			std::wstring dummy;
			std::wstring& output;
			std::wstring outputBackup;

			CharOf(const std::wstring& chars_) :
				chars(chars_),
				output(dummy)
			{
			}

			CharOf(const std::wstring& chars_, std::wstring& output_) :
				chars(chars_),
				output(output_)
			{
			}

			virtual ParserBase* NewClone() const
			{
				return new CharOf<minimumOccurrences>(*this);
			}
			virtual void SaveOutputState() { outputBackup = output; }
			virtual void RestoreOutputState() { output = outputBackup; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				int count = 0;

				while(true)
				{
					if(input.IsEOF() || (std::wstring::npos == chars.find(input.CurrentChar())))
					{
						// non-matching char
						return count >= minimumOccurrences;
					}

					output.push_back(input.CurrentChar());
					input.Advance();
					++ count;
				}
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"CharOf(\"" + chars + L"\")\r\n";
				return ret;
			}
		};

		template<int MinimumOccurrences>
		struct Space : public ParserBase
		{
			virtual ParserBase* NewClone() const
			{
				return new Space<MinimumOccurrences>(*this);
			}
			virtual void SaveOutputState() { }
			virtual void RestoreOutputState() { }
			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return Occurrences<MinimumOccurrences>(SpaceChar()).ParseRetainingStateOnError(result, input);
			}

			virtual std::wstring Dump(int indentLevel)
			{
				std::wstring ret;
				ret += std::wstring(indentLevel, ' ') + L"Space<>\r\n";
				return ret;
			}
		};

		typedef Space<0> Space0;
		typedef Space<1> Space1;


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

	}
}
