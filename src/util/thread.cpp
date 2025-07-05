#include "thread.h"

namespace {
    thread_local util::ThreadType g_threadType{ util::ThreadType::main };
}

namespace util
{
    void markWorkerThread()
    {
        g_threadType = ThreadType::worker;
    }

    bool isWorkerThread()
    {
        return g_threadType == ThreadType::worker;
    }

    void markOtherThread()
    {
        g_threadType = ThreadType::other;
    }

    bool isOtherThread()
    {
        return g_threadType == ThreadType::other;
    }
}
