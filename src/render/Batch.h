#pragma once

#include "asset/Assets.h"

#include "backend/gl/PerformanceCounters.h"

#include "BatchCommand.h"

namespace backend {
    class DrawBuffer;
}

class Program;
class RenderContext;
class MeshType;
class Node;
class MaterialVBO;
class Registry;
class EntityRegistry;


// NOTE KI use single shared UBO buffer for rendering
// => less resources needed
//
// https://stackoverflow.com/questions/15438605/can-a-vbo-be-bound-to-multiple-vaos
// https://www.khronos.org/opengl/wiki/Vertex_Specification#Index_buffers
//
class Batch final
{
public:
    Batch();
    ~Batch() = default;

    // https://stackoverflow.com/questions/7823845/disable-compiler-generated-copy-assignment-operator
    Batch(const Batch&) = delete;
    Batch& operator=(const Batch&) = delete;

    void add(
        const RenderContext& ctx,
        const int entityIndex) noexcept;

    void addAll(
        const RenderContext& ctx,
        const std::vector<int> entityIndeces) noexcept;

    void addInstanced(
        const RenderContext& ctx,
        int instancedEntityIndex,
        int firstEntityIndex,
        int count) noexcept;

    void bind() noexcept;

    void prepareView(
        const Assets& assets,
        Registry* registry,
        int entryCount = -1,
        int bufferCount = -1);

    void draw(
        const RenderContext& ctx,
        Node& node,
        Program* program);

    bool isFlushed() const noexcept
    {
        return m_entityIndeces.size() == 0;
    }

    void flush(
        const RenderContext& ctx);

    backend::gl::PerformanceCounters getCounters(bool clear);
    backend::gl::PerformanceCounters getCountersLocal(bool clear);

private:
    void addCommand(
        const RenderContext& ctx,
        const MeshType* type,
        Program* program) noexcept;

    bool inFrustum(
        const RenderContext& ctx,
        const int entityIndex) const noexcept;

private:
    bool m_prepared = false;

    bool m_frustumCPU = false;
    bool m_frustumGPU = false;

    std::vector<BatchCommand> m_batches;

    EntityRegistry* m_entityRegistry{ nullptr };

    std::vector<int> m_entityIndeces;

    std::unique_ptr<backend::DrawBuffer> m_draw;

    mutable unsigned long m_drawCount = 0;
    mutable unsigned long m_skipCount = 0;
};
