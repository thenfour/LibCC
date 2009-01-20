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
- floating point number parser
- profiling & testing
- better diagnostic stuff (output a sample input string or EBNF string, set breakpoints in the parse process)
- properly handle overflow on numbers?
- write a ScriptReader that can work on any stream, not just text?


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
		struct ScriptCursor
		{
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

		struct ScriptReader
		{
			virtual wchar_t CurrentChar() const = 0;
			virtual void Advance() = 0;
			virtual bool IsEOF() const = 0;
			virtual int GetLine() const = 0;
			virtual int GetColumn() const = 0;
			virtual ScriptCursor GetCursorCopy() const = 0;
			virtual void SetCursor(const ScriptCursor& c) = 0;
		};

		struct BasicStringReader :
			public ScriptReader
		{
		public:
			BasicStringReader()
			{
			}

			BasicStringReader(const std::wstring& s) :
				m_script(s)
			{
			}

			wchar_t CurrentChar() const
			{
				return m_script[m_cursor.pos];
			}

			void Advance()
			{
				InternalAdvance();
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

		// basic framework parsers ///////////////////////////////////////////////////////////////////////////////////////
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
			virtual bool ParseRetainingStateOnError(ScriptResult& result, ScriptReader& input)
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

			// useful for simple parsing stuff like CInteger(RefOutput(myint)).Parse(L"-1");
			virtual bool ParseSimple(const std::wstring& script)
			{
				ScriptResult result;
				return Parse(result, BasicStringReader(script));
			}

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

		// basic framework parsers ///////////////////////////////////////////////////////////////////////////////////////

		struct Passthrough :
			public ParserBase
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
		struct Occurrences :
			public ParserBase
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

		struct Not :
			public ParserBase
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

		struct Optional :
			public ParserBase
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

		struct Sequence :
			public ParserBase
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

		struct Or :
			public ParserBase
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

		struct Eof :
			public ParserBase
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


		
		// storage engines ///////////////////////////////////////////////////////////////////////////////////////
		template<typename Tin>
		struct OutputBase
		{
			virtual void Save(const Tin& val) = 0;
			virtual void SaveState() = 0;
			virtual void RestoreState() = 0;
			virtual OutputBase<Tin>* NewClone() const = 0;
		};

		template<typename Tin>
		struct OutputPtr
		{
			struct OutputBase<Tin>* p;

			OutputPtr() : p(0)
			{
			}
			OutputPtr(const OutputPtr<Tin>& rhs) : p(0)
			{
				Assign(rhs);
			}
			OutputPtr(OutputBase<Tin>* rhs) : p(0)
			{
				Assign(rhs);
			}
			OutputPtr(const OutputBase<Tin>& rhs) : p(0)
			{
				Assign(rhs);
			}

			~OutputPtr()
			{
				Release();
			}

			bool IsEmpty() const { return p == 0; }
			bool IsValid() const { return p != 0; }

			void Assign(const OutputPtr<Tin>& rhs)
			{
				Release();
				if(rhs.IsValid())
					p = rhs.p->NewClone();// copying creates a clone.
			}

			void Assign(const OutputBase<Tin>& rhs)
			{
				Release();
				p = rhs.NewClone();
			}

			void Assign(OutputBase<Tin>* rhs)
			{
				Release();
				p = rhs;
			}

			void Release()
			{
				delete p;
				p = 0;
			}

			OutputBase<Tin>* operator ->()
			{
				return p;
			}

			const OutputBase<Tin>* operator ->() const
			{
				return p;
			}
		};


		// NullOutput
		// output is not saved anywhere
		template<typename Tin>
		struct NullOutput :
			public OutputBase<Tin>
		{
			void Save(const Tin& val) { }
			void SaveState() { }
			void RestoreState() { }
			OutputBase<Tin>* NewClone() const { return new NullOutput<Tin>(*this); }
		};

		// RefOutput
		// wchar_t ch; Parser p = CharOf(L"abc", RefOutput(ch));
		// output is stored in a reference passed in by the caller
		template<typename Tin>
		struct RefOutputT :
			public OutputBase<Tin>
		{
			Tin& output;
			Tin outputBackup;
			RefOutputT(Tin& output_) :
				output(output_)
			{
			}
			void Save(const Tin& val)
			{
				output = val;
			}
			void SaveState() { outputBackup = output; }
			void RestoreState() { output = outputBackup; }
			OutputBase<Tin>* NewClone() const
			{
				return new RefOutputT<Tin>(*this);
			}
		};

		template<typename Tin>
		RefOutputT<Tin> RefOutput(Tin& outputVar)
		{
			return RefOutputT<Tin>(outputVar);
		}

		// InserterOutput
		// std::list<wchar_t> l;   Parser p = *CharOf(L"abc", InserterOutput<wchar_t>(l, std::back_inserter(l)));
		// output is stored in a reference passed in by the caller
		//
		// NOTE: i am not 100% certain that the inserter iterator is always valid after restoring state.
		template<typename Tin, typename Tinserter, typename Tcontainer>
		struct InserterOutputT :
			public OutputBase<Tin>
		{
			Tinserter outputInserter;
			Tcontainer& outputContainer;
			Tcontainer outputContainerBackup;

			InserterOutputT(Tcontainer& container, Tinserter& inserter) :
				outputContainer(container),
				outputInserter(inserter)
			{
			}
			void Save(const Tin& val)
			{
				*outputInserter = val;
				++outputInserter;
			}
			void SaveState() { outputContainerBackup = outputContainer; }
			void RestoreState() { outputContainer = outputContainerBackup; }
			OutputBase<Tin>* NewClone() const
			{
				return new InserterOutputT<Tin, Tinserter, Tcontainer>(*this);
			}
		};

		template<typename Tin, typename Tinserter, typename Tcontainer>
		InserterOutputT<Tin, Tinserter, Tcontainer> InserterOutput(Tcontainer& container, Tinserter& inserter)
		{
			return InserterOutputT<Tin, Tinserter, Tcontainer>(container, inserter);
		}

		// std::list<wchar_t> l;   CharOf(L"abc", BackInserterOutput<wchar_t>(l));
		template<typename Tin, typename Tcontainer>
		OutputPtr<Tin> BackInserterOutput(Tcontainer& outputVar)
		{
			return InserterOutput<Tin>(outputVar, std::back_inserter(outputVar));
		}

		template<typename Tin>
		OutputPtr<Tin> VectorOutput(std::vector<Tin>& outputVar)
		{
			return InserterOutput<Tin>(outputVar, std::back_inserter(outputVar));
		}

		template<typename Tin>
		OutputPtr<Tin> CharToStringOutput(std::basic_string<Tin>& outputVar)
		{
			return InserterOutput<Tin>(outputVar, std::back_inserter(outputVar));
		}


		// Some basic utility parsers ///////////////////////////////////////////////////////////////////////////////////////
		// this class used to end on line 711
		template<bool TCaseSensitive>
		struct CharT :
			public ParserBase
		{
			wchar_t match;
			OutputPtr<wchar_t> output;

			CharT() :
				match(0)
			{
				output.Assign(NullOutput<wchar_t>().NewClone());
			}

			CharT(wchar_t match_) :
				match(match_)
			{
				output.Assign(NullOutput<wchar_t>().NewClone());
			}

			CharT(wchar_t match_, const OutputPtr<wchar_t>& out) :
				match(match_),
				output(out)
			{
			}

			CharT(wchar_t match_, wchar_t& out) :
				match(match_)
			{
				output.Assign(RefOutput(out));
			}

			CharT(wchar_t match_, std::wstring& out) :
				match(match_)
			{
				output.Assign(CharToStringOutput(out));
			}

			virtual ParserBase* NewClone() const { return new CharT<TCaseSensitive>(*this); }

			virtual void SaveOutputState()
			{
				output->SaveState();
			}
			virtual void RestoreOutputState()
			{
				output->RestoreState();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
				{
					//result.Message(L"Unexpected end of file searching for character ...");
					return false;
				}

				wchar_t ch = input.CurrentChar();
				output->Save(ch);
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
			std::wstring match;
			OutputPtr<std::wstring> output;

			StrT()
			{
				output.Assign(NullOutput<std::wstring>());
			}

			StrT(const std::wstring& match) :
				match(match)
			{
				output.Assign(NullOutput<std::wstring>());
			}

			StrT(const std::wstring& match, const OutputPtr<std::wstring>& out) :
				match(match),
				output(out)
			{
			}

			StrT(const std::wstring& match, std::wstring& out) :
				match(match)
			{
				output.Assign(RefOutput<std::wstring>(out));
			}

			virtual ParserBase* NewClone() const { return new StrT<TCaseSensitive>(*this); }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
					return match.empty();

				std::wstring find;

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
					find.push_back(ch);
				}

				output->Save(find);
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
			OutputPtr<wchar_t> output;

			CharOfT(const std::wstring& chars_) :
				chars(chars_)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
				output.Assign(NullOutput<wchar_t>());
			}

			CharOfT(const std::wstring& chars_, const OutputPtr<wchar_t>& output_) :
				chars(chars_)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
				output.Assign(output_);
			}

			CharOfT(const std::wstring& chars_, wchar_t& output_) :
				chars(chars_)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
				output.Assign(RefOutput<wchar_t>(output_));
			}

			// this appends the char to the output. for convenience.
			CharOfT(const std::wstring& chars_, std::wstring& output_) :
				chars(chars_)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
				output.Assign(CharToStringOutput<wchar_t>(output_));
			}

			virtual ParserBase* NewClone() const
			{
				return new CharOfT<TCaseSensitive>(*this);
			}
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

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

				output->Save(ch);
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
			OutputPtr<wchar_t> output;

			CharRangeT(wchar_t lower_, wchar_t upper_) :
				lower(lower_),
				upper(upper_)
			{
				if(!TCaseSensitive)
				{
					lower = CharToLower(lower);
					upper = CharToLower(upper);
				}
				output.Assign(NullOutput<wchar_t>());
			}

			CharRangeT(wchar_t lower_, wchar_t upper_, const OutputPtr<wchar_t>& output_) :
				lower(lower_),
				upper(upper_),
				output(output_)
			{
				if(!TCaseSensitive)
				{
					lower = CharToLower(lower);
					upper = CharToLower(upper);
				}
			}

			CharRangeT(wchar_t lower_, wchar_t upper_, wchar_t& output_) :
				lower(lower_),
				upper(upper_)
			{
				if(!TCaseSensitive)
				{
					lower = CharToLower(lower);
					upper = CharToLower(upper);
				}
				output.Assign(RefOutput(output_));
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
				output.Assign(CharToStringOutput(output_));
			}

			virtual ParserBase* NewClone() const
			{
				return new CharRangeT<TCaseSensitive>(*this);
			}
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

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

				output->Save(ch);
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

		// utility string parsers ///////////////////////////////////////////////////////////////////////////////////////
		// parses C++-style string escape sequences
		struct StringEscapeParser :
			public ParserBase
		{
			OutputPtr<wchar_t> output;
			StringEscapeParser(){ output.Assign(NullOutput<wchar_t>()); }
			StringEscapeParser(const OutputPtr<wchar_t>& output_) : output(output_) { }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }
			virtual ParserBase* NewClone() const { return new StringEscapeParser(); }

			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"StringEscapeParser\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t escapeChar = 0;
				if(!(Char('\\') + Char(0, escapeChar)).ParseRetainingStateOnError(result, input))
					return false;
				switch(escapeChar)
				{
				case 'r':
					output->Save('\r');
					return true;
				case 'n':
					output->Save('\n');
					return true;
				case 't':
					output->Save('\t');
					return true;
				case '\"':
					output->Save('\"');
					return true;
				case '\\':
					output->Save('\\');
					return true;
				case '\'':
					output->Save('\'');
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
			public ParserBase
		{
			OutputPtr<std::wstring> output;
			StringParser(){ output.Assign(NullOutput<std::wstring>()); }
			StringParser(const OutputPtr<std::wstring>& output_) : output(output_) { }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"StringParser\r\n"; }
			virtual ParserBase* NewClone() const { return new StringParser(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				// input from char and output to string outputer
				std::wstring thisString;
				Parser noQuotes = +(!Space() && Char(0, thisString));
				Parser singleQuotes = Char('\'') + *(!CharOf(L"\'\r\n") && (StringEscapeParser(CharToStringOutput(thisString)) || Char(0, thisString))) + Char('\'');
				Parser doubleQuotes = Char('\"') + *(!CharOf(L"\"\r\n") && (StringEscapeParser(CharToStringOutput(thisString)) || Char(0, thisString))) + Char('\"');
				Parser p = singleQuotes || doubleQuotes || noQuotes;
				bool ret = p.ParseRetainingStateOnError(result, input);
				if(ret)
					output->Save(thisString);
				return ret;
			}
		};

		// integer parsers ///////////////////////////////////////////////////////////////////////////////////////
		template<typename IntT>
		struct UnsignedIntegerParserT :
			public ParserBase
		{
			std::wstring digits;
			int base;
			OutputPtr<IntT> output;

			UnsignedIntegerParserT() :
				base(10)
			{
				output.Assign(NullOutput<IntT>());
				InitDigits();
			}

			UnsignedIntegerParserT(int base_) :
				base(base_)
			{
				output.Assign(NullOutput<IntT>());
				InitDigits();
			}

			UnsignedIntegerParserT(const OutputPtr<IntT>& output_) :
				base(10),
				output(output_)
			{
				InitDigits();
			}

			UnsignedIntegerParserT(int base_, const OutputPtr<IntT>& output_) :
				base(base_),
				output(output_)
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
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

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
				IntT out = 0;
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
					out *= base;
					out += digit;
				}
				output->Save(out);
				return true;
			}
		};

		template<typename IntT>
		UnsignedIntegerParserT<IntT> UnsignedIntegerParser(int base, const OutputPtr<IntT>& output)
		{
			return UnsignedIntegerParserT<IntT>(base, output);
		}

		template<typename IntT>
		UnsignedIntegerParserT<IntT> UnsignedIntegerParser(const OutputPtr<IntT>& output)
		{
			return UnsignedIntegerParserT<IntT>(output);
		}

		// UIntegerHexT (signed 16-bit integer parser with prefix of 0x)
		template<typename IntT>
		struct UIntegerHexT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			UIntegerHexT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new UIntegerHexT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerHexT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (Str(L"0x") + UnsignedIntegerParser(16, output)).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT>
		UIntegerHexT<IntT> UIntegerHex(const OutputPtr<IntT>& output)
		{
			return UIntegerHexT<IntT>(output);
		}

		// UIntegerOct (signed 8-bit integer parser with prefix of 0)
		template<typename IntT>
		struct UIntegerOctT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			UIntegerOctT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new UIntegerOctT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerOctT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (Char('0') + UnsignedIntegerParser(8, output)).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT>
		UIntegerOctT<IntT> UIntegerOct(const OutputPtr<IntT>& output)
		{
			return UIntegerOctT<IntT>(output);
		}

		// UIntegerDecT (signed 10-bit integer parser)
		template<typename IntT>
		struct UIntegerDecT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			UIntegerDecT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new UIntegerDecT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerDecT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (!Char('0') + UnsignedIntegerParser(10, output) + !Char('b')).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT>
		UIntegerDecT<IntT> UIntegerDec(const OutputPtr<IntT>& output)
		{
			return UIntegerDecT<IntT>(output);
		}

		// UIntegerBinT (signed 2-bit integer parser with suffix of 'b')
		template<typename IntT>
		struct UIntegerBinT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			UIntegerBinT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new UIntegerBinT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerBinT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return (!Char('0') + UnsignedIntegerParser(2, output) + Char('b')).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT>
		UIntegerBinT<IntT> UIntegerBin(const OutputPtr<IntT>& output)
		{
			return UIntegerBinT<IntT>(output);
		}

		// SIntegerHexT (signed 16-bit integer parser with prefix of 0x)
		template<typename IntT>
		struct SIntegerHexT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			SIntegerHexT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new SIntegerHexT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"SIntegerHexT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 16;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + Str(L"0x") + UnsignedIntegerParserT<IntT>(base, RefOutput(temp)));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output->Save(sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT>
		SIntegerHexT<IntT> SIntegerHex(const OutputPtr<IntT>& output)
		{
			return SIntegerHexT<IntT>(output);
		}

		// IntegerOctT (signed 8-bit integer parser with prefix of 0)
		template<typename IntT>
		struct SIntegerOctT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			SIntegerOctT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new SIntegerOctT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"SIntegerOctT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 8;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + Char('0') + UnsignedIntegerParserT<IntT>(base, RefOutput(temp)));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output->Save(sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT>
		SIntegerOctT<IntT> SIntegerOct(const OutputPtr<IntT>& output)
		{
			return SIntegerOctT<IntT>(output);
		}

		// IntegerBinT (signed 2-bit integer parser with suffix of 'b')
		template<typename IntT>
		struct SIntegerBinT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			SIntegerBinT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new SIntegerBinT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"SIntegerBinT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 2;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + UnsignedIntegerParserT<IntT>(base, RefOutput(temp)) + Char('b'));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output->Save(sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT>
		SIntegerBinT<IntT> SIntegerBin(const OutputPtr<IntT>& output)
		{
			return SIntegerBinT<IntT>(output);
		}

		// IntegerDecT (signed 10-bit integer parser)
		template<typename IntT>
		struct SIntegerDecT :
			public ParserBase
		{
			OutputPtr<IntT> output;
			SIntegerDecT(const OutputPtr<IntT>& output_) : output(output_) { }
			virtual ParserBase* NewClone() const { return new SIntegerDecT<IntT>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"SIntegerDecT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 10;
				wchar_t sign = '+';// default to positive
				Parser p = (-CharOf(L"+-", sign) + !Char('0') + UnsignedIntegerParserT<IntT>(base, RefOutput(temp)) + !Char('b'));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output->Save(sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT>
		SIntegerDecT<IntT> SIntegerDec(const OutputPtr<IntT>& output)
		{
			return SIntegerDecT<IntT>(output);
		}

		// even more high-level.
		template<typename IntT>
		Parser CSignedInteger(const OutputPtr<IntT>& output)
		{
			return SIntegerDec(output) || SIntegerOct(output) || SIntegerBin(output) || SIntegerHex(output);
		}

		template<typename IntT>
		Parser CUnsignedInteger(const OutputPtr<IntT>& output)
		{
			return UIntegerDec(output) || UIntegerOct(output) || UIntegerBin(output) || UIntegerHex(output);
		}

		template<typename IntT>
		Parser CInteger(const OutputPtr<IntT>& output)
		{
			return CUnsignedInteger(output) || CSignedInteger(output);
		}

		template<typename IntT>
		Parser CInteger(IntT& output)
		{
			return CInteger<IntT>(RefOutput(output));
		}


		// Utility Script readers ///////////////////////////////////////////////////////////////////////////////////////

		// feeds a string into the parser, and converts all newlines into \n
		// and skips C++ style comments
		struct CScriptReader :
			public ScriptReader
		{
		public:
			CScriptReader()
			{
			}

			CScriptReader(const std::wstring& s) :
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

	}
}
