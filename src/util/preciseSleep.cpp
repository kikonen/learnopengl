#include "preciseSleep.h"

#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "Winmm.lib") // timeGetDevCaps, timeBeginPeriod


namespace
{
    static HANDLE g_timer;
    static int g_schedulerPeriodMs;
    static INT64 g_qpcPerSecond;
}

namespace util
{
    void preciseSleep(double seconds)
    {
        LARGE_INTEGER qpc;
        QueryPerformanceCounter(&qpc);
        INT64 targetQpc = (INT64)(qpc.QuadPart + seconds * g_qpcPerSecond);

        if (g_timer) // Try using a high resolution timer first.
        {
            const double TOLERANCE = 0.001'02;
            INT64 maxTicks = (INT64)g_schedulerPeriodMs * 9'500;
            for (;;) // Break sleep up into parts that are lower than scheduler period.
            {
                double remainingSeconds = (targetQpc - qpc.QuadPart) / (double)g_qpcPerSecond;
                INT64 sleepTicks = (INT64)((remainingSeconds - TOLERANCE) * 10'000'000);
                if (sleepTicks <= 0)
                    break;

                LARGE_INTEGER due;
                due.QuadPart = -(sleepTicks > maxTicks ? maxTicks : sleepTicks);
                SetWaitableTimerEx(g_timer, &due, 0, NULL, NULL, NULL, 0);
                WaitForSingleObject(g_timer, INFINITE);
                QueryPerformanceCounter(&qpc);
            }
        }
        else // Fallback to Sleep.
        {
            const double TOLERANCE = 0.000'02;
            double sleepMs = (seconds - TOLERANCE) * 1000 - g_schedulerPeriodMs; // Sleep for 1 scheduler period less than requested.
            int sleepSlices = (int)(sleepMs / g_schedulerPeriodMs);
            if (sleepSlices > 0)
                Sleep((DWORD)sleepSlices * g_schedulerPeriodMs);
            QueryPerformanceCounter(&qpc);
        }

        while (qpc.QuadPart < targetQpc) // Spin for any remaining time.
        {
            YieldProcessor();
            QueryPerformanceCounter(&qpc);
        }
    }

    void initPreciseSleep()
    {
        // Initialization
        g_timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        TIMECAPS caps;
        timeGetDevCaps(&caps, sizeof caps);
        timeBeginPeriod(caps.wPeriodMin);
        g_schedulerPeriodMs = (int)caps.wPeriodMin;
        LARGE_INTEGER qpf;
        QueryPerformanceFrequency(&qpf);
        g_qpcPerSecond = qpf.QuadPart;
    }
}
