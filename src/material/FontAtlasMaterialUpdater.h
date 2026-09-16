#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "render/TextureQuad.h"

#include "material/MaterialUpdater.h"

struct Material;

class FontAtlasMaterialUpdater : public MaterialUpdater
{
public:
    FontAtlasMaterialUpdater(
        ki::material_updater_id id,
        const std::string& name);

    ~FontAtlasMaterialUpdater();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    virtual void render(
        const render::RenderContext& ctx) override;

    virtual uint16_t getTexLayer(material::TextureType type) const noexcept override;

private:
    uint16_t m_layer{ 0 };

    int m_fontId{ -1 };
};
