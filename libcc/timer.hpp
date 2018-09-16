// LibCC ~ Carl Corcoran, https://github.com/thenfour/LibCC

#pragma once

#include <windows.h>

namespace LibCC
{
  class FPS
  {
  public:
    FPS() :
      m_fps(0),
      m_lasttick(0),
      m_interval(0),
      m_frames(0),
      m_totallasttick(0),
      m_totalframes(0)
    {
      LARGE_INTEGER lifreq;
      QueryPerformanceFrequency(&lifreq);
      m_freq = (double)lifreq.QuadPart;
    }

    void SetRecalcInterval(double secs)
    {
      m_interval = (LONGLONG)(secs * m_freq);
    }

    inline void OnFrame()
    {
      LONGLONG ct = GetCurrentTick();
      LONGLONG delta = ct - m_lasttick;
      m_frames ++;
      m_totalframes ++;

      if(delta > m_interval)
      {
        // recalc fps and reset m_frames
        m_fps = (double)m_frames / TicksToSeconds(delta);
        m_frames = 0;
        m_lasttick = ct;
      }
    }

    inline void ResetTotal()
    {
      m_totalframes = 0;
      m_totallasttick = GetCurrentTick();
    }

    inline double GetAvgFPS() const
    {
      LONGLONG ct = GetCurrentTick();
      LONGLONG delta = ct - m_totallasttick;
      return (double)m_totalframes / TicksToSeconds(delta);
    }

    inline double GetFPS() const
    {
      return m_fps;
    }

  private:

    inline double TicksToSeconds(LONGLONG n) const
    {
      return (double)n / m_freq;
    }

    inline static LONGLONG GetCurrentTick()
    {
      LARGE_INTEGER li;
      QueryPerformanceCounter(&li);
      return li.QuadPart;
    }
    double m_fps;
    double m_freq;// units per second
    long m_frames;// # of frames since last recalc
    LONGLONG m_interval;// how many units until we refresh m_fps
    LONGLONG m_lasttick;

    LONGLONG m_totallasttick;
    LONGLONG m_totalframes;
  };




  struct Timer
  {
    static double TicksToSeconds(uint64_t n) {
      LARGE_INTEGER lifreq;
      QueryPerformanceFrequency(&lifreq);
      auto m_freq = (double)lifreq.QuadPart;
      return (double)n / m_freq;
    }
    static uint64_t SecondsToTicks(double n) {
      LARGE_INTEGER lifreq;
      QueryPerformanceFrequency(&lifreq);
      return (uint64_t)(lifreq.QuadPart * n);
    }
    static LONGLONG GetCurrentTick() {
      LARGE_INTEGER li;
      QueryPerformanceCounter(&li);
      return li.QuadPart;
    }

    void Start() {
      ++m_isRunning;
      if (m_isRunning == 1) {
        m_whenStarted = GetCurrentTick();
      }
    }
    void Stop() {
      if (m_isRunning == 0) {
        return;// hard error.
      }
      m_isRunning--;
      if (m_isRunning == 0) {
        m_accum += GetCurrentTick() - m_whenStarted;
      }
    }
    void Reset() {
      m_isRunning = 0;
      m_whenStarted = 0;
      m_accum = 0;
    }
    LONGLONG GetElapsedTicks() const {
      if (m_isRunning == 0) {
        return m_accum;
      }
      return m_accum + GetCurrentTick() - m_whenStarted;
    }
    double GetElapsedSeconds() const {
      return TicksToSeconds(GetElapsedTicks());
    }
  private:
    int m_isRunning = 0; // Start() ++, Stop() --.
    LONGLONG m_whenStarted = 0;
    LONGLONG m_accum = 0;
  };



  struct ThreadCycleTimer
  {
    static uint64_t GetCurrentCycleCount() {
      uint64_t ret;
      QueryThreadCycleTime(GetCurrentThread(), &ret);
      return ret;
    }

    void Start() {
      ++m_isRunning;
      if (m_isRunning == 1) {
        m_whenStarted = GetCurrentCycleCount();
      }
    }
    void Stop() {
      if (m_isRunning == 0) {
        return;// hard error.
      }
      m_isRunning--;
      if (m_isRunning == 0) {
        m_accum += GetCurrentCycleCount() - m_whenStarted;
      }
    }
    void Reset() {
      m_isRunning = 0;
      m_whenStarted = 0;
      m_accum = 0;
    }
    uint64_t GetElapsedCycles() const {
      if (m_isRunning == 0) {
        return m_accum;
      }
      return m_accum + GetCurrentCycleCount() - m_whenStarted;
    }
  private:
    int m_isRunning = 0; // Start() ++, Stop() --.
    uint64_t m_whenStarted = 0;
    uint64_t m_accum = 0;
  }; 


}

