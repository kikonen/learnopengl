#pragma once

class RenderContext;

class Raycast final
{
public:
    Raycast();
    ~Raycast();

    int castRay(const RenderContext& ctx);
};

