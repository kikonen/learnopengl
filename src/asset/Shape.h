#pragma once

#include <string>
#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "Texture.h"

#include "ShapeSSBO.h"


constexpr int SHAPE_DIFFUSE_IDX = 0;
constexpr int SHAPE_EMISSION_IDX = 1;
constexpr int SHAPE_SPECULAR_IDX = 2;
constexpr int SHAPE_NORMAL_MAP_IDX = 3;
constexpr int SHAPE_TEXTURE_COUNT = 4;

//
// Texture data for single shape
//
struct Shape final
{
public:
    struct BoundTexture {
        Texture* texture = nullptr;
        int m_texIndex = -1;

        bool valid() {
            return texture;
        }
    };

public:
    Shape();

    Shape(const Shape&) = default;
    Shape(Shape&&) = default;
    Shape& operator=(const Shape&) = default;
    Shape& operator=(Shape&&) = default;

    ~Shape();

    void loadTextures(const Assets& assets);

    bool hasTex(int index) const;

    void prepare(const Assets& assets);

    const ShapeSSBO toSSBO() const;

    const std::string getTexturePath(
        const Assets& assets,
        const std::string& textureName);

private:
    void loadTexture(
        const Assets& assets,
        int idx,
        const std::string& name);

public:
    mutable int m_registeredIndex = -1;

    std::array<BoundTexture, SHAPE_TEXTURE_COUNT> m_textures;

    float rotation{ 0.f };

    std::string map_kd;
    std::string map_ks;
    std::string map_ke;
    std::string map_bump;
    float map_bump_strength{ 1.f };
};
