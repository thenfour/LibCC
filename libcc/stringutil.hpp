/*
  LibCC
  StringUtil Module
  (c) 2004-2008 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC_Format
	Official source code: http://svn.winprog.org/personal/carl/LibCC

	Original version: Nov 15, 2004

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
	THE GOALS of the string API:
	1) operate on either std::basic_string<xchar> or xchar* types freely
	2) will do automatic conversions to the largest string type (prefer wchar_t over char, no matter which order they were passed into the API)
	3) NO conversion if the strings have the same character types
	4) no unnecessary intermediate conversions to std::basic_strings<>

	... within reason. there are cases when it's impossible to follow all these rules.

	If you pass in a string<__int32> and a string<wchar_t> in the same function, conversion will happen to __int32.

	NOTE: ansi codepages are only supported in conversion functions. shit like StringEquals i'm not going to bother writing support for
	      converting ANSI codepages. Do it yourself dammit. In those cases, CP_ACP is assumed.

	NOTE: some linguistic features outlined by Unicode is just not supported. for example
	      - some characters are considered linguistically equal, but StringEquals() will still consider them different.
				- StringToUpper / StringToLower are probably not correct for all scripts

	TODO:
		Audit remaining APIs that don't follow the rules above. this includes:
		- StringReplace
		- StringToUpper
		- StringToLower


	API SUMMARY CONTAINED IN THIS FILE:
	
	DigitToChar
	StringBegin
	StringIsEnd
	CharConvert
	CharToLower
	CharToUpper
	XLastDitchStringCopy
	StringLength
	StringConvert
	ToUTF16
	ToANSI
	ToUTF8
	StringContains
	StringFindFirstOf
	StringFindLastOf
	StringSplitByString
	StringJoin
	StringTrim
	StringReplace
	StringToLower
	StringToUpper
	StringEquals
	StringEqualsI

*/

#pragma once


#include <string>
#include <tchar.h>
#include <malloc.h>// for alloca()
#include <math.h>// for fmod()
#include <vector>

#include "blob.hpp"
#include "float.hpp"

#ifdef WIN32
# include <windows.h>// for GetLastError() / LoadStrin / FormatMessage...
#endif


#pragma warning(push)

/*
  Disable "conditional expression is constant".  This code has conditions
  on integral template params, which are by design constant.  This is a lame
  warning.
*/
#pragma warning(disable:4127)
#pragma warning(disable:4312)// CharUpper() and CharLower() require me to cast some char -> PWSTR, which gives this error.
#pragma warning(disable:4311)// CharUpper() and CharLower() require me to cast some char -> PWSTR, which gives this error.
#pragma warning(disable:4996)// warning C4996: 'wcscpy' was declared deprecated  -- uh, i know how to use this function just fine, thanks.

#ifndef CCSTR_OPTION_AUTOCAST
#  define CCSTR_OPTION_AUTOCAST 0// set this to 1 and class Format can auto-cast into std::string
#endif

// Set up inline option
#ifdef _MSC_VER
# if (LIBCC_OPTION_INLINE == 1)// set this option
#   define LIBCC_INLINE __declspec(noinline)
# else
#   define LIBCC_INLINE inline
# endif
#else
# define LIBCC_INLINE inline
#endif


namespace LibCC
{
	// random utility function
	template<typename T, size_t N>
	size_t SizeofStaticArray(const T (&x)[N])
	{
		return N;
	}

  inline char DigitToChar(unsigned char d)
  {
    static const char Digits [] = "0123456789abcdefghijklmnopqrstuvwxyz";
    return d < (sizeof(Digits) / sizeof(char)) ? Digits[d] : 0;
  }
	
	// iterator stuff that works on both xchar* and std::basic_string<> --------------------------------------------------------------------------------------
	// TODO: write actual iterators for xchar* types (iterator, const_iterator, reverse_iterator, etc
	// StringBegin. ----------------------------------------------------
	template<typename Char>
	inline const Char* StringBegin(const Char* sz)
	{
		return sz;
	}
	template<typename Char>
	inline Char* StringBegin(Char* sz)
	{
		return sz;
	}
	template<typename Char>
	inline typename std::basic_string<Char>::iterator StringBegin(std::basic_string<Char>& sz)
	{
		return sz.begin();
	}
	template<typename Char>
	inline typename std::basic_string<Char>::const_iterator StringBegin(const std::basic_string<Char>& sz)
	{
		return sz.begin();
	}

	// StringIsEnd. --------------------------------------------------------------------------------------
	template<typename Char>
	inline bool StringIsEnd(const Char* it, const Char*)
	{
		return *it == 0;
	}
	template<typename Char>
	inline bool StringIsEnd(const typename std::basic_string<Char>::iterator& it, const std::basic_string<Char>& str)
	{
		return it == str.end();
	}
	template<typename Char>
	inline bool StringIsEnd(const typename std::basic_string<Char>::const_iterator& it, const std::basic_string<Char>& str)
	{
		return it == str.end();
	}


	// CharConvert. --------------------------------------------------------------------------------------
	// TODO. for now conversion of single characters is always just a cast.
	template<typename CharOut, typename CharIn>
	inline CharOut CharConvert(CharIn i, UINT inCP = CP_ACP, UINT outCP = CP_ACP)
	{
		return (CharOut)i;
	}


	// CharToLower. --------------------------------------------------------------------------------------
	// TODO: um, 
	inline wchar_t CharToLower(wchar_t c)
	{
		return (wchar_t)CharLowerW((PWSTR)c);
	}

	inline char CharToLower(char c)
	{
		return (char)CharLowerA((PSTR)c);
	}

	template<typename Char>
	inline Char CharToLower(Char c)
	{
		return (Char)CharToLower((wchar_t)c);// TODO: make this work for 32-bit unicode scalars
	}


	// CharToLower. --------------------------------------------------------------------------------------
	// TODO: um, 
	inline wchar_t CharToUpper(wchar_t c)
	{
		return (wchar_t)CharUpperW((PWSTR)c);
	}

	inline char CharToUpper(char c)
	{
		return (char)CharUpperA((PSTR)c);
	}

	template<typename Char>
	inline Char CharToUpper(Char c)
	{
		return (Char)CharToUpper((wchar_t)c);// TODO: make this work for 32-bit unicode scalars
	}


	// LastDitch functions --------------------------------------------------------------------------------------
	// these do char-for-char operations knowing that we probably don't understand their format.
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const std::basic_string<InChar>& in, std::basic_string<OutChar>& out)
	{
		// when there's no other way to convert from 1 string type to another, you can try this basic element-by-element copy
		out.clear();
		out.reserve(in.size());
		for(std::basic_string<InChar>::const_iterator it = in.begin(); it != in.end(); ++ it)
		{
			out.push_back(CharConvert<OutChar>(*it));
		}
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const InChar* in, std::basic_string<OutChar>& out)
	{
		// when there's no other way to convert from 1 string type to another, you can try this basic element-by-element copy
		out.clear();
		out.reserve(StringLength(in));
		for(; *in != 0; ++ in)
		{
			out.push_back(CharConvert<OutChar>(*in));
		}
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const InChar* in, OutChar* out)// out must already be allocated
	{
		for(; *in != 0; ++ in)
		{
			*out = *in;
			out ++;
		}
		*out = 0;
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const std::basic_string<InChar>& in, OutChar* out)// out must already be allocated
	{
		for(std::basic_string<InChar>::const_iterator it = in.begin(); it != in.end(); ++ it)
		{
			*out = *it;
			out ++;
		}
		*out = 0;
	}


	// StringLength. --------------------------------------------------------------------------------------
	template<typename Char>
	inline size_t StringLength(const Char* sz)// this only works for fixed-size char strings. not ANSI or MBCS or Unicode surrogates and shit.
	{
		size_t ret = 0;
		while(*sz ++)
		{
			++ ret;
		}
		return ret;
	}

	template<typename Char>
	inline size_t StringLength(const std::basic_string<Char>& sz)
	{
		return sz.size();
	}


	// StringConvert. this also acts as a StringCopy. --------------------------------------------------------------------------------------
#ifdef WIN32

	// TODO: http://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c
	inline HRESULT UTF16ToUTF32(const wchar_t*, size_t, Blob<__int32>&)
	{
		return E_NOTIMPL;
	}

	inline HRESULT UTF32ToUTF16(const wchar_t*, size_t, Blob<__int32>&)
	{
		return E_NOTIMPL;
	}

	// http://www.themssforum.com/MFC/WideCharToMultiByte-works/
	// converts from UTF-16 (true UTF-16 according to MS) to ANSI
	inline HRESULT ToANSI(const wchar_t* in, size_t inLength, Blob<BYTE>& out, UINT codepage = CP_ACP)
	{
		DWORD flags;
	 
		switch (codepage)
		{
		case 50220: case 50221: case 50222: case 50225:
		case 50227: case 50229: case 52936: case 54936:
		case 57002: case 57003: case 57004: case 57005:
		case 57006: case 57007: case 57008: case 57009:
		case 57010: case 57011: case 65000: case 42:
			flags = 0;
			break;
		case 65001:
			flags = 0; // or WC_ERR_INVALID_CHARS on winver >= 0x0600
			break;
		default:
			{
				flags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK;
				break;
			}
		}

		CPINFO cpinfo;
		if(0 == GetCPInfo(codepage, &cpinfo))
		{
			return E_FAIL;
		}
	 
		// get the length first
		BOOL usedDefaultChar = FALSE;
		int length = WideCharToMultiByte(codepage, flags, in, (int)inLength, NULL, 0,
			(flags & WC_NO_BEST_FIT_CHARS) ? (const CHAR*)cpinfo.DefaultChar : 0,
			(flags & WC_NO_BEST_FIT_CHARS) ? &usedDefaultChar : 0);

		if (length == 0)
			return E_FAIL;

		out.Alloc(length);// it is important to make sure the return Blob has the correct size here. so do not add +1 to this.

		WideCharToMultiByte(codepage, flags, in, (int)inLength, (LPSTR)out.GetBuffer(), length,
			(flags & WC_NO_BEST_FIT_CHARS) ? (const CHAR*)cpinfo.DefaultChar : 0,
			(flags & WC_NO_BEST_FIT_CHARS) ? &usedDefaultChar : 0);

		return S_OK;
	}

	// from ANSI to Unicode (real UTF-16)
	inline HRESULT ToUTF16(const BYTE* multistr, size_t sourceLength, std::wstring& widestr, UINT codepage = CP_ACP)
	{
		int length = MultiByteToWideChar(codepage, 0, (PCSTR)multistr, (int)sourceLength, NULL, 0);
		if (length == 0)
			return E_FAIL;
		Blob<WCHAR> buf;
		if(!buf.Alloc(length))
		{
			return E_OUTOFMEMORY;
		}
		MultiByteToWideChar(codepage, 0, (PCSTR)multistr, (int)sourceLength, buf.GetBuffer(), (int)length);
		widestr.assign(buf.GetBuffer(), buf.Size());
 
		return S_OK;
	}

	/*
		supported input / output types:
		std::basic_string<xchar>
		xchar*
		LibCC::Blob<xchar>  [ limited ]

		For each in/out type scenario
		a) basic_string -> basic_string
		a) xchar* -> basic_string
		
		here are the conversion cases to write functions for. it ends up being 5 functions to handle all cases:

		#1: ansi cp1 str -> ansi cp1 str    (no conversion)
		#2: utf16 str -> utf16 str          (no conversion)
		#3: unknown str -> unknown str      (no conversion)

		#4: ansi str -> utf16 str           (known conversion)
		#5: utf16 str -> ansi str           (known conversion)
		#6: ansi str -> other ansi str      (known conversion)

		#7: ansi str -> unknown str         (unknown conversion)
		#8: utf16 str -> unknown str        (unknown conversion)
		#9: unknown str -> utf16 str        (unknown conversion)
		#10: unknown str -> ansi str        (unknown conversion)

		TODO:
		#12: utf32 str -> utf32 str
		#13: utf32 str -> utf16 str
		#14: utf16 str -> utf32 str
		#15: ansi str -> utf32 str
		#16: utf32 str -> ansi str
		#17: utf32 str -> unknown str
		#18: unknown str -> utf32 str

		NOTE:
			- many functions take codepage arguments even though they will be ignored. this is to unify how they are called; more generic.
			  for an ignored codepage, just use 0.
	*/

	// basic_string -> basic_string --------------------------------------------------------------------------------------
	inline HRESULT StringConvert(const std::string& in, std::wstring& out, UINT fromcodepage = CP_ACP, UINT = CP_ACP)
	{
		return ToUTF16((const BYTE*)in.c_str(), in.length(), out, fromcodepage);
	}
	// case #5:
	inline HRESULT StringConvert(const std::wstring& in, std::string& out, UINT = CP_ACP, UINT tocodepage = CP_ACP)
	{
		Blob<BYTE> b;
		HRESULT hr = ToANSI(in.c_str(), in.length(), b, tocodepage);
		if(FAILED(hr)) return hr;
		out.assign((const char*)b.GetBuffer(), b.Size());
		return hr;
	}
	// case #1, #2, #3, #6:
	inline HRESULT StringConvert(const std::string& in, std::string& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			out = in;
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = StringConvert(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return StringConvert(intermediate, out, toCodepage);
	}
	// case #7, #8, #9, #10:
	template<typename CharIn, typename CharOut>
	inline HRESULT StringConvert(const std::basic_string<CharIn>& in, std::basic_string<CharOut>& out, UINT fromcodepage = CP_ACP, UINT tocodepage = CP_ACP)
	{
		XLastDitchStringCopy(in, out);
		return S_OK;
	}

	// xchar* -> basic_string --------------------------------------------------------------------------------------
	inline HRESULT StringConvert(const char* in, std::wstring& out, UINT fromcodepage = CP_ACP, UINT = CP_ACP)
	{
		return ToUTF16((const BYTE*)in, StringLength(in), out, fromcodepage);
	}
	// case #5:
	inline HRESULT StringConvert(const wchar_t* in, std::string& out, UINT = CP_ACP, UINT tocodepage = CP_ACP)
	{
		Blob<BYTE> b;
		HRESULT hr = ToANSI(in, StringLength(in), b, tocodepage);
		if(FAILED(hr)) return hr;
		out.assign((const char*)b.GetBuffer(), b.Size());
		return hr;
	}
	// case #1, #2, #3, #6:
	inline HRESULT StringConvert(const char* in, std::string& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			out = in;
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = StringConvert(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return StringConvert(intermediate, out, toCodepage);
	}
	// case #7, #8, #9, #10:
	template<typename CharIn, typename CharOut>
	inline HRESULT StringConvert(const CharIn* in, std::basic_string<CharOut>& out, UINT fromcodepage = CP_ACP, UINT tocodepage = CP_ACP)
	{
		XLastDitchStringCopy(in, out);
		return S_OK;
	}


	// ToUTF16. --------------------------------------------------------------------------------------
	template<typename Char>
	inline std::wstring ToUTF16(const Char* sz, UINT fromcodepage = CP_ACP)
	{
		std::wstring ret;
		StringConvert(sz, ret, fromcodepage);
		return ret;
	}
	template<typename Char>
	inline std::wstring ToUTF16(const std::basic_string<Char>& s, UINT fromcodepage = CP_ACP)
	{
		std::wstring ret;
		StringConvert(s, ret, fromcodepage);
		return ret;
	}
	

	// ToANSI. --------------------------------------------------------------------------------------
	template<typename Char>
	inline std::string ToANSI(const Char* sz, UINT fromcodepage = CP_ACP, UINT tocodepage = CP_ACP)
	{
		std::string ret;
		StringConvert(sz, ret, fromcodepage, tocodepage);
		return ret;
	}
	template<typename Char>
	inline std::string ToANSI(const std::basic_string<Char>& s, UINT fromcodepage = CP_ACP, UINT tocodepage = CP_ACP)
	{
		std::string ret;
		StringConvert(s, ret, fromcodepage, tocodepage);
		return ret;
	}


	// ToUTF8. --------------------------------------------------------------------------------------
	template<typename Char>
	inline std::string ToUTF8(const Char* sz, UINT fromcodepage = CP_ACP)
	{
		return ToANSI(sz, fromcodepage, CP_UTF8);
	}
	template<typename Char>
	inline std::string ToUTF8(const std::basic_string<Char>& s, UINT fromcodepage = CP_ACP)
	{
		return ToANSI(s, fromcodepage, CP_UTF8);
	}


	// --------------------------------------------------------------------------------------------------------------------
	// DUPLICATES OF ALL THE StringConvert APIs, but returning the string instead of the 2nd arg, throwing away the HRESULT 
	// basic_string -> basic_string --------------------------------------------------------------------------------------
	// case #7, #8, #9, #10:
	template<typename CharOut, typename CharIn>
	inline std::basic_string<CharOut> StringConvert(const std::basic_string<CharIn>& in, UINT fromcodepage, UINT tocodepage)
	{
		std::basic_string<CharOut> ret;
		XLastDitchStringCopy(in, ret);
		return ret;
	}
	template<typename CharOut, typename CharIn>
	inline std::basic_string<CharOut> StringConvert(const std::basic_string<CharIn>& in, UINT fromcodepage)
	{
		return StringConvert<CharOut, CharIn>(in, fromcodepage, CP_ACP);
	}
	template<typename CharOut, typename CharIn>
	inline std::basic_string<CharOut> StringConvert(const std::basic_string<CharIn>& in)
	{
		return StringConvert<CharOut, CharIn>(in, CP_ACP, CP_ACP);
	}
	// case #4:
	// note: error C2765: 'LibCC::StringConvertX' : an explicit specialization of a function template cannot have any default arguments
	template<>
	inline std::wstring StringConvert<wchar_t, char>(const std::string& in, UINT fromcodepage, UINT)
	{
		return ToUTF16(in, fromcodepage);
	}
	template<>
	inline std::wstring StringConvert<wchar_t, char>(const std::string& in, UINT fromcodepage)
	{
		return ToUTF16(in, fromcodepage);
	}
	template<>
	inline std::wstring StringConvert<wchar_t, char>(const std::string& in)
	{
		return ToUTF16(in);
	}
	template<>
	inline std::string StringConvert<char, wchar_t>(const std::wstring& in, UINT fromcodepage, UINT tocodepage)
	{
		return ToANSI(in, fromcodepage, tocodepage);
	}
	template<>
	inline std::string StringConvert<char, wchar_t>(const std::wstring& in, UINT fromcodepage)
	{
		return ToANSI(in, fromcodepage);
	}
	template<>
	inline std::string StringConvert<char, wchar_t>(const std::wstring& in)
	{
		return ToANSI(in);
	}
	// case #1, #2, #3, #6:
	template<>
	inline std::string StringConvert<char, char>(const std::string& in, UINT fromcodepage, UINT tocodepage)
	{
		return ToANSI(in, fromcodepage, tocodepage);
	}
	template<>
	inline std::string StringConvert<char, char>(const std::string& in, UINT fromcodepage)
	{
		return ToANSI(in, fromcodepage);
	}
	template<>
	inline std::string StringConvert<char, char>(const std::string& in)
	{
		return ToANSI(in);
	}


	// xchar* -> basic_string --------------------------------------------------------------------------------------
	// case #7, #8, #9, #10:
	template<typename CharOut, typename CharIn>
	inline std::basic_string<CharOut> StringConvert(const CharIn* in, UINT fromcodepage, UINT tocodepage)
	{
		std::basic_string<CharOut> ret;
		XLastDitchStringCopy(in, ret);
		return ret;
	}
	template<typename CharOut, typename CharIn>
	inline std::basic_string<CharOut> StringConvert(const CharIn* in, UINT fromcodepage)
	{
		return StringConvert<CharOut, CharIn>(in, fromcodepage, CP_ACP);
	}
	template<typename CharOut, typename CharIn>
	inline std::basic_string<CharOut> StringConvert(const CharIn* in)
	{
		return StringConvert<CharOut, CharIn>(in, CP_ACP, CP_ACP);
	}
	// case #4:
	// note: error C2765: 'LibCC::StringConvertX' : an explicit specialization of a function template cannot have any default arguments
	template<>
	inline std::wstring StringConvert<wchar_t, char>(const char* in, UINT fromcodepage, UINT)
	{
		return ToUTF16(in, fromcodepage);
	}
	template<>
	inline std::wstring StringConvert<wchar_t, char>(const char* in, UINT fromcodepage)
	{
		return ToUTF16(in, fromcodepage);
	}
	template<>
	inline std::wstring StringConvert<wchar_t, char>(const char* in)
	{
		return ToUTF16(in);
	}
	template<>
	inline std::string StringConvert<char, wchar_t>(const wchar_t* in, UINT fromcodepage, UINT tocodepage)
	{
		return ToANSI(in, fromcodepage, tocodepage);
	}
	template<>
	inline std::string StringConvert<char, wchar_t>(const wchar_t* in, UINT fromcodepage)
	{
		return ToANSI(in, fromcodepage);
	}
	template<>
	inline std::string StringConvert<char, wchar_t>(const wchar_t* in)
	{
		return ToANSI(in);
	}
	// case #1, #2, #3, #6:
	template<>
	inline std::string StringConvert<char, char>(const char* in, UINT fromcodepage, UINT tocodepage)
	{
		return ToANSI(in, fromcodepage, tocodepage);
	}
	template<>
	inline std::string StringConvert<char, char>(const char* in, UINT fromcodepage)
	{
		return ToANSI(in, fromcodepage);
	}
	template<>
	inline std::string StringConvert<char, char>(const char* in)
	{
		return ToANSI(in);
	}


	// StringContains. --------------------------------------------------------------------------------------
	template<typename TiteratorType, typename TstrType, typename Trhs>
	inline bool InternalStringContains(TstrType str, Trhs x)// not practical for most uses because it would be too ambiguous. this houses the basic generic algorithm
  {
		for(TiteratorType it = StringBegin(str); !StringIsEnd(it, str); ++ it)
		{
      if(*it == x) return true;
    }
    return false;
	}
	// no-conversion cases
  template<typename Char>
  inline bool StringContains(const Char* source, Char x)
  {
		return InternalStringContains<const Char*, const Char*, Char>(source, x);
	}
  template<typename Char>
	inline bool StringContains(const std::basic_string<Char>& source, Char x)
  {
		return InternalStringContains<std::basic_string<Char>::const_iterator, const std::basic_string<Char>&, Char>(source, x);
	}
	// requires conversion
  template<typename CharL, typename CharR>
  inline bool StringContains(const CharL* source, CharR x, int codepageLeft = CP_ACP)
  {
		if(sizeof(CharL) > sizeof(CharR))
		{
			return StringContains(source, CharConvert<CharL>(x));
		}
		else
		{
			std::basic_string<CharR> temp;
			StringConvert(source, temp, codepageLeft);
			return StringContains(temp, x);
		}
	}
	template<typename CharL, typename CharR>
	inline bool StringContains(const std::basic_string<CharL>& source, CharR x, int codepageLeft = CP_ACP)
  {
		if(sizeof(CharL) > sizeof(CharR))
		{
			return StringContains(source, CharConvert<CharL>(x));
		}
		else
		{
			std::basic_string<CharR> temp;
			StringConvert(source, temp, codepageLeft);
			return StringContains(temp, x);
		}
  }


	// StringFindFirstOf, returning index --------------------------------------------------------------------------------------

	// not practical for most uses because it would be too ambiguous. this houses the basic generic algorithm
	// only accepts 1 string type because we don't want to convert for every call to StringContains()
	template<typename TiteratorType, typename TstrType, typename TcharsType>
  inline std::string::size_type InternalStringFindFirstOf1(TstrType str, TcharsType chars)
  {
		std::string::size_type ret = 0;
		for(TiteratorType it = StringBegin(str); !StringIsEnd(it, str); ++ it)
		{
			if(StringContains(chars, *it))// we don't want to do a conversion here
				return ret;
			ret ++;
		}
    return std::string::npos;
  }
	// no-conversion cases
  template<typename Char>
	inline std::string::size_type StringFindFirstOf(const Char* s, const Char* chars)
  {
		return InternalStringFindFirstOf1<const Char*>(s, chars);
  }
  template<typename Char>
	inline std::string::size_type StringFindFirstOf(const Char* s, const std::basic_string<Char>& chars)
  {
		return InternalStringFindFirstOf1<const Char*>(s, chars);
  }
  template<typename Char>
	inline std::string::size_type StringFindFirstOf(const std::basic_string<Char>& s, const Char* chars)
  {
		return InternalStringFindFirstOf1<std::basic_string<Char>::const_iterator>(s, chars);
  }
  template<typename Char>
	inline std::string::size_type StringFindFirstOf(const std::basic_string<Char>& s, const std::basic_string<Char>& chars)
  {
		return InternalStringFindFirstOf1<std::basic_string<Char>::const_iterator>(s, chars);
  }

	// conversion cases
  template<typename CharL, typename CharR, typename Tleft, typename Tright>
	inline std::string::size_type InternalStringFindFirstOf2(Tleft s, Tright chars)// this func does conversion to the biggest type
  {
		if(sizeof(CharL) > sizeof(CharR))
		{
			std::basic_string<CharL> temp;
			StringConvert(chars, temp);
			return StringFindFirstOf(s, temp);
		}
		else
		{
			std::basic_string<CharR> temp;
			StringConvert(s, temp);
			return StringFindFirstOf(temp, chars);
		}
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindFirstOf(const CharL* s, const CharR* chars)
  {
		return InternalStringFindFirstOf2<CharL, CharR>(s, chars);
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindFirstOf(const CharL* s, const std::basic_string<CharR>& chars)
  {
		return InternalStringFindFirstOf2<CharL, CharR>(s, chars);
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindFirstOf(const std::basic_string<CharL>& s, const CharR* chars)
  {
		return InternalStringFindFirstOf2<CharL, CharR>(s, chars);
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindFirstOf(const std::basic_string<CharL>& s, const std::basic_string<CharR>& chars)
  {
		return InternalStringFindFirstOf2<CharL, CharR>(s, chars);
  }


	// StringFindLastOf. --------------------------------------------------------------------------------------
	template<typename TiteratorType, typename TstrType, typename TcharsType>
  inline std::string::size_type InternalStringFindLastOf1(TstrType str, TcharsType chars)
  {
		size_t len = StringLength(str);
		if(0 == len)
	    return std::string::npos;
		std::string::size_type ret = len - 1;
		TiteratorType it = StringBegin(str) + ret;// last char
		while(true)
		{
			if(StringContains(chars, *it))// we don't want to do a conversion here
				return ret;
			if(ret == 0)
				break;
			-- it;
			-- ret;
		}
    return std::string::npos;
  }
	// no-conversion cases
  template<typename Char>
	inline std::string::size_type StringFindLastOf(const Char* s, const Char* chars)
  {
		return InternalStringFindLastOf1<const Char*>(s, chars);
  }
  template<typename Char>
	inline std::string::size_type StringFindLastOf(const Char* s, const std::basic_string<Char>& chars)
  {
		return InternalStringFindLastOf1<const Char*>(s, chars);
  }
  template<typename Char>
	inline std::string::size_type StringFindLastOf(const std::basic_string<Char>& s, const Char* chars)
  {
		return InternalStringFindLastOf1<std::basic_string<Char>::const_iterator>(s, chars);
  }
  template<typename Char>
	inline std::string::size_type StringFindLastOf(const std::basic_string<Char>& s, const std::basic_string<Char>& chars)
  {
		return InternalStringFindLastOf1<std::basic_string<Char>::const_iterator>(s, chars);
  }

	// conversion cases
  template<typename CharL, typename CharR, typename Tleft, typename Tright>
	inline std::string::size_type InternalStringFindLastOf2(Tleft s, Tright chars)
  {
		if(sizeof(CharL) > sizeof(CharR))
		{
			std::basic_string<CharL> temp;
			StringConvert(chars, temp);
			return StringFindLastOf(s, temp);
		}
		else
		{
			std::basic_string<CharR> temp;
			StringConvert(s, temp);
			return StringFindLastOf(temp, chars);
		}
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindLastOf(const CharL* s, const CharR* chars)
  {
		return InternalStringFindLastOf2<CharL, CharR>(s, chars);
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindLastOf(const CharL* s, const std::basic_string<CharR>& chars)
  {
		return InternalStringFindLastOf2<CharL, CharR>(s, chars);
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindLastOf(const std::basic_string<CharL>& s, const CharR* chars)
  {
		return InternalStringFindLastOf2<CharL, CharR>(s, chars);
  }
  template<typename CharL, typename CharR>
	inline std::string::size_type StringFindLastOf(const std::basic_string<CharL>& s, const std::basic_string<CharR>& chars)
  {
		return InternalStringFindLastOf2<CharL, CharR>(s, chars);
  }


	// StringSplit --------------------------------------------------------------------------------------
	// TODO: make a string split that separates by simple char, or possible array of chars or array of strings (like .net)
	// sep is a whole string, not a bunch of possible separator chars.
  template<typename Tchar, class TiteratorIn, class TiteratorSep, typename TstrIn, typename TstrSep, class OutIt>
  inline void InternalStringSplitByString1(TstrIn in, TstrSep sep, OutIt dest)
  {
		if(StringLength(in) == 0)
			return;
		if(StringLength(sep) == 0)
		{
			*dest = in;
			++ dest;
			return;
		}
		typedef std::basic_string<Tchar> _String;
		_String token;
		TiteratorSep sepIt = StringBegin(sep);
		
		for(TiteratorIn it = StringBegin(in); !StringIsEnd(it, in); ++ it)
		{
			if(*it == *sepIt)
			{
				sepIt ++;
				if(StringIsEnd(sepIt, sep))
				{
					// we found the whole separator. push the previous token.
					*dest = token;
					++ dest;
					token.clear();
					sepIt = StringBegin(sep);
					// and if we are at the very end of the string, add a blank token (like "a,b,c," will give 4 results = a, b, c, and a blank one)
					if(StringIsEnd(it + 1, in))
					{
						*dest = token;
						++ dest;
						break;
					}
				}
				else
				{
					// just another character in the separator.
				}
			}
			else
			{
				// just another character in a TOKEN
				token.push_back(*it);
				sepIt = StringBegin(sep);
			}
		}
		// if there's a token at the end of the string, append it. blank tokens would have already been added.
		if(!token.empty())
		{
			*dest = token;
			++ dest;
		}
		return;
  }
	// no-conversion cases
  template<typename Char, class OutIt>
	inline void StringSplitByString(const std::basic_string<Char>& s, const std::basic_string<Char>& sep, OutIt dest)
  {
		InternalStringSplitByString1<Char, std::basic_string<Char>::const_iterator, std::basic_string<Char>::const_iterator>(s, sep, dest);
  }
  template<typename Char, class OutIt>
	inline void StringSplitByString(const std::basic_string<Char>& s, const Char* sep, OutIt dest)
  {
		InternalStringSplitByString1<Char, std::basic_string<Char>::const_iterator, const Char*>(s, sep, dest);
  }
  template<typename Char, class OutIt>
	inline void StringSplitByString(const Char* s, const std::basic_string<Char>& sep, OutIt dest)
  {
		InternalStringSplitByString1<Char, const Char*, std::basic_string<Char>::const_iterator>(s, sep, dest);
  }
  template<typename Char, class OutIt>
	inline void StringSplitByString(const Char* s, const Char* sep, OutIt dest)
  {
		InternalStringSplitByString1<Char, const Char*, const Char*>(s, sep, dest);
  }
	// conversion cases.
	// note that because we cannot possibly convert output strings to that which the destination iterator wants,
	// then we can ONLY convert the separator
  template<typename CharL, typename CharR, typename Tleft, typename Tright, typename TOutIt>
	inline void InternalStringSplitByString2(Tleft in, Tright sep, TOutIt dest)
  {
		std::basic_string<CharL> temp;
		StringConvert(sep, temp);
		StringSplitByString(in, temp, dest);
  }
  template<typename CharL, typename CharR, typename TOutIt>
	inline void StringSplitByString(const CharL* in, const CharR* sep, TOutIt dest)
  {
		InternalStringSplitByString2<CharL, CharR>(in, sep, dest);
  }
  template<typename CharL, typename CharR, typename TOutIt>
	inline void StringSplitByString(const CharL* in, const std::basic_string<CharR>& sep, TOutIt dest)
  {
		InternalStringSplitByString2<CharL, CharR>(in, sep, dest);
  }
  template<typename CharL, typename CharR, typename TOutIt>
	inline void StringSplitByString(const std::basic_string<CharL>& in, const CharR* sep, TOutIt dest)
  {
		InternalStringSplitByString2<CharL, CharR>(in, sep, dest);
  }
  template<typename CharL, typename CharR, typename TOutIt>
	inline void StringSplitByString(const std::basic_string<CharL>& in, const std::basic_string<CharR>& sep, TOutIt dest)
  {
		InternalStringSplitByString2<CharL, CharR>(in, sep, dest);
  }


	// StringJoin --------------------------------------------------------------------------------------
	// ability to join from / to mismatching strings, with appropriate conversion.
  template<typename Char, typename TSep, typename InIt>
  inline std::basic_string<Char> InternalStringJoin(InIt start, InIt end, TSep sep_)
  {
    std::basic_string<Char> r;
		std::basic_string<Char> temp;
		std::basic_string<Char> sep;
		StringConvert(sep_, sep);
    while(start != end)
    {
			StringConvert(*start, temp);
      r.append(temp);
      ++ start;
      if(start != end)
      {
        r.append(sep);
      }
    }
    return r;
  }
	// no-conversion cases
  template<typename InIt, typename Char>
  inline std::basic_string<Char> StringJoin(InIt start, InIt end, const std::basic_string<Char>& sep)
  {
		return InternalStringJoin<Char>(start, end, sep);
	}
  template<typename InIt, typename Char>
  inline std::basic_string<Char> StringJoin(InIt start, InIt end, const Char* sep)
  {
		return InternalStringJoin<Char>(start, end, sep);
	}
	// conversion cases... probably not that usable
  template<typename CharRet, typename CharSep, typename InIt>
  inline std::basic_string<CharRet> StringJoin(InIt start, InIt end, const std::basic_string<CharSep>& sep)
  {
		return InternalStringJoin<CharRet>(start, end, sep);
	}
  template<typename CharRet, typename CharSep, typename InIt>
  inline std::basic_string<CharRet> StringJoin(InIt start, InIt end, const CharSep* sep)
  {
		return InternalStringJoin<CharRet>(start, end, sep);
	}
  
	// StringTrim --------------------------------------------------------------------------------------
  template<typename Char, typename Titer, typename TStrIn, typename TStrChars>
  inline std::basic_string<Char> InternalStringTrim(TStrIn s, TStrChars chars)
  {
		std::basic_string<Char> ret;
		ret.reserve(StringLength(s));
		// first skip.
		Titer it = StringBegin(s);
		while(true)
		{
			if(StringIsEnd(it, s))// hit the end of the string during skipping; it only contains trim chars!
				return ret;
			if(!StringContains(chars, *it))
				break;
			++ it;
		}
		// it points to the first non trim char. now find the last one
		Titer itTemp = it;
		Titer lastNonTrimChar = it;
		while(true)
		{
			if(StringIsEnd(itTemp, s))
				break;
			if(!StringContains(chars, *itTemp))
				lastNonTrimChar = itTemp;
			++ itTemp;
		}
		// append chars between it and lastNonTrimChar
		while(true)
		{
			ret.push_back(*it);
			if(it == lastNonTrimChar)
				return ret;
			++ it;
		}
  }
	// no-conversion cases
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const std::basic_string<Char>& chars)
  {
		return InternalStringTrim<Char, std::basic_string<Char>::const_iterator>(s, chars);
  }
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const Char* s, const std::basic_string<Char>& chars)
  {
		return InternalStringTrim<Char, const Char*>(s, chars);
  }
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const Char* chars)
  {
		return InternalStringTrim<Char, std::basic_string<Char>::const_iterator>(s, chars);
  }
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const Char* s, const Char* chars)
  {
		return InternalStringTrim<Char, const Char*>(s, chars);
  }
	// conversion cases
  template<typename CharLeft, typename CharRight>
  inline std::basic_string<CharLeft> StringTrim(const std::basic_string<CharLeft>& s, const std::basic_string<CharRight>& chars)
  {
		std::basic_string<CharLeft> temp;
		StringConvert(chars, temp);
		return InternalStringTrim<CharLeft, std::basic_string<CharLeft>::const_iterator>(s, temp);
  }
  template<typename CharLeft, typename CharRight>
  inline std::basic_string<CharLeft> StringTrim(const CharLeft* s, const std::basic_string<CharRight>& chars)
  {
		std::basic_string<CharLeft> temp;
		StringConvert(chars, temp);
		return InternalStringTrim<CharLeft, const CharLeft*>(s, temp);
  }
  template<typename CharLeft, typename CharRight>
  inline std::basic_string<CharLeft> StringTrim(const std::basic_string<CharLeft>& s, const CharRight* chars)
  {
		std::basic_string<CharLeft> temp;
		StringConvert(chars, temp);
		return InternalStringTrim<CharLeft, std::basic_string<CharLeft>::const_iterator>(s, temp);
  }
  template<typename CharLeft, typename CharRight>
  inline std::basic_string<CharLeft> StringTrim(const CharLeft* s, const CharRight* chars)
  {
		std::basic_string<CharLeft> temp;
		StringConvert(chars, temp);
		return InternalStringTrim<CharLeft, const CharLeft*>(s, temp);
  }


	// StringReplace --------------------------------------------------------------------------------------
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const std::basic_string<Char>& searchString, const std::basic_string<Char>& replaceString)
  {
		if(searchString.empty()) return src;// you can't have a 0 length search string.
    typedef std::basic_string<Char> _String;
    _String r;
    size_t pos = 0;
		while(1)
		{
			size_t found = src.find(searchString, pos);
			if(found == _String::npos)
			{
				// append the remainder of src.
				r.append(src, pos, src.length() - pos);
				return r;
			}
			r.append(src, pos, found - pos);// append chunk from src.
			r.append(replaceString);// append replacement
			pos = found + searchString.length();// advance
		}
  }
  template<typename Char>// these overloads help compile with constant strings
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const std::basic_string<Char>& searchString, const Char* replaceString)
  {
		return StringReplace(src, searchString, std::basic_string<Char>(replaceString));
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const Char* searchString, const std::basic_string<Char>& replaceString)
  {
		return StringReplace(src, std::basic_string<Char>(searchString), replaceString);
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const Char* searchString, const Char* replaceString)
  {
		return StringReplace(src, std::basic_string<Char>(searchString), std::basic_string<Char>(replaceString));
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const std::basic_string<Char>& searchString, const std::basic_string<Char>& replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), searchString, replaceString);
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const std::basic_string<Char>& searchString, const Char* replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), searchString, std::basic_string<Char>(replaceString));
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const Char* searchString, const std::basic_string<Char>& replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), std::basic_string<Char>(searchString), replaceString);
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const Char* searchString, const Char* replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), std::basic_string<Char>(searchString), std::basic_string<Char>(replaceString));
	}

	// StringToUpper --------------------------------------------------------------------------------------
	// NOTE: the stdlib toupper functions do not handle unicode very well, so this will have to do.
	inline std::wstring StringToUpper(const std::wstring& s)
	{
		Blob<wchar_t> buf(s.length() + 1);
		wcscpy(buf.GetBuffer(), s.c_str());
		CharUpperBuffW(buf.GetBuffer(), (DWORD)s.length());
		return std::wstring(buf.GetBuffer());
	}
	inline std::string StringToUpper(const std::string& s)
	{
		Blob<char> buf(s.length() + 1);
		strcpy(buf.GetBuffer(), s.c_str());
		CharUpperBuffA(buf.GetBuffer(), (DWORD)s.length());
		return std::string(buf.GetBuffer());
	}
	template<typename Char>
	inline std::basic_string<Char> StringToUpper(const std::basic_string<Char>& s)// last-ditch for alternative char types
	{
    std::basic_string<Char> r;
    r.reserve(s.size());
    std::basic_string<Char>::const_iterator it;
    for(it = s.begin(); it != s.end(); ++ it)
    {
			if(*it < 0x8000)
			{
				r.push_back((Char)CharUpperW((PWSTR)*it));
      }
      else
      {
				r.push_back(*it);
      }
    }
    return r;
	}
  template<typename Char>
  inline std::basic_string<Char> StringToUpper(const Char* s)
  {
		return StringToUpper(std::basic_string<Char>(s));
  }


	// StringToLower --------------------------------------------------------------------------------------
	inline std::wstring StringToLower(const std::wstring& s)
	{
		Blob<wchar_t> buf(s.length() + 1);
		wcscpy(buf.GetBuffer(), s.c_str());
		CharLowerBuffW(buf.GetBuffer(), (DWORD)s.length());
		return std::wstring(buf.GetBuffer());
	}
	inline std::string StringToLower(const std::string& s)
	{
		Blob<char> buf(s.length() + 1);
		strcpy(buf.GetBuffer(), s.c_str());
		CharLowerBuffA(buf.GetBuffer(), (DWORD)s.length());
		return std::string(buf.GetBuffer());
	}
	template<typename Char>
	inline std::basic_string<Char> StringToLower(const std::basic_string<Char>& s)// last-ditch for alternative char types
	{
    std::basic_string<Char> r;
    r.reserve(s.size());
    std::basic_string<Char>::const_iterator it;
    for(it = s.begin(); it != s.end(); ++ it)
    {
			if(*it < 0x8000)
			{
				r.push_back((Char)CharLowerW((PWSTR)*it));
      }
      else
      {
				r.push_back(*it);
      }
    }
    return r;
	}
  template<typename Char>
  inline std::basic_string<Char> StringToLower(const Char* s)
  {
		return StringToLower(std::basic_string<Char>(s));
  }



	// StringEquals. --------------------------------------------------------------------------------------
	template<typename TiterL, typename TiterR, typename TstrL, typename TstrR>
	inline bool InternalStringEquals1(TstrL lhs, TstrR rhs)
  {
		TiterL itl = StringBegin(lhs);
		TiterR itr = StringBegin(rhs);
		while(true)
		{
			bool lend = StringIsEnd(itl, lhs);
			bool rend = StringIsEnd(itr, rhs);
			if(lend)
				return rend;
			if(rend)
				return lend;
			if(*itl != *itr)
				return false;
			++ itl;
			++ itr;
		}
		return true;
  }
	// no-conversion cases
	template<typename Char>
  inline bool StringEquals(const Char* lhs, const Char* rhs)
	{
		return InternalStringEquals1<const Char*, const Char*>(lhs, rhs);
	}
	template<typename Char>
	inline bool StringEquals(const Char* lhs, const std::basic_string<Char>& rhs)
	{
		return InternalStringEquals1<const Char*, std::basic_string<Char>::const_iterator>(lhs, rhs);
	}
	template<typename Char>
  inline bool StringEquals(const std::basic_string<Char>& lhs, const Char* rhs)
	{
		return InternalStringEquals1<std::basic_string<Char>::const_iterator, const Char*>(lhs, rhs);
	}
	template<typename Char>
	inline bool StringEquals(const std::basic_string<Char>& lhs, const std::basic_string<Char>& rhs)
	{
		return InternalStringEquals1<std::basic_string<Char>::const_iterator, std::basic_string<Char>::const_iterator>(lhs, rhs);
	}
	// conversion cases
  template<typename CharL, typename CharR, typename Tleft, typename Tright>
	inline bool InternalStringEquals2(Tleft lhs, Tright rhs)
  {
		if(sizeof(CharL) > sizeof(CharR))
		{
			std::basic_string<CharL> temp;
			StringConvert(rhs, temp);
			return StringEquals(lhs, temp);
		}
		else
		{
			std::basic_string<CharR> temp;
			StringConvert(lhs, temp);
			return StringEquals(temp, rhs);
		}
  }
  template<typename CharL, typename CharR>
	inline bool StringEquals(const CharL* lhs, const CharR* rhs)
  {
		return InternalStringEquals2<CharL, CharR>(lhs, rhs);
  }
  template<typename CharL, typename CharR>
	inline bool StringEquals(const CharL* lhs, const std::basic_string<CharR>& rhs)
  {
		return InternalStringEquals2<CharL, CharR>(lhs, rhs);
  }
  template<typename CharL, typename CharR>
	inline bool StringEquals(const std::basic_string<CharL>& lhs, const CharR* rhs)
  {
		return InternalStringEquals2<CharL, CharR>(lhs, rhs);
  }
  template<typename CharL, typename CharR>
	inline bool StringEquals(const std::basic_string<CharL>& lhs, const std::basic_string<CharR>& rhs)
  {
		return InternalStringEquals2<CharL, CharR>(lhs, rhs);
  }



	// StringEqualsI --------------------------------------------------------------------------------------
	template<typename TiterL, typename TiterR, typename TstrL, typename TstrR>
	inline bool InternalStringEqualsI1(TstrL lhs, TstrR rhs)
  {
		TiterL itl = StringBegin(lhs);
		TiterR itr = StringBegin(rhs);
		while(true)
		{
			bool lend = StringIsEnd(itl, lhs);
			bool rend = StringIsEnd(itr, rhs);
			if(lend)
				return rend;
			if(rend)
				return lend;
			if(CharToLower(*itl) != CharToLower(*itr))
				return false;
			++ itl;
			++ itr;
		}
		return true;
  }
	// no-conversion cases
	template<typename Char>
  inline bool StringEqualsI(const Char* lhs, const Char* rhs)
	{
		return InternalStringEqualsI1<const Char*, const Char*>(lhs, rhs);
	}
	template<typename Char>
	inline bool StringEqualsI(const Char* lhs, const std::basic_string<Char>& rhs)
	{
		return InternalStringEqualsI1<const Char*, std::basic_string<Char>::const_iterator>(lhs, rhs);
	}
	template<typename Char>
  inline bool StringEqualsI(const std::basic_string<Char>& lhs, const Char* rhs)
	{
		return InternalStringEqualsI1<std::basic_string<Char>::const_iterator, const Char*>(lhs, rhs);
	}
	template<typename Char>
	inline bool StringEqualsI(const std::basic_string<Char>& lhs, const std::basic_string<Char>& rhs)
	{
		return InternalStringEqualsI1<std::basic_string<Char>::const_iterator, std::basic_string<Char>::const_iterator>(lhs, rhs);
	}
	// conversion cases
  template<typename CharL, typename CharR, typename Tleft, typename Tright>
	inline bool InternalStringEqualsI2(Tleft lhs, Tright rhs)
  {
		if(sizeof(CharL) > sizeof(CharR))
		{
			std::basic_string<CharL> temp;
			StringConvert(rhs, temp);
			return StringEqualsI(lhs, temp);
		}
		else
		{
			std::basic_string<CharR> temp;
			StringConvert(lhs, temp);
			return StringEqualsI(temp, rhs);
		}
  }
  template<typename CharL, typename CharR>
	inline bool StringEqualsI(const CharL* lhs, const CharR* rhs)
  {
		return InternalStringEqualsI2<CharL, CharR>(lhs, rhs);
  }
  template<typename CharL, typename CharR>
	inline bool StringEqualsI(const CharL* lhs, const std::basic_string<CharR>& rhs)
  {
		return InternalStringEqualsI2<CharL, CharR>(lhs, rhs);
  }
  template<typename CharL, typename CharR>
	inline bool StringEqualsI(const std::basic_string<CharL>& lhs, const CharR* rhs)
  {
		return InternalStringEqualsI2<CharL, CharR>(lhs, rhs);
  }
  template<typename CharL, typename CharR>
	inline bool StringEqualsI(const std::basic_string<CharL>& lhs, const std::basic_string<CharR>& rhs)
  {
		return InternalStringEqualsI2<CharL, CharR>(lhs, rhs);
  }



#endif

}





















// LibCC::Format uses the following number -> string conversion routines



























namespace LibCC
{
	// faster than std::basic_string ?

	// this needs to be a POD for the QuickStringList optimized vector.
	template<typename _Char>
	struct QuickStringData
	{
		size_t m_len;
		size_t m_allocated;
		static const size_t staticBufferSize = 16;
		_Char staticBuffer[staticBufferSize];
		_Char* dynBuffer;
		_Char* p;
	};

	// attaches to QuickStringData to act like a std::wstring.
	template<typename _Char>
	struct QuickString
	{
	//private:
	//	QuickString<_Char>& operator =(QuickString<_Char>& rhs)
	//	{
	//		return *this;
	//	}
	//	QuickString(QuickString<_Char>& rhs)
	//	{
	//	}

	public:
		QuickString(QuickStringData<_Char>* data_) :
			data(data_)
		{
		}

		inline const _Char* c_str() const
		{
			return data->p;
		}

		inline size_t size() const
		{
			return data->m_len;
		}

		inline void append(const _Char* c)
		{
			size_t inputLen = LibCC::StringLength(c);
			AddAlloc(inputLen);
			_Char* i = data->p + data->m_len;
			memcpy(i, c, sizeof(_Char) * inputLen);
			i += inputLen;
			*i = 0;
			data->m_len += inputLen;
		}

		inline void push_back(_Char ch)
		{
			AddAlloc(1);
			_Char* i = data->p + data->m_len;
			*i = ch;
			++ i;
			*i = 0;
			data->m_len ++;
		}

		inline void reserve(size_t n)
		{
			if(data->m_allocated >= n)
				return;

			if(data->p != data->staticBuffer)
			{
				HeapFree(GetProcessHeap(), 0, data->p);
			}
			data->m_len = 0;
			data->m_allocated = n + 1;
			data->dynBuffer = (_Char*)HeapAlloc(GetProcessHeap(), 0, data->m_allocated * sizeof(_Char));
			data->p = data->dynBuffer;
		}

	private:
		inline void AddAlloc(size_t additional)
		{
			if(data->m_allocated < (data->m_len + 1 + additional))// 1 for null term
			{
				data->m_allocated = max(additional + 1, data->m_allocated << 1);
				_Char* newp = (_Char*)HeapAlloc(GetProcessHeap(), 0, data->m_allocated * sizeof(_Char));
				memcpy(newp, data->p, data->m_len * sizeof(_Char));
				if(data->p != data->staticBuffer)
				{
					HeapFree(GetProcessHeap(), 0, data->p);
				}
				data->p = newp;
			}
		}

		QuickStringData<_Char>* data;
	};

	// optimized vector which handles construction / destruction of QuickStringData, and hands out QuickString to act
	// somewhat like a std::string
	template<typename _Char>
	struct QuickStringList
	{
	//private:
	//	QuickStringList<_Char>& operator =(QuickStringList<_Char>& rhs)
	//	{
	//		return *this;
	//	}
	//	QuickStringList(QuickStringList<_Char>& rhs)
	//	{
	//	}

	public:
		QuickStringList() :
			m_listLen(0),
			m_listAllocated(listStaticBufferSize),
			listp(listStaticBuffer)
		{
		}

		// hope we can avoid this 
		QuickStringList<_Char>& operator =(const QuickStringList<_Char>& rhs)
		{
			clear();

			m_listLen = rhs.m_listLen;
			m_listAllocated = rhs.m_listAllocated;

			// allocate.
			if(m_listAllocated > listStaticBufferSize)
			{
				listp = (QuickStringData<_Char>*)HeapAlloc(GetProcessHeap(), 0, sizeof(QuickStringData<_Char>) * m_listAllocated);
				listp = listDynBuffer;
			}

			// copy.
			memcpy(listp, rhs.listp, m_listLen * sizeof(QuickStringData<_Char>));
			// fix up pointers to static data
			QuickStringData<_Char>* i = listp;
			QuickStringData<_Char>* end = listp + m_listLen;
			for(; i != end; ++ i)
			{
				if(i->m_allocated <= QuickStringData<_Char>::staticBufferSize)
					i->p = i->staticBuffer;
			}

			return *this;
		}

		QuickStringList(const QuickStringList<_Char>& rhs) :
			m_listLen(0),
			m_listAllocated(listStaticBufferSize),
			listp(listStaticBuffer)
		{
			*this = rhs;
		}


		~QuickStringList()
		{
			// free all strings
			clear();
			if(listp != listStaticBuffer)
			{
				HeapFree(GetProcessHeap(), 0, listp);
			}
		}

		size_t size() const
		{
			return m_listLen;
		}

		void clear()
		{
			QuickStringData<_Char>* i = listp;
			QuickStringData<_Char>* end = listp + m_listLen;
			for(;i != end; ++ i)
			{
				if(i->p != i->staticBuffer)
				{
					HeapFree(GetProcessHeap(), 0, i->p);
				}
			}
			m_listLen = 0;
		}

		QuickString<_Char> operator[] (size_t index)
		{
			return QuickString<_Char>(&listp[index]);
		}

		QuickString<_Char> operator[] (size_t index) const
		{
			return QuickString<_Char>(&listp[index]);
		}

		// creates room for X strings, and returns the first one available, UNCONSTRUCTED.
		QuickStringData<_Char>* AddAlloc(size_t additional)
		{
			if(m_listAllocated < (m_listLen + additional))
			{
				// realloc. todo: actually use realloc
				size_t newAllocated = max(m_listAllocated * 2, m_listLen + additional);
				QuickStringData<_Char>* newp = (QuickStringData<_Char>*)HeapAlloc(GetProcessHeap(), 0, sizeof(QuickStringData<_Char>) * newAllocated);
				// copy dynBuffer to newp
				memcpy(newp, listp, m_listLen * sizeof(QuickStringData<_Char>));
				//memset(p, 0, m_len * sizeof(QuickStringData<_Char>));// DEBUGGING PURPOSES ONLY
				if(listp != listStaticBuffer)
				{
					HeapFree(GetProcessHeap(), 0, listp);
				}

				m_listAllocated = newAllocated;
				listp = listDynBuffer;

				// fix up pointers to static data
				QuickStringData<_Char>* i = listp;
				QuickStringData<_Char>* end = listp + m_listLen;
				for(; i != end; ++ i)
				{
					if(i->m_allocated <= QuickStringData<_Char>::staticBufferSize)
						i->p = i->staticBuffer;
				}
			}
			m_listLen ++;
			return listp + m_listLen - 1;
		}

		void ConstructAlloc(QuickStringData<_Char>* data)
		{
			if(data->m_allocated > QuickStringData<_Char>::staticBufferSize)
			{
				data->dynBuffer = (_Char*)HeapAlloc(GetProcessHeap(), 0, data->m_allocated * sizeof(_Char));
				data->p = data->dynBuffer;
			}
			else
			{
				data->p = data->staticBuffer;
			}
		}

		QuickString<_Char> push_back()
		{
			QuickStringData<_Char>* back = AddAlloc(1);
			ConstructQuickString(back);
			return QuickString<_Char>(back);
		}

		void ConstructQuickString(QuickStringData<_Char>* data)
		{
			data->p = data->staticBuffer;
			data->m_len = 0;
			data->m_allocated = QuickStringData<_Char>::staticBufferSize;
		}

		QuickString<_Char> push_back(const _Char* s, _Char open, _Char close)
		{
			QuickStringData<_Char>* back = AddAlloc(1);
			ConstructQuickString(back, s, open, close);
			return QuickString<_Char>(back);
		}

		void ConstructQuickString(QuickStringData<_Char>* data, const _Char* s, _Char open, _Char close)
		{
			size_t inputLen = s == 0 ? 0 : LibCC::StringLength(s);
			data->m_len = inputLen + 2;
			data->m_allocated = max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
			ConstructAlloc(data);

			_Char* i = data->p;
			*i = open;
			++i;
			memcpy(i, s, sizeof(_Char) * inputLen);
			i += inputLen;
			*i = close;
			++i;
			*i = 0;
		}

		QuickString<_Char> push_back(const _Char* s, int maxLen)
		{
			QuickStringData<_Char>* back = AddAlloc(1);
			ConstructQuickString(back, s, maxLen);
			return QuickString<_Char>(back);
		}

		void ConstructQuickString(QuickStringData<_Char>* data, const _Char* s, int maxLen)
		{
			data->m_len = s == 0 ? 0 : min((int)LibCC::StringLength(s), maxLen);
			data->m_allocated = max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
			ConstructAlloc(data);

			_Char* i = data->p;
			memcpy(i, s, sizeof(_Char) * data->m_len);
			i += data->m_len;
			*i = 0;
		}

		QuickString<_Char> push_back(const _Char* s)
		{
			QuickStringData<_Char>* back = AddAlloc(1);
			ConstructQuickString(back, s);
			return QuickString<_Char>(back);
		}

		void ConstructQuickString(QuickStringData<_Char>* data, const _Char* s)
		{
			data->m_len = s == 0 ? 0 : LibCC::StringLength(s);
			data->m_allocated = max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
			ConstructAlloc(data);

			_Char* i = data->p;
			memcpy(i, s, sizeof(_Char) * data->m_len);
			i += data->m_len;
			*i = 0;
		}

		QuickString<_Char> push_back(_Char ch, size_t count)
		{
			QuickStringData<_Char>* back = AddAlloc(1);
			ConstructQuickString(back, ch, count);
			return QuickString<_Char>(back);
		}

		void ConstructQuickString(QuickStringData<_Char>* data, _Char ch, size_t count)
		{
			data->m_len = count;
			data->m_allocated = max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
			ConstructAlloc(data);

			_Char* i = data->p;
			_Char* end = i + count;
			while(i != end)
			{
				*i = ch;
				++ i;
			}
			*i = 0;
		}

		QuickString<_Char> push_back(const _Char* s, int maxLen, _Char open, _Char close)
		{
			QuickStringData<_Char>* back = AddAlloc(1);
			ConstructQuickString(back, s, maxLen, open, close);
			return QuickString<_Char>(back);
		}

		void ConstructQuickString(QuickStringData<_Char>* data, const _Char* s, int maxLen, _Char open, _Char close)
		{
			data->m_len = s == 0 ? min(maxLen, 2) : min((int)LibCC::StringLength(s) + 2, maxLen);
			data->m_allocated = max(data->m_len + 1, QuickStringData<_Char>::staticBufferSize);
			ConstructAlloc(data);

			_Char* i = data->p;
			if(data->m_len > 0)
			{
				*i = open;
				++i;
				if(data->m_len > 1)
				{
					memcpy(i, s, sizeof(_Char) * (data->m_len - 2));
					i += data->m_len - 2;
					*i = close;
					++i;
				}
			}
			*i = 0;
		}

		size_t m_listLen;
		size_t m_listAllocated;
		static const size_t listStaticBufferSize = 16;
		QuickStringData<_Char> listStaticBuffer[listStaticBufferSize];
		QuickStringData<_Char>* listDynBuffer;
		QuickStringData<_Char>* listp;
	};

	template<typename Tlhs, typename Trhs>
	inline void __StringAppend(QuickString<Tlhs>& lhs, Trhs* rhs)
	{
		lhs.append(StringConvert<Tlhs>(rhs).c_str());
	}

    template<typename _Char>
		inline void _RuntimeAppendZeroFloat(size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, QuickString<_Char>& output)
		{
			// zero.
			// pre-decimal part.
			// "-----0"
			if(IntegralWidthMin > 0)
			{
				// append padding
				output.reserve(output.size() + IntegralWidthMin);
				for(size_t i = 1; i < IntegralWidthMin; ++ i)
				{
					output.push_back(static_cast<_Char>(PaddingChar));
				}
				// append the integral zero
				output.push_back('0');
			}
			if(DecimalWidthMax)
			{
				// if there are any decimal digits to set, then just append ".0"
				output.reserve(output.size() + 2);
				output.push_back('.');
				output.push_back('0');
			}
		}

    template<typename FloatType, typename _Char>
    inline void _RuntimeAppendNormalizedFloat(FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, QuickString<_Char>& output)
		{
			// how do we know how many chars we will use?  we don't right now.
			_Char* buf = reinterpret_cast<_Char*>(_alloca(sizeof(_Char) * (2200 + IntegralWidthMin + DecimalWidthMax)));
			long IntegralWidthLeft = static_cast<long>(IntegralWidthMin);
			_Char* middle = buf + 2100 - DecimalWidthMax;
			_Char* sIntPart = middle;
			_Char* sDecPart = middle;
			FloatType::Mantissa _int;// integer part raw value
			FloatType::Mantissa _dec;// decimal part raw value
			FloatType::Exponent exp = _f.GetExponent();// exponent raw value
			FloatType::Mantissa m = _f.GetMantissa();
			size_t DecBits;// how many bits out of the mantissa are used by the decimal part?

			const size_t BasicTypeBits = sizeof(FloatType::BasicType)*8;
			if((exp < FloatType::MantissaBits) && (exp > (FloatType::MantissaBits - BasicTypeBits)))
			{
				// write the integral (before decimal point) part.
				DecBits = _f.MantissaBits - exp;
				_int = m >> DecBits;// the integer part.
				_dec = m & (((FloatType::Mantissa)1 << DecBits) - 1);
				do
				{
					--IntegralWidthLeft;
					*(-- sIntPart) = DigitToChar(static_cast<unsigned char>(_int % Base));
					_int = _int / static_cast<FloatType::Mantissa>(Base);
				}
				while(_int);

				while(IntegralWidthLeft > 0)
				{
					*(-- sIntPart) = static_cast<_Char>(PaddingChar);
					IntegralWidthLeft --;
				}

				// write the after-decimal part.  here we basically do long division!
				// the decimal part is basically a fraction that we convert bases on.
				// since we need to deal with a number as large as the denominator, this will only work
				// when DecBits is less than 32 (for single precsion)
				if(DecimalWidthMax)
				{
					size_t DecimalWidthLeft = DecimalWidthMax;
					middle[0] = '.';
					FloatType::Mantissa denominator = (FloatType::Mantissa)1 << DecBits;// same as 'capacity'.
					FloatType::Mantissa& numerator(_dec);
					numerator *= static_cast<FloatType::Mantissa>(Base);
					FloatType::Mantissa digit;
					while(numerator && DecimalWidthLeft)
					{
						digit = numerator / denominator;// integer division
						// add the digit, and drill down into the remainder.
						*(++ sDecPart) = DigitToChar(static_cast<unsigned char>(digit % Base));
						numerator -= digit * denominator;
						numerator *= static_cast<FloatType::Mantissa>(Base);
						-- DecimalWidthLeft;
					}
				}
				else
				{
					middle[0] = 0;
				}
			}
			else
			{
				// We are here because doing conversions would take large numbers - too large to hold in
				// a InternalType integral.  So until i can come up with a cooler way to do it, i will
				// just do floating point divides and 

				// do the integral part just like a normal int.
				FloatType::This integerPart(_f);
				integerPart.RemoveDecimal();
				integerPart.AbsoluteValue();
				FloatType::BasicType fBase = static_cast<FloatType::BasicType>(Base);
				do
				{
					IntegralWidthLeft --;
					// at this point integerPart has no decimal and Base of course doesnt.
					*(-- sIntPart) = DigitToChar(static_cast<unsigned char>(fmod(integerPart.m_BasicVal, fBase)));
					integerPart.m_BasicVal /= Base;
					integerPart.RemoveDecimal();
				}
				while(integerPart.m_BasicVal > 0);

				while(IntegralWidthLeft > 0)
				{
					*(-- sIntPart) = static_cast<_Char>(PaddingChar);
					IntegralWidthLeft --;
				}

				// now the decimal part.
				if(DecimalWidthMax)
				{
					size_t DecimalWidthLeft = DecimalWidthMax;
					middle[0] = '.';
					FloatType::This val(_f);
					val.AbsoluteValue();
					// remove integer part.
					FloatType::This integerPart(val);
					integerPart.RemoveDecimal();
					val.m_BasicVal -= integerPart.m_BasicVal;
					do
					{
						DecimalWidthLeft --;
						val.m_BasicVal *= Base;
						// isolate the integral part
						integerPart.m_BasicVal = val.m_BasicVal;
						integerPart.RemoveDecimal();
						*(++ sDecPart) = DigitToChar(static_cast<unsigned char>(fmod(integerPart.m_BasicVal, fBase)));
						// use the integral part to leave only the decimal part.
						val.m_BasicVal -= integerPart.m_BasicVal;
					}
					while((val.m_BasicVal > 0) && DecimalWidthLeft);
				}
				else
				{
					middle[0] = 0;
				}
			}

			// display the sign
			if(_f.IsNegative())
			{
				*(-- sIntPart) = '-';
			}
			else if(ForceSign)
			{
				*(-- sIntPart) = '+';
			}

			// null terminate
			*(++ sDecPart) = 0;

			__StringAppend(output, sIntPart);
		}

    /*
      Converts any floating point (LibCC::IEEEFloat<>) number to a string, and appends it just like any other string.
    */
    template<typename FloatType, typename _Char>
		inline void _RuntimeAppendFloat(const FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, QuickString<_Char>& output)
		{
			if(!(_f.m_val & _f.ExponentMask))
			{
				// exponont = 0.  that means its either zero or denormalized.
				if(_f.m_val & _f.MantissaMask)
				{
					// denormalized
					__StringAppend(output, "Unsupported denormalized number");
				}
				else
				{
					// zero
					return _RuntimeAppendZeroFloat(DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, output);
				}
			}
			else if((_f.m_val & _f.ExponentMask) == _f.ExponentMask)
			{
				// exponent = MAX.  either infinity or NAN.
				if(_f.IsPositiveInfinity())
				{
					__StringAppend(output, "+Inf");
				}
				else if(_f.IsNegativeInfinity())
				{
					__StringAppend(output, "-Inf");
				}
				else if(_f.IsQNaN())
				{
					__StringAppend(output, "QNaN");
				}
				else if(_f.IsSNaN())
				{
					__StringAppend(output, "SNaN");
				}
			}

			// normalized number.
			_RuntimeAppendNormalizedFloat(_f, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, output);
		}

    template<typename _Char, typename FloatType, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    inline void _AppendFloat(const FloatType& _f, QuickString<_Char>& output)
		{
	    return _RuntimeAppendFloat<FloatType>(_f, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, output);
		}

    template<size_t Width, typename T>
    struct _BufferSizeNeededInteger
    {
      // sizeof(T) * 8 == how many bits to store the value.  considering
      // the smallest base supported is base 2 (binary), thats exactly how
      // many digits maximum for an integer type.  +1 for null terminator
      // this is basically max(size based on width, size based on sizeof())
      // and +1 for the sign.
      static const long Value = (sizeof(T) * 8) + 2 > (Width + 1) ? (sizeof(T) * 8) + 2 : (Width + 1);
    };

    template<typename T>
    inline long _RuntimeBufferSizeNeededInteger(size_t Width)
		{
	    return (long)((sizeof(T) * 8) + 2 > (Width + 1) ? (sizeof(T) * 8) + 2 : (Width + 1));
		}

    // buf must point to a null terminator.  It is "pulled back" and the result is returned.
    // its simply faster to build the string in reverse order.
    template<typename T, typename _Char>
    inline _Char* _RuntimeUnsignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar)
		{
			long PadRemaining = static_cast<long>(Width);
			_Char _PadChar = PaddingChar;
			do
			{
				PadRemaining --;
				*(--buf) = static_cast<_Char>(DigitToChar(static_cast<unsigned char>(num % Base)));
				num = num / static_cast<T>(Base);
			}
			while(num);

			while(PadRemaining-- > 0)
			{
				*(--buf) = _PadChar;
			}
			return buf;
		}

    template<typename _Char, size_t Base, size_t Width, _Char PaddingChar, typename T>
    inline static _Char* _UnsignedNumberToString(_Char* buf, T num)
		{
			if(Base < 2)
			{
				static _Char x[] = { 0 };
				return x;
			}
			ptrdiff_t PadRemaining = static_cast<ptrdiff_t>(Width);
			_Char _PadChar = PaddingChar;
			do
			{
				PadRemaining --;
				*(--buf) = static_cast<_Char>(DigitToChar(static_cast<unsigned char>(num % Base)));
				num = num / static_cast<T>(Base);
			}
			while(num);

			while(PadRemaining-- > 0)
			{
				*(--buf) = _PadChar;
			}
			return buf;
		}

    // same thing, but params can be set at runtime
    template<typename T, typename _Char>
    inline static _Char* _RuntimeSignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar, bool ForceSign)
		{
			if(Base < 2)
			{
				static _Char x[] = { 0 };
				return x;
			}
			if(num < 0)
			{
				buf = _RuntimeUnsignedNumberToString(buf, -num, Base, Width-1, PaddingChar);
				*(--buf) = '-';
			}
			else
			{
				if(ForceSign)
				{
					buf = _RuntimeUnsignedNumberToString(buf, num, Base, Width-1, PaddingChar);
					*(--buf) = '+';
				}
				else
				{
					buf = _RuntimeUnsignedNumberToString(buf, num, Base, Width, PaddingChar);
				}
			}
			return buf;
		}

    template<typename _Char, size_t Base, size_t Width, _Char PaddingChar, bool ForceSign, typename T>
    inline static _Char* _SignedNumberToString(_Char* buf, T num)
		{
			if(num < 0)
			{
				buf = _UnsignedNumberToString<_Char, Base, Width-1, PaddingChar, T>(buf, -num);
				*(--buf) = '-';
			}
			else
			{
				if(ForceSign)
				{
					buf = _UnsignedNumberToString<_Char, Base, Width-1, PaddingChar, T>(buf, num);
					*(--buf) = '+';
				}
				else
				{
					buf = _UnsignedNumberToString<_Char, Base, Width, PaddingChar, T>(buf, num);
				}
			}
			return buf;
		}



}



















// LibCC::Format coming up...




























namespace LibCC
{
  // FormatX class declaration -----------------------------------------------------------------------------------
  template<typename Ch = char, typename Traits = std::char_traits<Ch>, typename Alloc = std::allocator<Ch> >
  class FormatX
  {
  public:
    typedef Ch _Char;
    typedef Traits _Traits;
    typedef Alloc _Alloc;
    typedef std::basic_string<_Char, _Traits, _Alloc> _String;
    typedef FormatX<_Char, _Traits, _Alloc> _This;

		static const _Char OpenQuote = '\"';
		static const _Char CloseQuote = '\"';

    static const _Char ReplaceChar = '%';
		static const _Char NamedArgOpenChar = '{';
		static const _Char NamedArgCloseChar = '}';
    static const _Char EscapeChar = '^';
    static const _Char NewlineChar = '|';

		inline static bool IsUnicode()
		{
			return sizeof(_Char) == sizeof(wchar_t);
		}

		// notepad, winword, ultraedit do not support these characters
		// but wordpad & devenv do. not really enough support to justify using these ever, considering anything that supports them will also support \r\n
		inline static void AppendNewLine(_String& s)
		{
#if LIBCC_UNICODENEWLINES == 1
			if(IsUnicode())
			{
				s.push_back(0x2028);// the Unicode line separator char
			}
			else
			{
				s.push_back('\r');
				s.push_back('\n');
			}
#else
				s.push_back('\r');
				s.push_back('\n');
#endif
		}

		inline static void AppendNewParagraph(_String& s)
		{
#if LIBCC_UNICODENEWLINES == 1
			if(IsUnicode())
			{
				s.push_back(0x2029);// the Unicode new paragraph char
			}
			else
			{
				s.push_back('\r');
				s.push_back('\n');
			}
#else
				s.push_back('\r');
				s.push_back('\n');
#endif
		}

    // Construction / Assignment
		LIBCC_INLINE FormatX() :
			m_isRendered(false),
			m_argumentCharSize(0)
		{
		}

    // Construction / Assignment

		//LIBCC_INLINE FormatX(const _This& rhs) :
		//	m_isRendered(rhs.m_isRendered),
		//	m_Format(rhs.m_Format),
		//	m_rendered(rhs.m_rendered),
		//	m_argumentCharSize(rhs.m_argumentCharSize)
		//{
		//	m_dynArguments = rhs.m_dynArguments;
		//}

		//_This& operator =(const _This& rhs)
		//{
		//	m_isRendered = rhs.m_isRendered;
		//	m_Format = rhs.m_Format;
		//	m_rendered = rhs.m_rendered;
		//	m_argumentCharSize = rhs.m_argumentCharSize;
		//	m_dynArguments = rhs.m_dynArguments;
		//	return *this;
		//}

		explicit LIBCC_INLINE FormatX(const _String& s) :
			m_Format(s),
			m_isRendered(false),
			m_argumentCharSize(0)
		{
		}

    explicit LIBCC_INLINE FormatX(const _Char* s) :
			m_Format(s),
			m_isRendered(false),
			m_argumentCharSize(0)
		{
		}

    template<typename CharX>
    explicit inline FormatX(const CharX* s) :
			m_isRendered(false),
			m_argumentCharSize(0)
		{
			StringConvert(s, m_Format);
		}

		template<typename CharX>
		explicit LIBCC_INLINE FormatX(const std::basic_string<CharX>& s) :
			m_isRendered(false),
			m_argumentCharSize(0)
		{
			StringConvert(s, m_Format);
		}

#ifdef WIN32
    // construct from stringtable resource
    LIBCC_INLINE FormatX(HINSTANCE hModule, UINT stringID) :
			m_isRendered(false),
			m_argumentCharSize(0)
		{
			LoadStringX(hModule, stringID, m_Format);
		}

    LIBCC_INLINE FormatX(UINT stringID) :
			m_isRendered(false),
			m_argumentCharSize(0)
		{
			LoadStringX(GetModuleHandle(NULL), stringID, m_Format);
		}
#endif

		LIBCC_INLINE void Clear()
		{
			m_argumentCharSize = 0;
			m_isRendered = false;
			m_dynArguments.clear();
			//m_dynArgumentCount.myval = 0;
			m_Format.clear();
		}

    template<typename CharX>
    LIBCC_INLINE void SetFormat(const CharX* s)
		{
			Clear();
			if(!s)
				return;
			StringConvert(s, m_Format);
		}

    LIBCC_INLINE void SetFormat(const _String& s)
		{
			Clear();
			m_Format = s;
		}

    LIBCC_INLINE void SetFormat(const _Char* s)
		{
			Clear();
			if(s == 0)
				return;
			m_Format = s;
		}

    template<typename CharX>
		LIBCC_INLINE void SetFormat(const std::basic_string<CharX>& s)
		{
			Clear();
			StringConvert(s, m_Format);
		}

#ifdef WIN32
    // assign from stringtable resource
    LIBCC_INLINE void SetFormat(HINSTANCE hModule, UINT stringID)
		{
			Clear();
			LoadStringX(hModule, stringID, m_Format);
		}

		LIBCC_INLINE void SetFormat(UINT stringID)
		{
			Clear();
			LoadStringX(GetModuleHandle(NULL), stringID, m_Format);
		}
#endif

		// "GET" methods
    LIBCC_INLINE const _String& Str() const
		{
			Render();
			return m_rendered;
		}
    LIBCC_INLINE const _Char* CStr() const
		{
			Render();
			return m_rendered.c_str();
		}
#if CCSTR_OPTION_AUTOCAST == 1
		LIBCC_INLINE operator _String() const
		{
			Render();
			return m_rendered;
		}
    LIBCC_INLINE operator const _Char*() const
		{
			Render();
			return m_rendered.c_str();
		}
#endif

    // POINTER - NOT portable when unsigned long != pointer size. -----------------------------
    template<typename T>
    LIBCC_INLINE _This& p(const T* v)
		{
			_Char arg[11] = { '0', 'x' };
			unsigned long temp = *(reinterpret_cast<unsigned long*>(&v));
			_UnsignedNumberToString<_Char, 16, 8, '0'>(arg + 10, temp);
			return s(arg);
		}

    // CHARACTER (count) -----------------------------
    template<typename T>
    LIBCC_INLINE _This& c(T v)
		{
			AddArg(static_cast<_Char>(v), 1);
			return *this;
		}

    template<typename T>
    LIBCC_INLINE _This& c(T v, size_t count)
		{
			AddArg(static_cast<_Char>(v), count);
			return *this;
		}

    // STRING (maxlen) -----------------------------
    template<size_t MaxLen>
		LIBCC_INLINE _This& s(const _Char* s)
		{
			AddArg(s, MaxLen);
			return *this;
		}

    LIBCC_INLINE _This& s(const _Char* s, size_t MaxLen)
		{
			AddArg(s, (int)MaxLen);
			return *this;
		}

    LIBCC_INLINE _This& s(const _Char* s)
		{
			AddArg(s);
			return *this;
		}

    template<size_t MaxLen>
		LIBCC_INLINE _This& s(const _String& s)
		{
			AddArg(s.c_str(), (int)MaxLen);
			return *this;
		}

    LIBCC_INLINE _This& s(const _String& s, size_t MaxLen)
		{
			AddArg(s.c_str(), (int)MaxLen);
			return *this;
		}

    LIBCC_INLINE _This& s(const _String& s)
		{
			AddArg(s.c_str());
			return *this;
		}

		// now all that but in foreign char types
    template<typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign)
		{
			if(foreign)
			{
				_String native;
				StringConvert(foreign, native);
				return s(native);
			}
			return *this;
		}

		/*
			dilemma here is... do we convert foreign to native first, or truncate the string first? if i want
			to support localized strings correctly i should convert first. but that may mean potentially creating
			copies of huge strings when maxlen might be very small. i figure that's a rare enough case that i
			should go for accuracy.
		*/
    template<size_t MaxLen, typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign)
		{
			if(foreign)
			{
				_String native;
				StringConvert(foreign, native);
				return s<MaxLen>(native);
			}
			return *this;
		}

    template<typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign, size_t MaxLen)
		{
			if(foreign)
			{
				_String native;
				StringConvert(foreign, native);
				return s(native, MaxLen);
			}
			return *this;
		}
    
		template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			StringConvert(x, native);
			return s(native);
		}
    
		template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			StringConvert(x, native);
			return s<MaxLen>(native);
		}
    
		template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen)
		{
			_String native;
			StringConvert(x, native);
			return s(native, MaxLen);
		}
		
		LIBCC_INLINE _This& NewLine()
		{
			AppendNewLine(m_Composite);
			return *this;
		}
		
		LIBCC_INLINE _This& NewParagraph()
		{
			AppendNewParagraph(m_Composite);
			return *this;
		}

    // QUOTED STRINGS (maxlen)
    template<size_t MaxLen>
    LIBCC_INLINE _This& qs(const _Char* s)
		{
			AddArg(s, MaxLen, OpenQuote, CloseQuote);
			return *this;
		}

    LIBCC_INLINE _This& qs(const _Char* s, size_t MaxLen)
		{
			AddArg(s, (int)MaxLen, OpenQuote, CloseQuote);
			return *this;
		}

    LIBCC_INLINE _This& qs(const _Char* s)
		{
			AddArg(s, OpenQuote, CloseQuote);
			return *this;
		}

		template<size_t MaxLen>
    LIBCC_INLINE _This& qs(const _String& s)
		{
			AddArg(s.c_str(), (int)MaxLen, OpenQuote, CloseQuote);
			return *this;
		}

    LIBCC_INLINE _This& qs(const _String& s, size_t MaxLen)
		{
			AddArg(s.c_str(), (int)MaxLen, OpenQuote, CloseQuote);
			return *this;
		}

    LIBCC_INLINE _This& qs(const _String& s)
		{
			AddArg(s.c_str(), OpenQuote, CloseQuote);
			return *this;
		}

		// (now the same stuff but with foreign characters)
    template<typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign)
		{
			if(foreign)
			{
				_String native;
				StringConvert(foreign, native);
				return qs(native);
			}
			return *this;
		}

    template<size_t MaxLen, typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign)
		{
			if(foreign)
			{
				_String native;
				StringConvert(foreign, native);
				return qs<MaxLen>(native);
			}
			return *this;
		}

    template<typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign, size_t MaxLen)
		{
			if(foreign)
			{
				_String native;
				StringConvert(foreign, native);
				return qs(native, MaxLen);
			}
			return *this;
		}

    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			StringConvert(x, native);
			return qs(native);
		}

    template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			StringConvert(x, native);
			return qs<MaxLen>(native);
		}

    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen)
		{
			_String native;
			StringConvert(x, native);
			return qs(native, MaxLen);
		}

    // UNSIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& ul(unsigned long n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned long>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_UnsignedNumberToString<_Char, Base, Width, PadChar>(p, n));
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& ul(unsigned long n)
		{
			return ul<Base, Width, '0'>(n);
		}
    
		template<size_t Base>
    LIBCC_INLINE _This& ul(unsigned long n)
		{
			return ul<Base, 0, '0'>(n);
		}

    LIBCC_INLINE _This& ul(unsigned long n)
		{
			return ul<10, 0, '0'>(n);
		}

    LIBCC_INLINE _This& ul(unsigned long n, size_t Base, size_t Width = 0, _Char PadChar = '0')
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeUnsignedNumberToString<unsigned long>(p, n, Base, Width, PadChar));
		}

    // SIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    LIBCC_INLINE _This& l(signed long n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, signed long>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_SignedNumberToString<_Char, Base, Width, PadChar, ForceShowSign>(p, n));
		}

    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& l(signed long n)
		{
			return l<Base, Width, PadChar, false>(n);
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& l(signed long n)
		{
			return l<Base, Width, '0', false>(n);
		}

    template<size_t Base>
    LIBCC_INLINE _This& l(signed long n)
		{
			return l<Base, 0, '0', false>(n);
		}

    LIBCC_INLINE _This& l(signed long n)
		{
			return l<10, 0, '0', false>(n);
		}

    LIBCC_INLINE _This& l(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeSignedNumberToString(p, n, Base, Width, PadChar, ForceShowSign));
		}

    // UNSIGNED INT (just stubs for ul()) -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    _This& ui(unsigned int n)
    {
      return ul<Base, Width, PadChar>(n);
    }
    template<size_t Base, size_t Width>
    _This& ui(unsigned int n)
    {
      return ul<Base, Width>(n);
    }
    template<size_t Base>
    _This& ui(unsigned int n)
    {
      return ul<Base>(n);
    }
    _This& ui(unsigned int n)
    {
      return ul(n);
    }
    _This& ui(unsigned int n, size_t Base, size_t Width = 0, _Char PadChar = '0')
    {
      return ul(n, Base, Width, PadChar);
    }

    // SIGNED INT -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    _This& i(signed int n)
    {
      return l<Base, Width, PadChar, ForceShowSign>(n);
    }
    template<size_t Base, size_t Width, _Char PadChar>
    _This& i(signed int n)
    {
      return l<Base, Width, PadChar>(n);
    }
    template<size_t Base, size_t Width>
    _This& i(signed int n)
    {
      return l<Base, Width>(n);
    }
    template<size_t Base>
    _This& i(signed int n)
    {
      return l<Base>(n);
    }
    _This& i(signed int n)
    {
      return l(n);
    }
    _This& i(signed int n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return l(n, Base, Width, PadChar, ForceShowSign);
    }

    // FLOAT ----------------------------- 3.14   [intwidth].[decwidth]
    // integralwidth is the MINIMUM digits.  Decimalwidth is the MAXIMUM digits.
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    LIBCC_INLINE _This& f(float val)
		{
	    _AppendFloat<_Char, SinglePrecisionFloat, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign>(val, m_Composite);
			return *this;
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
		}

    template<size_t DecimalWidthMax>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& f(float val)
		{
	    return f<2, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& f(float val, size_t DecimalWidthMax, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
		{
	    _RuntimeAppendFloat<SinglePrecisionFloat>(val, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, m_Composite);
			return *this;
		}

    // DOUBLE -----------------------------
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    LIBCC_INLINE _This& d(double val)
		{
			QuickString<_Char> back = AddArg();
	    _AppendFloat<_Char, DoublePrecisionFloat, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign>(val, back);
			m_argumentCharSize += back.size();
			return *this;
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
		}

    template<size_t DecimalWidthMax>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& d(double val)
		{
			return d<3, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& d(double val, size_t DecimalWidthMax, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
		{
			QuickString<_Char> n = AddArg();
	    _RuntimeAppendFloat<DoublePrecisionFloat, _Char>(val, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, n);
			m_argumentCharSize += n.size();
			return *this;
		}

    // UNSIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_UnsignedNumberToString<_Char, Base, Width, PadChar>(p, n));
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
	    return ui64<Base, Width, '0'>(n);
		}

    template<size_t Base> 
    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
	    return ui64<Base, 0, 0>(n);
		}

    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
	    return ui64<10, 0, 0>(n);
		}

    LIBCC_INLINE _This& ui64(unsigned __int64 n, size_t Base, size_t Width = 0, _Char PadChar = '0')
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeUnsignedNumberToString<unsigned __int64>(p, n, Base, Width, PadChar));
		}

    // SIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    LIBCC_INLINE _This& i64(signed __int64 n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_SignedNumberToString<Base, Width, PadChar, ForceShowSign>(p, n));
		}

    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<Base, Width, PadChar, false>(n);
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<Base, Width, '0', false>(n);
		}

    template<size_t Base>
    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<Base, 0, 0, false>(n);
		}

    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<10, 0, 0, false>(n);
		}

    LIBCC_INLINE _This& i64(signed __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeSignedNumberToString<signed __int64>(p, n, Base, Width, PadChar, ForceShowSign));
		}

    // GETLASTERROR() -----------------------------
#ifdef WIN32
    LIBCC_INLINE _This& gle(int code)
		{
			_String str;
			FormatMessageGLE(str, code);
			return s(str);
		}

    LIBCC_INLINE _This& gle()
		{
	    return gle(GetLastError());
		}
#endif

    // CONVENIENCE OPERATOR () -----------------------------
    _This& operator ()(int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return i(n, Base, Width, PadChar, ForceShowSign);
    }
    _This& operator ()(unsigned int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0')
    {
      return ui(n, Base, Width, PadChar);
    }
    _This& operator ()(__int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return i64(n, Base, Width, PadChar, ForceShowSign);
    }
    _This& operator ()(unsigned __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0')
    {
      return ui64(n, Base, Width, PadChar);
    }
    _This& operator ()(float n, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
    {
      return f(n, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, Base);
    }
    _This& operator ()(double n, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
    {
      return d(n, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, Base);
    }
    _This& operator ()(char* n)
    {
      return s(n);
    }
    _This& operator ()(wchar_t* n)
    {
      return s(n);
    }
    _This& operator ()(const std::string& n)
    {
      return s(n);
    }
    _This& operator ()(const std::wstring& n)
    {
      return s(n);
    }

  private:

		LIBCC_INLINE void Render() const
		{
			if(m_isRendered)
				return;
			int currentSequentialArg = 0;
			int highestUsedSequentialArg = -1;

			m_isRendered = true;
			m_rendered.clear();
			m_rendered.reserve(m_Format.size() + m_argumentCharSize);

			const _Char* begin = m_Format.c_str();
			const _Char* end = begin + m_Format.size();
			const _Char* it = begin;

			for(; it != end; ++it)
			{
				_Char ch = *it;
				switch(ch)
				{
				case EscapeChar:
					++ it;
					if(it != end)
					{
						m_rendered.push_back(*it);
					}
					break;
				case NewlineChar:
					AppendNewLine(m_rendered);
					break;
				case ReplaceChar:
					if(currentSequentialArg >= (int)m_dynArguments.size())
					{
						m_rendered.push_back(ch);// if you put too many replacechars, then just ignore it.
					}
					else
					{
						m_rendered.append(GetArg(currentSequentialArg).c_str());
						highestUsedSequentialArg = max(highestUsedSequentialArg, currentSequentialArg);
						++ currentSequentialArg;
					}
					break;
				case NamedArgOpenChar:
					{
						int argIndex = 0;
						const _Char* it2 = it;
						while(true)
						{
							++ it2;
							if(it2 == end)
							{
								// unclosed named arg.
								m_rendered.push_back(ch);
								break;
							}
							wchar_t ch2 = *it2;
							if(ch2 >= '0' && ch2 <= '9')
							{
								argIndex = (argIndex * 10) + (ch2 - '0');// construct an integer index
							}
							else if(ch2 == NamedArgCloseChar)
							{
								if(argIndex < (int)m_dynArguments.size())
								{
									// success!
									m_rendered.append(GetArg(argIndex).c_str());
									highestUsedSequentialArg = max(highestUsedSequentialArg, (int)argIndex);
									it = it2;// advance the cursor.
								}
								else
								{
									// index out of range
									m_rendered.push_back(ch);
								}
								break;
							}
							else
							{
								// unrecognized char
								m_rendered.push_back(ch);
								break;
							}
						}
						break;
					}
				default:
					m_rendered.push_back(ch);
					break;
				}
			}

			// append unused args. this is how the old Format() works.
			currentSequentialArg = highestUsedSequentialArg + 1;
			while(currentSequentialArg < (int)m_dynArguments.size())
			{
				m_rendered.append(GetArg(currentSequentialArg).c_str());
				currentSequentialArg ++;
			}
		}

    _String m_Format;// the original format string.  this plus arguments that are fed in is used to build m_Composite.
		mutable _String m_rendered;
		mutable bool m_isRendered;

		void AddArg(const _Char* s)
		{
			m_argumentCharSize += LibCC::StringLength(s);
			m_dynArguments.push_back(s);
		}

		void AddArg(const _Char* s, _Char open, _Char close)
		{
			m_argumentCharSize += LibCC::StringLength(s) + 2;
			m_dynArguments.push_back(s, open, close);
		}

		void AddArg(const _Char* s, int maxLen)
		{
			m_dynArguments.push_back(s, maxLen);
			m_argumentCharSize += maxLen;
		}

		void AddArg(const _Char* s, int maxLen, _Char open, _Char close)
		{
			m_dynArguments.push_back(s, maxLen, open, close);
			m_argumentCharSize += maxLen;
		}

		void AddArg(_Char ch, size_t count)
		{
			m_dynArguments.push_back(ch, count);
			m_argumentCharSize += count;
		}

		QuickString<_Char> AddArg()
		{
			return m_dynArguments.push_back();
		}

		const QuickString<_Char> GetArg(size_t i) const
		{
			return m_dynArguments[i];
		}

		size_t m_argumentCharSize;
		QuickStringList<_Char> m_dynArguments;

# ifdef WIN32
		// a couple functions here are copied from winapi for local use.
		template<typename Traits, typename Alloc>
		LIBCC_INLINE static void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code)
		{
			wchar_t* lpMsgBuf(0);
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
				0, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
			if(lpMsgBuf)
			{
				out = lpMsgBuf;
				LocalFree(lpMsgBuf);
			}
			else
			{
				out = L"Unknown error: ";
				wchar_t temp[50];
				_itow(code, temp, 50);
				out.append(temp);
			}
		}

		template<typename Char, typename Traits, typename Alloc>
		inline static void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code)
		{
			std::wstring s;
			FormatMessageGLE(s, code);

			StringConvert(s, out);
			return;
		}

		template<typename Traits, typename Alloc>
		LIBCC_INLINE static bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out)
		{
			static const int StaticBufferSize = 1024;
			static const int MaximumAllocSize = 5242880;// don't attempt loading strings larger than 10 megs
			bool r = false;
			wchar_t temp[StaticBufferSize];// start with fast stack buffer
			// LoadString returns the # of chars copied, not including the null terminator
			if(LoadStringW(hInstance, stringID, temp, StaticBufferSize) < (StaticBufferSize - 1))
			{
				out = temp;
				r = true;
			}
			else
			{
				// we loaded up the maximum size; the string was probably truncated.
				int size = StaticBufferSize * 2;// this # is in chars, not bytes
				while(1)
				{
					// allocate a buffer.
					if(size > MaximumAllocSize)
					{
						// failed... too large of a string.
						break;
					}
					wchar_t* buf = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), 0, size * sizeof(wchar_t)));
					if(LoadStringW(hInstance, stringID, buf, size) < (size - 1))
					{
						// got what we wanted.
						out = buf;
						HeapFree(GetProcessHeap(), 0, buf);
						r = true;
						break;
					}
					HeapFree(GetProcessHeap(), 0, buf);
					size <<= 1;// double the amount to allocate
				}
			}
			return r;
		}

		template<typename Char, typename Traits, typename Alloc>
		inline bool static LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out)
		{
			bool r = false;
			std::wstring ws;
			if(LoadStringX(hInstance, stringID, ws))
			{
				r = true;
				StringConvert(ws, out);
			}
			return r;
		}
# endif
  };

  typedef FormatX<char, std::char_traits<char>, std::allocator<char> > FormatA;
  typedef FormatX<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > FormatW;
  typedef FormatX<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > Format;
}

#pragma warning(pop)
