#pragma once

#include <vector>

namespace util {
    template <typename T, class U>
    struct DagItem
    {
        T parentId{ 0 };
        T nodeId{ 0 };
        U* data{ nullptr };
    };

    template <typename T, class U>
    struct DagSort {
        std::vector<DagItem<T, U>> sort(
            std::vector<DagItem<T, U>>& items);
    };
}
