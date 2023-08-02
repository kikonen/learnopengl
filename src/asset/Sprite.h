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
        const std::string& name,
        std::vector<Sprite>& sprites);

    static Sprite* findID(
        const int objectID,
        std::vector<Sprite>& sprites);

    static const Sprite* findID(
        const int objectID,
        const std::vector<Sprite>& sprites);

    void prepare(const Assets& assets);

    std::vector<Shape>& getShapes() {
        return m_shapes;
    }

public:
    int m_objectID;

    std::string m_name;
    std::vector<Shape> m_shapes;

    bool m_registered{ false };

private:
    bool m_prepared{ false };
};
