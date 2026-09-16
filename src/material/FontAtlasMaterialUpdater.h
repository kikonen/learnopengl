#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

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

    virtual GLuint getTexLayer(material::TextureType type) const noexcept override;

private:
    GLuint m_layer{ 0 };

    int m_fontId{ -1 };
};
