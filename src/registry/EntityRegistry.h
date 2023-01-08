#pragma once

#include <bitset>
#include <vector>

#include "kigl/GLBuffer.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "EntitySSBO.h"

class RenderContext;

//
// Manage SSBO buffer for registered entity instances
// - allow binding all matrices at *once*, and update them
//   once per frame instead of pushing them multiple times into dynamic VBOs
//
class EntityRegistry {
public:
    EntityRegistry(const Assets& assets);

    void prepare();
    void update(const RenderContext& ctx);
    void bind(const RenderContext& ctx);

    // const EntitySSBO& entry
    int add();
    EntitySSBO* get(int index);

    void markDirty(int index);

private:
    const Assets& m_assets;

    std::vector<EntitySSBO> m_entries;
    std::vector<bool> m_dirty;
    int m_minDirty = -1;
    int m_maxDirty = -1;

    GLBuffer m_ssbo{ "entitySSBO" };
};
