#include "AssimpMaterialLoader.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <set>

#include <glm/gtx/matrix_decompose.hpp>
#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "asset/Assets.h"

#include "util/glm_format.h"
#include "util/glm_util.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"
#include "util/Transform.h"

#include "animation/RigContainer.h"
#include "animation/RigNode.h"
#include "animation/Animation.h"
#include "animation/RigNodeChannel.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"
#include "animation/MeshInfo.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "material/InlineTexture.h"
#include "material/Image.h"

#include "util/assimp_util.h"

#include "AnimationLoader.h"
#include "LoadContext.h"
#include "RigNodeTreeGenerator.h"

namespace {
    struct TextureMapping
    {
        std::string name;
        aiTextureType asssimpType;
        TextureType type;
        bool gammaCorrect;
        bool checkAlpha{ false };
    };

    // Structure to hold PBR material properties
    struct PBRMaterialInfo
    {
        // Base color / Albedo
        glm::vec4 baseColorFactor;      // RGBA multiplier
        int baseColorTexture;          // Texture index (-1 if none)

        // Metallic-Roughness
        float metallicFactor;          // Metallic multiplier (0-1)
        float roughnessFactor;         // Roughness multiplier (0-1)
        int metallicRoughnessTexture;  // Combined texture index (-1 if none)

        // In glTF, metallic-roughness is a SINGLE texture with:
        // - Blue channel (B) = Metalness
        // - Green channel (G) = Roughness
        // - Red channel (R) = unused (sometimes occlusion)

        // Normal mapping
        int normalTexture;             // Normal map index (-1 if none)
        float normalScale;             // Normal map strength

        // Occlusion
        int occlusionTexture;          // AO texture index (-1 if none)
        float occlusionStrength;       // AO strength

        // Emissive
        glm::vec3 emissiveFactor;       // RGB emissive color
        int emissiveTexture;           // Emissive texture index (-1 if none)

        std::string workflow;          // "metallic-roughness" or "specular-glossiness"
    };

    enum class AlphaMode
    {
        opaque,
        mask,
        blend
    };

    struct MaterialRenderInfo
    {
        bool doubleSided;        // Should disable backface culling
        bool hasTransparency;    // Has alpha blending
        float opacity;           // Opacity value
        AlphaMode alphaMode;     // 0=opaque, 1=mask, 2=blend
        float alphaCutoff;       // For alpha masking
        std::string name;
    };
}

namespace
{
    //aiTextureType_OPACITY = 8,
    const std::vector<TextureMapping> TEXTURE_MAPPING = {
        {
            "diffuse",
            aiTextureType_DIFFUSE,
            TextureType::diffuse,
            true,
            true,
        },
        {
            "normal",
            aiTextureType_NORMALS,
            TextureType::map_normal,
            false,
            false,
        },
        {
            "emission",
            aiTextureType_EMISSIVE,
            TextureType::emission,
            true,
            true,
        },
        {
            "displacement",
            aiTextureType_DISPLACEMENT,
            TextureType::map_displacement,
            false,
            false,
        },
        {
            "metal",
            aiTextureType_METALNESS,
            TextureType::map_mrao,
            false,
            false,
        },
        // SAME as aiTextureType_DIFFUSE_METALNESS
        //{
        //    "roughness",
        //    aiTextureType_DIFFUSE_ROUGHNESS,
        //    TextureType::map_mrao,
        //    false,
        //},
        {
            "ambient_occlusion",
            aiTextureType_AMBIENT_OCCLUSION,
            TextureType::map_mrao,
            false,
            false,
        },
    };

    std::mutex m_lock;

    const char EMBEDDED_NAME_PREFIX = '*';
}

namespace
{
    // Extract PBR material properties from glTF/GLB material
    PBRMaterialInfo getPBRMaterialInfo(const aiMaterial* material)
    {
        PBRMaterialInfo info;

        // Initialize defaults
        info.baseColorFactor[0] = 1.0f;
        info.baseColorFactor[1] = 1.0f;
        info.baseColorFactor[2] = 1.0f;
        info.baseColorFactor[3] = 1.0f;
        info.metallicFactor = 1.0f;
        info.roughnessFactor = 1.0f;
        info.normalScale = 1.0f;
        info.occlusionStrength = 1.0f;
        info.emissiveFactor[0] = 0.0f;
        info.emissiveFactor[1] = 0.0f;
        info.emissiveFactor[2] = 0.0f;
        info.baseColorTexture = -1;
        info.metallicRoughnessTexture = -1;
        info.normalTexture = -1;
        info.occlusionTexture = -1;
        info.emissiveTexture = -1;
        info.workflow = "metallic-roughness";

        // Get base color factor
        aiColor4D baseColor;
        if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS) {
            info.baseColorFactor[0] = baseColor.r;
            info.baseColorFactor[1] = baseColor.g;
            info.baseColorFactor[2] = baseColor.b;
            info.baseColorFactor[3] = baseColor.a;
        }
        else {
            // Fallback to diffuse color
            aiColor4D diffuse;
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
                info.baseColorFactor[0] = diffuse.r;
                info.baseColorFactor[1] = diffuse.g;
                info.baseColorFactor[2] = diffuse.b;
                info.baseColorFactor[3] = diffuse.a;
            }
        }

        // Get metallic factor
        float metallic = 1.0f;
        if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
            info.metallicFactor = metallic;
        }

        // Get roughness factor
        float roughness = 1.0f;
        if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
            info.roughnessFactor = roughness;
        }

        // Get emissive factor
        aiColor3D emissive;
        if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
            info.emissiveFactor[0] = emissive.r;
            info.emissiveFactor[1] = emissive.g;
            info.emissiveFactor[2] = emissive.b;
        }

        // Get base color texture
        if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ||
            material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString path;
            aiTextureType texType = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0
                ? aiTextureType_BASE_COLOR
                : aiTextureType_DIFFUSE;
            if (material->GetTexture(texType, 0, &path) == AI_SUCCESS) {
                if (path.data[0] == '*') {
                    info.baseColorTexture = atoi(path.C_Str() + 1);
                }
            }
        }

        // Get metallic-roughness texture (CRITICAL: This is a combined texture!)
        if (material->GetTextureCount(aiTextureType_METALNESS) > 0) {
            aiString path;
            if (material->GetTexture(aiTextureType_METALNESS, 0, &path) == AI_SUCCESS) {
                if (path.data[0] == '*') {
                    info.metallicRoughnessTexture = atoi(path.C_Str() + 1);
                }
            }
        }
        // Alternative: Check for AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE

        // Get normal texture
        if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
            aiString path;
            if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS) {
                if (path.data[0] == '*') {
                    info.normalTexture = atoi(path.C_Str() + 1);
                }
            }

            //// Get normal scale
            //material->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0),
            //    info.normalScale);
        }

        // Get occlusion texture
        if (material->GetTextureCount(aiTextureType_LIGHTMAP) > 0 ||
            material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0) {
            aiString path;
            aiTextureType aoType = material->GetTextureCount(aiTextureType_LIGHTMAP) > 0
                ? aiTextureType_LIGHTMAP
                : aiTextureType_AMBIENT_OCCLUSION;
            if (material->GetTexture(aoType, 0, &path) == AI_SUCCESS) {
                if (path.data[0] == '*') {
                    info.occlusionTexture = atoi(path.C_Str() + 1);
                }
            }

            //// Get occlusion strength
            //material->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0),
            //    info.occlusionStrength);
        }

        // CRITICAL: Check if occlusion is packed with metallic-roughness
        // In glTF, occlusion CAN be in the same texture as metallic-roughness
        // This is indicated when both textures point to the same index
        if (info.occlusionTexture >= 0 && info.metallicRoughnessTexture >= 0) {
            if (info.occlusionTexture == info.metallicRoughnessTexture) {
                // ORM texture: Occlusion + Roughness + Metallic in one texture
                // R = Occlusion, G = Roughness, B = Metallic
                info.workflow = "ORM-packed"; // Indicates combined texture
            }
        }
        else if (info.occlusionTexture < 0 && info.metallicRoughnessTexture >= 0) {
            // No separate occlusion texture specified
            // Check if metallic-roughness texture might still contain occlusion in R channel
            // This is common but not always documented in the material
            info.workflow = "MR-possibly-ORM"; // Might have occlusion in red channel
        }

        // Get emissive texture
        if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
            aiString path;
            if (material->GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS) {
                if (path.data[0] == '*') {
                    info.emissiveTexture = atoi(path.C_Str() + 1);
                }
            }
        }

        return info;
    }

    // Extract material rendering properties
    MaterialRenderInfo getMaterialRenderInfo(const aiMaterial* material)
    {
        MaterialRenderInfo info;
        info.doubleSided = false;
        info.hasTransparency = false;
        info.opacity = 1.0f;
        info.alphaMode = AlphaMode::opaque; // opaque
        info.alphaCutoff = 0.5f;

        // Get material name
        aiString matName;
        if (material->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
            info.name = matName.C_Str();
        }

        // Check for double-sided flag (most important for backface culling)
        int twoSided = 0;
        if (material->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
            info.doubleSided = (twoSided != 0);
        }

        // Check opacity
        float opacity = 1.0f;
        if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
            info.opacity = opacity;
            info.hasTransparency = (opacity < 1.0f);
        }

        // IMPORTANT: glTF/GLB does NOT use separate opacity maps!
        // Instead, alpha/opacity comes from the Base Color texture's alpha channel
        // Check for gltf-specific alpha mode property
        // glTF uses "$mat.gltf.alphaMode" property
        aiString alphaMode;
        if (material->Get("$mat.gltf.alphaMode", 0, 0, alphaMode) == AI_SUCCESS) {
            std::string alphaModeStr = alphaMode.C_Str();
            if (alphaModeStr == "OPAQUE") {
                info.alphaMode = AlphaMode::opaque;
                // Base color alpha is ignored
            }
            else if (alphaModeStr == "MASK") {
                info.alphaMode = AlphaMode::mask;
                info.hasTransparency = true;
                // Alpha values below cutoff = fully transparent, above = fully opaque
                material->Get("$mat.gltf.alphaCutoff", 0, 0, info.alphaCutoff);
            }
            else if (alphaModeStr == "BLEND") {
                info.alphaMode = AlphaMode::blend;
                info.hasTransparency = true;
                // Alpha values cause proper blending
            }
        }

        // Alternative: Check shading model for transparency hints
        int shadingModel = 0;
        if (material->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == AI_SUCCESS) {
            // Some shading models imply transparency
            if (shadingModel == aiShadingMode_Blinn ||
                shadingModel == aiShadingMode_Phong) {
                // These can have transparency, check color alpha
            }
        }

        return info;
    }
}

namespace
{
    bool isEmbeddedTexture(const std::string name)
    {
        return name[0] == EMBEDDED_NAME_PREFIX;
    }

    bool saveEmbeddedTextureToFile(
        const std::vector<unsigned char> data,
        const std::string& outputPath)
    {
        std::ofstream file;
        file.open(outputPath.c_str());

        file.write((char*)data.data(), data.size());
        file.close();

        KI_INFO_OUT(fmt::format("WROTE: {}", outputPath));

        return true;
    }

    std::shared_ptr<InlineTexture> loadEmbeddedTexture(
        const std::string& name,
        const aiTexture* texture,
        TextureMapping texInfo,
        TextureSpec spec)
    {
        std::vector<unsigned char> data;
        int width = 0;
        int height = 0;
        int channels = 0;
        bool is16Bbit = false;
        bool hasAlpha = false;

        if (texture->mHeight == 0) {
            std::string format = "png_default";

            if (texture->achFormatHint[0] != '\0') {
                format = std::string(texture->achFormatHint);
            }

            // Compressed texture (PNG, JPG, etc.)
            size_t encodeSize = texture->mWidth;

            std::vector<unsigned char> encodedData;
            encodedData.resize(encodeSize);
            memcpy(encodedData.data(), texture->pcData, encodeSize);

            saveEmbeddedTextureToFile(
                encodedData,
                fmt::format("tmp/{}.{}", name, format));

            //if (texture->achFormatHint[0] != '\0') {
            //    std::cout << "Format hint: " << texture->achFormatHint << std::endl;
            //}

            Image image{ true };
            int res = image.loadFromMememory(encodedData);
            if (res) {
                return {};
            }

            width = image.m_width;
            height = image.m_height;
            channels = image.m_channels;
            is16Bbit = image.m_is16Bbit;
            hasAlpha = texInfo.checkAlpha && image.m_hasAlpha;

            const size_t channelSize = is16Bbit ? 2 : 1;
            const size_t size = width * height * channels * channelSize;

            data.resize(size);
            memcpy(data.data(), image.m_data, size);
        }
        else {
            width = texture->mWidth;
            height = texture->mHeight;
            channels = 4;

            // Uncompressed texture (raw RGBA data)
            const size_t size = width * height * channels;
            data.resize(size);

            // Convert aiTexel to raw RGBA bytes
            for (size_t i = 0; i < width * height; ++i)
            {
                data[i * 4 + 0] = texture->pcData[i].r;
                data[i * 4 + 1] = texture->pcData[i].g;
                data[i * 4 + 2] = texture->pcData[i].b;
                data[i * 4 + 3] = texture->pcData[i].a;
            }
        }

        return std::make_shared<InlineTexture>(
            name,
            data,
            width,
            height,
            channels,
            is16Bbit,
            hasAlpha,
            texInfo.gammaCorrect,
            spec);
    }

    std::shared_ptr<InlineTexture> loadInlineTexture(
        const std::string& meshSetName,
        const aiScene* scene,
        TextureMapping texInfo,
        TextureSpec spec,
        const aiMaterial* material,
        const std::string& path
    )
    {
        if (!isEmbeddedTexture(path)) return {};

        const auto materialName = const_cast<aiMaterial*>(material)->GetName().C_Str();
        unsigned int texCount = material->GetTextureCount(texInfo.asssimpType);

        for (unsigned int i = 0; i < texCount; i++) {
            int texIndex = atoi(path.c_str() + 1);
            const auto& texName = fmt::format(
                "inline-{}-{}-{}-{}",
                meshSetName, materialName, texInfo.name, texIndex);

            KI_INFO_OUT(fmt::format(
                "ASSIMP: TEXTURE mesh_set={}, material={}, name={}, index={}/{}",
                meshSetName,
                materialName,
                texName,
                i + 1,
                texCount));

            const aiTexture* tex = scene->mTextures[texIndex];
            return loadEmbeddedTexture(
                texName,
                tex,
                texInfo,
                spec);
        }
        //std::shared_ptr<InlineTexture> texture;
        //return texture;
        return {};
    }
}

namespace mesh_set
{
    AssimpMaterialLoader::AssimpMaterialLoader(
        const bool debug)
        : m_debug{ debug}
    {
    }

    AssimpMaterialLoader::~AssimpMaterialLoader()
    {
    }

    void AssimpMaterialLoader::processMaterials(
        const mesh::MeshSet& meshSet,
        std::vector<Material>& materials,
        std::map<size_t, size_t>& materialMapping,
        const aiScene* scene)
    {
        for (size_t n = 0; n < scene->mNumMaterials; ++n)
        {
            auto material = processMaterial(meshSet, scene, scene->mMaterials[n]);
            materials.push_back(material);
            materialMapping.insert({ n, materials.size() - 1 });
        }
    }

    Material AssimpMaterialLoader::processMaterial(
        const mesh::MeshSet& meshSet,
        const aiScene* scene,
        const aiMaterial* src)
    {
        const auto name = const_cast<aiMaterial*>(src)->GetName().C_Str();

        if (m_debug) {
            KI_INFO_OUT(fmt::format("ASSIMP: MATERIAL mesh_set={}, name={}, properties={}, allocated={}",
                meshSet.m_name,
                name,
                src->mNumProperties,
                src->mNumAllocated));
        }

        Material material;
        material.m_modelDir = meshSet.m_dir;
        material.m_name = name;

        PBRMaterialInfo materialInfo = getPBRMaterialInfo(src);
        MaterialRenderInfo renderInfo = getMaterialRenderInfo(src);

        {
            material.kd = materialInfo.baseColorFactor;
            if (renderInfo.hasTransparency)
            {
                material.kd.a = renderInfo.opacity;
            }
            material.ke = glm::vec4{ materialInfo.emissiveFactor, 0.f };
            material.m_metalnessFactor = materialInfo.metallicFactor;
            material.m_roughnessFactor = materialInfo.roughnessFactor;

            material.renderBack = renderInfo.doubleSided;
        }

        for (const auto& texInfo : TEXTURE_MAPPING)
        {
            int texIndex = 0;

            aiString assimpPath;
            aiTextureMapping mapping;
            unsigned int uvIndex;
            ai_real blend;
            aiTextureOp op;
            aiTextureMapMode wrapModeU;

            if (src->GetTexture(
                texInfo.asssimpType,
                texIndex,
                &assimpPath,
                &mapping,
                &uvIndex,
                &blend,
                &op,
                &wrapModeU) == AI_SUCCESS)
            {
                std::string path{ assimpPath.C_Str() };

                if (isEmbeddedTexture(path))
                {
                    TextureSpec spec;

                    switch (wrapModeU) {
                    case aiTextureMapMode_Wrap:
                        spec.wrapS = GL_REPEAT;
                        spec.wrapS = GL_REPEAT;
                        break;
                    case aiTextureMapMode_Clamp:
                        spec.wrapS = GL_CLAMP_TO_EDGE;
                        spec.wrapS = GL_CLAMP_TO_EDGE;
                        break;
                    case aiTextureMapMode_Mirror:
                        spec.wrapS = GL_MIRRORED_REPEAT;
                        spec.wrapS = GL_MIRRORED_REPEAT;
                        break;
                    case aiTextureMapMode_Decal:
                        spec.wrapS = GL_CLAMP_TO_BORDER;
                        spec.wrapS = GL_CLAMP_TO_BORDER;
                        break;
                    }

                    auto tex = loadInlineTexture(
                        meshSet.m_name,
                        scene,
                        texInfo,
                        spec,
                        src,
                        path);

                    if (texInfo.checkAlpha) {
                        material.alpha = tex->hasAlpha();
                        if (material.alpha) {
                            float alphaCutoff = 0.5f;

                            // Check for gltf-specific alpha mode property
                            // glTF uses "$mat.gltf.alphaMode" property
                            aiString alphaMode;
                            if (src->Get("$mat.gltf.alphaMode", 0, 0, alphaMode) == AI_SUCCESS) {
                                std::string alphaModeStr{ alphaMode.C_Str() };
                                if (alphaModeStr == "OPAQUE") {
                                    material.alpha = false;
                                }
                                else if (alphaModeStr == "MASK") {
                                    // Get alpha cutoff value for masking
                                    src->Get("$mat.gltf.alphaCutoff", 0, 0, alphaCutoff);
                                }
                                else if (alphaModeStr == "BLEND") {
                                    material.blend = true;
                                }
                            }
                        }
                    }

                    material.addinlineTexture(
                        texInfo.type,
                        tex);
                }
                else {
                    material.addTexture(
                        texInfo.type,
                        findTexturePath(meshSet, path),
                        true);
                }
            }
        }

        return material;
    }

    std::string AssimpMaterialLoader::findTexturePath(
        const mesh::MeshSet& meshSet,
        const std::string& origPath)
    {
        const auto& rootDir = meshSet.m_rootDir;
        const auto& meshName = meshSet.m_name;

        std::string assetPath = origPath;
        std::filesystem::path meshPath{ meshName };
        const auto parentPath = meshPath.parent_path();

        std::filesystem::path fsPath{ assetPath };
        //std::string assetPath2 = std::filesystem::weakly_canonical(fsPath).string();

        std::string filePath = util::joinPathExt(
            rootDir,
            parentPath.string(),
            assetPath, "");

        if (util::fileExists(filePath)) {
            //assetPath = util::joinPath(
            //    parentPath.filename().string(),
            //    assetPath);
            assetPath = filePath.substr(
                rootDir.length() + 1,
                filePath.length() - rootDir.length() - 1);
        }

        if (m_debug) {
            KI_INFO_OUT(fmt::format("ASSIMP: TEX mesh_set={}, path={}, was={}",
                meshSet.m_name,
                assetPath,
                origPath));
        }

        return assetPath;
    }
}
