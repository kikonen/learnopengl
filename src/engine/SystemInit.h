#pragma once

class SystemInit
{
public:
    static void init() noexcept;
    static void release() noexcept;
};
