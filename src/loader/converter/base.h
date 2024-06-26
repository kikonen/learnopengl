#pragma once

#include <memory>

namespace loader {
    class DocNode;

    //typedef loader::DocNode Node;

    enum class DocNodeType {
        undefined,
        null,
        scalar,
        sequence,
        map
    };
}
