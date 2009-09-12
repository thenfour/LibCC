/*
  Last updated May 27, 2005 carlc

  (c) 2004-2005 Carl Corcoran, carl@ript.net
  http://carl.ript.net/stringformat/
  http://carl.ript.net/wp
  http://mantis.winprog.org/
  http://svn.winprog.org/personal/carlc/stringformat

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
  ticks 
  A date and time expressed in 100-nanosecond units. 
*/

#pragma once


namespace LibCC
{
  enum DateTimeKind
  {
	  DateTimeKind_Unspecified,
	  DateTimeKind_UTC,
	  DateTimeKind_Local
  };

  enum Month
  {
	  Month_January = 1,
	  Month_February = 2,
	  Month_March = 3,
	  Month_April = 4,
	  Month_May = 5,
	  Month_June = 6,
	  Month_July = 7,
	  Month_August = 8,
	  Month_September = 9,
	  Month_October = 10,
	  Month_November = 11,
	  Month_December = 12
  };

  enum Weekday
  {
	  Weekday_Sunday = 1,
	  Weekday_Monday = 2,
	  Weekday_Tuesday = 3,
	  Weekday_Wednesday = 4,
	  Weekday_Thursday = 5,
	  Weekday_Friday = 6,
	  Weekday_Saturday = 7
  };


  class TimeSpan
  {
  public:
		TimeSpan() :
			m_ticks(0)
		{
		}

		TimeSpan(__int64 ticks) :
			m_ticks(ticks)
		{
		}

		TimeSpan(long hours, long minutes, long seconds) :
			m_ticks(HoursToTicks(hours) + MinutesToTicks(minutes) + SecondsToTicks(seconds))
		{
		}

		TimeSpan(long days, long hours, long minutes, long seconds) :
			m_ticks(DaysToTicks(days) + HoursToTicks(hours) + MinutesToTicks(minutes) + SecondsToTicks(seconds))
		{
		}

		TimeSpan(long days, long hours, long minutes, long seconds, long milliseconds) :
			m_ticks(DaysToTicks(days) + HoursToTicks(hours) + MinutesToTicks(minutes) + SecondsToTicks(seconds) + MillisecondsToTicks(milliseconds))
		{
		}

		TimeSpan(const TimeSpan& r) :
			m_ticks(r.m_ticks)
		{
		}

	  void Add(const TimeSpan& r)
		{
			m_ticks += r.m_ticks;
		}

	  void Negate()
		{
			m_ticks = -m_ticks;
		}

	  void Subtract(const TimeSpan& r)
		{
			m_ticks -= r.m_ticks;
		}

	  void Multiply(double f)
		{
			m_ticks = static_cast<__int64>(f * static_cast<double>(m_ticks));
		}

	  void Multiply(long x)
		{
			m_ticks *= x;
		}

	  void Divide(double f)
		{
			m_ticks = static_cast<__int64>(static_cast<double>(m_ticks) / f);
		}

	  void Divide(long x)
		{
			m_ticks /= x;
		}

	  long Compare(const TimeSpan& r) const
		{
			if(r.m_ticks == m_ticks) return 0;
			if(m_ticks < r.m_ticks) return -1;
			return 1;
		}

	  bool Equals(const TimeSpan& r) const
		{
			return m_ticks == r.m_ticks;
		}

	  bool LessThan(const TimeSpan& r) const
		{
			return m_ticks < r.m_ticks;
		}

	  bool GreaterThan(const TimeSpan& r) const
		{
			return m_ticks > r.m_ticks;
		}

	  bool LessThanOrEqualTo(const TimeSpan& r) const
		{
			return m_ticks <= r.m_ticks;
		}

	  bool GreaterThanOrEqualTo(const TimeSpan& r) const
		{
			return m_ticks >= r.m_ticks;
		}

	  __int64 GetTicks() const
		{
			return m_ticks;
		}

	  void ToFILETIME(FILETIME& ft) const
		{
			ft.dwHighDateTime = static_cast<DWORD>((m_ticks & 0xFFFFFFFF00000000) >> 32);
			ft.dwLowDateTime = static_cast<DWORD>(m_ticks & 0xFFFFFFFF);
		}

	  void FromFILETIME(const FILETIME& ft)
		{
			m_ticks = (static_cast<__int64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
		}

	  static long TicksToSeconds(__int64 t)
		{
		  return static_cast<long>(t / SecondsToTicks(__int64(1)));
		}

		template<typename T>
	  static __int64 MillisecondsToTicks(T s)
		{
		  return s * 10000;
		}

		template<typename T>
	  static __int64 SecondsToTicks(T s)
		{
			return MillisecondsToTicks(s * 1000);
		}

		template<typename T>
	  static __int64 MinutesToTicks(T s)
		{
			return SecondsToTicks(s * 60);
		}

		template<typename T>
	  static __int64 HoursToTicks(T s)
		{
			return MinutesToTicks(s * 60);
		}

		template<typename T>
	  static __int64 DaysToTicks(T s)
		{
			return HoursToTicks(s * 24);
		}

  private:
	  __int64 m_ticks;
  };


  class DateTime
  {
  public:
	  DateTime() :
			m_ticks(0),
			m_kind(DateTimeKind_Unspecified),
			m_stDirty(true)
		{
      m_stDirty = true;
		}

	  DateTime(unsigned __int64 ticks, DateTimeKind kind = DateTimeKind_Unspecified) :
			m_ticks(ticks),
			m_kind(kind),
			m_stDirty(true)
		{
		}

	  DateTime(const DateTime& r) :
			m_ticks(r.m_ticks),
			m_kind(r.m_kind),
			m_stDirty(r.m_stDirty),
			m_st(r.m_st)
		{
		}

	  DateTime(const SYSTEMTIME& r, DateTimeKind kind = DateTimeKind_Unspecified) :
			m_kind(kind),
			m_st(r),
			m_stDirty(false)
		{
			__FromCachedSystemTime();
		}

	  DateTime(long year, Month month, long day, DateTimeKind kind = DateTimeKind_Unspecified) :
			m_kind(kind),
			m_stDirty(false)
		{
			__ClearCachedSystemTime();
			m_st.wYear = static_cast<WORD>(year);
			m_st.wMonth = static_cast<WORD>(month);
			m_st.wDay = static_cast<WORD>(day);
			__FromCachedSystemTime();
		}

	  DateTime(long year, Month month, long day, long hour, long minute, long second, DateTimeKind kind = DateTimeKind_Unspecified) :
			m_kind(kind),
			m_stDirty(false)
		{
			__ClearCachedSystemTime();
			m_st.wYear = static_cast<WORD>(year);
			m_st.wMonth = static_cast<WORD>(month);
			m_st.wDay = static_cast<WORD>(day);
			m_st.wHour = static_cast<WORD>(hour);
			m_st.wMinute = static_cast<WORD>(minute);
			m_st.wSecond = static_cast<WORD>(second);
			__FromCachedSystemTime();
		}

	  DateTime(long year, Month month, long day, long hour, long minute, long second, long millisecond, DateTimeKind kind = DateTimeKind_Unspecified) :
			m_kind(kind),
			m_stDirty(false)
		{
			__ClearCachedSystemTime();
			m_st.wYear = static_cast<WORD>(year);
			m_st.wMonth = static_cast<WORD>(month);
			m_st.wDay = static_cast<WORD>(day);
			m_st.wHour = static_cast<WORD>(hour);
			m_st.wMinute = static_cast<WORD>(minute);
			m_st.wSecond = static_cast<WORD>(second);
			m_st.wMilliseconds = static_cast<WORD>(millisecond);
			__FromCachedSystemTime();
		}


	  void Assign(unsigned __int64 ticks, DateTimeKind kind)
		{
			m_ticks = ticks;
			m_kind = kind;
			m_stDirty = true;
		}

		void Add(const TimeSpan& r)
		{
			m_stDirty = true;
			m_ticks += r.GetTicks();
		}

		void Subtract(const TimeSpan& r)
		{
			m_stDirty = true;
			m_ticks -= r.GetTicks();
		}

		TimeSpan Subtract(const DateTime& r)
		{
			return TimeSpan(m_ticks - r.m_ticks);
		}

		// these are difficult because of time zone / daylight savings / carryover
		//void AddDays(__int16 d)
	  //void AddMonths(__int16 d);

		template<typename T>
		void AddHours(T d)
		{
			m_stDirty = true;
			m_ticks += TimeSpan::HoursToTicks( d);
		}

		template<typename T>
		void AddMilliseconds(T d)
		{
			m_stDirty = true;
			m_ticks += TimeSpan::MillisecondsToTicks(d);
		}

		template<typename T>
		void AddMinutes(T d)
		{
			m_stDirty = true;
			m_ticks += TimeSpan::MinutesToTicks(d);
		}

		template<typename T>
	  void AddSeconds(T v)
		{
			m_stDirty = true;
			m_ticks += TimeSpan::SecondsToTicks(v);
		}

		template<typename T>
	  void AddTicks(T ticks)
		{
			m_stDirty = true;
			m_ticks += ticks;
		}

	  void AddYears(__int16 d)
		{
			__GetCachedSystemTime().wYear += d;
			__FromCachedSystemTime();
		}

    long GetYear() { return __GetCachedSystemTime().wYear; };
    Month GetMonth() { return Month(__GetCachedSystemTime().wMonth); };
    long GetDay() { return __GetCachedSystemTime().wDay; };
    long GetHour() { return __GetCachedSystemTime().wHour; };
    long GetMinute() { return __GetCachedSystemTime().wMinute; };
    long GetSecond() { return __GetCachedSystemTime().wSecond; };
    long GetMillisecond() { return __GetCachedSystemTime().wMilliseconds; };

	  long Compare(const DateTime& s) const
		{
			if(m_ticks < s.m_ticks)
			{
				return -1;
			}
			if(m_ticks > s.m_ticks)
			{
				return 1;
			}
			return 0;
		}

	  bool Equals(const DateTime& s)
		{
			return m_ticks == s.m_ticks;
		}

	  //bool IsDaylightSavingTime();
	  unsigned __int64 ToBinary()
		{
			return m_ticks;
		}

	  //double ToOADate();
	  //DateTime ToLocal();// adjust for daylight savings and time zone
	  //DateTime ToUTC();// adjust for daylight savings and time zone
	  //std::string ToString();

    void FromUnixTime(time_t t)
		{
			FILETIME ft;
			LONGLONG ll;
			ll = Int32x32To64(t, 10000000) + 116444736000000000;
			ft.dwLowDateTime = (DWORD)ll;
			ft.dwHighDateTime = (DWORD)(ll >> 32);
			FromFileTime(ft);
		}

   // void ToUnixTime(time_t&) const;
   // time_t ToUnixTime() const;

   // void FromVariant(VARIANT& v);
   // void ToVariant(_variant_t&) const;
   // _variant_t ToVariant() const;

	  void FromFileTime(const FILETIME& ft)
		{
			m_stDirty = true;
			m_ticks = (static_cast<unsigned __int64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
		}

	  FILETIME ToFileTime() const
		{
			FILETIME ft;
			ft.dwHighDateTime = static_cast<DWORD>((m_ticks & 0xFFFFFFFF00000000) >> 32);
			ft.dwLowDateTime = static_cast<DWORD>(m_ticks & 0xFFFFFFFF);
			return ft;
		}

	  void FromSystemTime(const SYSTEMTIME& ft)
		{
			memcpy(&m_st, &ft, sizeof(ft));
			m_stDirty = false;
			__FromCachedSystemTime();
		}

	  SYSTEMTIME ToSystemTime()
		{
			return __GetCachedSystemTime();
		}

	  static long DaysInMonth(long year, Month m)
		{
			switch(m)
			{
			case Month_January:
			case Month_March:
			case Month_May:
			case Month_July:
			case Month_August:
			case Month_October:
			case Month_December:
				return 31;
			case Month_April:
			case Month_June:
			case Month_September:
			case Month_November:
				return 30;
			case Month_February:
				return IsLeapYear(year) ? 29 : 28;
			}

			return 0;
		}

	  static bool IsLeapYear(long y)
		{
			return ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0);
		}


    static DateTime Now()
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			return DateTime(st, DateTimeKind_Local);
		}

    static DateTime UtcNow()
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			return DateTime(st, DateTimeKind_UTC);
		}

  private:
    void __FromCachedSystemTime()
    {
	    FILETIME ft;
	    SystemTimeToFileTime(&m_st, &ft);
	    FromFileTime(ft);
      m_stDirty = false;
    }

    void __ClearCachedSystemTime()
    {
      memset(&m_st, 0, sizeof(m_st));
    }

    SYSTEMTIME& __GetCachedSystemTime()
    {
      if(m_stDirty)
      {
				FILETIME ft = ToFileTime();
	      FileTimeToSystemTime(&ft, &m_st);
        m_stDirty = false;
      }
      return m_st;
    }

	  unsigned __int64 m_ticks;
	  DateTimeKind m_kind;
    SYSTEMTIME m_st;
    bool m_stDirty;// true if m_st is set;
  };

}

