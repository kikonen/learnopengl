#pragma once

#include "NodeGenerator.h"

//
// Instance node entities based into grid
//
class GridGenerator final : public NodeGenerator {
public:
    GridGenerator();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& container) override;

    virtual void update(
        const RenderContext& ctx,
        Node& container,
        Node* containerParent) override;

private:
    void updateInstances(
        const RenderContext& ctx,
        Node& container,
        Node* containerParent);

    void prepareInstances(
        const Assets& assets,
        Registry* registry,
        Node& container);

public:
    int m_xCount{ 1 };
    int m_yCount{ 1 };
    int m_zCount{ 1 };

    double m_xStep{ 0 };
    double m_yStep{ 0 };
    double m_zStep{ 0 };
};
