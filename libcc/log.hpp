// LibCC ~ Carl Corcoran, https://github.com/thenfour/LibCC

#pragma once

#ifndef WIN32
#error LibCC::Log is only available on Windows platforms.
#endif

#include "stringutil.hpp"
#include "winapi.hpp"
#include <vector>
#include <process.h>
#include <shlwapi.h>
#include <commctrl.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")

// defaults
#ifndef LIBCC_ENABLE_LOG_FILE
# define LIBCC_ENABLE_LOG_FILE true// allow logging to file
#endif

#ifndef LIBCC_ENABLE_LOG_DEBUG
# define LIBCC_ENABLE_LOG_DEBUG true// allow OutputDebugString()
#endif

#ifndef LIBCC_ENABLE_LOG_WINDOW
# define LIBCC_ENABLE_LOG_WINDOW true
#endif

#ifndef LIBCC_LOG_WINDOW_WIDTH
# define LIBCC_LOG_WINDOW_WIDTH 300
#endif

#ifndef LIBCC_LOG_WINDOW_HEIGHT
# define LIBCC_LOG_WINDOW_HEIGHT 300
#endif

namespace LibCC
{
	class Log
	{

		friend struct LogReference;

		bool m_unicodeFileFormat;
		bool m_writeHeader;
		bool m_enableWindow;
		bool m_enableFile;
		bool m_enableDebug;
		bool m_enableStdOut;
		static const DWORD m_width = LIBCC_LOG_WINDOW_WIDTH;
		static const DWORD m_height = LIBCC_LOG_WINDOW_HEIGHT;

		static const size_t IndentSize = 2;

		bool m_enableRotate;
		DWORD m_rotateKeepCount;
		DWORD m_rotateByteSize;

		// ref count stuff only used by LogReference
		int m_refcount;
		int AddRef()
		{
			return ++ m_refcount;
		}
		int Release()
		{
			m_refcount --;
			if(!m_refcount)
			{
				delete this;
				return 0;
			}
			return m_refcount;
		}

		inline bool WindowEnabled() const
		{
			return LIBCC_ENABLE_LOG_WINDOW && m_enableWindow;
		}
		inline bool FileEnabled() const
		{
			return LIBCC_ENABLE_LOG_FILE && m_enableFile;
		}
		inline bool DebugEnabled() const
		{
			return LIBCC_ENABLE_LOG_DEBUG && m_enableDebug;
		}
		inline bool StdOutEnabled() const
		{
			return LIBCC_ENABLE_LOG_DEBUG && m_enableStdOut;
		}
		inline bool EnabledAtAll() const
		{
			return DebugEnabled() || FileEnabled() || WindowEnabled() || StdOutEnabled();
		}
		
	public:
		typedef std::wstring _String;

		Log() :
			m_hMain(0),
			m_hEdit(0),
			m_hThread(0),
			m_hInitialized(0),
			m_hTab(0),
			m_enableWindow(false),
			m_enableFile(false),
			m_enableDebug(false),
			m_enableStdOut(false),
			m_writeHeader(false),
			m_enableRotate(false)
		{
		}
		template<typename XChar>
		Log(const std::basic_string<XChar>& fileName, HINSTANCE hInstance, bool enableDebug = true, bool enableWindow = true, bool enableFile = true, bool unicodeFileFormat = true, bool enableStdOut = false, bool writeHeader = true)
		{
			Create(fileName, hInstance, enableDebug, enableWindow, enableFile, unicodeFileFormat, enableStdOut, writeHeader);
		}
		template<typename XChar>
		Log(const XChar* fileName, HINSTANCE hInstance, bool enableDebug = true, bool enableWindow = true, bool enableFile = true, bool unicodeFileFormat = true, bool enableStdOut = false, bool writeHeader = true)
		{
			Create(fileName, hInstance, enableDebug, enableWindow, enableFile, unicodeFileFormat, enableStdOut, writeHeader);
		}

		~Log()
		{
			Destroy();
		}

		bool IsCreated() const
		{
			return m_hThread != 0;
		}

		// filename
		template<typename XChar>
		void Create(const std::basic_string<XChar>& fileName, HINSTANCE hInstance, bool enableDebug = true, bool enableWindow = true, bool enableFile = true, bool unicodeFileFormat = true, bool enableStdOut = false, bool writeHeader = true)
		{
			m_hMain = 0;
			m_hEdit = 0;
			m_hThread = 0;
			m_hInitialized = 0;
			m_hTab = 0;
			m_enableDebug = enableDebug;
			m_enableWindow = enableWindow;
			m_enableFile = enableFile;
			m_unicodeFileFormat = unicodeFileFormat;
			m_enableStdOut = enableStdOut;
			m_writeHeader = writeHeader;
			m_enableRotate = false;
			if(EnabledAtAll())
			{
				std::wstring fileNameW;
				StringConvert(fileName, fileNameW);
				m_fileNames.push_back(fileNameW);

				m_hInstance = hInstance;
				m_hInitialized = CreateEvent(0, FALSE, FALSE, 0);
				m_hThread = (HANDLE)_beginthread(Log::ThreadProc, 0, this);
				//SetThreadPriority( m_hThread, THREAD_PRIORITY_NORMAL ); // i don't know what the point of this was.
				WaitForSingleObject(m_hInitialized, INFINITE);

				if(m_writeHeader)
				{
					Message(L"-------------------------------");
					Message(L"Starting log");
				}

				CloseHandle(m_hInitialized);
				m_hInitialized = 0;
			}
		}
		template<typename XChar>
		void Create(const XChar* fileName, HINSTANCE hInstance, bool enableDebug = true, bool enableWindow = true, bool enableFile = true, bool unicodeFileFormat = true, bool enableStdOut = false, bool writeHeader = true)
		{
			Create(std::basic_string<XChar>(fileName), hInstance, enableDebug, enableWindow, enableFile, unicodeFileFormat, enableStdOut, writeHeader);
		}

		// if you want your logs to go to multiple places...
		void AddFilename(const std::wstring& path)
		{
			m_fileNames.push_back(path);
		}

		std::vector<std::wstring> GetFilenames() const
		{
			return m_fileNames;
		}

		void Destroy()
		{
			if(m_hThread)
			{
				if(m_writeHeader)
				{
					Message(L"Stopping log");
				}
				PostMessage(m_hMain, WM_LogExit, 0, 0);
				WaitForSingleObject(m_hThread, INFINITE);
				m_hThread = 0;
			}
		}

		void Indent()
		{
			if(EnabledAtAll())
			{
				SendMessageW(m_hMain, WM_Indent, 0, GetCurrentThreadId());
			}
		}

		void Outdent()
		{
			if(EnabledAtAll())
			{
				SendMessageW(m_hMain, WM_Outdent, 0, GetCurrentThreadId());
			}
		}

		int GetIndentLevel()
		{
			if(EnabledAtAll())
			{
				return (int)SendMessageW(m_hMain, WM_GetIndentLevel, 0, GetCurrentThreadId());
			}
			return 0;
		}

		std::wstring GetIndentStringW()
		{
			return std::wstring(GetIndentLevel() * IndentSize, L' ');
		}

		std::string GetIndentStringA()
		{
			return std::string(GetIndentLevel() * IndentSize, ' ');
		}
    
    // 2 string args...
    void Message(const _String& s1, const _String& s2)
    {
			if(EnabledAtAll() && m_hThread)
			{
				MessageInfo* pNew = new MessageInfo();
				GetLocalTime(&pNew->localTime);
				StringConvert(s1, pNew->s1);
				StringConvert(s2, pNew->s2);
				pNew->threadID = GetCurrentThreadId();

				SendMessage(m_hMain, WM_LogMessage, 0, reinterpret_cast<LPARAM>(pNew));// doesnt return until it's done.
				delete pNew;
			}
    }
    template<typename XChar, typename YChar>
    void Message(const std::basic_string<XChar>& x, const std::basic_string<YChar>& y)
    {
			if(EnabledAtAll())
			{
				_String s1;
				_String s2;
				StringConvert(x, s1);
				StringConvert(y, s2);
				Message(s1, s2);
			}
    }
    template<typename XChar, typename YChar>
    void Message(const std::basic_string<XChar>& x, const YChar* y)
    {
			if(EnabledAtAll())
			{
				Message(x, std::basic_string<YChar>(y));
			}
    }
    template<typename XChar, typename YChar>
    void Message(const XChar* x, const std::basic_string<YChar>& y)
    {
			if(EnabledAtAll())
			{
				Message(std::basic_string<XChar>(x), y);
			}
    }
    template<typename XChar, typename YChar>
    void Message(const XChar* x, const YChar* y)
    {
			if(EnabledAtAll())
			{
				Message(std::basic_string<XChar>(x), std::basic_string<YChar>(y));
			}
    }
    
    // 1 string arg...
    template<typename XChar>
    void Message(const XChar* s)
    {
			if(EnabledAtAll())
			{
				Message(std::basic_string<XChar>(s));
			}
    }
    template<typename XChar>
    void Message(const std::basic_string<XChar>& s)
    {
			if(EnabledAtAll())
			{
				Message(s, std::string(""));
			}
    }
    template<typename XChar>
    void Message(const FormatX<XChar>& s)
    {
			if(EnabledAtAll())
			{
				Message(s.Str(), std::string(""));
			}
    }

	void EnableRotation(bool enable, DWORD count, DWORD size)
	{
		m_enableRotate = enable;
		if(m_enableRotate)
		{
			m_rotateKeepCount = count;
			m_rotateByteSize = size;
		}
	}

  private:
    static void __cdecl ThreadProc(void* p)
    {
	    static_cast<Log*>(p)->ThreadProc();
    }

    void ThreadProc()
    {
			// you *cannot* call Message() from this thread.  this thread needs to pump messages 24/7
			WNDCLASSW wc = {0};
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = Log::MainProc;
			wc.hInstance = m_hInstance;
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			wc.lpszClassName =  m_fileNames[0].c_str();
			RegisterClassW(&wc);

			// determine placement.
			HANDLE m_hGlobalSemaphore;
			DWORD m_x;
			DWORD m_y;
			m_hGlobalSemaphore = CreateSemaphoreW(0, 0, 1000, L"LibCC_LogWindowCount");
			// increase ref count and get the previous count.
			LONG i;
			ReleaseSemaphore(m_hGlobalSemaphore, 1, &i);
			int screenColumns = GetSystemMetrics(SM_CXSCREEN) / m_width;
			m_x = m_width * (i % screenColumns);
			m_y = m_height * (i / screenColumns);

			m_hMain = CreateWindowExW(0, m_fileNames[0].c_str(), PathFindFileNameW(m_fileNames[0].c_str()), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
				m_x, m_y, m_width, m_height, 0, 0, m_hInstance, this);

			if(WindowEnabled())
			{
				ShowWindow(m_hMain, SW_SHOW);
			}
			SetEvent(m_hInitialized);

			MSG msg;
			while(GetMessage(&msg, 0, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			CloseHandle(m_hGlobalSemaphore);
			return;
    }
    
    static LRESULT CALLBACK MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
			Log* pThis = static_cast<Log*>(GetPropW(hWnd, L"LibCC Log Window"));
			switch(uMsg)
			{
			case WM_CLOSE:
				return 0;
			case WM_SIZE:
				{
					if(pThis->WindowEnabled())
					{
						RECT rc;
						GetClientRect(hWnd, &rc);

						HDWP hdwp = BeginDeferWindowPos(static_cast<int>(pThis->m_threads.size() + 2));// + tabctrl + composite edit
						// tab ctrl
						DeferWindowPos(hdwp, pThis->m_hTab, 0, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
						TabCtrl_AdjustRect(pThis->m_hTab, FALSE, &rc);
						// all tab edit box
						DeferWindowPos(hdwp, pThis->m_hEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
						for(std::vector<ThreadInfo>::const_iterator it = pThis->m_threads.begin(); it != pThis->m_threads.end(); ++ it)
						{
							// thread edit box
							DeferWindowPos(hdwp, it->hEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
						}

						EndDeferWindowPos(hdwp);
					}
					break;
				}
			case WM_DESTROY:
				{
					PostQuitMessage(0);
					return 0;
				}
			case WM_LogMessage:
				{
					MessageInfo& mi = *(MessageInfo*)lParam;
					ThreadInfo& ti = pThis->GetThreadInfo(mi.threadID);

					// convert all newline chars into something else
					for(_String::iterator it = mi.s1.begin(); it != mi.s1.end(); ++ it)
					{
						if(*it == '\r') *it = '~';
						if(*it == '\n') *it = '~';
					}
					for(_String::iterator it = mi.s2.begin(); it != mi.s2.end(); ++ it)
					{
						if(*it == '\r') *it = '~';
						if(*it == '\n') *it = '~';
					}

					// create indent string
					_String indent;
					for(size_t i = 0; i < ti.indent * IndentSize; i ++)
					{
						indent.push_back(' ');
					}

					SYSTEMTIME st;
					GetLocalTime(&st);

					std::wstring file;
					if(pThis->DebugEnabled() || pThis->FileEnabled())
					{
						file = LibCC::FormatW(L"[%-%-%;%:%:%][%] %%%|")
							.ul<10,4>(st.wYear)
							.ul<10,2>(st.wMonth)
							.ul<10,2>(st.wDay)
							.ul<10,2>(st.wHour)
							.ul<10,2>(st.wMinute)
							.ul<10,2>(st.wSecond)
							.ul<16,8,'0'>(mi.threadID)
							.s(indent)
							.s(mi.s1)
							.s(mi.s2)
							.Str();
					}

					// do ods
					if(pThis->DebugEnabled())
					{
						OutputDebugStringW(file.c_str());
					}

					// do file
					if(pThis->FileEnabled())
					{
						for(std::vector<std::wstring>::iterator it = pThis->m_fileNames.begin(); it != pThis->m_fileNames.end(); ++ it)
						{
							std::wstring& fileName = *it;

							if(pThis->m_enableRotate)
							{
								if(pThis->FileNeedsRotate(fileName))
								{
									pThis->RotateFile(fileName);
								}
							}

							HANDLE h = CreateFileW(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
							if(h && h != INVALID_HANDLE_VALUE)
							{
								DWORD bw;
								if(pThis->m_unicodeFileFormat)
								{
									DWORD fileSize = GetFileSize(h, NULL);
									if(fileSize == 0)
									{
										// the file is new. write out in UTF-8
										std::string a;
										StringConvert(file, a, 0, CP_UTF8);
										WriteFile(h, a.c_str(), (DWORD)a.size(), &bw, 0);
									}
									else
									{
										// file is old, is it UTF-16?

										// After an arbitrary yet reasonable amount of time, this check should probably be removed
										// as it only matters to not corrupt existing UTF16 log files as of r237 - 2011-01-20

										SetFilePointer(h, 0, NULL, FILE_BEGIN);
										WORD bom = 0;
										DWORD br;
										ReadFile(h, &bom, 2, &br, NULL);
										SetFilePointer(h, 0, NULL, FILE_END);

										if(bom == 0xFEFF) // UTF-16 Byte Order Mark
										{
											// UTF-16 file
											WriteFile(h, file.c_str(), (DWORD)(sizeof(wchar_t) * file.size()), &br, 0);
										}
										else
										{
											// Write UTF8
											std::string a;
											StringConvert(file, a, 0, CP_UTF8);
											WriteFile(h, a.c_str(), (DWORD)a.size(), &bw, 0);
										}
									}
								}
								else
								{
									std::string a;
									StringConvert(file, a);
									WriteFile(h, a.c_str(), (DWORD)a.size(), &bw, 0);
								}
								CloseHandle(h);
							}
						}
					}

					// do gui
					if(pThis->WindowEnabled() || pThis->StdOutEnabled())
					{
						std::wstring gui(LibCC::FormatW(L"%%%|").s(indent).s(mi.s1).s(mi.s2).Str());

						if(pThis->WindowEnabled())
						{
							int ndx = GetWindowTextLength(pThis->m_hEdit);
							SendMessage(pThis->m_hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
							SendMessageW(pThis->m_hEdit, EM_REPLACESEL, 0, (LPARAM)gui.c_str());

							ndx = GetWindowTextLength(ti.hEdit);
							SendMessage(ti.hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
							SendMessageW(ti.hEdit, EM_REPLACESEL, 0, (LPARAM)gui.c_str());
						}
						if(pThis->StdOutEnabled())
						{
							StdOutPrint(gui);
						}
					}
					
					return 0;
				}
			case WM_Indent:
				{
					ThreadInfo& ti = pThis->GetThreadInfo(static_cast<DWORD>(lParam));
					ti.indent ++;
					return 0;
				}
			case WM_Outdent:
				{
					ThreadInfo& ti = pThis->GetThreadInfo(static_cast<DWORD>(lParam));
					ti.indent --;
					return 0;
				}
			case WM_GetIndentLevel:
				{
					ThreadInfo& ti = pThis->GetThreadInfo(static_cast<DWORD>(lParam));
					return ti.indent;
				}
			case WM_LogExit:
				{
					DestroyWindow(hWnd);
					return 0;
				}
			case WM_NOTIFY:
				{
					if(pThis->WindowEnabled())
					{
						NMHDR& h = *(NMHDR*)lParam;
						if(h.hwndFrom == pThis->m_hTab)
						{
							if(h.code == TCN_SELCHANGE)
							{
								int iItem = TabCtrl_GetCurSel(pThis->m_hTab);
								if(iItem == 0)
								{
									// show the ALL tab, hide all others.
									SetWindowPos(pThis->m_hEdit, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
									for(std::vector<ThreadInfo>::const_iterator it = pThis->m_threads.begin(); it != pThis->m_threads.end(); ++ it)
									{
										ShowWindow(it->hEdit, SW_HIDE);
									}
								}
								else
								{
									// hide the ALL tab, hide all others, show the selected one.
									ShowWindow(pThis->m_hEdit, SW_HIDE);
									for(std::vector<ThreadInfo>::const_iterator it = pThis->m_threads.begin(); it != pThis->m_threads.end(); ++ it)
									{
										if(it->tabItem == iItem)
										{
											SetWindowPos(it->hEdit, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
										}
										else
										{
											ShowWindow(it->hEdit, SW_HIDE);
										}
									}
								}

								return 0;
							}
						}
					}
					break;
				}
			case WM_CREATE:
				{
					CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
					pThis = static_cast<Log*>(pcs->lpCreateParams);
					SetPropW(hWnd, L"LibCC Log Window", static_cast<HANDLE>(pThis));
					pThis->m_hMain = hWnd;
					if(pThis->WindowEnabled())
					{
						HDC dc = GetDC(hWnd);
						pThis->m_hFont = CreateFontW(-MulDiv(10, GetDeviceCaps(dc, LOGPIXELSY), 72),0,0,0,0,0,0,0,0,0,0,0,0,L"Courier");
						ReleaseDC(hWnd, dc);

						pThis->m_hTab = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE, 27, 74, 3, 2, hWnd, 0, pThis->m_hInstance, 0);
						SendMessageW(pThis->m_hTab, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
						TabCtrl_SetItemSize(pThis->m_hTab, 16, 16);

						// create the first tab for the composite
						TCITEMW tci = {0};
						tci.mask = TCIF_TEXT;
						tci.pszText = L"All";
						TabCtrl_InsertItem(pThis->m_hTab, 0, &tci);

						pThis->m_hEdit = CreateWindowExW(0, L"EDIT", L"", WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE,
							27, 74, 3, 2, pThis->m_hMain, 0, 0, 0);

						SetWindowPos(pThis->m_hEdit, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

						SendMessageW(pThis->m_hEdit, WM_SETFONT, (WPARAM)pThis->m_hFont, TRUE);
						SendMessageW(pThis->m_hEdit, EM_SETLIMITTEXT, (WPARAM)0, 0);
					}
					return 0;
				}
			case WM_PAINT:
				PAINTSTRUCT ps;
				BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
				return 0;
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

    static const UINT WM_LogMessage = WM_APP;
    static const UINT WM_LogExit = WM_APP + 2;
    static const UINT WM_Indent = WM_APP + 3;
    static const UINT WM_Outdent = WM_APP + 4;
    static const UINT WM_GetIndentLevel = WM_APP + 5;

    // each thread gets a different edit, tab item, and id.
    struct ThreadInfo
    {
      ThreadInfo() :
        hEdit(0),
        threadID(0),
        tabItem(0),
        indent(0)
      {
      }
      ThreadInfo(const ThreadInfo& rhs) 
      {
        operator =(rhs);
      }
      ThreadInfo& operator =(const ThreadInfo& rhs) 
      {
        hEdit = rhs.hEdit;
        threadID = rhs.threadID;
        tabItem = rhs.tabItem;
        indent = rhs.indent;
        return *this;
      }
      HWND hEdit;
      DWORD threadID;
      int tabItem;
      size_t indent;
    };

    ThreadInfo& GetThreadInfo(DWORD threadID)
    {
			for(std::vector<ThreadInfo>::iterator it = m_threads.begin(); it != m_threads.end(); ++ it)
			{
				if(it->threadID == threadID)
				{
					return *it;
				}
			}

			// omg; not found.  create a new one.
			HWND hNewEdit = 0;
			int newTabItem = 0;
			if(WindowEnabled())
			{
				newTabItem = TabCtrl_GetItemCount(m_hTab);
				_String text(LibCC::FormatW(L"%").ul(threadID).Str());
				TCITEMW tci = {0};
				tci.mask = TCIF_TEXT;
				tci.pszText = const_cast<PWSTR>(text.c_str());// i hate these damn non-const structs
				TabCtrl_InsertItem(m_hTab, newTabItem, &tci);

				hNewEdit = CreateWindowExW(0, L"EDIT", L"",
					WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | WS_CHILD,
					27, 74, 3, 2, m_hMain, 0, m_hInstance, 0);
				SendMessage(hNewEdit, WM_SETFONT, (WPARAM)m_hFont, TRUE);
				SendMessage(hNewEdit, EM_SETLIMITTEXT, (WPARAM)0, 0);
				PostMessage(m_hMain, WM_SIZE, 0, 0);
			}
			m_threads.push_back(ThreadInfo());
			ThreadInfo& ret(m_threads.back());
			ret.tabItem = newTabItem;
			ret.threadID = threadID;
			ret.hEdit = hNewEdit;
	    
			return ret;
    }

	void RotateFile( const std::wstring& fileName ) 
	{
		const wchar_t *fn = fileName.c_str();
		const wchar_t* ext = PathFindExtensionW(fn);
		
		std::wstring fileNameBase = fileName.substr(0, ext - fn);
		std::wstring fileNameExt = ext;

		for(int i = m_rotateKeepCount; i >= 0; --i)
		{
			std::wstring fromFile;
			if(i == 0)
			{
				fromFile = fileName;
			}
			else
			{
				fromFile = LibCC::FormatW(L"%.%%")(fileNameBase).i(i - 1)(fileNameExt).Str();
			}
			std::wstring toFile = LibCC::FormatW(L"%.%%")(fileNameBase).i(i)(fileNameExt).Str();

			if(i == (int)m_rotateKeepCount)
			{
				// Delete the oldest file
				DeleteFileW(fromFile.c_str());
			}
			else
			{
				// Move each old file to next position
				MoveFileW(fromFile.c_str(), toFile.c_str());
			}
		}
	}

	bool FileNeedsRotate( const std::wstring& fileName ) 
	{
		WIN32_FILE_ATTRIBUTE_DATA fad = {};
		GetFileAttributesExW(fileName.c_str(), GetFileExInfoStandard, &fad);
		return fad.nFileSizeLow > m_rotateByteSize;
	}

    struct MessageInfo
    {
      _String s1;
      _String s2;
      DWORD threadID;
      SYSTEMTIME localTime;
    };

    HANDLE m_hInitialized;
    HANDLE m_hThread;
    HWND m_hMain;
    HWND m_hEdit;// composite of all threads.
    HFONT m_hFont;

    HWND m_hTab;
    std::vector<ThreadInfo> m_threads;

    HINSTANCE m_hInstance;
		std::vector<std::wstring> m_fileNames;
	};

	extern Log* g_pLog;

  class LogScopeMessage
  {
  public:
		template<typename XChar>
    LogScopeMessage(const XChar* op, Log* pLog = g_pLog) :
      m_pLog(pLog)
    {
      m_pLog->Message(std::wstring(L"{ "), std::basic_string<XChar>(op));
      m_pLog->Indent();
    }
		template<typename XChar>
    LogScopeMessage(const std::basic_string<XChar>& op, Log* pLog = g_pLog) :
      m_pLog(pLog)
    {
      m_pLog->Message(std::wstring(L"{ "), op);
      m_pLog->Indent();
    }
    
    ~LogScopeMessage()
    {
      if(m_pLog)
      {
        m_pLog->Outdent();
        m_pLog->Message("}");
      }
    }

    Log* m_pLog;
  };

	// stick one of these in a class if it needs to use a Log during its lifetime. use it similar to how you use Log
	// the idea is that the global log (LibCC::g_pLog) may be used throughout lots of stuff in an application. if something shuts down and
	// just deletes it, it may cause problems for everything else. it needs to be ref-counted in a simple way, and i don't feel like depending
	// on shared_ptr for this.
	struct LogReference
	{
		Log*& m_p;

		Log* operator ->()
		{
			return m_p;
		}

		Log& operator *()
		{
			return *m_p;
		}

		LogReference(Log*& p = g_pLog) :
			m_p(p)
		{
			if(!m_p)
			{
				m_p = new Log();
			}
			else
			{
				m_p->AddRef();
			}
		}
		~LogReference()
		{
			if(!m_p)// because we're mixing ref-counted ptrs with potentially-not-ref-counted, do a check here to see if it's already been deleted.
				return;
			if(!m_p->Release())
				m_p = 0;
		}

		template<typename XChar>
    void Create(const std::basic_string<XChar>& fileName, HINSTANCE hInstance, bool enableDebug = true, bool enableWindow = true, bool enableFile = true, bool unicodeFileFormat = true, bool enableStdOut = false, bool writeHeader = true)
		{
			if(!m_p->IsCreated())
				m_p->Create(fileName, hInstance, enableDebug, enableWindow, enableFile, unicodeFileFormat, enableStdOut, writeHeader);
		}

		template<typename XChar>
    void Create(const XChar* fileName, HINSTANCE hInstance, bool enableDebug = true, bool enableWindow = true, bool enableFile = true, bool unicodeFileFormat = true, bool enableStdOut = false, bool writeHeader = true)
		{
			if(!m_p->IsCreated())
				m_p->Create(fileName, hInstance, enableDebug, enableWindow, enableFile, unicodeFileFormat, enableStdOut, writeHeader);
		}

	};
}
#include "timer.hpp"
namespace LibCC
{
  class LogScopeMessageTimer
  {
  public:
		template<typename XChar>
    LogScopeMessageTimer(const XChar* op, Log* pLog = g_pLog) :
      m_pLog(pLog)
    {
			t.Tick();
			if(!m_pLog)
				return;
      m_pLog->Message(std::wstring(L"{ "), std::basic_string<XChar>(op));
      m_pLog->Indent();
    }
		template<typename XChar>
    LogScopeMessageTimer(const std::basic_string<XChar>& op, Log* pLog = g_pLog) :
      m_pLog(pLog)
    {
			t.Tick();
			if(!m_pLog)
				return;
      m_pLog->Message(std::wstring(L"{ "), op);
      m_pLog->Indent();
    }
    
    ~LogScopeMessageTimer()
    {
      if(m_pLog)
      {
        m_pLog->Outdent();
				m_pLog->Message(LibCC::FormatA("} (% seconds)").d<4>(t.GetElapsedSeconds()));
      }
    }

		Timer t;
    Log* m_pLog;
  };
}
