#pragma once

#include <vector>

class Assets;
class Sprite;

class SpriteSheet final {
public:
    SpriteSheet();
    ~SpriteSheet();

    void prepare(const Assets& assets);

public:
    int m_objectID;

private:
    bool m_prepared{ false };

    std::vector<Sprite> m_sprites;
};
