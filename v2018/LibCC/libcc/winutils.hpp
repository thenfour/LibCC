/*

  Encapsulating winapi functions more natively to C++

*/
#ifndef LIBCC_WINUTIL_H
#define LIBCC_WINUTIL_H

#include "format.hpp"

#include <string>
#include <Windows.h>

namespace cc {


  inline std::wstring _GetModuleFullPath(uintptr_t aModule)
  {
    std::wstring ret(MAX_PATH, '\0');
    size_t len;
    while (true) {
      len = (size_t)::GetModuleFileNameW((HMODULE)aModule,
        (PWSTR)ret.data(), ret.size());
      if (len == 0) {
        return L"";
      }
      if (len == ret.size() && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        ret.resize(ret.size() * 2);
      }
      else {
        break;
      }
    }
    ret.resize(len);
    return ret;
  }


  // a couple functions here are copied from winapi for local use.
  inline void _FormatMessage(std::wstring& out, int code)
  {
    wchar_t* lpMsgBuf(0);
    ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
      0, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
    if (lpMsgBuf)
    {
      out = lpMsgBuf;
      LocalFree(lpMsgBuf);
    }
    else
    {
      out = format(L"Unknown error: %", code);
    }
  }

  inline static bool _LoadString(HINSTANCE hInstance, UINT stringID, std::wstring& out)
  {
    static const int StaticBufferSize = 1024;
    static const int MaximumAllocSize = 5242880;// don't attempt loading strings larger than 10 megs
    bool r = false;
    wchar_t temp[StaticBufferSize];// start with fast stack buffer
    // LoadString returns the # of chars copied, not including the null terminator
    if (LoadStringW(hInstance, stringID, temp, StaticBufferSize) < (StaticBufferSize - 1))
    {
      out = temp;
      r = true;
    }
    else
    {
      // we loaded up the maximum size; the string was probably truncated.
      int size = StaticBufferSize * 2;// this # is in chars, not bytes
      while (1)
      {
        // allocate a buffer.
        if (size > MaximumAllocSize)
        {
          // failed... too large of a string.
          break;
        }
        wchar_t* buf = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), 0, size * sizeof(wchar_t)));
        if (LoadStringW(hInstance, stringID, buf, size) < (size - 1))
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

}

#endif // LIBCC_WINUTIL_H
