#include "thread.h"

namespace {
    thread_local bool g_isWorker{ false };
}

namespace util
{
    void markWorkerThread()
    {
        g_isWorker = true;
    }

    bool isWorkerThread()
    {
        return g_isWorker;
    }
}
