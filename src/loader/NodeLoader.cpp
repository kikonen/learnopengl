#include "NodeLoader.h"

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

#include "loader/converter/YamlConverter.h"
#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    NodeLoader::NodeLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void NodeLoader::loadNodes(
        const loader::DocNode& node,
        std::vector<NodeRoot>& nodes,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& nodeRoot = nodes.emplace_back();
            loadNode(
                entry,
                nodeRoot,
                loaders);
        }
    }

    void NodeLoader::loadNode(
        const loader::DocNode& node,
        NodeRoot& nodeRoot,
        Loaders& loaders) const
    {
        loadNodeClone(
            node,
            nodeRoot.base,
            nodeRoot.clones,
            true,
            loaders);
    }

    void NodeLoader::loadNodeClone(
        const loader::DocNode& node,
        NodeData& data,
        std::vector<NodeData>& clones,
        bool recurse,
        Loaders& loaders) const
    {
        bool hasClones = false;

        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "prefab") {
                // NOTE KI loaded as "pre step"
            }
            else if (k == "type") {
                data.typeId = readId(v);
            }
            else if (k == "xtype") {
                data.enabled = false;
                data.typeId = readId(v);
            }
            else if (k == "xid") {
                data.baseId = readId(v);
                data.enabled = false;
            }
            else if (k == "id") {
                data.baseId = readId(v);
            }
            else if (k == "parent_id" || k == "parent") {
                data.parentBaseId = readId(v);
            }
            else if (k == "ignored_by") {
                data.ignoredByBaseId = readId(v);
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "desc") {
                data.desc = readString(v);
            }
            else if (k == "active") {
                data.active = readBool(v);
            }
            else if (k == "position" || k == "pos") {
                data.position = readVec3(v);
            }
            else if (k == "rot" || k == "rotation") {
                data.rotation = readDegreesRotation(v);
            }
            else if (k == "scale") {
                data.scale = readScale3(v);
            }
            else if (k == "repeat") {
                loadRepeat(v, data.repeat);
            }
            else if (k == "tiling") {
                loadTiling(v, data.tiling);
            }
            else if (k == "generator") {
                loaders.m_generatorLoader.loadGenerator(v, data.generator, loaders);
            }
            else if (k == "selected") {
                data.selected = readBool(v);
            }
            else if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "clone_position_offset") {
                data.clonePositionOffset = readVec3(v);
            }
            else if (k == "tile") {
                data.tile = readVec3(v);
            }
            else if (k == "clones") {
                if (recurse)
                    hasClones = true;
            }
            else {
                reportUnknown("node_entry", k, v);
            }
        }

        if (hasClones) {
            for (const auto& pair : node.getNodes()) {
                const std::string& k = pair.getName();
                const loader::DocNode& v = pair.getNode();

                if (k == "clones") {
                    for (const auto& node : v.getNodes()) {
                        // NOTE KI intialize with current data
                        NodeData clone = data;
                        std::vector<NodeData> dummy{};
                        loadNodeClone(
                            node,
                            clone,
                            dummy,
                            false,
                            loaders);
                        clones.push_back(clone);
                    }
                }
            }
        }
    }
}
