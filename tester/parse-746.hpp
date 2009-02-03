/*
	THIS IS ADAPTED FROM REVISION 746 FROM FEB 2 2009
	OF PARSE.HPP FOR COMPARISON WITH THE LATEST VERSION.
*/

#pragma once

#include <vector>
#include <stack>

namespace LibCC_746
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

			// for measuring the range of many operations. this is used by Or() to determine which parser got further in the script,
			// to make a best guess at which path is what the user intended.
			// this will begin a measurement operation. use in conjunction with PopMeasure().
			virtual void PushMeasure() = 0;

			// after calling PushMeasure(), use this to end the measurement operation. it returns the maximum span that the cursor traveled during the measurement.
			virtual int PopMeasure() = 0;
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
				int oldPos = m_cursor.pos;
				if(oldPos > c.pos)
					UpdateMeasurements();
				m_cursor = c;
				if(oldPos < c.pos)
					UpdateMeasurements();
			}

			virtual const std::wstring& GetRawInput() const
			{
				return m_script;
			}

			// our "stack" of measurement ops.
			// the pair is position_low, position_high. every time the cursor moves, every item on this stack gets updated.
			// TOP of the stack is back().
			std::vector<std::pair<int, int> > m_measurements;

			virtual void PushMeasure()
			{
				m_measurements.push_back(std::pair<int, int>(m_cursor.pos, m_cursor.pos));
			}

			// after calling PushMeasure(), use this to end the measurement operation. it returns the maximum span that the cursor traveled during the measurement.
			virtual int PopMeasure()
			{
				UpdateMeasurements();
				int ret = m_measurements.back().second - m_measurements.back().first;
				m_measurements.pop_back();
				return ret;
			}

		private:
			void UpdateMeasurements()
			{
				for(std::vector<std::pair<int, int> >::iterator it = m_measurements.begin(); it != m_measurements.end(); ++ it)
				{
					it->second = max(it->second, m_cursor.pos);
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


		inline std::wstring _DebugSanitize(const std::wstring& in)
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
		inline std::wstring _DebugCursorToString(const ScriptCursor& cur, const ScriptReader& input)
		{
			const int amount = 6;
			const std::wstring& script(input.GetRawInput());
			int nleft = min(cur.pos, amount);
			std::wstring left = script.substr(cur.pos - nleft, nleft);
			int nright = (int)script.size() - cur.pos;
			nright = min(nright, amount);
			std::wstring right = script.substr(cur.pos, nright);
			left = _DebugSanitize(left);
			right = _DebugSanitize(right);

			return LibCC::FormatW(L"Pos:%(%,%) \"%^|%\"")
				.i(cur.pos)
				.i(cur.line)
				.i(cur.column)
				.s(left)
				.s(right)
				.Str();
		}

		// [Abinooeeueue...oooeueeee]
		inline std::wstring _DebugSubStr(const ScriptCursor& leftCursor, const ScriptCursor& rightCursor, ScriptReader& input)
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

			left = _DebugSanitize(left);
			right = _DebugSanitize(right);

			return LibCC::FormatW(L"[%%%]")
				.s(left)
				.s(useEllipses ? ellipses : L"")
				.s(right)
				.Str();
		}

		// basic framework parsers ///////////////////////////////////////////////////////////////////////////////////////
		struct ParseResult
		{
			virtual void IncrementTraceIndent() = 0;
			virtual void DecrementTraceIndent() = 0;
			virtual void SetTraceEnabled(bool b) = 0;
			virtual bool IsTraceEnabled() const = 0;
			virtual void Trace(const std::wstring& msg) = 0;
			virtual void RenderMessage(const std::wstring& msg) = 0;

			void ParserMessage(const std::wstring& msg)
			{
				if(m_messagingStack.size() > 0)
				{
					m_messagingStack.top()->push_back(msg);
				}
				else
				{
					RenderMessage(msg);
				}
			}

			void ParserMessage(const std::wstring& msg, const ScriptCursor& pos, const ScriptReader& input)
			{
				std::wstring msg2 = LibCC::FormatW(L"%, at %")
					(msg)
					(_DebugCursorToString(pos, input))
					.Str();
				ParserMessage(msg2);
			}

			// this should suppress output but continue to accept messages. so, they should be put in staging.
			virtual void PushParserMessaging(std::vector<std::wstring>* receiver)
			{
				m_messagingStack.push(receiver);
			}

			// after PushParserMessaging(), call this. if emitMessages == true then the messages
			// that have been generated since the corresponding Push() are used. if false, then they are thrown away.
			virtual void PopParserMessaging(bool emitMessages)
			{
				PopParserMessaging(emitMessages, *m_messagingStack.top());
			}

			// allows you to override the messages to be emitted. Useful if you need to maybe get a few sets of
			// error messagse, then decide later which messagse need to be emitted. Or() uses this.
			virtual void PopParserMessaging(bool emitMessages, std::vector<std::wstring>& poppedMessages)
			{
				m_messagingStack.pop();

				if(!emitMessages)
					return;

				// there are still other messages
				for(std::vector<std::wstring>::iterator it = poppedMessages.begin(); it != poppedMessages.end(); ++ it)
				{
					ParserMessage(*it);
				}
			}

		private:
			std::stack<std::vector<std::wstring>* > m_messagingStack;
		};

		// built in ParseResult class which holds all messages in memory.
		struct ParseResultMem :
			ParseResult
		{
			int traceIndentLevel;
			bool traceEnabled;

			ParseResultMem() :
				traceIndentLevel(0),
				traceEnabled(false)
			{
			}
			virtual void IncrementTraceIndent() { ++traceIndentLevel; }
			virtual void DecrementTraceIndent() { --traceIndentLevel; }
			virtual void SetTraceEnabled(bool b) { traceEnabled = b; }
			virtual bool IsTraceEnabled() const { return traceEnabled; }

			// parse debugging trace messages
			std::vector<std::wstring> trace;

			virtual void Trace(const std::wstring& msg)
			{
				std::wstring x;
				for(int i = 0; i < traceIndentLevel; ++ i)
				{
					x.append(L"  ");
				}
				x.append(msg);
				trace.push_back(x);
			}

			// parser evaluation messages
			std::vector<std::wstring> parseMessages;

			virtual void RenderMessage(const std::wstring& msg)
			{
				parseMessages.push_back(msg);
			}
		};

		// Parser base classes ///////////////////////////////////////////////////////////////////////////////////////


		struct ParserRoot
		{
			bool IsTraceEnabled;

			ParserRoot() :
				IsTraceEnabled(true)
			{
			}

			virtual ~ParserRoot()
			{
			}

			virtual bool ParseRetainingStateOnError(ParseResult& result, ScriptReader& input)
			{
				ScriptCursor oldCursor = input.GetCursorCopy();
				SaveOutputState(input);

				if(result.IsTraceEnabled() && IsTraceEnabled)
				{
					result.Trace(LibCC::FormatW(L"Parsing '%' from %")
						(GetParserName())
						(_DebugCursorToString(oldCursor, input))
						.Str());
					result.Trace(L"{");
					result.IncrementTraceIndent();
				}

				bool wasTracingEnabled = result.IsTraceEnabled();
				if(!IsTraceEnabled)
				{
					result.SetTraceEnabled(false);
				}

				bool ret = Parse(result, input);
				result.SetTraceEnabled(wasTracingEnabled);

				if(!ret)
				{
					if(result.IsTraceEnabled() && IsTraceEnabled)
					{
						ScriptCursor newCursor = input.GetCursorCopy();
						result.Trace(LibCC::FormatW(L"=false '%' from % to % %")
							(GetParserName())
							(_DebugCursorToString(oldCursor, input))
							(_DebugCursorToString(newCursor, input))
							(_DebugSubStr(oldCursor, newCursor, input))
							.Str());
						result.DecrementTraceIndent();
						result.Trace(L"}");
					}
					RestoreOutputState(input);
					input.SetCursor(oldCursor);
					return false;
				}

				if(result.IsTraceEnabled() && IsTraceEnabled)
				{
					ScriptCursor newCursor = input.GetCursorCopy();
					result.Trace(LibCC::FormatW(L"=TRUE '%' from % to % %")
						(GetParserName())
						(_DebugCursorToString(oldCursor, input))
						(_DebugCursorToString(newCursor, input))
						(_DebugSubStr(oldCursor, newCursor, input))
						.Str());
					result.DecrementTraceIndent();
					result.Trace(L"}");
				}
				return true;
			}

			virtual ParserRoot* NewClone() const = 0;
			virtual void Delete(ParserRoot*) const = 0;

			virtual void SaveOutputState(ScriptReader& input) = 0;
			virtual void RestoreOutputState(ScriptReader& input) = 0;

			virtual bool Parse(ParseResult& result, ScriptReader& input) = 0;

			// useful for simple parsing stuff like CInteger(RefOutput(myint)).Parse(L"-1");
			virtual bool ParseSimple(const std::wstring& script)
			{
				ParseResultMem result;
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


		template<typename Tderived>
		struct ParserBase :
			public ParserRoot
		{
			typedef Tderived Derived;

			virtual ~ParserBase() { }

			virtual ParserRoot* NewClone() const
			{
				return new Tderived(*(Tderived*)this);
			}

			virtual void Delete(ParserRoot* ptr) const
			{
				delete (Tderived*)ptr;
			}
		};

		// basic framework parsers ///////////////////////////////////////////////////////////////////////////////////////

		template<typename Tchild>
		struct PassthroughT :
			public ParserBase<PassthroughT<Tchild> >
		{
		private:
			Tchild m_child;

		public:
			std::wstring falseMessage;
			std::wstring trueMessage;

			PassthroughT(const Tchild& rhs, const std::wstring trueMessage_, const std::wstring falseMessage_) :
				m_child(rhs),
				trueMessage(trueMessage_),
				falseMessage(falseMessage_)
			{
			}

			virtual void SaveOutputState(ScriptReader& input)
			{
				m_child.SaveOutputState(input);
			}

			virtual void RestoreOutputState(ScriptReader& input)
			{
				m_child.RestoreOutputState(input);
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				bool r = m_child.ParseRetainingStateOnError(result, input);
				if(r && !trueMessage.empty())
					result.ParserMessage(trueMessage, input.GetCursorCopy(), input);
				if(!r && !falseMessage.empty())
				result.ParserMessage(falseMessage, input.GetCursorCopy(), input);
				return r;
			}
		};

		template<typename Tchild>
		inline PassthroughT<Tchild> Passthrough(const Tchild& child)
		{
			return PassthroughT<Tchild>(child, L"", L"");
		}

		template<typename Tchild>
		inline PassthroughT<Tchild> FalseMsg(const std::wstring& msg, const Tchild& child)
		{
			return PassthroughT<Tchild>(child, L"", msg);
		}

		template<typename Tchild>
		inline PassthroughT<Tchild> TrueMsg(const std::wstring& msg, const Tchild& child)
		{
			return PassthroughT<Tchild>(child, msg, L"");
		}

		// basically this is a smart ptr for parsers allowing you to do Parser p = <any kind of parser here>;
		// to assign a parser to a variable. Causes a new/copy/delete though so don't do this when it could
		// be passed a million times.
		struct Parser :
			public ParserBase<Parser>
		{
			ParserRoot* m_child;

			Parser() :
				m_child(0)
			{
			}

			Parser(const Parser& rhs) :
				m_child(0)
			{
				if(rhs.m_child == 0)
					return;
				m_child = rhs.m_child->NewClone();
			}

			Parser(const ParserRoot& rhs)
			{
				m_child = rhs.NewClone();
			}

			Parser& operator = (const Parser& rhs)
			{
				Release();
				if(rhs.m_child != 0)
				{
					m_child = rhs.m_child->NewClone();
				}
				return *this;
			}

			Parser& operator = (const ParserRoot& rhs)
			{
				Release();
				m_child = rhs.NewClone();
				return *this;
			}

			~Parser()
			{
				Release();
			}

			void Release()
			{
				if(!m_child)
					return;
				m_child->Delete(m_child);
				m_child = 0;
			}

			virtual void SaveOutputState(ScriptReader& input)
			{
				if(!m_child)
					return;
				m_child->SaveOutputState(input);
			}

			virtual void RestoreOutputState(ScriptReader& input)
			{
				if(!m_child)
					return;
				m_child->RestoreOutputState(input);
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				if(!m_child)
					return true;
				return m_child->ParseRetainingStateOnError(result, input);
			}
		};

		// sentinel so that parsers like SequenceT or OrT don't have to always check if the child is valid.
		struct NothingParser :
			public ParserBase<Parser>
		{
			virtual void SaveOutputState(ScriptReader& input) { }
			virtual void RestoreOutputState(ScriptReader& input) { }
			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				return true;
			}
		};

		// acts as a breakpoint during the parsing process.
		template<typename Tchild>
		struct BreakT :
			public ParserBase<BreakT<Tchild> >
		{
		private:
			Tchild m_child;

		public:
			BreakT(const Tchild& rhs) :
				m_child(rhs)
			{
			}

			virtual void SaveOutputState(ScriptReader& input) { m_child.SaveOutputState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { m_child.RestoreOutputState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				ScriptCursor oldCursor = input.GetCursorCopy();
				std::wstring oldContext = _DebugCursorToString(oldCursor, input);
				//__asm int 3;
				if(m_child.IsEmpty())
				{
					return true;
				}
				bool r = m_child->ParseRetainingStateOnError(result, input);
				ScriptCursor newCursor = input.GetCursorCopy();
				std::wstring newContext = _DebugCursorToString(newCursor, input);
				return r;
			}
		};

		template<typename Tchild>
		BreakT<Tchild> Break(const Tchild& child)
		{
			return BreakT<Tchild>(child);
		}


		struct ParseMsg :
			public ParserBase<ParseMsg>
		{
			std::wstring msg;
			bool ret;

			ParseMsg(const std::wstring& msg_, bool ret_ = false) :
				msg(msg_),
				ret(ret_)
			{
			}

			virtual void SaveOutputState(ScriptReader& input) { }
			virtual void RestoreOutputState(ScriptReader& input) { }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				if(!msg.empty())
					result.ParserMessage(msg, input.GetCursorCopy(), input);
				return ret;
			}
		};


		template<typename Tchild, int MinimumOccurrences, bool skipWhitespaceBetween, bool suppressParseMessages = false>
		struct OccurrencesT :
			public ParserBase<OccurrencesT<Tchild, MinimumOccurrences, skipWhitespaceBetween, suppressParseMessages> >
		{
		private:
			Tchild m_child;

		public:
			OccurrencesT(const Tchild& rhs) :
					m_child(rhs)
			{
			}

			virtual void SaveOutputState(ScriptReader& input) { m_child.SaveOutputState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { m_child.RestoreOutputState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				int count = 0;
				
				while(true)
				{
					int oldCursor = input.GetCursorCopy().pos;
					std::vector<std::wstring> parserMessages;
					result.PushParserMessaging(&parserMessages);
					if(!m_child.ParseRetainingStateOnError(result, input))
					{
						if(count < MinimumOccurrences)
						{
							//result.Message(L"Not enough occurrences.");
							result.PopParserMessaging(true);// output error messages.
							return false;
						}
						else
						{
							result.PopParserMessaging(!suppressParseMessages);// don't output errors; it was just a trial.
							return true;
						}
					}
					result.PopParserMessaging(true);
					count ++;
					// if it succeeded but the cursor is in the same place, then we're headed for an endless loop.
					if(oldCursor == input.GetCursorCopy().pos)
					{
						__asm int 3;// TODO: some kind of cool error handling.
						result.PopParserMessaging(true);
						return false;
					}

					if(skipWhitespaceBetween)
					{
						while(!input.IsEOF() && LibCC::StringContains(WhitespaceChars(), input.CurrentChar()))
						{
							input.Advance();
						}
					}
				}
			}
		};

		template<int MinimumOccurrences, bool skipWhitespaceBetween, typename Tchild>
		OccurrencesT<Tchild, MinimumOccurrences, skipWhitespaceBetween, false> Occurrences(const Tchild& ch)
		{
			return OccurrencesT<Tchild, MinimumOccurrences, skipWhitespaceBetween, false>(ch);
		}

		template<int MinimumOccurrences, bool skipWhitespaceBetween, bool suppressParseMessages, typename Tchild>
		OccurrencesT<Tchild, MinimumOccurrences, skipWhitespaceBetween, suppressParseMessages> Occurrences(const Tchild& ch)
		{
			return OccurrencesT<Tchild, MinimumOccurrences, skipWhitespaceBetween, suppressParseMessages>(ch);
		}

		template<typename Tchild>
		OccurrencesT<Tchild, 0, false> ZeroOrMore(const Tchild& ch)
		{
			return OccurrencesT<Tchild, 0, false>(ch);
		}

		template<typename Tchild>
		OccurrencesT<Tchild, 0, true> ZeroOrMoreS(const Tchild& ch)
		{
			return OccurrencesT<Tchild, 0, true>(ch);
		}

		template<typename Tchild>
		OccurrencesT<Tchild, 1, false> OneOrMore(const Tchild& ch)
		{
			return OccurrencesT<Tchild, 1, false>(ch);
		}

		template<typename Tchild>
		OccurrencesT<Tchild, 1, true> OneOrMoreS(const Tchild& ch)
		{
			return OccurrencesT<Tchild, 1, true>(ch);
		}


		template<typename Tchild>
		struct NotT :
			public ParserBase<NotT<Tchild> >
		{
		private:
			Tchild m_child;

		public:
			NotT(const Tchild& rhs) :
					m_child(rhs)
			{
			}

			virtual void SaveOutputState(ScriptReader& input) { m_child.SaveOutputState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { m_child.RestoreOutputState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				std::vector<std::wstring> parseMessages;
				result.PushParserMessaging(&parseMessages);
				bool r = m_child.ParseRetainingStateOnError(result, input);
				result.PopParserMessaging(false);// never output messaging from INSIDE m_child. it won't make sense because it will all be negative to the actual interpretation.
				return !r;// <-- this is the NOT.
			}
		};

		template<typename Tchild>
		NotT<Tchild> Not(const Tchild& child)
		{
			return NotT<Tchild>(child);
		}


		template<typename Tchild>
		struct OptionalT :
			public ParserBase<OptionalT<Tchild> >
		{
		private:
			Tchild m_child;

		public:
			OptionalT(const Tchild& rhs) :
				m_child(rhs)
			{
			}

			virtual void SaveOutputState(ScriptReader& input) { m_child.SaveOutputState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { m_child.RestoreOutputState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				// if the child fails to parse, then don't emit error messages. it was optional.
				std::vector<std::wstring> parseMessages;
				result.PushParserMessaging(&parseMessages);
				bool r = m_child.ParseRetainingStateOnError(result, input);
				result.PopParserMessaging(r);
				return true;
			}
		};

		template<typename Tchild>
		OptionalT<Tchild> Optional(const Tchild& child)
		{
			return OptionalT<Tchild>(child);
		}


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

		template<bool skipWhitespaceBetween, typename Tlhs, typename Trhs>
		struct SequenceT :
			public ParserBase<SequenceT<skipWhitespaceBetween, Tlhs, Trhs> >
		{
			Tlhs lhs;
			Trhs rhs;

			SequenceT(const Tlhs& _lhs, const Trhs& _rhs) :
				lhs(_lhs),
				rhs(_rhs)
			{
			}

			virtual void SaveOutputState(ScriptReader& input)
			{
				lhs.SaveOutputState(input);
				rhs.SaveOutputState(input);
			}

			virtual void RestoreOutputState(ScriptReader& input)
			{
				lhs.RestoreOutputState(input);
				rhs.RestoreOutputState(input);
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				if(!lhs.ParseRetainingStateOnError(result, input))
					return false;
				if(skipWhitespaceBetween)
				{
					while(!input.IsEOF() && LibCC::StringContains(WhitespaceChars(), input.CurrentChar()))
					{
						input.Advance();
					}
				}
				if(!rhs.ParseRetainingStateOnError(result, input))
					return false;
				return true;
			}
		};

		template<bool skipWhitespaceBetween, typename Ta, typename Tb>
		SequenceT<skipWhitespaceBetween, Ta, Tb> Sequence(const Ta& a, const Tb& b)
		{
			return SequenceT<skipWhitespaceBetween, Ta, Tb>(a,b);
		}

		template<bool skipWhitespaceBetween, typename Ta, typename Tb, typename Tc>
		SequenceT<skipWhitespaceBetween, Ta, SequenceT<skipWhitespaceBetween, Tb, Tc> >
			Sequence(const Ta& a, const Tb& b, const Tc& c)
		{
			return
				Sequence<skipWhitespaceBetween>(
					a, Sequence<skipWhitespaceBetween>(b,c));
		}

		template<bool skipWhitespaceBetween, typename Ta, typename Tb, typename Tc, typename Td>
		SequenceT<skipWhitespaceBetween, Ta, SequenceT<skipWhitespaceBetween, Tb, SequenceT<skipWhitespaceBetween, Tc, Td> > >
			Sequence(const Ta& a, const Tb& b, const Tc& c, const Td& d)
		{
			return
				Sequence<skipWhitespaceBetween>(
					a, Sequence<skipWhitespaceBetween>(
						b, Sequence<skipWhitespaceBetween>(c,d)));
		}

		template<bool skipWhitespaceBetween, typename Ta, typename Tb, typename Tc, typename Td, typename Te>
		SequenceT<skipWhitespaceBetween, Ta, SequenceT<skipWhitespaceBetween, Tb, SequenceT<skipWhitespaceBetween, Tc, SequenceT<skipWhitespaceBetween, Td, Te> > > >
			Sequence(const Ta& a, const Tb& b, const Tc& c, const Td& d, const Te& e)
		{
			return
				Sequence<skipWhitespaceBetween>(
					a, Sequence<skipWhitespaceBetween>(
						b, Sequence<skipWhitespaceBetween>(
							c, Sequence<skipWhitespaceBetween>(d,e))));
		}

		template<typename Tlhs, typename Trhs>
		struct OrT :
			public ParserBase<OrT<Tlhs, Trhs> >
		{
			Tlhs lhs;
			Trhs rhs;

			OrT(const Tlhs& _lhs, const Trhs& _rhs) :
				lhs(_lhs),
				rhs(_rhs)
			{
			}

			virtual void SaveOutputState(ScriptReader& input)
			{
				lhs.SaveOutputState(input);
				rhs.SaveOutputState(input);
			}

			virtual void RestoreOutputState(ScriptReader& input)
			{
				lhs.RestoreOutputState(input);
				rhs.RestoreOutputState(input);
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				// this will suppress parse messages for parse paths that are just trials basically.
				// if both children fail, you will only get messaging from the rhs.
				std::vector<std::wstring> lhsParseMessages;
				result.PushParserMessaging(&lhsParseMessages);
				input.PushMeasure();
				if(lhs.ParseRetainingStateOnError(result, input))
				{
					input.PopMeasure();
					result.PopParserMessaging(true);
					return true;
				}
				int lhsDistance = input.PopMeasure();
				result.PopParserMessaging(false);// it failed to parse. but we have another try so don't output the errors yet... ideally here we would take the messages from the child which made it further. but i don't have a way yet to determine that.

				std::vector<std::wstring> rhsParseMessages;
				result.PushParserMessaging(&rhsParseMessages);
				input.PushMeasure();
				if(rhs.ParseRetainingStateOnError(result, input))
				{
					input.PopMeasure();
					result.PopParserMessaging(true);
					return true;
				}
				int rhsDistance = input.PopMeasure();

				if(rhsDistance >= lhsDistance)
				{
					// right-hand side wins the right to display its error messages.
					result.PopParserMessaging(true);
				}
				else
				{
					result.PopParserMessaging(true, lhsParseMessages);
				}

				return false;
			}
		};

		// or, or3, or4

		template<typename Ta, typename Tb>
		OrT<Ta, Tb> Or(const Ta& a, const Tb& b)
		{
			return OrT<Ta, Tb>(a,b);
		}

		template<typename Ta, typename Tb, typename Tc>
		OrT<Ta, OrT<Tb, Tc> >
			Or(const Ta& a, const Tb& b, const Tc& c)
		{
			return Or(a, Or(b,c));
		}

		template<typename Ta, typename Tb, typename Tc, typename Td>
		OrT<Ta, OrT<Tb, OrT<Tc, Td> > >
			Or(const Ta& a, const Tb& b, const Tc& c, const Td& d)
		{
			return Or(a, Or(b, Or(c, d)));
		}


		struct Eof :
			public ParserBase<Eof>
		{
			virtual void SaveOutputState(ScriptReader& input) { }
			virtual void RestoreOutputState(ScriptReader& input) { }
			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				return input.IsEOF();
			}
		};


		// storage engines ///////////////////////////////////////////////////////////////////////////////////////


		// NullOutput
		// output is not saved anywhere
		template<typename Tin>
		struct NullOutput
		{
			Tin dummy;
			void Save(ScriptReader& input, const Tin& val) { }
			void SaveState(ScriptReader& input) { }
			void RestoreState(ScriptReader& input) { }
			const Tin& Value() const { return dummy; }
		};

		// RefOutput
		// wchar_t ch; Parser p = CharOf(L"abc", RefOutput(ch));
		// output is stored in a reference passed in by the caller
		template<typename Tin>
		struct RefOutputT
		{
			Tin& output;
			Tin outputBackup;
			RefOutputT(Tin& output_) :
				output(output_)
			{
			}
			void Save(ScriptReader& input, const Tin& val)
			{
				output = val;
			}
			void SaveState(ScriptReader& input) { outputBackup = output; }
			void RestoreState(ScriptReader& input) { output = outputBackup; }
			const Tin& Value() const { return output; }
		};

		template<typename Tin>
		inline RefOutputT<Tin> RefOutput(Tin& outputVar)
		{
			return RefOutputT<Tin>(outputVar);
		}

		// ExistsOutput
		// result is actually thrown away, but if it's assigned at all, then the bool reference is set.
		// you need to use the template var for this. ExistsOutput<wchar_t>(b);
		template<typename Tin, typename Tout>
		struct ExistsOutputT
		{
			Tout valToSet;
			Tout& output;
			Tout outputBackup;
			Tin dummy;

			ExistsOutputT(Tout& output_, const Tout& valToSet_) :
				output(output_),
				outputBackup(output),
				valToSet(valToSet_)
			{
			}

			void Save(ScriptReader& input, const Tin& val)
			{
				output = valToSet;
			}
			void SaveState(ScriptReader& input) { outputBackup = output; }
			void RestoreState(ScriptReader& input) { output = outputBackup; }
			const Tin& Value() const { return dummy; }
		};

		template<typename Tin, typename Tout>
		ExistsOutputT<Tin, Tout> ExistsOutput(const Tin& throwaway, Tout& outputVar, const Tout& valToSet)
		{
			return ExistsOutputT<Tin, Tout>(outputVar, valToSet);
		}

		template<typename Tin>
		ExistsOutputT<Tin, bool> ExistsOutput(const Tin& throwaway, bool& outputVar)
		{
			return ExistsOutputT<Tin, bool>(outputVar, true);
		}

		template<typename Tin>
		ExistsOutputT<Tin, bool> ExistsOutput(const Tin& throwaway, bool& outputVar, bool valToSet)
		{
			return ExistsOutputT<Tin, bool>(outputVar, valToSet);
		}

		// InserterOutput
		// std::list<wchar_t> l;   Parser p = *CharOf(L"abc", InserterOutput<wchar_t>(l, std::back_inserter(l)));
		// output is stored in a reference passed in by the caller
		//
		// NOTE: i am not 100% certain that the inserter iterator is always valid after restoring state.
		template<typename Tin, typename Tinserter, typename Tcontainer>
		struct InserterOutputT
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
			void Save(ScriptReader& input, const Tin& val)
			{
				*outputInserter = val;
				++outputInserter;
			}
			void SaveState(ScriptReader& input) { outputContainerBackup = outputContainer; }
			void RestoreState(ScriptReader& input) { outputContainer = outputContainerBackup; }
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
		InserterOutputT<Tin, std::back_insert_iterator<Tcontainer>, Tcontainer> BackInserterOutput(Tcontainer& outputVar)
		{
			return InserterOutput<Tin>(outputVar, std::back_inserter(outputVar));
		}

		template<typename Tin>
		InserterOutputT<Tin, std::back_insert_iterator<std::vector<Tin> >, std::vector<Tin>> VectorOutput(std::vector<Tin>& outputVar)
		{
			return InserterOutput<Tin>(outputVar, std::back_inserter(outputVar));
		}

		template<typename Tin>
		InserterOutputT<Tin, std::back_insert_iterator<std::basic_string<Tin> >, std::basic_string<Tin>> CharToStringOutput(std::basic_string<Tin>& outputVar)
		{
			return InserterOutput<Tin>(outputVar, std::back_inserter(outputVar));
		}

		// TeeOutput(ExistsOutput(), SomethingElse())
		// branches output into two
		template<typename Tin, typename ToutputA, typename ToutputB>
		struct TeeOutputT
		{
			ToutputA outputA;
			ToutputB outputB;

			TeeOutputT(const ToutputA& outputA_, const ToutputB& outputB_) :
				outputA(outputA_),
				outputB(outputB_)
			{
			}

			const Tin& Value() const { return outputA->Value(); }

			void Save(ScriptReader& input, const Tin& val)
			{
				outputA.Save(input, val);
				outputB.Save(input, val);
			}
			void SaveState(ScriptReader& input)
			{
				outputA.SaveState(input);
				outputB.SaveState(input);
			}
			void RestoreState(ScriptReader& input)
			{
				outputA.RestoreState(input);
				outputB.RestoreState(input);
			}
		};

		template<typename Tin, typename ToutputA, typename ToutputB>
		TeeOutputT<Tin, ToutputA, ToutputB> TeeOutput(const ToutputA& a, const ToutputB& b)
		{
			return TeeOutputT<Tin, ToutputA, ToutputB>(a, b);
		}

		// ContextOutput - puts script cursor context somewhere. only saved if the parse was successful (on Save()).
		template<typename Tin, typename Toutput>
		struct ContextOutputT
		{
			Toutput output;
			ScriptCursor cursorToUse;
			Tin dummy;

			ContextOutputT(const Toutput& output_) :
				output(output_)
			{
			}

			void Save(ScriptReader& input, const Tin& val)
			{
				output.Save(input, cursorToUse);
			}
			void SaveState(ScriptReader& input)
			{
				// i know that if we're saving state then the script is at the BEGINNING of this parse. so use THIS, not the 
				// position given in Save().
				cursorToUse = input.GetCursorCopy();

				output.SaveState(input);
			}
			void RestoreState(ScriptReader& input) { output.RestoreState(input); }
			const Tin& Value() const { return dummy; }
		};

		// converts in type to an output that accepts ScriptCursor
		template<typename Tin, typename Toutput>
		ContextOutputT<Tin, Toutput> ContextOutput(const Tin& throwaway, const Toutput& output_)
		{
			return ContextOutputT<Tin, Toutput>(output_);
		}

		template<typename Tin>
		ContextOutputT<Tin, RefOutputT<ScriptCursor> > ContextOutput(const Tin& throwaway, ScriptCursor& output_)
		{
			return ContextOutputT<Tin, RefOutputT<ScriptCursor> >(RefOutput(output_));
		}

		// Some basic utility parsers ///////////////////////////////////////////////////////////////////////////////////////
		// this class used to end on line 711
		template<bool TCaseSensitive, typename Toutput>
		struct CharT :
			public ParserBase<CharT<TCaseSensitive, Toutput> >
		{
			wchar_t match;
			Toutput output;

			CharT(const Toutput& output_, wchar_t match_) :
				match(match_),
				output(output_)
			{
			}

			CharT(const Toutput& output_) :
				match(0),
				output(output_)
			{
			}

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"Char%('%')")
					.s(TCaseSensitive ? L"" : L"I")
					.c(match == 0 ? L'0' : match)
					.Str();
			}

			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
				{
					//result.Message(L"Unexpected end of file searching for character ...");
					return false;
				}

				parsed = input.CurrentChar();
				output.Save(input, parsed);
				input.Advance();

				if(match == 0)
				{
					return true;
				}

				if(match == parsed)
					return true;

				if(!TCaseSensitive)
				{
					if(LibCC::CharToLower(parsed) == LibCC::CharToLower(match))
						return true;
				}
				//result.Message(L"Unexpected character");
				return false;
			}
		};

		template<typename Toutput>
		inline CharT<true, Toutput> Char(wchar_t ch, const Toutput& output_)
		{
			return CharT<true, Toutput>(output_, ch);
		}

		template<typename Toutput>
		inline CharT<true, Toutput> AnyChar(const Toutput& output_)
		{
			return Char(0, output_);
		}

		inline CharT<true, NullOutput<wchar_t> > Char()
		{
			return CharT<true, NullOutput<wchar_t> >(NullOutput<wchar_t>(), 0);
		}

		inline CharT<true, NullOutput<wchar_t> > Char(wchar_t ch)
		{
			return CharT<true, NullOutput<wchar_t> >(NullOutput<wchar_t>(), ch);
		}

		inline CharT<true, RefOutputT<wchar_t> > Char(wchar_t ch, wchar_t& out)
		{
			return CharT<true, RefOutputT<wchar_t> >(RefOutput(out), ch);
		}

		inline CharT<true, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > Char(wchar_t ch, std::wstring& out)
		{
			return CharT<true, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > (CharToStringOutput(out), ch);
		}


		template<typename Toutput>
		inline CharT<false, Toutput> CharI(wchar_t ch, const Toutput& output_)
		{
			return CharT<false, Toutput>(output_, ch);
		}

		template<typename Toutput>
		inline CharT<false, Toutput> AnyCharI(const Toutput& output_)
		{
			return CharI(0, output_);
		}

		inline CharT<false, NullOutput<wchar_t> > CharI()// redundant to Char() i admit. but why not.
		{
			return CharT<false, NullOutput<wchar_t> >(NullOutput<wchar_t>(), 0);
		}

		inline CharT<false, NullOutput<wchar_t> > CharI(wchar_t ch)
		{
			return CharT<false, NullOutput<wchar_t> >(NullOutput<wchar_t>(), ch);
		}

		inline CharT<false, RefOutputT<wchar_t> > CharI(wchar_t ch, wchar_t& out)
		{
			return CharT<false, RefOutputT<wchar_t> >(RefOutput(out), ch);
		}

		inline CharT<false, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > CharI(wchar_t ch, std::wstring& out)
		{
			return CharT<false, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > (CharToStringOutput(out), ch);
		}


		template<bool TCaseSensitive, typename Toutput>
		struct StrT : public ParserBase<StrT<TCaseSensitive, Toutput> >
		{
			std::wstring match;
			Toutput output;

			StrT(const std::wstring& match, const Toutput& out) :
				match(match),
				output(out)
			{
			}

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"Str%(%)")
					.s(TCaseSensitive ? L"" : L"I")
					.qs(match)
					.Str();
			}

			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
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
					if(!TCaseSensitive && (LibCC::CharToLower(ch) != LibCC::CharToLower(*it)))
					{
						return false;
					}
					parsed.push_back(ch);
				}

				output.Save(input, parsed);
				return true;// made it all the way
			}
		};


		template<typename Toutput>
		inline StrT<true, Toutput> Str(const std::wstring& ch, const Toutput& output_)
		{
			return StrT<true, Toutput>(ch, output_);
		}

		inline StrT<true, NullOutput<std::wstring> > Str(const std::wstring& ch)
		{
			return StrT<true, NullOutput<std::wstring> >(ch, NullOutput<std::wstring>());
		}

		inline StrT<true, RefOutputT<std::wstring> > Str(const std::wstring& ch, std::wstring& out)
		{
			return StrT<true, RefOutputT<std::wstring> >(ch, RefOutput(out));
		}

		template<typename Toutput>
		inline StrT<false, Toutput> StrI(const std::wstring& ch, const Toutput& output_)
		{
			return StrT<false, Toutput>(ch, output_);
		}

		inline StrT<false, NullOutput<std::wstring> > StrI(const std::wstring& ch)
		{
			return StrT<false, NullOutput<std::wstring> >(ch, NullOutput<std::wstring>());
		}

		inline StrT<false, RefOutputT<std::wstring> > StrI(const std::wstring& ch, std::wstring& out)
		{
			return StrT<false, RefOutputT<std::wstring> >(ch, RefOutput(out));
		}


		// matches exactly 1 whitespace char
		struct Space :
			public ParserBase<Space>
		{
			virtual std::wstring GetParserName() const { return L"Space"; }

			virtual void SaveOutputState(ScriptReader& input) { }
			virtual void RestoreOutputState(ScriptReader& input) { }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
					return false;
				parsed = input.CurrentChar();
				if(!LibCC::StringContains(WhitespaceChars(), parsed))
				{
					// non-matching char
					return false;
				}

				input.Advance();
				return true;
			}
		};

		template<bool TCaseSensitive, typename Toutput>
		struct CharOfT :
			public ParserBase<CharOfT<TCaseSensitive, Toutput> >
		{
			std::wstring chars;
			Toutput output;

			CharOfT(const std::wstring& chars_, const Toutput& output_) :
				chars(chars_),
				output(output_)
			{
				if(!TCaseSensitive)
					chars = LibCC::StringToLower(chars);
			}

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"CharOf%(%)")
					.s(TCaseSensitive ? L"" : L"I")
					.qs(chars)
					.Str();
			}
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				wchar_t parsed = 0;
				if(input.IsEOF())
					return false;

				parsed = input.CurrentChar();
				if(TCaseSensitive && (std::wstring::npos == chars.find(parsed)))
				{
					return false;
				}
				if(!TCaseSensitive && (std::wstring::npos == chars.find(LibCC::CharToLower(parsed))))
				{
					return false;
				}

				output.Save(input, parsed);
				input.Advance();
				return true;
			}
		};


		template<typename Toutput>
		inline CharOfT<true, Toutput> CharOf(const std::wstring& chars, const Toutput& output_)
		{
			return CharOfT<true, Toutput>(chars, output_);
		}

		inline CharOfT<true, NullOutput<wchar_t> > CharOf(const std::wstring& chars)
		{
			return CharOfT<true, NullOutput<wchar_t> >(chars, NullOutput<wchar_t>());
		}

		inline CharOfT<true, RefOutputT<wchar_t> > CharOf(const std::wstring& chars, wchar_t& out)
		{
			return CharOfT<true, RefOutputT<wchar_t> >(chars, RefOutput(out));
		}

		inline CharOfT<true, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > CharOf(const std::wstring& chars, std::wstring& out)
		{
			return CharOfT<true, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > (chars, CharToStringOutput(out));
		}


		template<typename Toutput>
		inline CharOfT<false, Toutput> CharOfI(const std::wstring& chars, const Toutput& output_)
		{
			return CharOfT<false, Toutput>(chars, output_);
		}

		inline CharOfT<false, NullOutput<wchar_t> > CharOfI(const std::wstring& chars)
		{
			return CharOfT<false, NullOutput<wchar_t> >(chars, NullOutput<wchar_t>());
		}

		inline CharOfT<false, RefOutputT<wchar_t> > CharOfI(const std::wstring& chars, wchar_t& out)
		{
			return CharOfT<false, RefOutputT<wchar_t> >(chars, RefOutput(out));
		}

		inline CharOfT<false, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > CharOfI(const std::wstring& chars, std::wstring& out)
		{
			return CharOfT<false, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > (chars, CharToStringOutput(out));
		}


		template<bool TCaseSensitive, typename Toutput>
		struct CharRangeT :
			public ParserBase<CharRangeT<TCaseSensitive, Toutput> >
		{
			wchar_t lower;
			wchar_t upper;
			Toutput output;

			CharRangeT(wchar_t lower_, wchar_t upper_, const Toutput& output_) :
				lower(lower_),
				upper(upper_),
				output(output_)
			{
				if(!TCaseSensitive)
				{
					lower = LibCC::CharToLower(lower);
					upper = LibCC::CharToLower(upper);
				}
			}

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"CharRange%('%'-'%')")
					.s(TCaseSensitive ? L"" : L"I")
					.c(lower)
					.c(upper)
					.Str();
			}
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
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
					wchar_t chL = LibCC::CharToLower(parsed);
					if((chL < lower) || (chL > upper))// lower & upper are already lowercase
						return false;
				}

				output.Save(input, parsed);
				input.Advance();
				return true;
			}
		};


		template<typename Toutput>
		inline CharRangeT<true, Toutput> CharRange(wchar_t low, wchar_t high, const Toutput& output_)
		{
			return CharRangeT<true, Toutput>(low, high, output_);
		}

		inline CharRangeT<true, NullOutput<wchar_t> > CharRange(wchar_t low, wchar_t high)
		{
			return CharRangeT<true, NullOutput<wchar_t> >(low, high, NullOutput<wchar_t>());
		}

		inline CharRangeT<true, RefOutputT<wchar_t> > CharRange(wchar_t low, wchar_t high, wchar_t& out)
		{
			return CharRangeT<true, RefOutputT<wchar_t> >(low, high, RefOutput(out));
		}

		inline CharRangeT<true, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > CharRange(wchar_t low, wchar_t high, std::wstring& out)
		{
			return CharRangeT<true, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > (low, high, CharToStringOutput(out));
		}


		template<typename Toutput>
		inline CharRangeT<false, Toutput> CharRangeI(wchar_t low, wchar_t high, const Toutput& output_)
		{
			return CharRangeT<false, Toutput>(low, high, output_);
		}

		inline CharRangeT<false, NullOutput<wchar_t> > CharRangeI(wchar_t low, wchar_t high)
		{
			return CharRangeT<false, NullOutput<wchar_t> >(low, high, NullOutput<wchar_t>());
		}

		inline CharRangeT<false, RefOutputT<wchar_t> > CharRangeI(wchar_t low, wchar_t high, wchar_t& out)
		{
			return CharRangeT<false, RefOutputT<wchar_t> >(low, high, RefOutput(out));
		}

		inline CharRangeT<false, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > CharRangeI(wchar_t low, wchar_t high, std::wstring& out)
		{
			return CharRangeT<false, InserterOutputT<wchar_t, std::back_insert_iterator<std::wstring>, std::wstring> > (low, high, CharToStringOutput(out));
		}


		struct Eol :
			public ParserBase<Eol>
		{
			virtual std::wstring GetParserName() const { return L"EOL"; }

			virtual void SaveOutputState(ScriptReader& input) { }
			virtual void RestoreOutputState(ScriptReader& input) { }

			// basically we match \r, \n, or \r\n
			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				if(input.IsEOF())// count EOF as EOL
					return true;
				return (Or(Or(Str(L"\r\n"), Char('\r')), Char('\n'))).ParseRetainingStateOnError(result, input);
			}
		};

		// utility string parsers ///////////////////////////////////////////////////////////////////////////////////////
		// parses C++-style string escape sequences
		template<typename Toutput>
		struct StringEscapeParserT :
			public ParserBase<StringEscapeParserT<Toutput> >
		{
			Toutput output;
			StringEscapeParserT(const Toutput& output_) :
				output(output_)
			{
			}
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }
			virtual std::wstring GetParserName() const { return L"StringEscapeParserT"; }
			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				wchar_t escapeChar = 0;
				Parser p = Sequence<false>(Char('\\'), Char(0, escapeChar));
				if(!(p.ParseRetainingStateOnError(result, input)))
					return false;
				switch(escapeChar)
				{
				case 'r':
					output.Save(input, '\r');
					return true;
				case 'n':
					output.Save(input, '\n');
					return true;
				case 't':
					output.Save(input, '\t');
					return true;
				case '\"':
					output.Save(input, '\"');
					return true;
				case '\\':
					output.Save(input, '\\');
					return true;
				case '\'':
					output.Save(input, '\'');
					return true;
				// TODO: handle other escape sequences
				default:
					// unrecognized escape
					return false;
				}
			}
		};

		template<typename Toutput>
		StringEscapeParserT<Toutput> StringEscapeParser(const Toutput& output_)
		{
			return StringEscapeParserT<Toutput>(output_);
		}


		// helps parsing javascript-style strings
		template<typename Toutput>
		struct StringParserT :
			public ParserBase<StringParserT<Toutput> >
		{
			Toutput output;
			StringParserT(const Toutput& output_) : output(output_) { }

			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }
			virtual std::wstring GetParserName() const { return L"StringParser"; }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				std::wstring parsed;

				Parser singleQuotes =
					Sequence<false>
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
					Sequence<false>
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
				Parser p = Or(singleQuotes, doubleQuotes);
				bool ret = p.ParseRetainingStateOnError(result, input);
				if(ret)
					output.Save(input, parsed);
				return ret;
			}
		};

		template<typename Toutput>
		inline StringParserT<Toutput> StringParser(const Toutput& output)
		{
			return StringParserT<Toutput>(output);
		}

		inline StringParserT<RefOutputT<std::wstring> > StringParser(std::wstring& output)
		{
			return StringParser(RefOutput(output));
		}

		template<typename Tch>
		inline int CharToDigit(Tch ch)
		{
			ch = LibCC::CharToLower(ch);
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
		template<typename IntT, typename Toutput>
		struct UnsignedIntegerParserT :
			public ParserBase<UnsignedIntegerParserT<IntT, Toutput> >
		{
			std::wstring digits;
			int base;
			Toutput output;

			UnsignedIntegerParserT(int base_, const Toutput& output_) :
				base(base_),
				output(output_)
			{
				InitDigits();
			}

			void InitDigits()
			{
				digits = std::wstring(L"0123456789abcdefghijklmnopqrstuvwxyz", base);
			}

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UnsignedIntegerParser(base=%)").i(base).Str();
			}
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual bool Parse(ParseResult& result, ScriptReader& input)
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
				output.Save(input, out);
				return true;
			}
		};

		template<typename IntT, typename Toutput>
		UnsignedIntegerParserT<IntT, Toutput> UnsignedIntegerParser(int base, const Toutput& output)
		{
			return UnsignedIntegerParserT<IntT, Toutput>(base, output);
		}

		template<typename IntT, typename Toutput>
		UnsignedIntegerParserT<IntT, Toutput> UnsignedIntegerParser(const Toutput& output)
		{
			return UnsignedIntegerParserT<IntT, Toutput>(output);
		}

		// UIntegerHexT (signed 16-bit integer parser with prefix of 0x)
		template<typename IntT, typename Toutput>
		struct UIntegerHexT :
			public ParserBase<UIntegerHexT<IntT, Toutput> >
		{
			Toutput output;
			UIntegerHexT(const Toutput& output_) : output(output_) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerHexT\r\n"; }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerHexT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				return Sequence<false>(Str(L"0x"), UnsignedIntegerParser<IntT>(16, output)).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT, typename Toutput>
		UIntegerHexT<IntT, Toutput> UIntegerHex(const Toutput& output)
		{
			return UIntegerHexT<IntT, Toutput>(output);
		}

		// UIntegerOct (signed 8-bit integer parser with prefix of 0)
		template<typename IntT, typename Toutput>
		struct UIntegerOctT :
			public ParserBase<UIntegerOctT<IntT, Toutput> >
		{
			Toutput output;
			UIntegerOctT(const Toutput& output_) : output(output_) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerOctT\r\n"; }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerOctT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				return
					Sequence<false>
					(
						Char('0'),
						Not(Char('x')),
						UnsignedIntegerParser<IntT>(8, output),
						Not(Char('b'))
					)
					.ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT, typename Toutput>
		UIntegerOctT<IntT, Toutput> UIntegerOct(const Toutput& output)
		{
			return UIntegerOctT<IntT, Toutput>(output);
		}

		// UIntegerDecT (signed 10-bit integer parser)
		template<typename IntT, typename Toutput>
		struct UIntegerDecT :
			public ParserBase<UIntegerDecT<IntT, Toutput> >
		{
			Toutput output;
			UIntegerDecT(const Toutput& output_) : output(output_) { }
			virtual std::wstring Dump(int indentLevel) { return std::wstring(indentLevel, ' ') + L"UIntegerDecT\r\n"; }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerDecT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				return UnsignedIntegerParser<IntT>(10, output).ParseRetainingStateOnError(result, input);
			}
		};

		template<typename IntT, typename Toutput>
		UIntegerDecT<IntT, Toutput> UIntegerDec(const Toutput& output)
		{
			return UIntegerDecT<IntT, Toutput>(output);
		}

		// UIntegerBinT (signed 2-bit integer parser with suffix of 'b')
		template<typename IntT, typename Toutput>
		struct UIntegerBinT :
			public ParserBase<UIntegerBinT<IntT, Toutput> >
		{
			Toutput output;
			UIntegerBinT(const Toutput& output_) : output(output_) { }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UIntegerBinT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				return Sequence<false>
					(
						UnsignedIntegerParser<IntT>(2, output),
						Char('b')
					).ParseRetainingStateOnError(result, input);
				return true;
			}
		};

		template<typename IntT, typename Toutput>
		UIntegerBinT<IntT, Toutput> UIntegerBin(const Toutput& output)
		{
			return UIntegerBinT<IntT, Toutput>(output);
		}

		// SIntegerHexT (signed or unsigned 16-bit integer parser with prefix of 0x)
		template<typename IntT, typename Toutput>
		struct SIntegerHexT :
			public ParserBase<SIntegerHexT<IntT, Toutput> >
		{
			Toutput output;
			SIntegerHexT(const Toutput& output_) : output(output_) { }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerHexT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 16;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence<false>
					(
						Optional(CharOf(L"+-", sign)),
						Str(L"0x"),
						UnsignedIntegerParserT<IntT, RefOutputT<IntT> >(base, RefOutput(temp))
					);
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output.Save(input, sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT, typename Toutput>
		SIntegerHexT<IntT, Toutput> SIntegerHex(const Toutput& output)
		{
			return SIntegerHexT<IntT, Toutput>(output);
		}

		// SIntegerOctT (signed or unsigned 8-bit integer parser with prefix of 0)
		template<typename IntT, typename Toutput>
		struct SIntegerOctT :
			public ParserBase<SIntegerOctT<IntT, Toutput> >
		{
			Toutput output;
			SIntegerOctT(const Toutput& output_) : output(output_) { }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerOctT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 8;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence<false>
					(
						Optional(CharOf(L"+-", sign)),
						Char('0'),
						Not(Char('x')),
						UnsignedIntegerParserT<IntT, RefOutputT<IntT> >(base, RefOutput(temp)),
						Not(Char('b'))
					);
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output.Save(input, sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT, typename Toutput>
		SIntegerOctT<IntT, Toutput> SIntegerOct(const Toutput& output)
		{
			return SIntegerOctT<IntT, Toutput>(output);
		}

		// SIntegerBinT (signed or unsigned 2-bit integer parser with suffix of 'b')
		template<typename IntT, typename Toutput>
		struct SIntegerBinT :
			public ParserBase<SIntegerBinT<IntT, Toutput> >
		{
			Toutput output;
			SIntegerBinT(const Toutput& output_) : output(output_) { }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerBinT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 2;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence<false>
					(
						Optional(CharOf(L"+-", sign)),
						UnsignedIntegerParserT<IntT, RefOutputT<IntT> >(base, RefOutput(temp)),
						Char('b')
					);
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output.Save(input, sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT, typename Toutput>
		SIntegerBinT<IntT, Toutput> SIntegerBin(const Toutput& output)
		{
			return SIntegerBinT<IntT, Toutput>(output);
		}

		// SIntegerDecT (signed or unsigned 10-bit integer parser)
		template<typename IntT, typename Toutput>
		struct SIntegerDecT :
			public ParserBase<SIntegerDecT<IntT, Toutput> >
		{
			Toutput output;
			SIntegerDecT(const Toutput& output_) : output(output_) { }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SIntegerDecT").Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
			{
				IntT temp;
				const int base = 10;
				wchar_t sign = '+';// default to positive
				Parser p =
					Sequence<false>
					(
						Optional(CharOf(L"+-", sign)),
						UnsignedIntegerParserT<IntT, RefOutputT<IntT> >(base, RefOutput(temp))
					);
				if(!p.ParseRetainingStateOnError(result, input))
					return false;
				output.Save(input, sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename IntT, typename Toutput>
		SIntegerDecT<IntT, Toutput> SIntegerDec(const Toutput& output)
		{
			return SIntegerDecT<IntT, Toutput>(output);
		}

		// even more high-level.
		template<typename IntT, typename Toutput>
		Parser CSignedInteger(const Toutput& output)// these are all signed or unsigned.
		{
			return Or(SIntegerHex<IntT>(output), SIntegerOct<IntT>(output), SIntegerBin<IntT>(output), SIntegerDec<IntT>(output));
		}

		template<typename IntT, typename Toutput>
		Parser CUnsignedInteger(const Toutput& output)
		{
			return Or(UIntegerHex<IntT>(output), UIntegerOct<IntT>(output), UIntegerBin<IntT>(output), UIntegerDec<IntT>(output));
		}

		template<typename IntT, typename Toutput>
		Parser CInteger(const Toutput& output)// same as CSignedInteger
		{
			return Or(SIntegerHex<IntT>(output), SIntegerOct<IntT>(output), SIntegerBin<IntT>(output), SIntegerDec<IntT>(output));
		}

		// if this is named CInteger, then the compiler gets it confused with the overload CInteger(const OutputPtr<IntT>& output)
		template<typename IntT>
		Parser CInteger2(IntT& output)
		{
			return CInteger<IntT, RefOutputT<IntT> >(RefOutput(output));
		}

		// Floating point parser ///////////////////////////////////////////////////////////////////////////////////////
		// parses an unsigned rational number. the reason i don't add sign parsing here is because
		// some syntaxes might have prefixes like "-0x100" so I don't want to do the prefix parsing here. Just the
		// actual numeric part. So this parses a number in a pre-determined base.
		// bases which use 'e' as a digit character won't be able to use exponents
		template<typename T, typename Toutput>
		struct UnsignedRationalParserT :
			public ParserBase<UnsignedRationalParserT<T, Toutput> >
		{
			int base;
			std::wstring digits;
			Toutput output;

			UnsignedRationalParserT(const Toutput& output_, int base_) :
				output(output_),
				base(base_)
			{
				InitDigits();
			}

			void InitDigits()
			{
				digits = std::wstring(L"0123456789abcdefghijklmnopqrstuvwxyz", base);
			}

			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"UnsignedRationalParser(base=%)").i(base).Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
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

				output.Save(input, res);

				return true;
			}
		};

		// matches signed or unsigned rationals
		template<typename T, typename Toutput>
		struct SignedRationalParserT :
			public ParserBase<SignedRationalParserT<T, Toutput> >
		{
			int base;
			Toutput output;

			SignedRationalParserT(const Toutput& output_, int base_) :
				output(output_),
				base(base_)
			{
			}

			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }

			virtual std::wstring GetParserName() const
			{
				return LibCC::FormatW(L"SignedRationalParser(base=%)").i(base).Str();
			}

			virtual bool Parse(ParseResult& result, ScriptReader& input)
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
				output.Save(input, sign == '-' ? -temp : temp);
				return true;
			}
		};

		template<typename T, typename Toutput>
		SignedRationalParserT<T, Toutput> Rational(const Toutput& output)
		{
			return SignedRationalParserT<T, Toutput>(output);
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
				int oldPos = m_cursor.pos;
				if(oldPos > c.pos)
					UpdateMeasurements();
				m_cursor = c;
				if(oldPos < c.pos)
					UpdateMeasurements();
			}

			virtual const std::wstring& GetRawInput() const
			{
				return m_script;
			}

			// our "stack" of measurement ops. just like BasicScriptReader
			std::vector<std::pair<int, int> > m_measurements;

			virtual void PushMeasure()
			{
				m_measurements.push_back(std::pair<int, int>(m_cursor.pos, m_cursor.pos));
			}

			virtual int PopMeasure()
			{
				UpdateMeasurements();
				int ret = m_measurements.back().second - m_measurements.back().first;
				m_measurements.pop_back();
				return ret;
			}

		private:
			void UpdateMeasurements()
			{
				for(std::vector<std::pair<int, int> >::iterator it = m_measurements.begin(); it != m_measurements.end(); ++ it)
				{
					it->second = max(it->second, m_cursor.pos);
				}
			}

			bool CursorStartsWith(const wchar_t* s)
			{
				const wchar_t* i = m_script.c_str() + m_cursor.pos;
				while(true)
				{
					if(*s == 0)
						return true;
					if(*s != *i)
						return false;
					++ s;
					++ i;
				}
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
		// and implement constructors and bool Parse(ParseResult& result, ScriptReader& input)
		template<typename Toutput, typename Tderived>
		struct ParserWithOutput :
			public ParserBase<Tderived>
		{
			Toutput output;
			ParserWithOutput() { }
			ParserWithOutput(const Toutput& output_) : output(output_) { }
			virtual void SaveOutputState(ScriptReader& input) { output.SaveState(input); }
			virtual void RestoreOutputState(ScriptReader& input) { output.RestoreState(input); }
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
			x & y	Match x and y                                                     -- would require a custom parser
			x – y	Match x but not y                                                 -- my equiv is (Not(y) >> x)
			x ^ y	Match x or y but not both                                         -- would require a custom parser
			x [ function expression ]	Execute the function/functor if x is matched  -- would require a custom parser
			x % y	Match x one or more times, separated by occurrences of y          -- would require a custom parser
		*/
		template<typename Tlhs>
		inline OccurrencesT<Tlhs, 0, false> operator * (const ParserBase<Tlhs>& lhs)
		{
			return ZeroOrMore(static_cast<const Tlhs&>(lhs));
		}

		template<typename Tlhs>
		inline OccurrencesT<Tlhs, 0, true> operator & (const ParserBase<Tlhs>& lhs)
		{
			return ZeroOrMoreS(static_cast<const Tlhs&>(lhs));
		}

		template<typename Tlhs>
		inline OccurrencesT<Tlhs, 1, false> operator + (const ParserBase<Tlhs>& lhs)
		{
			return OneOrMore(static_cast<const Tlhs&>(lhs));
		}

		template<typename Tlhs>
		inline OccurrencesT<Tlhs, 1, true> operator ++ (const ParserBase<Tlhs>& lhs)
		{
			return OneOrMoreS(static_cast<const Tlhs&>(lhs));
		}

		template<typename Tlhs>
		inline NotT<Tlhs> operator - (const ParserBase<Tlhs>& lhs)
		{
			return Not(static_cast<const Tlhs&>(lhs));
		}

		template<typename Tlhs>
		inline OptionalT<Tlhs> operator ! (const ParserBase<Tlhs>& lhs)
		{
			return Optional(static_cast<const Tlhs&>(lhs));
		}

		template<typename Tlhs, typename Trhs>
		inline SequenceT<false, Tlhs, Trhs> operator >> (const ParserBase<Tlhs>& lhs, const ParserBase<Trhs>& rhs)
		{
			return Sequence<false>(static_cast<const Tlhs&>(lhs), static_cast<const Trhs&>(rhs));
		}

		template<typename Tlhs, typename Trhs>
		inline SequenceT<true, Tlhs, Trhs> operator + (const ParserBase<Tlhs>& lhs, const ParserBase<Trhs>& rhs)
		{
			return Sequence<true>(static_cast<const Tlhs&>(lhs), static_cast<const Trhs&>(rhs));
		}

		template<typename Tlhs, typename Trhs>
		inline OrT<Tlhs, Trhs> operator | (const ParserBase<Tlhs>& lhs, const ParserBase<Trhs>& rhs)
		{
			return Or<Tlhs, Trhs>(static_cast<const Tlhs&>(lhs), static_cast<const Trhs&>(rhs));
		}
#endif
	}
}
