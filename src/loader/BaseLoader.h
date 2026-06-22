#pragma once

#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "util/Ref.h"

#include "converter/base.h"

#include "ki/size.h"

namespace event {
    class Dispatcher;
}

class Registry;

namespace loader
{
    struct Context;
    class Loaders;

    class BaseLoader
    {
    public:
        BaseLoader(
            const util::Ref<Context>& ctx);

        ~BaseLoader();

        void setRegistry(const util::Ref<Registry>& registry);

        bool fileExists(std::string_view filename) const;
        std::string readFile(std::string_view filename) const;

    public:
        util::Ref<Context> m_ctx;

        util::Ref<Registry> m_registry;
        util::Ref<event::Dispatcher> m_dispatcherWorker { nullptr };
        util::Ref<event::Dispatcher> m_dispatcherView{ nullptr };
    };
}
