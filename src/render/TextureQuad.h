#pragma once


namespace render {
    class TextureQuad {
    public:
        static TextureQuad& get() noexcept;

        TextureQuad() = default;
        ~TextureQuad() = default;

        void draw();

        void drawInstanced(int instanceCount);
    };
}
