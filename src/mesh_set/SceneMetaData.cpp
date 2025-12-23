#include "SceneMetaData.h"

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

#include "util/assimp_util.h"

namespace mesh_set
{
    SceneMetaData::SceneMetaData()
    {
    }

    void SceneMetaData::dumpMetaData(
        const aiScene* scene)
    {
        dumpMetaData(scene, scene->mRootNode, 0);
    }

    void SceneMetaData::dumpMetaData(
        const aiScene* scene,
        const aiNode* node,
        int level)
    {
        const std::string nodeName{assimp_util::normalizeName(node->mName) };
        const aiMetadata* meta = node->mMetaData;
        if (!meta) return;

        for (size_t i = 0; i < meta->mNumProperties; i++) {
            const auto& key = meta->mKeys[i];
            const auto& value = meta->mValues[i];

            std::string formattedKey{ key.C_Str() };
            std::string formattedValue;

            bool boolValue;
            int32_t intValue;
            //uint32_t uintValue;
            //int64_t longValue;
            //uint64_t ulongValue;
            float floatValue;
            double doubleValue;
            aiString stringValue;

            switch (value.mType) {
            case AI_BOOL:
                meta->Get(key, boolValue);
                formattedValue = fmt::format("{}", boolValue);
                break;
            case AI_INT32:
                meta->Get(key, intValue);
                formattedValue = fmt::format("{}", intValue);
                break;
                //case AI_UINT64:
                //    meta->Get(key, ulongValue);
                //    formattedValue = fmt::format("{}", ulongValue);
                //    break;
            case AI_FLOAT:
                meta->Get(key, floatValue);
                formattedValue = fmt::format("{}", floatValue);
                break;
            case AI_DOUBLE:
                meta->Get(key, doubleValue);
                formattedValue = fmt::format("{}", doubleValue);
                break;
            case AI_AISTRING:
                meta->Get(key, stringValue);
                formattedValue = fmt::format("{}", stringValue.C_Str());
                break;
            case AI_AIVECTOR3D:
                break;
                //case AI_AIMETADATA:
                //    break;
                //case AI_INT64:
                //    meta->Get(key, longValue);
                //    formattedValue = fmt::format("{}", longValue);
                //    break;
                //case AI_UINT32:
                //    meta->Get(key, uintValue);
                //    formattedValue = fmt::format("{}", uintValue);
                //    break;
            default:
                formattedValue = "<unknown>";
                break;
            }

            KI_INFO_OUT(fmt::format(
                "ASSIMP: META level={}, node={}, key={}, value={}",
                level,
                nodeName,
                formattedKey,
                formattedValue));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            dumpMetaData(scene, node->mChildren[i], level + 1);
        }
    }
}
