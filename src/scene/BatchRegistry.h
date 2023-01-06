#pragma once

#include <map>

#include "Batch.h"

struct GLVertexArray;
class RenderContext;
class Node;
class Shader;

class BatchRegistry final {
public:
    BatchRegistry(const Assets& assets);

    inline Batch* getBatch(int vao);

    void prepare();

    void prepareVAO(
        GLVertexArray& vao,
        bool singleMaterial);

    void draw(
        const RenderContext& ctx,
        Node& node,
        Shader* shader);

    void flush(
        const RenderContext& ctx,
        bool release = true);

public:
    bool m_useObjectIDBuffer = false;
    bool m_highlight = false;

private:
    const Assets& m_assets;

    // { vaoId: batch, ... }
    std::map<int, Batch> m_batches;

    Batch* m_current{ nullptr };
};
