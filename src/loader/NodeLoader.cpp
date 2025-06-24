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

#include "NodeData.h"

namespace loader {
    NodeLoader::NodeLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void NodeLoader::loadNodes(
        const loader::DocNode& node,
        std::vector<NodeData>& nodes,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& nodeData = nodes.emplace_back();
            loadNode(
                entry,
                nodeData,
                loaders);
        }
    }

    void NodeLoader::loadNode(
        const loader::DocNode& node,
        NodeData& nodeData,
        Loaders& loaders) const
    {
        loadNodeClone(
            node,
            nodeData,
            true,
            loaders);
    }

    void NodeLoader::loadNodeClone(
        const loader::DocNode& node,
        NodeData& data,
        bool recurse,
        Loaders& loaders) const
    {
        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "children") {
                // NOTE KI loaded as "post step"
            }
            else if (k == "clones") {
                // NOTE KI loaded as "post step"
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
                float tiling = readFractional(v);
                data.tilingX = tiling;
                data.tilingY = tiling;
            }
            else if (k == "tiling_x") {
                data.tilingX = readFractional(v);
            }
            else if (k == "tiling_y") {
                data.tilingY = readFractional(v);
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
            else {
                reportUnknown("node_entry", k, v);
            }
        }

        {
            const auto& childrenNode = node.findNode("children");
            if (!childrenNode.isNull()) {
                data.children = std::make_shared<std::vector<NodeData>>();

                for (const auto& entry : childrenNode.getNodes()) {
                    auto& child = data.children->emplace_back();
                    loadNode(
                        entry,
                        child,
                        loaders);
                }
            }
        }

        {
            const auto& clonesNode = node.findNode("clones");

            if (!clonesNode.isNull()) {
                data.clones = std::make_shared<std::vector<NodeData>>();

                for (const auto& node : clonesNode.getNodes()) {
                    // NOTE KI intialize with current data
                    NodeData clone = data;

                    loadNodeClone(
                        node,
                        clone,
                        false,
                        loaders);
                    data.clones->push_back(clone);
                }
            }
        }
    }
}
