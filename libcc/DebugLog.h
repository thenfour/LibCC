/*
  LibCC Release "March 9, 2007"
  DebugLog Module
  (c) 2004-2007 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC

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


#pragma once

#ifndef WIN32
#error LibCC::DebugLog is only available on Windows platforms.
#endif

#include "StringUtil.h"
#include <vector>
#include <process.h>
#include <shlwapi.h>
#include <commctrl.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")

// defaults
#ifndef LIBCC_ENABLE_LOG_FILE
# define LIBCC_ENABLE_LOG_FILE 1// allow logging to file
#endif

#ifndef LIBCC_ENABLE_LOG_ODSF
# define LIBCC_ENABLE_LOG_ODSF 1// allow OutputDebugString()
#endif

#ifndef LIBCC_ENABLE_LOG_WINDOW
# define LIBCC_ENABLE_LOG_WINDOW 1
#endif

// if any logging features are requested, then use the real log class.
#ifndef LIBCC_ENABLE_LOG_AT_ALL
#	if (LIBCC_ENABLE_LOG_WINDOW == 1) || (LIBCC_ENABLE_LOG_FILE == 1) || (LIBCC_ENABLE_LOG_ODSF == 1)
#		define LIBCC_ENABLE_LOG_AT_ALL 1
#	else
#		define LIBCC_ENABLE_LOG_AT_ALL 0
#	endif
#endif

namespace LibCC
{
  class LogWindow
  {
		// use this class to help find the right coords for a debug log window.
		class LogWindowPlacementHelper
		{
		public:
			static const DWORD m_width = 300;
			static const DWORD m_height = 300;
			DWORD m_x;
			DWORD m_y;

			static const wchar_t* GetSemaphoreName()
			{
				return L"LibCC_LogWindowCount";
			}

			LogWindowPlacementHelper()
			{
				// determine placement.
				m_hGlobalSemaphore = CreateSemaphoreW(0, 0, 1000, GetSemaphoreName());
				// increase ref count and get the previous count.
				LONG i;
				ReleaseSemaphore(m_hGlobalSemaphore, 1, &i);
				int screenColumns = GetSystemMetrics(SM_CXSCREEN) / m_width;
				m_x = m_width * (i % screenColumns);
				m_y = m_height * (i / screenColumns);
			}

			~LogWindowPlacementHelper()
			{
				CloseHandle(m_hGlobalSemaphore);
			}

			HANDLE m_hGlobalSemaphore;
		};
  public:
		typedef std::wstring _String;

		LogWindow() :
			m_hMain(0),
			m_hEdit(0),
			m_hThread(0),
			m_hInitialized(0),
			m_hTab(0)
		{
		}

		~LogWindow()
		{
		}

		// filename
		template<typename XChar>
    void Create(const std::basic_string<XChar>& fileName, HINSTANCE hInstance)
    {
			ConvertString(fileName, m_fileName);
			m_hInstance = hInstance;
			m_hInitialized = CreateEvent(0, FALSE, FALSE, 0);
			m_hThread = (HANDLE)_beginthread(LogWindow::ThreadProc, 0, this);
			//SetThreadPriority( m_hThread, THREAD_PRIORITY_NORMAL ); // i don't know what the point of this was.
			WaitForSingleObject(m_hInitialized, INFINITE);

			Message(L"-------------------------------");
			Message(L"Starting log");

			CloseHandle(m_hInitialized);
			m_hInitialized = 0;
    }
 		template<typename XChar>
    void Create(const XChar* fileName, HINSTANCE hInstance)
    {
			Create(std::basic_string<XChar>(fileName), hInstance);
    }

    void Destroy()
    {
			Message(L"Stopping log");

			// wait.
			if(m_hThread)
			{
				PostMessage(m_hMain, WM_LogExit, 0, 0);
				WaitForSingleObject(m_hThread, INFINITE);
			}
			m_hThread = 0;
    }

		void Indent()
		{
			SendMessageW(m_hMain, WM_Indent, 0, GetCurrentThreadId());
		}

		void Outdent()
		{
			SendMessageW(m_hMain, WM_Outdent, 0, GetCurrentThreadId());
		}
    
    // 2 string args...
    void Message(const _String& s1, const _String& s2)
    {
			if(m_hThread)
			{
				MessageInfo* pNew = new MessageInfo();
				GetLocalTime(&pNew->localTime);
				ConvertString(s1, pNew->s1);
				ConvertString(s2, pNew->s2);
				pNew->threadID = GetCurrentThreadId();

				SendMessage(m_hMain, WM_LogMessage, 0, reinterpret_cast<LPARAM>(pNew));// doesnt return until it's done.
				delete pNew;
			}
    }
    template<typename XChar, typename YChar>
    void Message(const std::basic_string<XChar>& x, const std::basic_string<YChar>& y)
    {
			_String s1;
			_String s2;
			ConvertString(x, s1);
			ConvertString(y, s2);
			Message(s1, s2);
    }
    template<typename XChar, typename YChar>
    void Message(const std::basic_string<XChar>& x, const YChar* y)
    {
			Message(x, std::basic_string<YChar>(y));
    }
    template<typename XChar, typename YChar>
    void Message(const XChar* x, const std::basic_string<YChar>& y)
    {
			Message(std::basic_string<XChar>(x), y);
    }
    template<typename XChar, typename YChar>
    void Message(const XChar* x, const YChar* y)
    {
			Message(std::basic_string<XChar>(x), std::basic_string<YChar>(y));
    }
    
    // 1 string arg...
    template<typename XChar>
    void Message(const XChar* s)
    {
			Message(std::basic_string<XChar>(s));
    }
    template<typename XChar>
    void Message(const std::basic_string<XChar>& s)
    {
			Message(s, std::string(""));
    }
    template<typename XChar>
    void Message(const FormatX<XChar>& s)
    {
			Message(s.Str(), std::string(""));
    }

  private:
    static void __cdecl ThreadProc(void* p)
    {
	    static_cast<LogWindow*>(p)->ThreadProc();
    }

    void ThreadProc()
    {
			// you *cannot* call Message() from this thread.  this thread needs to pump messages 24/7
			WNDCLASSW wc = {0};
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = LogWindow::MainProc;
			wc.hInstance = m_hInstance;
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			wc.lpszClassName =  m_fileName.c_str();
			RegisterClassW(&wc);

			LogWindowPlacementHelper placement;

			m_hMain = CreateWindowExW(0, m_fileName.c_str(), PathFindFileNameW(m_fileName.c_str()), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
				placement.m_x, placement.m_y, placement.m_width, placement.m_height, 0, 0, m_hInstance, this);

	#if (LIBCC_ENABLE_LOG_WINDOW == 1)
			ShowWindow(m_hMain, SW_SHOW);
	#endif
			SetEvent(m_hInitialized);

			MSG msg;
			while(GetMessage(&msg, 0, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			return;
    }
    
    static LRESULT CALLBACK MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
			LogWindow* pThis = static_cast<LogWindow*>(GetPropW(hWnd, L"LibCC Log Window"));
			switch(uMsg)
			{
			case WM_CLOSE:
				return 0;
	#if (LIBCC_ENABLE_LOG_WINDOW == 1)
			case WM_SIZE:
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
					break;
				}
	#endif
			case WM_DESTROY:
				{
					PostQuitMessage(0);
					return 0;
				}
			case WM_LogMessage:
				{
					MessageInfo& mi = *(MessageInfo*)lParam;
					ThreadInfo& ti = pThis->GetThreadInfo(mi.threadID);

					static const size_t IndentSize = 2;

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

					std::wstring file(LibCC::FormatW("[%-%-%;%:%:%][%] %%%|")
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
						.Str());
					std::wstring gui(LibCC::FormatW("%%%|").s(indent).s(mi.s1).s(mi.s2).Str());

					// do ods
	#if (LIBCC_ENABLE_LOG_ODSF == 1)
					OutputDebugStringW(file.c_str());
	#endif

					// do file
	#if (LIBCC_ENABLE_LOG_FILE == 1)
					HANDLE h = CreateFileW(pThis->m_fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
					if(h && h != INVALID_HANDLE_VALUE)
					{
						SetFilePointer(h, 0, 0, FILE_END);
						DWORD br;
						std::string a;
						ConvertString(file, a);
						WriteFile(h, a.c_str(), (DWORD)a.size(), &br, 0);
						CloseHandle(h);
					}
	#endif

					// do gui
	#if (LIBCC_ENABLE_LOG_WINDOW == 1)
					int ndx = GetWindowTextLength(pThis->m_hEdit);
					SendMessage(pThis->m_hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
					SendMessageW(pThis->m_hEdit, EM_REPLACESEL, 0, (LPARAM)gui.c_str());

					ndx = GetWindowTextLength(ti.hEdit);
					SendMessage(ti.hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
					SendMessageW(ti.hEdit, EM_REPLACESEL, 0, (LPARAM)gui.c_str());
	#endif
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
			case WM_LogExit:
				{
					DestroyWindow(hWnd);
					return 0;
				}
			case WM_NOTIFY:
				{
	#if (LIBCC_ENABLE_LOG_WINDOW == 1)
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
	#endif
					break;
				}
			case WM_CREATE:
				{
					CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
					pThis = static_cast<LogWindow*>(pcs->lpCreateParams);
					SetPropW(hWnd, L"LibCC Log Window", static_cast<HANDLE>(pThis));
					pThis->m_hMain = hWnd;
	#if (LIBCC_ENABLE_LOG_WINDOW == 1)
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
	#endif

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
	#if (LIBCC_ENABLE_LOG_WINDOW == 1)
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
	#endif
			m_threads.push_back(ThreadInfo());
			ThreadInfo& ret(m_threads.back());
			ret.tabItem = newTabItem;
			ret.threadID = threadID;
			ret.hEdit = hNewEdit;
	    
			return ret;
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
    std::wstring m_fileName;
  };

  class LogScopeMessage
  {
  public:
		template<typename XChar>
    LogScopeMessage(const XChar* op, LogWindow* pLog) :
      m_pLog(pLog)
    {
      m_pLog->Message(std::wstring(L"{ "), std::basic_string<XChar>(op));
      m_pLog->Indent();
    }
		template<typename XChar>
    LogScopeMessage(const std::basic_string<XChar>& op, LogWindow* pLog) :
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

    LogWindow* m_pLog;
  };
//#endif// LIBCC_ENABLE_LOG_AT_ALL
}
