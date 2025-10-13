#pragma once

#include <assert.h>

#define ASSERT_WT() assert(util::isWorkerThread())
#define ASSERT_OTHER() assert(util::isOtherThread())
#define ASSERT_RT() assert(!(util::isWorkerThread() || util::isOtherThread()))

namespace util
{
    enum class ThreadType {
        worker,
        other,
        main
    };

    void markWorkerThread();
    bool isWorkerThread();

    void markOtherThread();
    bool isOtherThread();
}
