#pragma once

#include <glm/glm.hpp>

#include "asset/Material.h"
#include "Node.h"

class Sprite final : public Node
{
public:
    static std::shared_ptr<NodeType> getNodeType(
        const Assets& assets,
        ShaderRegistry& shaders,
        const std::string& path,
        const std::string& normalMapPath);

    Sprite(std::shared_ptr<NodeType> type, glm::vec2 size);
    virtual ~Sprite();

private:
    static Material createMaterial(
        const Assets& assets, 
        const std::string& path,
        const std::string& normalMapPath);

    static Mesh* getMesh(
        const Assets& assets, 
        const Material& material);

public:
    const glm::vec2 size;
};

