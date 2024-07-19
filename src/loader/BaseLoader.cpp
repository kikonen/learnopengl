#include "BaseLoader.h"

#include <mutex>
#include <fstream>
#include <regex>
#include <filesystem>
#include <iomanip>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/Log.h"
#include "ki/sid.h"

#include "util/Util.h"

#include "engine/AsyncLoader.h"

#include "registry/Registry.h"

#include "MaterialData.h"

#include "loader/document.h"

namespace {
    //std::regex UUID_RE = std::regex("[0-9]{8}-[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{8}");
}

namespace loader
{
    //static const float DEF_ALPHA = 1.0;

    BaseLoader::BaseLoader(
        Context ctx)
        : m_ctx(ctx)
    {
    }

    BaseLoader::~BaseLoader() = default;

    void BaseLoader::setRegistry(std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = m_registry->m_dispatcherWorker;
    }

    std::string BaseLoader::readFile(std::string_view filename) const
    {
        return util::readFile(m_ctx.m_dirName, filename);
    }
}
