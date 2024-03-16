#pragma once

#include "gui/Frame.h"


class EditorFrame : public Frame
{
public:
    EditorFrame(Window& window);

    void prepare(const PrepareContext& ctx) override;
    void draw(const RenderContext& ctx) override;
};

