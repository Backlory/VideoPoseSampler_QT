
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <locale>
#include <codecvt>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include "highresolutiontime.h"

namespace fs = boost::filesystem;

bool usleep(int64_t usec);


double HSTime::wall_time()
{
    static const boost::posix_time::ptime t0(boost::gregorian::date(1970, 1, 1));
    const boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration delta(t1 - t0);
    return delta.total_microseconds() / 1000000.0;
}

void HSTime::sleep_for(double dt)
{
    using namespace std::chrono;
    using clock = high_resolution_clock;
    using elapse = duration<int64_t, nanoseconds>;
    steady_clock::time_point t = clock::now() + nanoseconds(int64_t(dt * 1e9));

    while (t - clock::now() > microseconds(100)) {
        if (!usleep(100)) {
            std::this_thread::sleep_for(microseconds(100));
        }
    }
    while (clock::now() < t) {
        if (!usleep(1)) {
            std::this_thread::sleep_for(microseconds(1));
        }
    }
}

std::string HSTime::timestamp()
{
    using namespace boost::posix_time;
    // set your formatting
    std::stringstream is;
    is.imbue(std::locale(is.getloc(), new boost::posix_time::time_facet("%Y%m%d_%H%M%S_%f")));
    is << second_clock::local_time();
    return is.str();
}


#if defined(WINDOWS) || defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
#include <windows.h>
#include <iomanip>

// From: https://gist.github.com/GoaLitiuM/aff9fbfa4294fa6c1680

unsigned long setHighestTimerResolution(unsigned long timer_res_us)
{
    static const HINSTANCE ntdll = LoadLibraryA("ntdll.dll");
    static unsigned long timer_current_res = ULONG_MAX;
    if (ntdll != NULL) {
        typedef long(NTAPI * pNtSetTimerResolution)(
            unsigned long RequestedResolution, BOOLEAN Set, unsigned long *ActualResolution);

        pNtSetTimerResolution NtSetTimerResolution
            = (pNtSetTimerResolution)GetProcAddress(ntdll, "NtSetTimerResolution");
        if (NtSetTimerResolution != NULL) {
            // bounds are validated and set to the highest allowed resolution
            NtSetTimerResolution(timer_res_us, TRUE, &timer_current_res);
        }
        // we can decrement the internal reference count by one
        // and NTDLL.DLL still remains loaded in the process
        FreeLibrary(ntdll);
    }
    return timer_current_res;
}

bool usleep(int64_t usec)
{
    static size_t currentResolution;
    if (currentResolution == 0)
        currentResolution = setHighestTimerResolution(1);

    HANDLE timer;
    LARGE_INTEGER period;

    // negative values are for relative time
    period.QuadPart = -(10 * usec);

    if (!(timer = CreateWaitableTimerW(NULL, TRUE, NULL))) {
        return false;
    }
    if (SetWaitableTimer(timer, &period, 0, NULL, NULL, 0)) {
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
        return false;
    }
    CloseHandle(timer);
    return true;
}
#endif
