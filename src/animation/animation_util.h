#pragma once

#include <string>
#include <memory>

#include "MetaData.h"

namespace animation {
    std::unique_ptr<animation::Metadata> loadMetaData(const std::string& meshFilePath);
}
