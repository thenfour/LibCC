LibCC 1

TODO:
x make blob tests.
x string conversion shit
x debug log
	x support disabling efficiently
	x support unicode log fils
	- add format support to more functions
	- support disabling log-by-log
	- support an output hook
	- message classes for selective compiling
	- time zone to use for timestamp
	- log size limiting or other splitting techniques
o test EVERY function... i know they don't even all build correctly.
	x string
		x add ConvertString() overloads which return the resulting string? Actually "ToUnicode" or "ToMBCS" would be appropriate for that.
	- format
		x Use the line-separator character (0x2028) and the paragraph-separator character (0x2029) to divide plain text. A new line is begun after each line separator. A new paragraph is begun after each paragraph separator. 
	- blob
	- result
	- pathmatchspec
	- log

- make libcc compile on other compilers
- Make Format work on other platforms, handle win32 shit correctly. don't depend on winapi.h
- Deal with string conversions correctly (big annoying gross task that will never get done)
- Possibly make it more modular so you don't need to port around so many header files


TESTS TO WRITE:

x  inline void StringSplit(const std::basic_string<Char>& s, const std::basic_string<Char>& sep, OutIt dest)
x  inline std::basic_string<Char> StringJoin(InIt start, InIt end, const std::basic_string<Char>& sep)
x inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const std::basic_string<Char>& chars)
x inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const std::basic_string<Char>& searchString, const std::basic_string<Char>& replaceString)
x inline std::basic_string<Char> StringToUpper(const std::basic_string<Char> &s)
x inline std::basic_string<Char> StringToLower(const std::basic_string<Char> &s)
x inline bool StringEquals(const std::basic_string<Char>& lhs, const Trhs& rhs)
x	inline bool XStringEquals(const std::basic_string<Char>& lhs, const char* rhs)
x inline bool XStringContains(const char* source, Char x)
x inline std::string::size_type XStringFindFirstOf(const std::basic_string<Char>& s, const char* chars)
x inline std::string::size_type XStringFindLastOf(const std::basic_string<Char>& s, const char* chars)



-	inline HRESULT ConvertString(const std::wstring& widestr, Blob<BYTE>& out, UINT codepage)
-	inline HRESULT ConvertString(const std::wstring& widestr, std::string& out, UINT codepage)
-	inline HRESULT ConvertString(const std::string& in, std::string& out)
-	inline HRESULT ConvertString(const char* in, std::string& out)
	template<typename Char>
-	inline HRESULT ConvertString(const std::basic_string<Char>& lhs, std::string& out)
	template<typename Char>
-	inline HRESULT ConvertString(const Char* lhs, std::string& out)
-	inline HRESULT ConvertString(const std::string& multistr, std::wstring& widestr, UINT codepage = CP_ACP)
-	inline HRESULT ConvertString(const char* multistr, std::wstring& widestr, UINT codepage = CP_ACP)
-	inline HRESULT ConvertString(const std::wstring& lhs, std::wstring& widestr)
-	inline HRESULT ConvertString(const wchar_t* lhs, std::wstring& widestr)
	template<typename CharA, typename CharB>
-	inline HRESULT ConvertString(const std::basic_string<CharA>& lhs, std::basic_string<CharB>& widestr)
	template<typename CharA, typename CharB>
-	inline HRESULT ConvertString(const CharA* lhs, std::basic_string<CharB>& widestr)
	
	
-	inline HRESULT ToUTF8(const std::wstring& widestr, std::string& out)
-	inline HRESULT ToUTF8(const wchar_t* widestr, std::string& out)
-	inline HRESULT ToUTF8(const char* in, std::string& out)
-	inline HRESULT ToUTF8(const std::string& in, std::string& out)




















  template<typename Ch = char, typename Traits = std::char_traits<Ch>, typename Alloc = std::allocator<Ch> >
  class FormatX
  {
  public:
    LIBCC_INLINE const _String Str() const;
    LIBCC_INLINE const _Char* CStr() const;

    LIBCC_INLINE FormatX();
    LIBCC_INLINE FormatX(const _String& s);
    LIBCC_INLINE FormatX(const _Char* s);
    LIBCC_INLINE FormatX(const _This& r);
    template<typename CharX>
    explicit inline FormatX(const CharX* s);

    LIBCC_INLINE FormatX(HINSTANCE hModule, UINT stringID);
    LIBCC_INLINE FormatX(UINT stringID);

    template<typename CharX>
    LIBCC_INLINE void SetFormat(const CharX* s);
    LIBCC_INLINE void SetFormat(const _String& s);
    LIBCC_INLINE void SetFormat(const _Char* s);

    LIBCC_INLINE void SetFormat(HINSTANCE hModule, UINT stringID);
    LIBCC_INLINE void SetFormat(UINT stringID);

    // POINTER -----------------------------
    template<typename T>
    LIBCC_INLINE _This& p(const T* v);

    // CHARACTER -----------------------------
    template<typename T>
    LIBCC_INLINE _This& c(T v);
    template<typename T>
    LIBCC_INLINE _This& c(T v, size_t count);

    // STRING -----------------------------
    template<size_t MaxLen>
    LIBCC_INLINE _This& s(const _Char* s);
    LIBCC_INLINE _This& s(const _Char* s, size_t MaxLen);
    LIBCC_INLINE _This& s(const _Char* s);
    template<typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign);
    template<size_t MaxLen, typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign);
    template<typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign, size_t MaxLen);
    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen);
    LIBCC_INLINE _This& NewLine();

    // QUOTED STRINGS
    template<size_t MaxLen>
    LIBCC_INLINE _This& qs(const _Char* s);
    LIBCC_INLINE _This& qs(const _Char* s, size_t MaxLen);
    LIBCC_INLINE _This& qs(const _Char* s);
    template<typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign);
    template<size_t MaxLen, typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign);
    template<typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign, size_t MaxLen);
    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen);

    // UNSIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& ul(unsigned long n);
    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& ul(unsigned long n);
    template<size_t Base>
    LIBCC_INLINE _This& ul(unsigned long n);
    LIBCC_INLINE _This& ul(unsigned long n);
    LIBCC_INLINE _This& ul(unsigned long n, size_t Base, size_t Width = 0, _Char PadChar = '0');

    // SIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    LIBCC_INLINE _This& l(signed long n);
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& l(signed long n);
    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& l(signed long n);
    template<size_t Base>
    LIBCC_INLINE _This& l(signed long n);
    LIBCC_INLINE _This& l(signed long n);
    LIBCC_INLINE _This& l(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false);

    // UNSIGNED INT (just stubs for ul()) -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    _This& ui(unsigned long n)
    template<size_t Base, size_t Width>
    _This& ui(unsigned long n)
    template<size_t Base>
    _This& ui(unsigned long n)
    _This& ui(unsigned long n)
    _This& ui(unsigned long n, size_t Base, size_t Width = 0, _Char PadChar = '0')

    // SIGNED INT -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    _This& i(signed long n)
    template<size_t Base, size_t Width, _Char PadChar>
    _This& i(signed long n)
    template<size_t Base, size_t Width>
    _This& i(signed long n)
    template<size_t Base>
    _This& i(signed long n)
    _This& i(signed long n)
    _This& i(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)

    // FLOAT ----------------------------- 3.14   [intwidth].[decwidth]
    // integralwidth is the MINIMUM digits.  Decimalwidth is the MAXIMUM digits.
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    LIBCC_INLINE _This& f(float val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& f(float val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    LIBCC_INLINE _This& f(float val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    LIBCC_INLINE _This& f(float val);
    template<size_t DecimalWidthMax>
    LIBCC_INLINE _This& f(float val);
    LIBCC_INLINE _This& f(float val, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10);

    // DOUBLE -----------------------------
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    LIBCC_INLINE _This& d(double val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& d(double val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    LIBCC_INLINE _This& d(double val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    LIBCC_INLINE _This& d(double val);
    template<size_t DecimalWidthMax>
    LIBCC_INLINE _This& d(double val);
    LIBCC_INLINE _This& d(double val, size_t DecimalWidthMax = 3, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10);

    // UNSIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& ui64(unsigned __int64 n);
    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& ui64(unsigned __int64 n);
    template<size_t Base> 
    LIBCC_INLINE _This& ui64(unsigned __int64 n);
    LIBCC_INLINE _This& ui64(unsigned __int64 n);
    LIBCC_INLINE _This& ui64(unsigned __int64 n, size_t Base, size_t Width = 0, _Char PadChar = '0');

    // SIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    LIBCC_INLINE _This& i64(signed __int64 n);
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& i64(__int64 n);
    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& i64(__int64 n);
    template<size_t Base>
    LIBCC_INLINE _This& i64(__int64 n);
    LIBCC_INLINE _This& i64(__int64 n);
    LIBCC_INLINE _This& i64(signed __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false);

    // GETLASTERROR() -----------------------------
    LIBCC_INLINE _This& gle(int code);
    LIBCC_INLINE _This& gle();

    // CONVENIENCE OPERATOR () -----------------------------
    _This& operator ()(int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    _This& operator ()(unsigned int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0')
    _This& operator ()(__int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    _This& operator ()(unsigned __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0')
    _This& operator ()(float n, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
    _This& operator ()(double n, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
    _This& operator ()(char* n)
    _This& operator ()(wchar_t* n)
    _This& operator ()(const std::string& n)
    _This& operator ()(const std::wstring& n)
  };
