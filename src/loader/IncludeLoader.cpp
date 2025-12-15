#include "IncludeLoader.h"

#include <string>
#include <vector>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/glm_format.h"

#include "asset/Assets.h"

#include "Context.h"
#include "Loaders.h"
#include "NodeTypeData.h"
#include "NodeData.h"
#include "CompositeData.h"
#include "DecalData.h"
#include "ScriptData.h"
#include "MetaData.h"
#include "SceneData.h"
#include "MaterialUpdaterData.h"

#include "loader/converter/YamlConverter.h"
#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    IncludeLoader::IncludeLoader(
        const std::shared_ptr<Context>& ctx)
        : BaseLoader(ctx)
    {
    }

    void IncludeLoader::loadIncludes(
        const loader::DocNode& node,
        SceneData& sceneData,
        Loaders& loaders) const
    {
        if (node.isNull()) return;

        for (const auto& entry : node.getNodes()) {
            loadInclude(
                entry,
                sceneData,
                loaders);
        }
    }

    void IncludeLoader::loadInclude(
        const loader::DocNode& node,
        SceneData& sceneData,
        Loaders& loaders) const
    {
        if (node.isNull()) return;

        std::string path;
        bool pathRead = false;
        bool disabled = false;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "path") {
                path = readString(v);
                pathRead = true;
                break;
            }
            else if (k == "xpath") {
                path = readString(v);
                pathRead = true;
                disabled = true;
                return;
            }
        }

        if (path.empty()) {
            path = readString(node);
        }

        if (path.empty()) return;

        {
            std::filesystem::path filePath{ path };
            if (filePath.extension().empty()) {
                path += ".yml";
            }
        }

        const auto& fullPath = util::joinPath(m_ctx->m_dirName, path);

        if (disabled) {
            KI_INFO_OUT(fmt::format("SKIP: include={}", fullPath));
            return;
        }

        KI_INFO_OUT(fmt::format("LOAD: include={}", fullPath));

        {
            auto* includeDoc = sceneData.findInclude(fullPath);
            if (includeDoc)
            {
                throw fmt::format("INVALID: recursive_include- path={}", fullPath);
            }
        }

        if (!util::fileExists(fullPath))
        {
            throw fmt::format("INVALID: include_missing - path={}", fullPath);
        }

        loader::YamlConverter converter;
        auto doc = converter.load(fullPath);

        auto fileIndex = sceneData.m_includeFiles.size();
        sceneData.m_includeFiles.emplace_back(fullPath, loader::DocNode::getNull());

        loadScene(
            doc,
            sceneData,
            loaders);

        sceneData.m_includeFiles[fileIndex].second = doc;
    }

    void IncludeLoader::loadScene(
        const loader::DocNode& node,
        SceneData& sceneData,
        Loaders& loaders) const
    {
        auto& l = loaders;

        loadMeta(node.findNode("meta"), *sceneData.m_meta);

        l.m_includeLoader.loadIncludes(node.findNode("includes"), sceneData, loaders);

        l.m_skyboxLoader.loadSkybox(node.findNode("skybox"), *sceneData.m_skybox);

        l.m_rootLoader.loadRoot(node.findNode("root"), *sceneData.m_root);
        l.m_scriptLoader.loadScriptSystem(node.findNode("script"), *sceneData.m_scriptSystemData);
        l.m_materialUpdaterLoader.loadMaterialUpdaters(
            node.findNode("material_updaters"),
            sceneData.m_materialUpdaters,
            loaders);

        l.m_nodeTypeLoader.loadNodeTypes(
            node.findNode("types"),
            sceneData,
            sceneData.m_nodeTypes,
            l);

        l.m_compositeLoader.loadComposites(
            node.findNode("composites"),
            sceneData.m_composites,
            l);

        l.m_particleLoader.loadParticles(
            node.findNode("particles"),
            sceneData.m_particles,
            l);

        l.m_nodeLoader.loadNodes(
            node.findNode("nodes"),
            sceneData.m_nodes,
            l);

        l.m_decalLoader.loadDecals(
            node.findNode("decals"),
            sceneData.m_decals,
            l);
    }

    void IncludeLoader::loadMeta(
        const loader::DocNode& node,
        MetaData& data) const
    {
        if (node.isNull()) return;

        data.name = "<noname>";
        //data.modelsDir = assets.modelsDir;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "name") {
                data.name = "";// readString(v);
            }
            //else if (k == "assetsDir") {
            //    data.assetsDir = readString(v);
            //}
            //else if (k == "modelsDir") {
            //    data.modelsDir = readString(v);
            //}
            else {
                reportUnknown("meta_entry", k, v);
            }
        }
    }

}
