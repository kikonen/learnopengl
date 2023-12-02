#pragma once

#include <vector>
#include <string>

#include "asset/Shape.h"

class Assets;

class Sprite final {
public:
    Sprite();
    ~Sprite();

    static Sprite* find(
        std::string_view name,
        std::vector<Sprite>& sprites);

    static Sprite* findID(
        const ki::sprite_id id,
        std::vector<Sprite>& sprites);

    static const Sprite* findID(
        const ki::sprite_id id,
        const std::vector<Sprite>& sprites);

    void prepare(const Assets& assets);

    std::vector<Shape>& getShapes() {
        return m_shapes;
    }

public:
    ki::sprite_id m_id;

    std::string m_name;
    std::vector<Shape> m_shapes;

private:
    bool m_prepared{ false };
    bool m_loaded{ false };
};
