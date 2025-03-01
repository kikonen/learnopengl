#include "TextSystem.h"

#include "util/thread.h"

#include "shader/Shader.h"

#include "text/vao/TextVAO.h"

namespace {
    static text::TextSystem s_instance;
}

namespace text
{
    text::TextSystem& TextSystem::get() noexcept
    {
        return s_instance;
    }

    TextSystem::TextSystem()
        : m_textVao{ std::make_unique<text::TextVAO>("text") }
    {}

    TextSystem::~TextSystem() = default;

    void TextSystem::clear()
    {
        ASSERT_RT();

        m_textVao->clear();
    }

    void TextSystem::shutdown()
    {
        ASSERT_RT();

        clear();
    }

    void TextSystem::prepare()
    {
        ASSERT_RT();

        m_textVao->prepare();

        clear();
    }

    void TextSystem::updateRT(const UpdateContext& ctx)
    {
        m_textVao->updateRT();
    }
}
