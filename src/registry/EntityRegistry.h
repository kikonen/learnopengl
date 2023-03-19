#pragma once

#include <bitset>
#include <vector>

#include "kigl/GLBuffer.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "EntitySSBO.h"

class UpdateContext;
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
    void update(const UpdateContext& ctx);
    void bind(const RenderContext& ctx);

    // index of entity
    int addEntity();

    // @return first index of range
    int addEntityRange(const size_t count);

    EntitySSBO* getEntity(int index);
    EntitySSBO* updateEntity(int index, bool dirty);

    void markDirty(int index);

private:
    void processNodes(const UpdateContext& ctx);

private:
    const Assets& m_assets;

    std::vector<EntitySSBO> m_entries;
    std::vector<bool> m_dirty;
    int m_minDirty = -1;
    int m_maxDirty = -1;

    GLBuffer m_ssbo{ "entitySSBO" };
};
