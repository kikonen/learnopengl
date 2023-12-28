#pragma once

#define ASSERT_WORKER() assert(util::isWorkerThread())

namespace util
{
    void markWorkerThread();
    bool isWorkerThread();
}
