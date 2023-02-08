#pragma once

#include "InstancedController.h"

//
// Instance node entities based into grid
//
class GridController final : public NodeController {
public:
    GridController();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

private:
    void createInstances(
        const Assets& assets,
        Registry* registry,
        Node& node);

    void updateInstances(
        const RenderContext& ctx,
        Node& node);

public:
    int m_xCount{ 1 };
    int m_yCount{ 1 };
    int m_zCount{ 1 };

    double m_xStep{ 0 };
    double m_yStep{ 0 };
    double m_zStep{ 0 };

private:
    int m_matrixLevel = -1;
};
