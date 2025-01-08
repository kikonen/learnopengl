#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "ki/size.h"
#include "ki/sid.h"

#include "render/TextureQuad.h"

#include "material/MaterialUpdater.h"

struct Material;

class FontAtlasMaterialUpdater : public MaterialUpdater
{
public:
    FontAtlasMaterialUpdater(
        ki::StringID id,
        const std::string& name);

    ~FontAtlasMaterialUpdater();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    virtual GLuint64 getTexHandle(TextureType type) const noexcept override;

private:
    //GLuint64 m_handle{ 0 };
    //GLuint m_samplerId{ 0 };
};
