#include "TextSystem.h"

#include "asset/Shader.h"

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

    void TextSystem::prepare()
    {
        m_textVao->prepare();
    }

    void TextSystem::updateRT(const UpdateContext& ctx)
    {
        m_textVao->updateRT();
    }
}
