#pragma once


namespace render {
    class ScreenTri {
    public:
        static ScreenTri& get() noexcept;

        ScreenTri() = default;
        ~ScreenTri() = default;

        void draw();
    };
}
