#pragma once

#include "scene/RenderContext.h"
#include "asset/Shader.h"

class NodeType;
class Node;

class Batch final
{
public:
    Batch();

    void add(const glm::mat4& model, const glm::mat3& normal, int objectID);
    void reserve(int count);
    int size();

    void prepare(NodeType* type);

    void update(unsigned int count);
    void bind(const RenderContext& ctx, Shader* shader);
    void draw(const RenderContext& ctx, Node* node, Shader* shader);

    void flush(const RenderContext& ctx, NodeType* type);

public:
    bool prepared = false;
    unsigned int batchSize = 0;

    bool staticBuffer = false;

    bool dirty = false;
    bool objectId = false;

private:
    std::vector<glm::mat4> modelMatrices;
    std::vector<glm::mat3> normalMatrices;
    std::vector<glm::vec4> objectIDs;

    unsigned int modelBuffer = -1;
    unsigned int normalBuffer = -1;
    unsigned int objectIDBuffer = -1;
};

