#include "BaseLoader.h"

#include <mutex>
#include <fstream>
#include <regex>

#include <fmt/format.h>

#include "util/Log.h"
#include "ki/yaml.h"
#include "ki/sid.h"

#include "util/Util.h"

#include "engine/AsyncLoader.h"

#include "registry/Registry.h"

namespace {
    //std::regex UUID_RE = std::regex("[0-9]{8}-[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{8}");
}

namespace loader
{
    //static const float DEF_ALPHA = 1.0;

    BaseLoader::BaseLoader(
        Context ctx)
        : m_assets(ctx.m_assets),
        m_ctx(ctx)
    {
    }

    void BaseLoader::setRegistry(std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = m_registry->m_dispatcher;
    }

    void BaseLoader::loadTiling(
        const YAML::Node& node,
        Tiling& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

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

    void BaseLoader::loadRepeat(
        const YAML::Node& node,
        Repeat& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

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

    std::string BaseLoader::readString(const YAML::Node& node) const
    {
        return node.as<std::string>();
    }

    bool BaseLoader::readBool(const YAML::Node& node) const
    {
        if (!util::isBool(node.as<std::string>())) {
            KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
            return false;
        }

        return node.as<bool>();
    }

    int BaseLoader::readInt(const YAML::Node& node) const
    {
        if (!util::isInt(node.as<std::string>())) {
            KI_WARN(fmt::format("invalid int{}", renderNode(node)));
            return 0;
        }

        return node.as<int>();
    }

    float BaseLoader::readFloat(const YAML::Node& node) const
    {
        if (!util::isFloat(node.as<std::string>())) {
            KI_WARN(fmt::format("invalid float {}", renderNode(node)));
            return 0.f;
        }

        return node.as<float>();
    }

    std::vector<int> BaseLoader::readIntVector(const YAML::Node& node, int reserve) const
    {
        std::vector<int> a;
        a.reserve(reserve);

        for (const auto& e : node) {
            a.push_back(readInt(e));
        }

        return a;
    }

    std::vector<float> BaseLoader::readFloatVector(const YAML::Node& node, int reserve) const
    {
        std::vector<float> a;
        a.reserve(reserve);

        for (const auto& e : node) {
            a.push_back(readFloat(e));
        }

        return a;
    }

    glm::vec2 BaseLoader::readVec2(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

    glm::vec3 BaseLoader::readVec3(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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
                // FILL x, 0, z
                a.push_back(a[1]);
                a[1] = 0.f;
            }
            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto v = readFloat(node);
        return glm::vec3{ v };
    }

    glm::vec4 BaseLoader::readVec4(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

    glm::uvec3 BaseLoader::readUVec3(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

        auto v = node.as<unsigned int>();
        return glm::uvec3{ v };
    }

    glm::vec3 BaseLoader::readScale3(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
            auto a = readFloatVector(node, 3);

            while (a.size() < 3) {
                a.push_back(1.0f);
            }

            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto scale = readFloat(node);
        return glm::vec3{ scale };
    }

    glm::vec3 BaseLoader::readRGB(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

        auto r = readFloat(node);
        return glm::vec3{ r, r, r };
    }

    glm::vec4 BaseLoader::readRGBA(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

        auto r = readFloat(node);
        return glm::vec4{ r, r, r, DEF_ALPHA };
    }

    glm::quat BaseLoader::readQuat(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

    glm::vec3 BaseLoader::readDegreesRotation(const YAML::Node& node) const
    {
        if (node.IsSequence()) {
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

    glm::vec2 BaseLoader::readRefractionRatio(const YAML::Node& node) const
    {
        auto a = readFloatVector(node, 2);

        // NOTE KI check if just single number
        if (a.size() < 1) {
            a.push_back(1.0);
        }
        return glm::vec2{ a[0], a[1] };
    }

    std::tuple<ki::node_id, std::string> BaseLoader::resolveId(
        const BaseId& baseId,
        const int cloneIndex,
        const glm::uvec3& tile,
        bool automatic)
    {
        if (baseId.empty()) {
            return { 0, "" };
        }

        std::string key = expandMacros(baseId.m_path, cloneIndex, tile, automatic);

        if (key == ROOT_ID) {
            return { m_ctx.m_assets.rootId, "<root>" };
        }
        else {
            auto nodeId = SID(key);
            KI_DEBUG(fmt::format("SID: sid={}, key={}", nodeId, key));
            return { nodeId, key };
        }

    }

    std::string BaseLoader::expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile,
        bool automatic)
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
                out = fmt::format("{}_{}_{}_{}", out, tile.x, tile.y, tile.z);
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

        if (automatic) {
            if (!handledClone && cloneIndex > 0) {
                out = fmt::format("{}_{}", out, cloneIndex);
            }

            if (!handledTile && (tile.x > 0 || tile.y > 0 || tile.z > 0)) {
                out = fmt::format("{}_{}_{}_{}", out, tile.x, tile.y, tile.z);
            }
        }

        return out;
    }

    BaseId BaseLoader::readId(const YAML::Node& node) const
    {
        BaseId baseId;

        baseId.m_path = node.as<std::string>();

        return baseId;
    }

    const std::string BaseLoader::resolveTexturePath(std::string_view path) const
    {
        return std::string{ path };
    }

    std::string BaseLoader::readFile(std::string_view filename) const
    {
        return util::readFile(m_ctx.m_dirName, filename);
    }

    void BaseLoader::reportUnknown(
        std::string_view scope,
        std::string_view k,
        const YAML::Node& v) const
    {
        std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
        KI_WARN_OUT(fmt::format("{} {}: {}={}", prefix, scope, k, renderNode(v)));
    }

    std::string BaseLoader::renderNode(
        const YAML::Node& v) const
    {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }
}
