/*

usage:

cc::format("x=%, y=%, z=%", x, y, z);
where x, y, z are strings or have a default auto conversion function.
if you need better than default formatting, use a to_string() type of function.
many are provided similar to the old LibCC::Format():

d<base, minwidth, paddingchar>()
qs()

*/
#ifndef LIBCC_STRINGUTIL_H
#define LIBCC_STRINGUTIL_H

#include <string>
#include <locale>
#include <codecvt>

namespace cc {

  inline char DigitToChar(unsigned char d) {
    static const char Digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    return d < (sizeof(Digits) / sizeof(char)) ? Digits[d] : 0;
  }

  template<typename T>
  inline std::string toString(const T& n)
  {
    return std::to_string(n);
  }

  template<typename Tch>
  inline std::basic_string<Tch> toString(const char *x) { return x; }

  inline std::string qs(const std::string& s) {
    return std::string("\"") + s + "\"";
  }

  // last-resort where we don't understand the string types.
  template<typename Tto, typename Tfrom>
  inline std::basic_string<Tto> toString(const std::basic_string<Tfrom>& from) {
    std::basic_string<Tto> ret;
    ret.reserve(from.size());
    for (auto ch : from) {
      ret.push_back((Tto)ch);
    }
    return ret;
  }

  // pass-through for same-typed strings.
  template<typename Tto>
  inline std::basic_string<Tto>& toString(std::basic_string<Tto>& from) {
    return from;
  }
  // utf-16 to utf-8
  inline std::string toString(std::wstring& from) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(from);
  }
  // utf-8 to utf-16
  inline std::wstring toString(std::string& from) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(from);
  }
  // todo: utf-32 or other known string formats.

  template<size_t Base, int minWidth, char PaddingChar, typename T>
  inline static std::string UnsignedNumberToString(T num) {
    std::string ret;
    if (Base < 2) {
      return ret;
    }
    ptrdiff_t PadRemaining = static_cast<ptrdiff_t>(minWidth);
    do {
      PadRemaining--;
      ret.push_back(DigitToChar(static_cast<unsigned char>(num % Base)));
      num = num / static_cast<T>(Base);
    } while (num);

    while (PadRemaining-- > 0) {
      ret.push_back(PaddingChar);
    }
    return std::string(ret.rbegin(), ret.rend());
  }

  template<size_t Base, int minWidth, char PaddingChar, bool ForceSign, typename T>
  inline static std::string SignedNumberToString(T num) {
    if (num < 0)
      return std::string("-") + UnsignedNumberToString<Base, minWidth - 1, PaddingChar, T>(-num);
    if (ForceSign)
      return std::string("+") + UnsignedNumberToString<Base, minWidth - 1, PaddingChar, T>(-num);
    return UnsignedNumberToString<Base, minWidth, PaddingChar, T>(num);
  }

  template<int base = 10, int minWidth = 0, char padChar = '0', bool forceSign = false>
  inline std::string d(int n) {
    return SignedNumberToString<base, minWidth, padChar, forceSign>(n);
  }

  template<typename Tch>
  inline void format(std::basic_string<Tch>& fmt, size_t& cursor) { }

  template<typename Tch, typename Targ, typename ...Targs>
  inline void format(std::basic_string<Tch>& fmt, size_t& cursor, const Targ& arg, Targs...args) {
    std::basic_string<Tch> sarg = toString<Tch>(toString<Tch>(arg));
    auto p = fmt.find('%', cursor);
    if (p == std::basic_string<Tch>::npos)
      return;
    fmt.replace(fmt.begin() + p, fmt.begin() + p + 1, sarg);
    cursor = p + sarg.length();

    format(fmt, cursor, args...);// do remaining args
  }

  template<typename Tch, typename ...Targs>
  inline std::basic_string<Tch> format(std::basic_string<Tch> fmt, Targs...args) {
    size_t cursor = 0;
    format(fmt, cursor, args...);
    return fmt;
  }

  template<typename Tch, typename ...Targs>
  inline std::basic_string<Tch> format(const Tch *fmt_, Targs...args) {
    std::basic_string<Tch> fmt = fmt_;
    size_t cursor = 0;
    format(fmt, cursor, args...);
    return fmt;
  }




}



#endif // LIBCC_STRINGUTIL_H
