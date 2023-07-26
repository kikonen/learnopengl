#pragma once

#include <string>
#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "Texture.h"

#include "SpriteSSBO.h"


constexpr int SPRITE_DIFFUSE_IDX = 0;
constexpr int SPRITE_EMISSION_IDX = 1;
constexpr int SPRITE_SPECULAR_IDX = 2;
constexpr int SPRITE_NORMAL_MAP_IDX = 3;
constexpr int SPRITE_TEXTURE_COUNT = 4;

//
// Texture data for single sprite
//
struct Sprite final
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
    Sprite();

    Sprite(const Sprite&) = default;
    Sprite(Sprite&&) = default;
    Sprite& operator=(const Sprite&) = default;
    Sprite& operator=(Sprite&&) = default;

    ~Sprite();

    void loadTextures(const Assets& assets);

    bool hasTex(int index) const;

    void prepare(const Assets& assets);

    const SpriteSSBO toSSBO() const;

    static Sprite* find(
        const std::string& name,
        std::vector<Sprite>& sprites);

    static Sprite* findID(
        const int objectID,
        std::vector<Sprite>& sprites);

    static const Sprite* findID(
        const int objectID,
        const std::vector<Sprite>& sprites);

    const std::string getTexturePath(
        const Assets& assets,
        const std::string& textureName);

private:
    void loadTexture(
        const Assets& assets,
        int idx,
        const std::string& name);

public:
    int m_objectID;

    std::string m_name;
    std::string m_path;

    mutable int m_registeredIndex = -1;

    std::string m_spriteDir;

    std::array<BoundTexture, SPRITE_TEXTURE_COUNT> m_textures;

    std::string map_kd;
    std::string map_ks;
    std::string map_ke;
    std::string map_bump;


    static const int DEFAULT_ID = 0;
private:
    bool m_prepared = false;

    bool m_loaded = false;
};
