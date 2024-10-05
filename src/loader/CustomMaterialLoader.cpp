#include "CustomMaterialLoader.h"

#include "util/Util.h"

#include "mesh/MeshFlags.h"

#include "material/FrameBufferMaterial.h"

#include "loader/document.h"
#include "loader_util.h"
#include "loader/Loaders.h"

namespace loader {
    CustomMaterialLoader::CustomMaterialLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void CustomMaterialLoader::loadCustomMaterial(
        const loader::DocNode& node,
        CustomMaterialData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = CustomMaterialType::none;
                }
                else if (type == "skybox") {
                    data.type = CustomMaterialType::skybox;
                }
                else {
                    reportUnknown("custom_material_type", k, v);
                }
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else {
                reportUnknown("custom_material_entry", k, v);
            }
        }
    }

    std::unique_ptr<CustomMaterial> CustomMaterialLoader::createCustomMaterial(
        const CustomMaterialData& data,
        Loaders& loaders)
    {
        //switch (data.type) {
        //case CustomMaterialType::framebuffer: {
        //    auto cm = std::make_unique<FrameBufferMaterial>("fbo", false);

        //    cm->setMaterial(&data.materialData.material);
        //    cm->m_material->loadTextures();

        //    loaders.m_materialLoader.resolveMaterial({}, *cm->m_material);

        //    return cm;
        //}
        //}

        return nullptr;
    }
}
