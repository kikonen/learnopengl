#include "loader_util.h"

#include <sstream>
#include <mutex>
#include <fstream>
#include <regex>
#include <filesystem>
#include <iomanip>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "pool/IdGenerator.h"
#include "pool/TypeHandle.h"

#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"

#include "ki/sid.h"
#include "ki/sid_format.h"

#include "loader/MaterialData.h"
#include "loader/document.h"

#include "TerrainTiling.h"
#include "NodeTypeData.h"

#include "Context.h"

namespace {
    IdGenerator<ki::type_id> ID_GENERATOR;

    const float DEF_ALPHA = 1.0;

    const std::string MACRO_STEP_CLONE{ "c" };
    const std::string MACRO_STEP_TILE{ "t" };
    const std::string MACRO_STEP_X{ "x" };
    const std::string MACRO_STEP_Y{ "y" };
    const std::string MACRO_STEP_Z{ "z" };
}

namespace loader {
    std::pair<std::string, bool> resolveIncludePath(
        const Context& ctx,
        const std::string& currentDir,
        const std::string& filePath)
    {
        if (filePath.empty()) return { filePath, false };

        std::string path = filePath;
        {
            std::filesystem::path filePath{ path };
            if (filePath.extension().empty()) {
                path += ".yml";
            }
        }

        std::string fullPath = path;

        if (!util::fileExists(fullPath)) {
            fullPath = util::joinPath(currentDir, path);
            if (!util::fileExists(fullPath)) {
                fullPath = util::joinPath(ctx.m_dirName, fullPath);
            }
        }

        if (!util::fileExists(fullPath)) {
            fullPath = util::joinPath(ctx.m_dirName, path);
        }

        if (!util::fileExists(fullPath)) {
            fullPath = util::joinPath(ctx.m_assetsDir, path);
        }

        if (!util::fileExists(fullPath)) {
            return { path, false };
        }

        return { fullPath, true };
    }

    void loadTerrainTiling(
        const loader::DocNode& node,
        TerrainTiling& data)
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "tiles") {
                data.tiles = readUVec3(v);
            }
            else if (k == "tile_size") {
                data.tile_size = readInt(v);
            }
            else if (k == "height_scale") {
                data.height_scale = readFloat(v);
            }
            else if (k == "vertical_range" || k == "vert_range") {
                data.vertical_range = readVec2(v);
            }
            else if (k == "horizontal_scale" || k == "horiz_scale") {
                data.horizontal_scale = readFloat(v);
            }
            else {
                reportUnknown("tiling_entry", k, v);
            }
        }
    }

    void loadRepeat(
        const loader::DocNode& node,
        Repeat& data)
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "x_count") {
                data.xCount = readInt(v);
            }
            else if (k == "y_count") {
                data.yCount = readInt(v);
            }
            else if (k == "z_count") {
                data.zCount = readInt(v);
            }
            else if (k == "x_step") {
                data.xStep = readFloat(v);
            }
            else if (k == "y_step") {
                data.yStep = readFloat(v);
            }
            else if (k == "z_step") {
                data.zStep = readFloat(v);
            }
            else {
                reportUnknown("repeat_entry", k, v);
            }
        }
    }

    bool readEnabled(
        const std::string& key)
    {
        return key[0] == 'x';
    }

    std::optional<bool> readEnabled(
        const std::string& key,
        std::optional<bool> defaultValue)
    {
        if (key[0] == 'x') {
            return false;
        }

        return defaultValue.has_value() ? defaultValue : true;
    }

    const std::string& readString(const loader::DocNode& node)
    {
        return node.asString();
    }

    bool readBool(const loader::DocNode& node)
    {
        if (!util::isBool(readString(node))) {
            KI_CRITICAL(fmt::format("invalid bool={}", renderNode(node)));
            return false;
        }

        return node.asBool();
    }

    int readInt(const loader::DocNode& node)
    {
        if (!util::isInt(readString(node))) {
            KI_CRITICAL(fmt::format("invalid int{}", renderNode(node)));
            return 0;
        }

        return node.asInt();
    }

    float readFloat(const loader::DocNode& node)
    {
        if (!util::isFloat(readString(node))) {
            KI_CRITICAL(fmt::format("invalid float {}", renderNode(node)));
            return 0.f;
        }

        return node.asFloat();
    }

    std::vector<std::string> readStringVector(const loader::DocNode& node, int reserve)
    {
        std::vector<std::string> a;
        a.reserve(reserve);

        if (node.isSequence()) {
            for (const auto& e : node.getNodes()) {
                a.push_back(readString(e));
            }
        }
        else {
            a.push_back(readString(node));
        }

        return a;
    }

    std::vector<int> readIntVector(const loader::DocNode& node, int reserve)
    {
        std::vector<int> a;
        a.reserve(reserve);

        if (node.isSequence()) {
            for (const auto& e : node.getNodes()) {
                a.push_back(readInt(e));
            }
        }
        else {
            a.push_back(readInt(node));
        }

        return a;
    }

    std::vector<float> readFloatVector(const loader::DocNode& node, int reserve)
    {
        std::vector<float> a;
        a.reserve(reserve);

        if (node.isSequence()) {
            for (const auto& e : node.getNodes()) {
                a.push_back(readFloat(e));
            }
        }
        else {
            a.push_back(readFloat(node));
        }

        return a;
    }

    glm::vec2 readVec2(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 2);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                // FILL x, x
                a.push_back(a[0]);
            }

            return glm::vec2{ a[0], a[1] };
        }

        auto v = readFloat(node);
        return glm::vec2{ v };
    }

    glm::vec3 readVec3(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 3);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                // FILL x, x, x
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                // FILL 0 (more predictable then (x, 0, z) plane)
                a.push_back(0);
            }
            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto v = readFloat(node);
        return glm::vec3{ v };
    }

    glm::vec4 readVec4(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 4);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(1.f);
            }
            else if (a.size() == 1) {
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                // FilL: x, 0, z, 1
                a.push_back(a[1]);
                a[1] = 0.f;
                // w == 1.f
                a.push_back(1.f);
            }
            else if (a.size() == 3) {
                // FILL x, y, z, 1
                // w == 1.f
                a.push_back(1.f);
            }

            return glm::vec4{ a[0], a[1], a[2], a[3] };
        }

        auto v = readFloat(node);
        return glm::vec4{ v };
    }

    glm::uvec2 readUVec2(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readIntVector(node, 2);

            if (a.size() == 0) {
                a.push_back(0);
                a.push_back(0);
            }
            else if (a.size() == 1) {
                // FILL x, x, x
                a.push_back(a[0]);
            }

            return glm::uvec2{ a[0], a[1] };
        }

        auto v = static_cast<unsigned int>(readInt(node));
        return glm::uvec2{ v };
    }

    glm::uvec3 readUVec3(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readIntVector(node, 3);

            if (a.size() == 0) {
                a.push_back(0);
                a.push_back(0);
                a.push_back(0);
            }
            else if (a.size() == 1) {
                // FILL x, x, x
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                // FILL x, 0, z
                a.push_back(a[1]);
                a[1] = 0;
            }

            return glm::uvec3{ a[0], a[1], a[2] };
        }

        auto v = static_cast<unsigned int>(readInt(node));
        return glm::uvec3{ v };
    }

    glm::uvec4 readUVec4(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readIntVector(node, 4);

            if (a.size() == 0) {
                a.push_back(0);
                a.push_back(0);
                a.push_back(0);
                a.push_back(0);
            }
            else if (a.size() == 1) {
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                // FilL: x, 0, z, 0
                a.push_back(a[1]);
                a[1] = 0;
                a.push_back(0);
            }
            else if (a.size() == 3) {
                // FILL x, y, z, 0
                a.push_back(0);
            }

            return glm::uvec4{ a[0], a[1], a[2], a[3] };
        }

        auto v = static_cast<unsigned int>(readInt(node));
        return glm::uvec4{ v };
    }

    glm::vec3 readScale3(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 3);

            if (a.size() == 0) {
                // NOTE KI default to 1 instead of 0 for scale
                a.push_back(1.f);
                a.push_back(1.f);
                a.push_back(1.f);
            }
            else if (a.size() == 1) {
                // FILL x, x, x
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                // FILL x, 1, z
                a.push_back(a[1]);
                a[1] = 1.f;
            }

            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto scale = readFloat(node);
        return glm::vec3{ scale };
    }

    glm::vec3 decodeHexToVec3(const std::string& hex)
    {
        return glm::vec3(
            std::stoul(hex.substr(0, 2), nullptr, 16) / 255.0f,
            std::stoul(hex.substr(2, 2), nullptr, 16) / 255.0f,
            std::stoul(hex.substr(4, 2), nullptr, 16) / 255.0f
        );
    }

    glm::vec4 decodeHexToVec4(const std::string& hex)
    {
        return glm::vec4(
            std::stoul(hex.substr(0, 2), nullptr, 16) / 255.0f,
            std::stoul(hex.substr(2, 2), nullptr, 16) / 255.0f,
            std::stoul(hex.substr(4, 2), nullptr, 16) / 255.0f,
            std::stoul(hex.substr(6, 2), nullptr, 16) / 255.0f
        );
    }

    glm::vec3 readRGB(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 3);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                a.push_back(a[0]);
            }

            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto encoded = readString(node);
        if (util::isFloat(encoded)) {
            auto r = readFloat(node);
            return glm::vec3{ r, r, r };
        }
        else {
            if (encoded.size() == 6) {
                auto a = decodeHexToVec3(encoded);
                return glm::vec3{ a.r, a.g, a.b };
            }
            else if (encoded.size() == 8) {
                return decodeHexToVec4(encoded);
            }
            else {
                return glm::vec3{ 1, 1, 1 };
            }
        }
    }

    glm::vec4 readRGBA(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 4);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                a.push_back(a[0]);
            }

            // NOTE KI check if alpha is missing
            if (a.size() < 4) {
                a.push_back(DEF_ALPHA);
            }

            return glm::vec4{ a[0], a[1], a[2], a[3] };
        }

        auto encoded = readString(node);
        if (util::isFloat(encoded)) {
            auto r = readFloat(node);
            return glm::vec4{ r, r, r, DEF_ALPHA };
        }
        else {
            if (encoded.size() == 6) {
                auto a = decodeHexToVec3(encoded);
                return glm::vec4{ a.r, a.g, a.b, 1.f };
            } else if (encoded.size() == 8) {
                return decodeHexToVec4(encoded);
            }
            else {
                return glm::vec4{1, 1, 1, 1};
            }
        }
    }

    glm::quat readQuat(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 4);

            while (a.size() < 3) {
                a.push_back(0.0f);
            }

            // NOTE KI {w, x, y, z }
            glm::quat q{ a[0], a[1], a[2], a[3] };
            return q;// glm::normalize(q);
        }

        auto v = readFloat(node);
        // NOTE KI {w, x, y, z }
        glm::quat q{ v, 0.f, 0.f, 0.f };
        return q;// glm::normalize(q);
    }

    glm::vec3 readDegreesRotation(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 3);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                a.push_back(0);
                a.push_back(a[0]);
                a.push_back(0);
            }
            else if (a.size() == 2) {
                a.push_back(a[0]);
            }

            return { a[0], a[1], a[2] };
        }

        auto a = readFloat(node);
        return { 0, a, 0 };
    }

    float readFractional(const loader::DocNode& node)
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 2);
            if (a.size() < 1) {
                a.push_back(1.0);
            }
            return a[0] / a[1];
        }

        return readFloat(node);
    }

    glm::vec2 readRefractionRatio(const loader::DocNode& node)
    {
        auto a = readFloatVector(node, 2);

        // NOTE KI check if just single number
        if (a.size() < 1) {
            a.push_back(1.0);
        }
        return { a[0], a[1] };
    }

    float convertRefractionRatio(const glm::vec2 ratio)
    {
        return ratio[1] != 0 ? ratio[0] / ratio[1] : ratio[0];
    }

    std::tuple<ki::node_id, std::string> resolveNodeId(
        const BaseId& typeId,
        const BaseId& baseId)
    {
        const auto& assets = Assets::get();

        if (baseId.empty()) {
            if (typeId.empty()) return {};

            const auto& typeName = SID_NAME(SID(typeId.getId()));
            const auto& nodeName = fmt::format(
                "<{}>-{}",
                typeName, ID_GENERATOR.nextId());
            return { SID(nodeName), nodeName };
        }

        const std::string& key = baseId.getId();

        if (key == ROOT_ID) {
            return { assets.rootId, SID_NAME(assets.rootId) };
        }
        else {
            auto nodeId = SID(key);
            KI_DEBUG(fmt::format("SID: sid={}, key={}", nodeId, key));
            return { nodeId, key };
        }
    }

    std::string expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        std::string out{ str };

        bool handledClone = false;
        bool handledTile = false;

        {
            const auto pos = out.find("{c}");
            if (pos != std::string::npos) {
                handledClone = true;
                out.replace(pos, 3, fmt::format("{}", cloneIndex));
            }
        }
        {
            const auto pos = out.find("{t}");
            if (pos != std::string::npos) {
                handledClone = true;
                out.replace(pos, 3, fmt::format("{}x{}x{}", tile.x, tile.y, tile.z));
            }
        }
        {
            const auto pos = out.find("{x}");
            if (pos != std::string::npos) {
                handledTile = true;
                out.replace(pos, 3, fmt::format("{}", tile.x));
            }
        }
        {
            const auto pos = out.find("{y}");
            if (pos != std::string::npos) {
                handledTile = true;
                out.replace(pos, 3, fmt::format("{}", tile.y));
            }
        }
        {
            const auto pos = out.find("{z}");
            if (pos != std::string::npos) {
                handledTile = true;
                out.replace(pos, 3, fmt::format("{}", tile.z));
            }
        }

        return out;
    }

    BaseId readId(const loader::DocNode& node)
    {
        return { readString(node) };
    }

    int readLayer(const loader::DocNode& node)
    {
        const auto& name = util::toLower(readString(node));
        const auto* layer = LayerInfo::findLayer(name);
        return layer ? layer->m_index : LAYER_NONE_INDEX;
    }

    const model::NodeType* findNodeType(
        BaseId baseId)
    {
        auto typeId = SID(baseId.getId());
        return pool::TypeHandle::toType(typeId);
    }

    NodeTypeData* findNodeTypeData(
        BaseId baseId,
        std::vector<NodeTypeData>& nodeTypes)
    {
        const auto& it = std::find_if(
            nodeTypes.begin(),
            nodeTypes.end(),
            [&baseId](const auto& e) { return e.baseId == baseId; });
        return it != nodeTypes.end() ? &(*it) : nullptr;
    }

    const NodeTypeData* findNodeTypeData(
        BaseId baseId,
        const std::vector<NodeTypeData>& nodeTypes)
    {
        const auto& it = std::find_if(
            nodeTypes.cbegin(),
            nodeTypes.cend(),
            [&baseId](const auto& e) { return e.baseId == baseId; });
        return it != nodeTypes.end() ? &(*it) : nullptr;
    }

    const Material* findMaterial(
        std::string_view name,
        const std::vector<MaterialData>& materials)
    {
        const auto& it = std::find_if(
            materials.cbegin(),
            materials.cend(),
            [&name](const auto& m) { return m.material.m_name == name; });
        return it != materials.end() ? &(it->material) : nullptr;
    }

    bool isIgnore(std::string_view k)
    {
        return k.empty() || k[0] == 'x';
    }

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const loader::DocNode& v)
    {
        std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
        KI_WARN_OUT(fmt::format("LOADER::{} {}: {}={}", prefix, scope, k, renderNode(v)));
    }

    std::string renderNode(
        const loader::DocNode& v)
    {
        std::stringstream ss;
        ss << v.str();
        return ss.str();
    }

    std::string renderNodes(
        const std::vector<loader::DocNode>& nodes)
    {
        std::stringstream ss;
        for (const auto& entry : nodes) {
            ss << renderNode(entry);
            ss << ", ";
        }
        return ss.str();
    }
}
