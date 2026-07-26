#pragma once

#include "MeshRenderer.h"

class PhysicsRenderer : public MeshRenderer
{
public:
    virtual void prepareRT(const PrepareContext& ctx) override;

protected:
    void updateImpl(
        const UpdateContext& ctx) override;
};
