#pragma once

#include "ki/GL.h"

struct GLBufferRange {
    int m_index = 0;
    int m_offset = 0;
    GLsync m_sync = 0;
};
