#pragma once

namespace render
{
    class FrameBuffer;

    struct PassContext
    {
        render::FrameBuffer* buffer{ nullptr };
        int attachmentIndex{ -1 };
    };
}
