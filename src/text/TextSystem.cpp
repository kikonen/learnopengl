#include "TextSystem.h"

#include "util/thread.h"

#include "shader/Shader.h"

#include "text/vao/TextVAO.h"

namespace
{
    static text::TextSystem* s_system{ nullptr };
}

namespace text
{
    void TextSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new TextSystem();
    }

    void TextSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    TextSystem& TextSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
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
