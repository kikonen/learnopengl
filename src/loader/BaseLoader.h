#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "converter/base.h"

#include "Context.h"

#include "ki/size.h"

namespace event {
    class Dispatcher;
}

class Registry;

namespace loader
{
    class Loaders;

    class BaseLoader
    {
    public:
        BaseLoader(
            Context ctx);

        ~BaseLoader();

        void setRegistry(std::shared_ptr<Registry> registry);

        bool fileExists(std::string_view filename) const;
        std::string readFile(std::string_view filename) const;

    public:
        Context m_ctx;

        std::shared_ptr<Registry> m_registry;
        event::Dispatcher* m_dispatcher { nullptr };
    };
}
