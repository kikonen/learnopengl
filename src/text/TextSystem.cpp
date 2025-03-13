#include "TextSystem.h"

#include "util/thread.h"

#include "shader/Shader.h"

#include "text/vao/TextVAO.h"

namespace
{
    static text::TextSystem* s_engine{ nullptr };
}

namespace text
{
    void TextSystem::init() noexcept
    {
        s_engine = new TextSystem();
    }

    void TextSystem::release() noexcept
    {
        auto* s = s_engine;
        s_engine = nullptr;
        delete s;
    }

    TextSystem& TextSystem::get() noexcept
    {
        assert(s_engine);
        return *s_engine;
    }
}

namespace text
{
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
