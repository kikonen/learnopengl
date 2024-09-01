#pragma once

#include "NodeGenerator.h"

#include "mesh/MeshTransform.h"

struct PrepareContext;

class AsteroidBeltGenerator final : public NodeGenerator
{
public:
    AsteroidBeltGenerator(int asteroidCount);

    virtual void prepareWT(
        const PrepareContext& ctx,
        Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        const Node& container) override;

private:
    void updateAsteroids(
        const UpdateContext& ctx,
        const Node& container,
        bool rotate);

    void createAsteroids(
        const PrepareContext& ctx,
        const Node& container);

    void initAsteroids(
        const PrepareContext& ctx,
        const Node& container,
        std::vector<mesh::MeshTransform>& transforms);

    void rotateAsteroids(
        const UpdateContext& ctx,
        const Node& container);

    virtual void bindBatch(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        render::Batch& batch,
        const Node& container,
        const Snapshot& snapshot) override;

private:
    std::vector<mesh::MeshTransform> m_transforms;

    const int m_asteroidCount;
    const float m_radius;
    const float m_offset;
    const int m_updateStep;

    int m_updateIndex = 0;
};

