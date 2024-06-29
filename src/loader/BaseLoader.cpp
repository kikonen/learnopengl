#include "BaseLoader.h"

#include <mutex>
#include <fstream>
#include <regex>
#include <filesystem>
#include <iomanip>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/Log.h"
#include "ki/sid.h"

#include "util/Util.h"

#include "engine/AsyncLoader.h"

#include "registry/Registry.h"

#include "MaterialData.h"

#include "loader/document.h"

namespace {
    //std::regex UUID_RE = std::regex("[0-9]{8}-[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{8}");
}

namespace loader
{
    //static const float DEF_ALPHA = 1.0;

    BaseLoader::BaseLoader(
        Context ctx)
        : m_ctx(ctx)
    {
    }

    BaseLoader::~BaseLoader() = default;

    void BaseLoader::setRegistry(std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = m_registry->m_dispatcherWorker;
    }

    void BaseLoader::loadTiling(
        const loader::DocNode& node,
        Tiling& data) const
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

    void BaseLoader::loadRepeat(
        const loader::DocNode& node,
        Repeat& data) const
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

    const std::string& BaseLoader::readString(const loader::DocNode& node) const
    {
        return node.asString();
    }

    bool BaseLoader::readBool(const loader::DocNode& node) const
    {
        if (!util::isBool(readString(node))) {
            KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
            return false;
        }

        return node.asBool();
    }

    int BaseLoader::readInt(const loader::DocNode& node) const
    {
        if (!util::isInt(readString(node))) {
            KI_WARN(fmt::format("invalid int{}", renderNode(node)));
            return 0;
        }

        return node.asInt();
    }

    float BaseLoader::readFloat(const loader::DocNode& node) const
    {
        if (!util::isFloat(readString(node))) {
            KI_WARN(fmt::format("invalid float {}", renderNode(node)));
            return 0.f;
        }

        return node.asFloat();
    }

    std::vector<int> BaseLoader::readIntVector(const loader::DocNode& node, int reserve) const
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

    std::vector<float> BaseLoader::readFloatVector(const loader::DocNode& node, int reserve) const
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

    glm::vec2 BaseLoader::readVec2(const loader::DocNode& node) const
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

    glm::vec3 BaseLoader::readVec3(const loader::DocNode& node) const
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
                // FILL x, 0, z
                a.push_back(a[1]);
                a[1] = 0.f;
            }
            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto v = readFloat(node);
        return glm::vec3{ v };
    }

    glm::vec4 BaseLoader::readVec4(const loader::DocNode& node) const
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

    glm::uvec3 BaseLoader::readUVec3(const loader::DocNode& node) const
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

    glm::vec3 BaseLoader::readScale3(const loader::DocNode& node) const
    {
        if (node.isSequence()) {
            auto a = readFloatVector(node, 3);

            while (a.size() < 3) {
                a.push_back(1.0f);
            }

            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto scale = readFloat(node);
        return glm::vec3{ scale };
    }

    glm::vec3 BaseLoader::readRGB(const loader::DocNode& node) const
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

        auto r = readFloat(node);
        return glm::vec3{ r, r, r };
    }

    glm::vec4 BaseLoader::readRGBA(const loader::DocNode& node) const
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

        auto r = readFloat(node);
        return glm::vec4{ r, r, r, DEF_ALPHA };
    }

    glm::quat BaseLoader::readQuat(const loader::DocNode& node) const
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

    glm::vec3 BaseLoader::readDegreesRotation(const loader::DocNode& node) const
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

    float BaseLoader::readFractional(const loader::DocNode& node) const
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

    glm::vec2 BaseLoader::readRefractionRatio(const loader::DocNode& node) const
    {
        auto a = readFloatVector(node, 2);

        // NOTE KI check if just single number
        if (a.size() < 1) {
            a.push_back(1.0);
        }
        return { a[0], a[1] };
    }

    float BaseLoader::convertRefractionRatio(const glm::vec2 ratio) const noexcept {
        return ratio[1] != 0 ? ratio[0] / ratio[1] : ratio[0];
    }

    std::tuple<ki::node_id, std::string> BaseLoader::resolveId(
        const BaseId& baseId,
        const int cloneIndex,
        const glm::uvec3& tile,
        bool automatic)
    {
        const auto& assets = Assets::get();

        if (baseId.empty()) {
            return { 0, "" };
        }

        std::string key = expandMacros(baseId.m_path, cloneIndex, tile, automatic);

        if (key == ROOT_ID) {
            return { assets.rootId, "<root>" };
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

    BaseId BaseLoader::readId(const loader::DocNode& node) const
    {
        BaseId baseId;

        baseId.m_path = readString(node);

        return baseId;
    }

    std::string BaseLoader::resolveTexturePath(
        std::string_view pathName,
        bool useCompressed) const
    {
        const auto& assets = Assets::get();

        std::filesystem::path filePath{ pathName };

        if (useCompressed && assets.compressedTexturesEnabled) {
            std::filesystem::path ktxPath{ pathName };
            ktxPath.replace_extension(".ktx");

            const auto fullPath = util::joinPath(
                assets.assetsDir,
                ktxPath.string());

            if (util::fileExists(fullPath)) {
                filePath = ktxPath;
            }
        }

        return filePath.string();
    }

    std::string BaseLoader::readFile(std::string_view filename) const
    {
        return util::readFile(m_ctx.m_dirName, filename);
    }

    const Material* BaseLoader::findMaterial(
        std::string_view name,
        const std::vector<MaterialData>& materials) const
    {
        const auto& it = std::find_if(
            materials.cbegin(),
            materials.cend(),
            [&name](const auto& m) { return m.material.m_name == name; });
        return it != materials.end() ? &(it->material) : nullptr;
    }

    void BaseLoader::reportUnknown(
        std::string_view scope,
        std::string_view k,
        const loader::DocNode& v) const
    {
        std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
        KI_WARN_OUT(fmt::format("{} {}: {}={}", prefix, scope, k, renderNode(v)));
    }

    std::string BaseLoader::renderNode(
        const loader::DocNode& v) const
    {
        std::stringstream ss;
        ss << v.str();
        return ss.str();
    }
}
