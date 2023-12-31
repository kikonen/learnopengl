#pragma once

#define ASSERT_WT() assert(util::isWorkerThread())
#define ASSERT_RT() assert(!util::isWorkerThread())

namespace util
{
    void markWorkerThread();
    bool isWorkerThread();
}
