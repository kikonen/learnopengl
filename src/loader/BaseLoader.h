#pragma once

#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

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
            const std::shared_ptr<Context>& ctx);

        ~BaseLoader();

        void setRegistry(Registry* registry);

        bool fileExists(std::string_view filename) const;
        std::string readFile(std::string_view filename) const;

    public:
        std::shared_ptr<Context> m_ctx;

        Registry* m_registry;
        event::Dispatcher* m_dispatcher { nullptr };
    };
}
