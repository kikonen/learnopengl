#pragma once

#include <memory>

#include "util/util.h"

struct UpdateContext;

namespace mesh {
    class TextMesh;
}

namespace text {
    class TextVAO;

    class TextSystem {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static text::TextSystem& get() noexcept;

        TextSystem();
        ~TextSystem();

        void clear();

        void shutdown();
        void prepare();

        void updateRT(const UpdateContext& ctx);

        text::TextVAO* getTextVAO()
        {
            return m_textVao.get();
        }

    private:
        std::unique_ptr<text::TextVAO> m_textVao;
    };
}
