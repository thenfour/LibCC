// LibCC ~ Carl Corcoran, https://github.com/thenfour/LibCC

#pragma once

#ifdef WIN32

#include <string>
#include "StringUtil.hpp"

#include <windows.h>// for windows types
#include <shlwapi.h>// for Path* functions
#include <shlobj.h>// for SHGetFolderPath
#pragma comment(lib, "version.lib")

namespace LibCC
{
  // Win32 Wrappers Declaration -----------------------------------------------------------------------------------
  template<typename Traits, typename Alloc>
  void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code);
  template<typename Char, typename Traits, typename Alloc>
  void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code);

  template<typename Traits, typename Alloc>
  bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out);
  template<typename Char, typename Traits, typename Alloc>
  bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out);

  // RegCreateKeyEx
  template<typename Char>
  inline LONG RegCreateKeyExX(HKEY hKey, const Char* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult, DWORD* pdwDisposition)
  {
    std::wstring buf;
		StringConvert(szSubKey, buf);
    return RegCreateKeyExW(hKey, buf.c_str(), 0, 0, dwOptions, Sam, 0, pResult, pdwDisposition);
  }

  inline LONG RegCreateKeyExX(HKEY hKey, const wchar_t* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult, DWORD* pdwDisposition)
  {
    return RegCreateKeyExW(hKey, szSubKey, 0, 0, dwOptions, Sam, 0, pResult, pdwDisposition);
  }

  // RegOpenKeyEx
  template<typename Char>
  inline LONG RegOpenKeyExX(HKEY hKey, const Char* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult)
  {
    std::wstring buf = ToUTF16(szSubKey);
    return RegOpenKeyExW(hKey, buf.c_str(), dwOptions, Sam, pResult);
  }

  inline LONG RegOpenKeyExX(HKEY hKey, const wchar_t* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult)
  {
    LONG l = RegOpenKeyExW(hKey, szSubKey, dwOptions, Sam, pResult);
    return l;
  }

  // RegDeleteKeyEx
  inline LONG RegDeleteValueX(HKEY hKey, PCWSTR lpValueName)
  {
    return RegDeleteValueW(hKey, lpValueName);
  }
  template<typename Char>
  inline LONG RegDeleteValueX(HKEY hKey, const Char* lpValueName)
  {
    std::wstring buf = ToUTF16(lpValueName);
    return RegDeleteValueW(hKey, buf.c_str());
  }

  // RegSetValueEx
  template<typename Char>
  inline LONG RegSetValueExStringX( HKEY hKey, const Char* lpValueName, DWORD Reserved, DWORD dwType, const Char* strX)
  {
    std::wstring valueName;
		std::wstring strW = ToUTF16(strX);
    if(lpValueName)
    {
      StringConvert(lpValueName, valueName);
    }
    return RegSetValueExW(hKey, lpValueName ? valueName.c_str() : 0, Reserved, REG_SZ, (const BYTE*)(strW.c_str()), (DWORD)(strW.size() + 1) * sizeof(wchar_t));
  }
  inline LONG RegSetValueExStringX( HKEY hKey, const wchar_t* lpValueName, DWORD Reserved, DWORD, const wchar_t* str)
  {
    return RegSetValueExW(hKey, lpValueName, Reserved, REG_SZ, (const BYTE*)str, (DWORD)(wcslen(str) + 1) * sizeof(wchar_t));
  }

  inline LONG RegSetValueExX(HKEY hKey, const wchar_t* lpValueName, DWORD dwType, const BYTE* lpData, DWORD cbData)
  {
    return RegSetValueExW(hKey, lpValueName, 0, dwType, lpData, cbData);
  }
  template<typename Char>
  inline LONG RegSetValueExX( HKEY hKey, const Char* lpValueName, DWORD dwType, const BYTE* lpData, DWORD cbData)
  {
    std::wstring valueName;
    if(lpValueName)
    {
			valueName = ToUTF16(lpValueName);
    }
    return RegSetValueExX(hKey, lpValueName ? valueName.c_str() : 0, dwType, lpData, cbData);
  }

  // RegQueryValueEx
  inline LONG RegQueryValueExStringX(HKEY hKey, const wchar_t* lpValueName, DWORD, DWORD, std::wstring& str)
  {
    // get the length of the thing.
    DWORD size;
    DWORD type;
    LONG r;
    if(ERROR_SUCCESS == (r = RegQueryValueExW(hKey, lpValueName, 0, &type, 0, &size)))
    {
      if(type == REG_SZ)
      {
        std::vector<BYTE> buf(size + sizeof(wchar_t));
        size = (DWORD)buf.size();
        if(ERROR_SUCCESS == (r = RegQueryValueExW(hKey, lpValueName, 0, 0, buf.data(), &size)))
        {
          str = (const wchar_t*)buf.data();
        }
      }
    }
    return r;
  }
  template<typename Char>
  inline LONG RegQueryValueExStringX(HKEY hKey, const Char* lpValueName, DWORD Reserved, std::basic_string<Char>& strX)
  {
    std::wstring valueName;
    std::wstring strW;
    if(lpValueName)
    {
      StringConvert(lpValueName, valueName);
    }
    LONG r;
    if(ERROR_SUCCESS == (r = RegQueryValueExStringX(hKey, lpValueName ? valueName.c_str() : 0, Reserved, REG_SZ, strW)))
    {
      StringConvert(strW, strX);
    }
    return r;
  }

  inline LONG RegQueryValueExX(HKEY hKey, const wchar_t* valueName, DWORD* type, BYTE* data, DWORD* cbData)
  {
    return RegQueryValueExW(hKey, valueName, 0, type, data, cbData);
  }
  template<typename Char>
  inline LONG RegQueryValueExX(HKEY hKey, const Char* szValueName, DWORD* type, BYTE* data, DWORD* cbData)
  {
    std::wstring valueName;
    if(szValueName)
    {
      std::wstring strW;
      StringConvert(szValueName, valueName);
    }
    return RegQueryValueExX(hKey, szValueName ? valueName.c_str() : 0, type, data, cbData);
  }

  // RegDeleteKey
  inline LONG RegDeleteKeyX(HKEY hKey, const wchar_t* subKey)
  {
    return RegDeleteKeyW(hKey, subKey);
  }
  template<typename Char>
  inline LONG RegDeleteKeyX(HKEY hKey, const Char* subKey)
  {
    std::wstring strW;
    XLastDitchStringCopy(subKey, strW);
    return RegDeleteKeyW(hKey, strW);
  }

  // RegEnumKeyEx
  // returns ERROR_NO_MORE_ITEMS or ERROR_SUCCESS
  // if maxNameSize == 0, then we will compute it for you.  Use it between calls on the same key for performance
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::wstring& outName, DWORD& maxNameSize)
  {
    FILETIME temp;
    LONG ret;
    std::vector<wchar_t> buf;
    DWORD size;

    outName.clear();

    // get maximum subkey name length.
    if(!maxNameSize)
    {
      if(ERROR_SUCCESS != (ret = RegQueryInfoKeyW(hKey, 0, 0, 0, 0, &maxNameSize, 0, 0, 0, 0, 0, 0)))
      {
        return ret;
      }
      maxNameSize += 2;// for safety
    }

    buf.resize(maxNameSize);

    // make the call
    size = static_cast<DWORD>(buf.size());
    ret = RegEnumKeyExW(hKey, dwIndex, buf.data(), &size, 0, 0, 0, &temp);
    if(ret == ERROR_SUCCESS)
    {
      outName = buf.data();
    }

    return ret;
  }
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::wstring& outName)
  {
    DWORD maxNameSize = 0;
    return RegEnumKeyExX(hKey, dwIndex, outName, maxNameSize);
  }
  template<typename Char>
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::basic_string<Char>& outName, DWORD& maxNameSize)
  {
    std::wstring outNameW;
    LONG ret = RegEnumKeyExX(hKey, dwIndex, outNameW, maxNameSize);
		StringConvert(outNameW, outName);
    return ret;
  }
  template<typename Char>
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::basic_string<Char>& outName)
  {
    std::wstring outNameW;
    DWORD maxNameSize = 0;
    LONG ret = RegEnumKeyExX(hKey, dwIndex, outNameW, maxNameSize);
		StringConvert(outNameW, outName);
    return ret;
  }

  inline bool IsValidHandle(HANDLE a)
  {
    return (a != 0) && (a != INVALID_HANDLE_VALUE);
  }

  inline bool IsBadHandle(HANDLE a)
  {
    return !IsValidHandle(a);
  }
  #define E_TERRORISM_NOT_RULED_OUT E_FAIL
  template<typename Char>
  inline bool PathIsAbsolute(const std::basic_string<Char>& path)
  {
      bool r = false;
      if(path.length() >= 3)
      {
          std::basic_string<Char> sSearch = path.substr(1, 2);
          if(StringEquals(sSearch, ":/") || StringEquals(sSearch, ":\\"))
          {
              r = true;
          }
      }
      return r;
  }

  template<typename Char>
  std::basic_string<Char> PathRemoveFilename(const std::basic_string<Char>& path)
  {
      std::basic_string<Char>::size_type nLastSlash = 0;
      nLastSlash = StringFindLastOf(path, "\\/");
      if(nLastSlash == std::string::npos) return path;

      return path.substr(0, nLastSlash);
  }

  template<typename Char>
  inline bool IsPathSeparator(Char c)
  {
    switch(c)
    {
    case '\\':
    case '/':
      return true;
    }
    return false;
  }

  template<typename Char>
  std::basic_string<Char> PathJoin(const std::basic_string<Char>& dir, const std::basic_string<Char>& file)
  {
    if(dir.empty()) return file;
    Char c = *(dir.rbegin());
    switch(c)
    {
    case '\\':
    case '/':
      return dir + file;
    }
    std::basic_string<Char> ret(dir);
    ret.push_back('\\');
    ret.append(file);
    return ret;
  }

  template<typename Char, typename Traits, typename Alloc>
  bool GetCurrentDirectoryX(std::basic_string<Char, Traits, Alloc>& out)
  {
    Blob<Char> buf;
    //TCHAR* tsz = 0;
    DWORD dwRet = 0;
    bool r = false;

    dwRet = GetCurrentDirectoryW(static_cast<DWORD>(buf.Size()), buf.GetBuffer());
    if(dwRet)
    {
      if(dwRet > static_cast<DWORD>(buf.Size()))
      {
        // It needs more space.
        if(buf.Alloc(dwRet+2))
        {
          if(::GetCurrentDirectory(static_cast<DWORD>(buf.Size()), buf.GetBuffer()))
          {
            out = buf.GetBuffer();
            r = true;
          }
        }
      }
      else
      {
        out = buf.GetBuffer();
        r = true;
      }
    }

    return r;
  }

  // GetTempPath
  template<typename Char, typename Traits, typename Alloc>
  inline bool GetTempPathX(std::basic_string<Char, Traits, Alloc>& sOut)
  {
    bool r = false;
    DWORD size;
    wchar_t temp[2];

    size = GetTempPathW(1, temp);
    if(size)
    {
      Blob<wchar_t> buf(size + 1);
			if(GetTempPathW(buf.Size(), buf.GetBuffer()))
      {
        XLastDitchStringCopy(buf.GetBuffer(), sOut);
        r = true;
      }
    }
    return r;
  }

  // LoadLibrary
  template<typename Char>
  inline HMODULE LoadLibraryX(const Char* buffer)
  {
    Blob<wchar_t, false, BlobTraits<true, MAX_PATH> > t;
    t.Alloc(StringLength(buffer) + 1);
    t.GetWritableBuffer()[0] = 0;
    XLastDitchStringCopy(buffer, t.GetWritableBuffer());
    return LoadLibraryW(t.GetBuffer());
  }

  inline HMODULE LoadLibraryX(const wchar_t* buffer)
  {
    return LoadLibraryW(buffer);
  }

  // LoadString
  template<typename Char>
  inline int LoadStringX(HINSTANCE hInstance, UINT id, Char* buffer, int buffermax)
  {
    Blob<wchar_t, false, BlobTraits<true, 1024> > t;
    t.Alloc(buffermax + 1);
    t.GetWritableBuffer()[0] = 0;
    int r = LoadStringW(hInstance, id, t.GetWritableBuffer(), buffermax);
    XLastDitchStringCopy(t.GetBuffer(), buffer);
    return r;
  }

  inline int LoadStringX(HINSTANCE hInstance, UINT id, wchar_t* buffer, int buffermax)
  {
    return LoadStringW(hInstance, id, buffer, buffermax);
  }

	inline std::wstring PathAppendX(IN const std::wstring& lhs, const std::wstring& rhs)
  {
		std::vector<wchar_t> b(lhs.size() + rhs.size() + 4);// +2 for null term + a backslash if necessary.
		XLastDitchStringCopy(lhs.c_str(), b.data());
		PathAppendW(b.data(), rhs.c_str());
		return std::wstring(b.data());
  }

  template<typename Char>
	inline std::basic_string<Char> PathAppendX(IN const std::basic_string<Char>& lhs, const std::basic_string<Char>& rhs)
  {
		std::basic_string<wchar_t> lhsW, rhsW, retW;
		StringConvert<Char, wchar_t>(lhs, lhsW);
		StringConvert<Char, wchar_t>(rhs, rhsW);
		retW = PathAppendX(lhsW, rhsW);
		std::basic_string<Char> ret;
		StringConvert<wchar_t, Char>(retW, ret);
		return ret;
  }

  template<typename Char>
  inline bool PathFileExistsX(const Char* path)
  {
    BlobTypes<Char>::PathBlob t;
    XLastDitchStringCopy(path, t);
    bool r = (TRUE == PathFileExistsW(t.GetBuffer()));
    return r;
  }

  inline int PathFileExistsX(const wchar_t* path)
  {
    return TRUE == PathFileExistsW(path);
  }

  template<typename Char, typename Traits, typename Alloc>
  inline HRESULT SHGetFolderPathX(std::basic_string<Char, Traits, Alloc>& sOut, int nFolder)
  {
    bool r = false;
		Blob<wchar_t, BlobTraits<true, MAX_PATH> > buf;
		HRESULT hr;
		if(SUCCEEDED(hr = SHGetFolderPathW(NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, buf.GetBuffer())))
    {
			StringConvert(buf.GetBuffer(), sOut);
    }
		return hr;
  }

 // template<typename Char, typename HandleType>// handle can be either HINSTANCE or HMODULE
	//inline std::basic_string<Char> GetModuleFilenameX(HandleType h)
	//{
	//	if(h == 0)
	//		h = GetModuleHandle(0);
	//	wchar_t ret[MAX_PATH+1];
	//	ret[0] = 0;
	//	::GetModuleFileNameW((HMODULE)h, ret, MAX_PATH);
	//	return ret;
	//}

  template<typename HandleType>// handle can be either HINSTANCE or HMODULE
  inline std::wstring GetModuleFilenameW(HandleType aModule)
  {
    std::wstring ret(MAX_PATH, '\0');
    size_t len;
    while (true) {
      len = (size_t)::GetModuleFileNameW((HMODULE)aModule,
        (PWSTR)ret.data(), (DWORD)ret.size());
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
  inline std::wstring GetModuleFilenameW()
  {
    return GetModuleFilenameW(0);
  }

  template<typename Char, typename HandleType>
  inline std::basic_string<Char> GetModuleFilenameX(HandleType aModule)
  {
    return StringConvert<Char>(GetModuleFilenameW<HandleType>(aModule));
  }
  template<typename Char>
  inline std::basic_string<Char> GetModuleFilenameX()
  {
    return GetModuleFilenameX<Char, HMODULE>(0);
  }

 // template<typename HandleType>// handle can be either HINSTANCE or HMODULE
	//inline std::wstring GetModuleFilenameW(HandleType h)
	//{
	//	return GetModuleFilenameX<wchar_t>(h);
	//}

}











//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// IMPLEMENTATION ////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace LibCC
{
  // Win32 wrappers Implementation  -----------------------------------------------------------------------------------
  template<typename Traits, typename Alloc>
  void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code)
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
  void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code)
  {
    std::wstring s;
    FormatMessageGLE(s, code);
    XLastDitchStringCopy(out, s);
    return;
  }

  template<typename Traits, typename Alloc>
  bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out)
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
  bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out)
  {
    bool r = false;
    std::wstring ws;
    if(LoadStringX(hInstance, stringID, ws))
    {
      r = true;
      XLastDitchStringCopy(out, ws);
    }
    return r;
  }


	inline void StdOutPrint(const std::string& s)
	{
		DWORD bw;
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s.c_str(), (DWORD)s.size(), &bw, 0);
	}

	template<typename Char>
	inline void StdOutPrint(const std::basic_string<Char>& s)
	{
		std::string A;
		LibCC::StringConvert(s, A);
		StdOutPrint(A);
	}

	template<typename Char>
	inline bool RevealInExplorer(const std::basic_string<Char>& path)
	{
		std::wstring cmdLine = LibCC::FormatW(L"explorer /select, %").qs(path).Str();

		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		GetStartupInfo(&si);
		LibCC::Blob<wchar_t> stupidBullshit(cmdLine.size() + 1);
		wcscpy(stupidBullshit.GetBuffer(), cmdLine.c_str());
		if(!CreateProcess(0, stupidBullshit.GetBuffer(), 0, 0, FALSE, 0, 0, 0, &si, &pi))
			return false;

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		return true;
	}


	class Version
	{
	public:
		struct LANGANDCODEPAGE
		{
			WORD wLanguage;
			WORD wCodePage;
		};

		Version() :
			m_a(0),
			m_b(0),
			m_c(0),
			m_d(0)
		{
		}

		void FromFile(const std::basic_string<WCHAR>& pathW)
		{
			DWORD wtf;
			DWORD size;

			Translations.clear();

			size = GetFileVersionInfoSizeW(pathW.c_str(), &wtf);
			if(!size)
				return;

      data.resize(size);
			if(!GetFileVersionInfoW(pathW.c_str(), 0, size, data.data()))
				return;

			// fixed info (main shit)
			VS_FIXEDFILEINFO* ffi = 0;
			UINT ffilen = 0;
			if(!VerQueryValueW(data.data(), L"\\", (void**)&ffi, &ffilen))
				return;
			m_a = HIWORD(ffi->dwFileVersionMS);
			m_b = LOWORD(ffi->dwFileVersionMS);
			m_c = HIWORD(ffi->dwProductVersionLS);
			m_d = LOWORD(ffi->dwProductVersionLS);

			// Read the list of languages and code pages.
			LANGANDCODEPAGE* lpTranslate;
			VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &ffilen);

			for(unsigned i = 0; i < (ffilen / sizeof(LANGANDCODEPAGE)); i++)
			{
				Translations.push_back(lpTranslate[i]);
			}
		}

		WORD GetA() const { return m_a; }
		WORD GetB() const { return m_b; }
		WORD GetC() const { return m_c; }
		WORD GetD() const { return m_d; }

		std::wstring GetRegisteredTo(WORD lang, WORD codepage)
		{
			return GetString(LibCC::FormatW(L"\\StringFileInfo\\%%\\RegisteredTo").ul<16,4>(lang).ul<16,4>(codepage).Str());
		}

		std::wstring GetLegalCopyright(WORD lang, WORD codepage)
		{
			return GetString(LibCC::FormatW(L"\\StringFileInfo\\%%\\LegalCopyright").ul<16,4>(lang).ul<16,4>(codepage).Str());
		}

		std::wstring GetFileDescription(WORD lang, WORD codepage)
		{
			return GetString(LibCC::FormatW(L"\\StringFileInfo\\%%\\FileDescription").ul<16,4>(lang).ul<16,4>(codepage).Str());
		}
		
		std::vector<LANGANDCODEPAGE> Translations;

	private:

		std::wstring GetString(const std::wstring& name)
		{
			PCWSTR pStr;
			UINT ffilen = 0;
			if(!VerQueryValueW(data.data(), name.c_str(), (void**)&pStr, &ffilen))
				return L"";
			return pStr;
		}

		std::vector<BYTE> data;

		WORD m_a;
		WORD m_b;
		WORD m_c;
		WORD m_d;
	};

}


#endif// WIN32



