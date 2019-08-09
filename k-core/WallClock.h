#ifndef SYN_TIME_H
#define SYN_TIME_H

#include <assert.h>

#ifdef WIN32
#include <time.h>
#include <windows.h>
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif


struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
 
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}


#else
// MAC
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#   include <sys/time.h>
#endif


class WallClock
{
public:
    WallClock()
    {
        mLastTime = 0.0f;
        mFreq = getFreq();
        mFreqInv = (1.0 / (double)mFreq);
        mLargestAllowedForDelta = 9999999.0f;
        mSmallestAllowedForDelta = 0.0f;
    }

    inline double getTime()
    {
#ifdef WIN32
        DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0x01);
        //DWORD_PTR oldmask2 = ::SetProcessAffinityMask(::GetCurrentProcess(), 0x01);
        LARGE_INTEGER i;
        QueryPerformanceCounter( &i );
        //static int count = 0;
        //if (30 < count++) /// ick a branch?  what's worse, div or branch... :(
        {
            // cpus with power saving have changing frequency... ick. :(
            LARGE_INTEGER f;
            QueryPerformanceFrequency( &f );
            mFreqInv = (1.0 / (double)f.QuadPart);
            //count = 0;
        }
        ::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
        //::SetProcessAffinityMask(::GetCurrentProcess(), oldmask2);

        return mFreqInv * i.QuadPart; // time
#else
		struct timeval start;
		gettimeofday(&start, NULL);
        double result = double(start.tv_sec) + (double(start.tv_usec) / 1000000.0);
		return result;
#endif
    }

    inline double getDelta()
    {
        if (mLastTime == 0.0)
        {
            mLastTime = getTime();
            return mSmallestAllowedForDelta;
        }
        double curtime = getTime();
        double dt = curtime - mLastTime;
        mLastTime = curtime;
        dt = 0.0 < dt ? dt : 0.0; // can be negative on cpus with powersave
        dt = mLargestAllowedForDelta < dt ? mLargestAllowedForDelta :
            dt < mSmallestAllowedForDelta ? mSmallestAllowedForDelta : dt;
        return dt;
    }

    void setRange( double lower, double upper )
    {
       mSmallestAllowedForDelta = lower;
       mLargestAllowedForDelta = upper;
    }

    inline void begin()
    {
#ifdef WIN32
        mOldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0x01);
        QueryPerformanceCounter( &mBeginTime );
#else
		assert(false);
#endif
    }

    inline double end()
    {
#ifdef WIN32
        QueryPerformanceCounter( &mEndTime );
        ::SetThreadAffinityMask(::GetCurrentThread(), mOldmask);
        return double( mCycles = mEndTime.QuadPart - mBeginTime.QuadPart );
#else
		assert(false);
        return 0.0;
#endif
    }
#ifdef WIN32
    inline double cycles() const { return (double)mCycles; }
#endif

#ifdef WIN32
    DWORD_PTR mOldmask;
    LARGE_INTEGER mBeginTime;
    LARGE_INTEGER mEndTime;
    long long mCycles;
#endif

    inline unsigned long long getFreq() const
    {
#ifdef WIN32
        LARGE_INTEGER i;
        DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0x01);
        QueryPerformanceFrequency( &i );
        ::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
        return i.QuadPart;   // frequency
#else
		return 1;
#endif
    }

    unsigned long long mFreq;
    double mFreqInv;
    double mLastTime;
    double mLargestAllowedForDelta, mSmallestAllowedForDelta;
};




#endif
