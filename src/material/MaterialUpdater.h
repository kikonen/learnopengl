#pragma once

#include <string>
#include <memory>

#include "ki/size.h"
#include "ki/sid.h"

#include "kigl/kigl.h"

#include "TextureType.h"

class RenderContext;
struct PrepareContext;
struct Material;

class MaterialUpdater {
public:
    MaterialUpdater(
        ki::StringID id,
        const std::string& name);

    ~MaterialUpdater();

    virtual void prepareRT(
        const PrepareContext& ctx);

    virtual void render(
        const RenderContext& ctx);

    void setNeedUpdate(bool needUpdate)
    {
        m_needUpdate = needUpdate;
    }

    bool isBeedUpdate() const noexcept
    {
        return m_needUpdate;
    }

    void setDirty(bool dirty)
    {
        m_dirty = dirty;
    }

    bool isDirty() const noexcept
    {
        return m_dirty;
    }

    void setMaterial(const Material* src) noexcept;

    virtual GLuint64 getTexHandle(TextureType type) const noexcept
    {
        return 0;
    }

public:
    ki::StringID m_id{ 0 };

    std::unique_ptr<Material> m_material;
    std::vector<ki::material_id> m_dependentMaterials;

protected:
    bool m_prepared{ false };
    bool m_dirty{ true };
    bool m_needUpdate{ true };

    std::string m_name;
};
