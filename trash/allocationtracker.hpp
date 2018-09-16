 /*
  LibCC
  AllocationTracker Module
  (c) 2002-2008 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC
	Official source code: http://svn.winprog.org/personal/carl/LibCC

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

Usage:
To track allocations of a class, just add this as a member of the class, and 

Add this to a .cpp file to instantiate the extern:
	LibCC::AllocationManager* LibCC::g_pAllocationManager;

And add this as the 1st line of WinMain:
	LibCC::AllocationManager::Instance allocationManager;

For any class you want to track allocations of, add an allocation token thingy. The simplest
way is like this, without specifying which kind of class it is:
	class MyClass
	{
	private:
		LibCC::Allocation m_allocationToken;// name it whatever you want.
	};

If you want to specify a class name to help reporting and distinguish between different object types,
	class MyClass
	{
	private:
		LibCC::Allocation m_allocationToken;// name it whatever you want.

		MyClass() :
			m_allocationToken("MyClass")
		{
		}
	};

Now when you need to report memory leaks (probably as one of the last things in WinMain?), do:
	LibCC::g_pAllocationManager->Report();

Reports look like this in your debugger output:

	(MyClass)(0x12341234)                             <-- an allocation happening while g_pAllocationManager is 0
	[Begin: Reporting allocation leaks]
		(MyClass): 1 refs outstanding
			cookie:1, pointer:0x0012fad8                  <-- each allocation for a particular class name gets a sequential cookie number you can use to set a breakpoint on.
	[End: Reporting allocation leaks]
	~(MyClass)(0x0012fad8) [unmanaged]                <-- a deallocation happening while g_pAllocationManager is 0


You can also set a breakpoint using:
	LibCC::g_pAllocationManager->RegisterBreakpoint("MyClass", cookieNumber);


*/

#pragma once

#include "StringUtil.hpp"
#include <map>
#include <set>

namespace LibCC
{
  struct AllocationInfo
  {
		void* pointer;
    int cookie;// ALWAYS a sequential number, only valid in the context of the class.
  };

  // each class type has its own allocation set
  class AllocationClass
  {
  public:
		AllocationClass() :
			m_nextCookie(0)
		{
		}

		void RegisterAllocation(void* pointer, AllocationInfo& out)
		{
			++ m_nextCookie;
			out.cookie = m_nextCookie;
			out.pointer = pointer;
			m_allocations[out.cookie] = out;
		}

		void RegisterDeallocation(const AllocationInfo& info)
		{
			m_allocations.erase(info.cookie);
		}

		size_t GetReferenceCount() const
		{
			return m_allocations.size();
		}

		void Report() const
		{
			std::map<int, AllocationInfo>::const_iterator it;
			for(it = m_allocations.begin(); it != m_allocations.end(); ++ it)
			{
				OutputDebugStringA(FormatA("    cookie:%, pointer:%|").i(it->second.cookie).p(it->second.pointer).CStr());
			}
		}

  private:
    std::map<int, AllocationInfo> m_allocations;
    int m_nextCookie;
  };

  typedef std::pair<int, std::string> AllocationBreakpoint;

  class AllocationManager
  {
		class CriticalSection
		{
		public:
			class Lock// scope lock. don't let the critical section get deleted during this.
			{
			public:
				Lock(CriticalSection& critsec) : critsec_(critsec) { critsec_.Enter(); }
				~Lock() { critsec_.Leave(); }
			private:
				CriticalSection& critsec_;
			};

			CriticalSection() { ::InitializeCriticalSection(&cs_); }
			~CriticalSection() { ::DeleteCriticalSection(&cs_); }
		private:
			void Enter() { ::EnterCriticalSection(&cs_); }
			void Leave() { ::LeaveCriticalSection(&cs_); }
			CRITICAL_SECTION cs_;
		};

  public:
		void RegisterBreakpoint(const std::string& className, int cookie)
		{
			CriticalSection::Lock l(m_cs);
			m_breakpoints.insert(AllocationBreakpoint(cookie, className));
		}

		void RegisterAllocation(const std::string& className, void* pointer, AllocationInfo& out)
		{
			bool _break = false;
			{
				CriticalSection::Lock l(m_cs);
				m_classMap[className].RegisterAllocation(pointer, out);
				if(m_breakpoints.count(AllocationBreakpoint(out.cookie, className)))
				{
					_break = true;
				}
			}
			if(_break)
			{
				__asm int 3;
			}
		}

		void RegisterDeallocation(const std::string& className, const AllocationInfo& info)
		{
			CriticalSection::Lock l(m_cs);
			m_classMap[className].RegisterDeallocation(info);
		}

		void Report()
		{
			CriticalSection::Lock l(m_cs);
			OutputDebugStringA("[Begin: Reporting allocation leaks]\r\n");
			size_t totalRefsOutstanding = 0;

			std::map<std::string, AllocationClass>::const_iterator it;
			for(it = m_classMap.begin(); it != m_classMap.end(); ++ it)
			{
				size_t refs = it->second.GetReferenceCount();
				totalRefsOutstanding += refs;
				OutputDebugStringA(FormatA("  %: % refs outstanding|").s(it->first).i((int)refs).CStr());
				if(refs)
				{
					it->second.Report();
				}
			}
			if(0 == totalRefsOutstanding)
			{
				OutputDebugStringA("  (none - congrats.)\r\n");
			}

			OutputDebugStringA("[End: Reporting allocation leaks]\r\n");
		}

  private:
    std::map<std::string, AllocationClass> m_classMap;
    std::set<AllocationBreakpoint> m_breakpoints;
    CriticalSection m_cs;
  };

  extern AllocationManager* g_pAllocationManager;

  // this is the class that you use out there to make this system work.
  class Allocation
  {
  public:
		Allocation(const std::string& className) :
			m_name(className)
		{
			if(g_pAllocationManager)
			{
				g_pAllocationManager->RegisterAllocation(m_name, this, m_info);
			}
			else
			{
				OutputDebugString(Format("%(%) [unmanaged]|").s(m_name).p(this).CStr());
			}
		}
		Allocation()
		{
			m_name = "(unnamed)";
			if(g_pAllocationManager)
			{
				g_pAllocationManager->RegisterAllocation(m_name, this, m_info);
			}
			else
			{
				OutputDebugString(Format("%(%) [unmanaged]|").s(m_name).p(this).CStr());
			}
		}

		~Allocation()
		{
			if(g_pAllocationManager)
			{
				g_pAllocationManager->RegisterDeallocation(m_name.c_str(), m_info);
			}
			else
			{
				OutputDebugString(Format("~%(%) [unmanaged]|").s(m_name).p(this).CStr());
			}
		}

  private:
    AllocationInfo m_info;
    std::string m_name;
  };

}


