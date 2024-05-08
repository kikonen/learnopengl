#pragma once

#include "ShapeSSBO.h"

//#include "asset/Material.h"

//
// Extra data for sprite shape
//
struct Shape final
{
public:
    Shape();

    Shape(const Shape&) = default;
    Shape(Shape&&) = default;
    Shape& operator=(const Shape&) = default;
    Shape& operator=(Shape&&) = default;

    ~Shape();

    const ShapeSSBO toSSBO() const;

private:

public:
    mutable int m_registeredIndex{ -1 };

    float m_rotation{ 0.f };

    //Material m_material;
};
