/*
  LibCC
  Parse Module
  (c) 2009 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC_Parse
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
			virtual const std::wstring& GetRawInput() const = 0;
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

			virtual const std::wstring& GetRawInput() const
			{
				return m_script;
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
			int indentLevel;
			bool verboseParseOutput;

			ScriptResult() :
				indentLevel(0),
				verboseParseOutput(false)
			{
			}

			std::vector<std::wstring> messages;
			void Message(const std::wstring& msg)
			{
				std::wstring x;
				for(int i = 0; i < indentLevel; ++ i)
				{
					x.append(L"  ");
				}
				x.append(msg);
				messages.push_back(x);
			}
		};

		struct ParserBase
		{
		private:
			std::wstring DebugSanitize(const std::wstring& in) const
			{
				std::wstring out;
				for(std::wstring::const_iterator it = in.begin(); it != in.end(); ++ it)
				{
					if(*it == '\r') out.push_back('.');
					else if(*it == '\n') out.push_back('.');
					else if(*it == '|') out.push_back('.');
					else out.push_back(*it);
				}
				return out;
			}

			// Pos:288(4,12) "xxx|xxx"
			std::wstring DebugCursorToString(const ScriptCursor& cur, ScriptReader& input) const
			{
				const int amount = 6;
				const std::wstring& script(input.GetRawInput());
				int nleft = min(cur.pos, amount);
				std::wstring left = script.substr(cur.pos - nleft, nleft);
				int nright = (int)script.size() - cur.pos;
				nright = min(nright, amount);
				std::wstring right = script.substr(cur.pos, nright);
				left = DebugSanitize(left);
				right = DebugSanitize(right);

				return LibCC::FormatW(L"Pos:%(%,%) \"%^|%\"")
					.i(cur.pos)
					.i(cur.line)
					.i(cur.column)
					.s(left)
					.s(right)
					.Str();
			}

			// [Abinooeeueue...oooeueeee]
			std::wstring DebugSubStr(const ScriptCursor& leftCursor, const ScriptCursor& rightCursor, ScriptReader& input) const
			{
				int amount = 50;
				std::wstring ellipses = L"...";
				const std::wstring& script(input.GetRawInput());
				int len = rightCursor.pos - leftCursor.pos;

				bool useEllipses = len > amount;
				if(useEllipses)
				{
					len = amount - (int)ellipses.size();
				}

				int nleft = len / 2;
				int nright = len - nleft;
				std::wstring left = script.substr(leftCursor.pos, nleft);
				std::wstring right = script.substr(rightCursor.pos - nright, nright);

				left = DebugSanitize(left);
				right = DebugSanitize(right);

				return LibCC::FormatW(L"[%%%]")
					.s(left)
					.s(useEllipses ? ellipses : L"")
					.s(right)
					.Str();
			}

		public:
			bool IsVerboseLoggingEnabled;

			ParserBase() :
				IsVerboseLoggingEnabled(true)
			{
			}

			virtual bool ParseRetainingStateOnError(ScriptResult& result, ScriptReader& input)
			{
				ScriptCursor oldCursor = input.GetCursorCopy();
				SaveOutputState();

				if(result.verboseParseOutput && IsVerboseLoggingEnabled)
				{
					result.Message(LibCC::FormatW(L"Parsing '%' from %")
						(GetParserName())
						(DebugCursorToString(oldCursor, input))
						.Str());
					result.Message(L"{");
					result.indentLevel ++;
				}

				bool wasVerboseLoggingEnabled = result.verboseParseOutput;
				if(!IsVerboseLoggingEnabled)
				{
					result.verboseParseOutput = false;
				}

				bool ret = Parse(result, input);
				result.verboseParseOutput = wasVerboseLoggingEnabled;

				if(!ret)
				{
					if(result.verboseParseOutput && IsVerboseLoggingEnabled)
					{
						ScriptCursor newCursor = input.GetCursorCopy();
						result.Message(LibCC::FormatW(L"=false '%' from % to % %")
							(GetParserName())
							(DebugCursorToString(oldCursor, input))
							(DebugCursorToString(newCursor, input))
							(DebugSubStr(oldCursor, newCursor, input))
							.Str());
						result.indentLevel --;
						result.Message(L"}");
					}
					RestoreOutputState();
					input.SetCursor(oldCursor);
					return false;
				}

				if(result.verboseParseOutput && IsVerboseLoggingEnabled)
				{
					ScriptCursor newCursor = input.GetCursorCopy();
					result.Message(LibCC::FormatW(L"=TRUE '%' from % to % %")
						(GetParserName())
						(DebugCursorToString(oldCursor, input))
						(DebugCursorToString(newCursor, input))
						(DebugSubStr(oldCursor, newCursor, input))
						.Str());
					result.indentLevel --;
					result.Message(L"}");
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
			virtual std::wstring GetParserName() const
			{
				return L"(unnamed)";
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
			ParserPtr(const ParserBase& rhs) : p(0)
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

			void Assign(const ParserBase& rhs)
			{
				Release();
				p = rhs.NewClone();
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
			std::wstring m_name;

		public:
			Passthrough& operator =(const Passthrough& rhs)
			{
				m_name = rhs.m_name;
				m_child = rhs.m_child;
				return *this;
			}

			Passthrough& operator =(const ParserBase& rhs)
			{
				m_name = L"Passthrough";
				m_child.Assign(rhs);
				return *this;
			}

			Passthrough()
			{
				m_name = L"Passthrough";
			}

			Passthrough(const ParserBase& rhs)
			{
				m_child.Assign(rhs);
				m_name = L"Passthrough";
			}

			Passthrough(const std::wstring& subName, const ParserBase& rhs)
			{
				m_child.Assign(rhs);
				m_name = subName;
			}

			Passthrough(const std::wstring& subName, bool enableVerboseLoggingForChild_, const ParserBase& rhs)
			{
				m_child.Assign(rhs);
				m_child->IsVerboseLoggingEnabled = enableVerboseLoggingForChild_;
				m_name = subName;
			}

			virtual ParserBase* NewClone() const { return new Passthrough(*this); }
			virtual std::wstring GetParserName() const { return m_name; }

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

		template<int MinimumOccurrences, bool skipWhitespaceBetween>
		struct Occurrences :
			public ParserBase
		{
		private:
			ParserPtr m_child;

		public:
			Occurrences()
			{
			}

			Occurrences(const Occurrences<MinimumOccurrences, skipWhitespaceBetween>& rhs)
			{
				if(rhs.m_child.IsEmpty())
					return;
				m_child.Assign(rhs.m_child->NewClone());
			}

			Occurrences(const ParserBase& rhs)
			{
				m_child.Assign(rhs);
			}

			virtual ParserBase* NewClone() const { return new Occurrences<MinimumOccurrences, skipWhitespaceBetween>(*this); }
			virtual std::wstring GetParserName() const
			{
				std::wstring skipBehavior = skipWhitespaceBetween ? L"skip" : L"no skip";
				if(MinimumOccurrences == 0)
					return LibCC::FormatW(L"Zero or more (%)").s(skipBehavior).Str();
				if(MinimumOccurrences == 1)
					return LibCC::FormatW(L"One or more (%)").s(skipBehavior).Str();
				return LibCC::FormatW(L"Occurrences<%,%>").i(MinimumOccurrences).s(skipBehavior).Str();
			}

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

					if(skipWhitespaceBetween)
					{
						while(!input.IsEOF() && StringContains(WhitespaceChars(), input.CurrentChar()))
						{
							input.Advance();
						}
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

		typedef Occurrences<0, false> ZeroOrMore;
		typedef Occurrences<0, true> ZeroOrMoreS;
		typedef Occurrences<0, false> OneOrMore;
		typedef Occurrences<0, true> OneOrMoreS;

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
				m_child.Assign(rhs);
			}

			virtual ParserBase* NewClone() const { return new Not(*this); }
			virtual std::wstring GetParserName() const { return L"Not"; }

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
				m_child.Assign(rhs);
			}

			virtual ParserBase* NewClone() const { return new Optional(*this); }
			virtual std::wstring GetParserName() const { return L"Optional"; }

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

		inline const wchar_t* WhitespaceChars()
		{
			static const wchar_t x[] = {
				'\t', '\n', '\r', ' ',
				0x0085,// NEL (control character next line)
				0x00A0,// NBSP (NO-BREAK SPACE)
				0x1680,// OGHAM SPACE MARK
				0x180E,// MONGOLIAN VOWEL SEPARATOR
				0x2000,// EN QUAD
				0x2001,// EM QUAD
				0x2002,// EN SPACE
				0x2003,// EM SPACE
				0x2004,// THREE-PER-EM SPACE
				0x2005,// FOUR-PER-EM SPACE
				0x2006,// SIX-PER-EM SPACE
				0x2007,// FIGURE SPACE
				0x2008,// PUNCTUATION SPACE
				0x2009,// THIN SPACE
				0x200A,// HAIR SPACE
				0x2028,// LS (LINE SEPARATOR)
				0x2029,// PS (PARAGRAPH SEPARATOR)
				0x202F,// NNBSP (NARROW NO-BREAK SPACE)
				0x205F,// MMSP (MEDIUM MATHEMATICAL SPACE)
				0x3000,// IDEOGRAPHIC SPACE
				0xFEFF// ZERO WIDTH NO-BREAK SPACE
 			};
			return x;
		}

		template<bool skipWhitespaceBetween>
		struct Sequence :
			public ParserBase
		{
			ParserPtr lhs;
			ParserPtr rhs;

			Sequence(const ParserBase& _lhs, const ParserBase& _rhs)
			{
				lhs.Assign(_lhs);
				rhs.Assign(_rhs);
			}

			virtual ParserBase* NewClone() const { return new Sequence(*this); }
			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"Sequence<%>").s(skipWhitespaceBetween ? L"skip" : L"no skip").Str();
			}

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
				if(skipWhitespaceBetween)
				{
					while(!input.IsEOF() && StringContains(WhitespaceChars(), input.CurrentChar()))
					{
						input.Advance();
					}
				}
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

		template<bool skipWhitespaceBetween>
		Sequence<skipWhitespaceBetween> Sequence3(const ParserBase& a, const ParserBase& b, const ParserBase& c)
		{
			return
				Sequence<skipWhitespaceBetween>(
					Sequence<skipWhitespaceBetween>(a,b),
					c);
		}

		template<bool skipWhitespaceBetween>
		Sequence<skipWhitespaceBetween> Sequence4(const ParserBase& a, const ParserBase& b, const ParserBase& c, const ParserBase& d)
		{
			return
				Sequence<skipWhitespaceBetween>(
					Sequence<skipWhitespaceBetween>(
						Sequence<skipWhitespaceBetween>(a,b),
						c),
					d);
		}

		struct Or :
			public ParserBase
		{
			ParserPtr lhs;
			ParserPtr rhs;

			Or(const ParserBase& _lhs, const ParserBase& _rhs)
			{
				lhs.Assign(_lhs);
				rhs.Assign(_rhs);
			}

			virtual ParserBase* NewClone() const { return new Or(*this); }
			virtual std::wstring GetParserName() const { return L"Or"; }

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

		Or Or3(const ParserBase& a, const ParserBase& b, const ParserBase& c)
		{
			return Or(Or(a,b),c);
		}

		Or Or4(const ParserBase& a, const ParserBase& b, const ParserBase& c, const ParserBase& d)
		{
			return Or(Or(Or(a,b),c),d);
		}

		struct Eof :
			public ParserBase
		{
			virtual ParserBase* NewClone() const { return new Eof(); }
			virtual std::wstring GetParserName() const { return L"EOF"; }

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
			//virtual Tin& Value() = 0; <-- if you need to set data, use Save().
			virtual const Tin& Value() const = 0;
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
			Tin dummy;
			void Save(const Tin& val) { }
			void SaveState() { }
			void RestoreState() { }
			//Tin& Value() { return dummy; }
			const Tin& Value() const { return dummy; }
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
			//Tin& Value() { return output; }
			const Tin& Value() const { return output; }
		};

		template<typename Tin>
		RefOutputT<Tin> RefOutput(Tin& outputVar)
		{
			return RefOutputT<Tin>(outputVar);
		}

		// ExistsOutput
		// result is actually thrown away, but if it's assigned at all, then the bool reference is set.
		// you need to use the template var for this. ExistsOutput<wchar_t>(b);
		template<typename Tin>
		struct ExistsOutput :
			public OutputBase<Tin>
		{
			bool& output;
			bool outputBackup;
			Tin dummy;

			ExistsOutput(bool& output_) :
				output(output_),
				outputBackup(output)
			{
			}
			void Save(const Tin& val)
			{
				output = true;
			}
			void SaveState() { outputBackup = output; }
			void RestoreState() { output = outputBackup; }
			OutputBase<Tin>* NewClone() const
			{
				return new ExistsOutput<Tin>(*this);
			}
			//Tin& Value() { return dummy; }
			const Tin& Value() const { return dummy; }
		};

		template<typename Tin>
		ExistsOutput<Tin> ExistsOutput2(const Tin& throwaway, bool& outputVar)
		{
			return ExistsOutput<Tin>(outputVar);
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
			Tin dummy;// InserterOutput cannot support Value().
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
			//Tin& Value() { return *outputInserterRef; }
			const Tin& Value() const
			{
				// throw an exception or something; this is not supported for inserteroutput.
				__asm int 3;
				return dummy;
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
				output.Assign(NullOutput<wchar_t>());
			}

			CharT(wchar_t match_) :
				match(match_)
			{
				output.Assign(NullOutput<wchar_t>());
			}

			CharT(const OutputPtr<wchar_t>& out) :
				match(0),
				output(out)
			{
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
			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"Char%('%')")
					.s(TCaseSensitive ? L"" : L"I")
					.c(match == 0 ? L'0' : match)
					.Str();
			}

			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
				{
					//result.Message(L"Unexpected end of file searching for character ...");
					return false;
				}

				parsed = input.CurrentChar();
				output->Save(parsed);
				input.Advance();

				if(match == 0)
				{
					return true;
				}

				if(match == parsed)
					return true;

				if(!TCaseSensitive)
				{
					if(CharToLower(parsed) == CharToLower(match))
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
			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"Str%(%)")
					.s(TCaseSensitive ? L"" : L"I")
					.qs(match)
					.Str();
			}

			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())
					return match.empty();

				std::wstring parsed;

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
					if(!TCaseSensitive && (CharToLower(ch) != CharToLower(*it)))
					{
						return false;
					}
					parsed.push_back(ch);
				}

				output->Save(parsed);
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
			virtual ParserBase* NewClone() const { return new Space(); }
			virtual std::wstring GetParserName() const { return L"Space"; }

			virtual void SaveOutputState() { }
			virtual void RestoreOutputState() { }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
					return false;
				parsed = input.CurrentChar();
				if(!StringContains(WhitespaceChars(), parsed))
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
				chars(chars_),
				output(output_)
			{
				if(!TCaseSensitive)
					chars = StringToLower(chars);
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
			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"CharOf%(%)")
					.s(TCaseSensitive ? L"" : L"I")
					.qs(chars)
					.Str();
			}
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
					return false;

				parsed = input.CurrentChar();
				if(TCaseSensitive && (std::wstring::npos == chars.find(parsed)))
				{
					return false;
				}
				if(!TCaseSensitive && (std::wstring::npos == chars.find(CharToLower(parsed))))
				{
					return false;
				}

				output->Save(parsed);
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
				upper(upper_)
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
			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"CharRange%('%'-'%')")
					.s(TCaseSensitive ? L"" : L"I")
					.c(lower)
					.c(upper)
					.Str();
			}
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
					return false;
				parsed = input.CurrentChar();
				if(TCaseSensitive && ((parsed < lower) || (parsed > upper)))
				{
					return false;
				}
				if(!TCaseSensitive)
				{
					wchar_t chL = CharToLower(parsed);
					if((chL < lower) || (chL > upper))// lower & upper are already lowercase
						return false;
				}

				output->Save(parsed);
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
			virtual ParserBase* NewClone() const { return new Eol(); }
			virtual std::wstring GetParserName() const { return L"EOL"; }

			virtual void SaveOutputState() { }
			virtual void RestoreOutputState() { }

			// basically we match \r, \n, or \r\n
			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				if(input.IsEOF())// count EOF as EOL
					return true;
				return (Or(Or(Str(L"\r\n"), Char('\r')), Char('\n'))).ParseRetainingStateOnError(result, input);
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
			virtual std::wstring GetParserName() const { return L"StringEscapeParser"; }

			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"StringEscapeParser\r\n"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				wchar_t escapeChar = 0;
				Parser p = Sequence<false>(Char('\\'), Char(0, escapeChar));
				if(!(p.ParseRetainingStateOnError(result, input)))
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
			StringParser() { output.Assign(NullOutput<std::wstring>()); }
			StringParser(const OutputPtr<std::wstring>& output_) : output(output_) { }
			StringParser(std::wstring& output_) { output.Assign(RefOutput(output_)); }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"StringParser\r\n"; }
			virtual ParserBase* NewClone() const { return new StringParser(*this); }
			virtual std::wstring GetParserName() const { return L"StringParser"; }

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				std::wstring parsed;
				// input from char and output to string outputer
				Parser noQuotes =Occurrences<1,false>(Sequence<false>(Not(Space()), Char(0, parsed)));

				Parser singleQuotes =
					Sequence3<false>
					(
						Char('\''),
						Occurrences<0, false>
						(
							Sequence<false>
							(
								Not(CharOf(L"\'\r\n")),
								Or
								(
									StringEscapeParser(CharToStringOutput(parsed)),
									Char(0, parsed)
								)
							)
						),
						Char('\'')
					);
				Parser doubleQuotes =
					Sequence3<false>
					(
						Char('\"'),
						Occurrences<0, false>
						(
							Sequence<false>
							(
								Not(CharOf(L"\"\r\n")),
								Or
								(
									StringEscapeParser(CharToStringOutput(parsed)),
									Char(0, parsed)
								)
							)
						),
						Char('\"')
					);
				Parser p = Passthrough(L"*StringParser", false, Or3(singleQuotes, doubleQuotes, noQuotes));
				bool ret = p.ParseRetainingStateOnError(result, input);
				if(ret)
					output->Save(parsed);
				return ret;
			}
		};

		template<typename Tch>
		inline int CharToDigit(Tch ch)
		{
			ch = CharToLower(ch);
			if(ch >= '0' && ch <= '9')
			{
				return ch - '0';
			}
			if(ch >= 'a' && ch <= 'z')
			{
				return 10 + ch - 'a';
			}
			return 0;
		}

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
			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UnsignedIntegerParser(base=%)").i(base).Str();
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

				Parser p = OneOrMore(CharOfI(digits, outputStr));
				if(!p.ParseRetainingStateOnError(result, input))
					return false;// no digits at all. maybe whitespace? or a non-digit char?

				// convert from outputStr to integer
				IntT out = 0;
				for(std::wstring::const_iterator it = outputStr.begin(); it != outputStr.end(); ++ it)
				{
					out *= base;
					out += CharToDigit(*it);
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerHexT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return Sequence(Str(L"0x"), UnsignedIntegerParser(16, output)).ParseRetainingStateOnError(result, input);
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerOctT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return
					Sequence3<false>
					(
						Char('0'),
						Not(Char('x')),
						UnsignedIntegerParser(8, output)
					)
					.ParseRetainingStateOnError(result, input);
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerDecT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return
					Sequence3<false>
					(
						Not(Char('0')),
						UnsignedIntegerParser(10, output),
						Not(Char('b'))
					).ParseRetainingStateOnError(result, input);
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerBinT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				return
					Sequence3<false>
					(
						Not(Char('0')),
						UnsignedIntegerParser(2, output),
						Char('b')
					).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT>
		UIntegerBinT<IntT> UIntegerBin(const OutputPtr<IntT>& output)
		{
			return UIntegerBinT<IntT>(output);
		}

		// SIntegerHexT (signed or unsigned 16-bit integer parser with prefix of 0x)
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerHexT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 16;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence3<false>
					(
						Optional(CharOf(L"+-", sign)),
						Str(L"0x"),
						UnsignedIntegerParserT<IntT>(base, RefOutput(temp))
					);
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

		// SIntegerOctT (signed or unsigned 8-bit integer parser with prefix of 0)
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerOctT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 8;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence4<false>
					(
						Optional(CharOf(L"+-", sign)),
						Char('0'),
						!Char('x'),
						UnsignedIntegerParserT<IntT>(base, RefOutput(temp))
					);
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

		// SIntegerBinT (signed or unsigned 2-bit integer parser with suffix of 'b')
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerBinT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 2;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence3<false>
					(
						Optional(CharOf(L"+-", sign)),
						UnsignedIntegerParserT<IntT>(base, RefOutput(temp)),
						Char('b')
					);
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

		// SIntegerDecT (signed or unsigned 10-bit integer parser)
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

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerDecT").Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 10;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence4<false>
					(
						Optional(CharOf(L"+-", sign)),
						Not(Char('0')),
						UnsignedIntegerParserT<IntT>(base, RefOutput(temp)),
						Not(Char('b'))
					);
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
		Parser CSignedInteger(const OutputPtr<IntT>& output)// these are all signed or unsigned.
		{
			return Or4(SIntegerDec(output), SIntegerOct(output), SIntegerBin(output), SIntegerHex(output));
		}

		template<typename IntT>
		Parser CUnsignedInteger(const OutputPtr<IntT>& output)
		{
			return Or4(UIntegerDec(output), UIntegerOct(output), UIntegerBin(output), UIntegerHex(output));
		}

		template<typename IntT>
		Parser CInteger(const OutputPtr<IntT>& output)// same as CSignedInteger
		{
			return Or4(SIntegerDec(output), SIntegerOct(output), SIntegerBin(output), SIntegerHex(output));
		}

		// if this is named CInteger, then the compiler gets it confused with the overload CInteger(const OutputPtr<IntT>& output)
		template<typename IntT>
		Parser CInteger2(IntT& output)
		{
			return Passthrough(L"CInteger2", false, CInteger<IntT>(RefOutput(output)));
		}

		// Floating point parser ///////////////////////////////////////////////////////////////////////////////////////
		// parses an unsigned rational number. the reason i don't add sign parsing here is because
		// some syntaxes might have prefixes like "-0x100" so I don't want to do the prefix parsing here. Just the
		// actual numeric part. So this parses a number in a pre-determined base.
		// bases which use 'e' as a digit character won't be able to use exponents
		template<typename T>
		struct UnsignedRationalParserT :
			public ParserBase
		{
			int base;
			std::wstring digits;
			OutputPtr<T> output;

			UnsignedRationalParserT() :
				base(10)
			{
				output.Assign(NullOutput<T>());
				InitDigits();
			}

			UnsignedRationalParserT(const OutputPtr<T>& output_) :
				output(output_),
				base(10)
			{
				InitDigits();
			}

			UnsignedRationalParserT(int base_) :
				base(base_)
			{
				output.Assign(NullOutput<T>());
				InitDigits();
			}

			UnsignedRationalParserT(const OutputPtr<T>& output_, int base_) :
				output(output_),
				base(base_)
			{
				InitDigits();
			}

			void InitDigits()
			{
				digits = std::wstring(L"0123456789abcdefghijklmnopqrstuvwxyz", base);
			}

			virtual ParserBase* NewClone() const { return new UnsignedRationalParserT<T>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UnsignedRationalParserT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UnsignedRationalParser(base=%)").i(base).Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				// 1234      1.234      1.234e-56        1e56     1.23e+45
				std::wstring preDecimalPart;
				std::wstring decimalPart;
				wchar_t exponentSign = L'+';
				std::wstring exponentPart;

				Parser preDecimalP = ZeroOrMore(CharOfI(digits, preDecimalPart));
				Parser decimalP = Sequence(Char('.'), ZeroOrMore(CharOfI(digits, decimalPart)));
				Parser exponentP =
					Sequence3<false>
					(
						CharI('e'),
						Optional(CharOf(L"+-", exponentSign)),
						OneOrMore(CharOfI(digits, exponentPart))
					);

				Parser rationalParserP = Sequence3(preDecimalP, Optional(decimalP), Optional(exponentP));
				
				if(!rationalParserP.ParseRetainingStateOnError(result, input))
					return false;

				// because basically ALL parts of the thing are optional, check if there's nothing of value.
				// this could also be done in the parser grammar but this is simpler looking.
				if(preDecimalPart.size() == 0 && decimalPart.size() == 0 && exponentPart.size() == 0)
					return false;

				// build the output.

				// pre-decimal
				T predecimal = 0;
				for(std::wstring::const_iterator it = preDecimalPart.begin(); it != preDecimalPart.end(); ++ it)
				{
					predecimal *= base;
					predecimal += CharToDigit(*it);
				}

				// post-decimal part.  .12     1 / base
				T decimal = 0;
				for(std::wstring::const_reverse_iterator it = decimalPart.rbegin(); it != decimalPart.rend(); ++ it)
				{
					decimal += CharToDigit(*it);
					decimal /= base;
				}

				// exponent part. TODO: is there a more efficient way of doing this???
				T res = (predecimal + decimal);
				if(preDecimalPart.size() > 0)
				{
					T exp = 0;
					for(std::wstring::const_iterator it = exponentPart.begin(); it != exponentPart.end(); ++ it)
					{
						exp *= base;
						exp += CharToDigit(*it);
					}
					if(exponentSign == L'+')
					{
						for(int i = 0; i < exp; ++i)
						{
							res *= base;
						}
					}
					else
					{
						for(int i = 0; i < exp; ++i)
						{
							res /= base;
						}
					}
				}

				output->Save(res);

				return true;
			}
		};

		// matches signed or unsigned rationals
		template<typename T>
		struct SignedRationalParserT :
			public ParserBase
		{
			int base;
			OutputPtr<T> output;

			SignedRationalParserT() :
				base(10)
			{
				output.Assign(NullOutput<T>());
			}

			SignedRationalParserT(const OutputPtr<T>& output_) :
				output(output_),
				base(10)
			{
			}

			SignedRationalParserT(int base_) :
				base(base_)
			{
				output.Assign(NullOutput<T>());
			}

			SignedRationalParserT(const OutputPtr<T>& output_, int base_) :
				output(output_),
				base(base_)
			{
			}

			virtual ParserBase* NewClone() const { return new SignedRationalParserT<T>(*this); }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"SignedRationalParserT\r\n"; }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SignedRationalParser(base=%)").i(base).Str();
			}

			virtual bool Parse(ScriptResult& result, ScriptReader& input)
			{
				T temp;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence
					(
						Optional(CharOf(L"+-", sign)),
						UnsignedRationalParserT<T>(RefOutput(temp), base)
					);
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output->Save(sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename T>
		SignedRationalParserT<T> Rational(const OutputPtr<T>& output)
		{
			return SignedRationalParserT<T>(output);
		}

		template<typename T>
		SignedRationalParserT<T> Rational2(T& output)
		{
			return SignedRationalParserT<T>(RefOutput(output));
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

			virtual const std::wstring& GetRawInput() const
			{
				return m_script;
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

		// small utility base class when you need to make your own custom parsers
		// - no Dump()
		// - assumes a single output var called "output" (optional to use it though)
		// - assumes copy constructor on clone
		// so you just need to derive like struct MyIntegerParser : ParserWithOutput<int, MyIntegerParser>
		// and implement constructors and bool Parse(ScriptResult& result, ScriptReader& input)
		template<typename Toutput, typename Tderived>
		struct ParserWithOutput :
			public ParserBase
		{
			OutputPtr<Toutput> output;
			virtual ParserBase* NewClone() const { return new Tderived(*(Tderived*)this); }
			virtual void SaveOutputState() { output->SaveState(); }
			virtual void RestoreOutputState() { output->RestoreState(); }
		};



		// put operators all together, at the end, for a couple reasons:
		// - operators may change. for this reason, i don't want to use operators in the parser framework. thus, i put them here at the end.
		// - you might want to turn them off, do a build, and look at compiler errors in order to detect all occurrences of them
		//   in your own code, because you can't just do a ctrl+f for them.
#if 1
		/*
			unary operators: http://msdn.microsoft.com/en-us/library/f672kxz8.aspx
			binary operators: http://msdn.microsoft.com/en-us/library/czs2584d.aspx

			Operators as defined both by SP1R1T and LibCC::Parse (without whitespace skipping)
			x >> y	Match x followed by y (Sequence)
			x | y	Match x or y (Or)
			*x	Match x zero or more times (ZeroOrMore)
			+x	Match x one or more times (OneOrMore)
			!x	Match x zero or one time (Optional)

			My own operators:
			x + y Match x followed by y, skipping whitespace (Sequence)
			&x zero or more, skipping whitespace (ZeroOrMoreS)
			++x one or more, skipping whitespace (OneOrMoreS)
			-x do not match x (Not)

			SP1R1T operators that i don't support:
			x & y	Match x and y
			x – y	Match x but not y
			x ^ y	Match x or y but not both
			x [ function expression ]	Execute the function/functor if x is matched
			x % y	Match x one or more times, separated by occurrences of y
		*/
		ZeroOrMore operator * (const ParserBase& lhs)
		{
			return ZeroOrMore(lhs);
		}

		ZeroOrMoreS operator & (const ParserBase& lhs)
		{
			return ZeroOrMoreS(lhs);
		}

		OneOrMore operator + (const ParserBase& lhs)
		{
			return OneOrMore(lhs);
		}

		OneOrMoreS operator ++ (const ParserBase& lhs)
		{
			return OneOrMoreS(lhs);
		}

		Not operator - (const ParserBase& lhs)
		{
			return Not(lhs);
		}

		Optional operator ! (const ParserBase& lhs)
		{
			return Optional(lhs);
		}

		Sequence<false> operator >> (const ParserBase& lhs, const ParserBase& rhs)
		{
			return Sequence<false>(lhs, rhs);
		}

		Sequence<true> operator + (const ParserBase& lhs, const ParserBase& rhs)
		{
			return Sequence<true>(lhs, rhs);
		}

		Or operator | (const ParserBase& lhs, const ParserBase& rhs)
		{
			return Or(lhs, rhs);
		}
#endif
	}
}
