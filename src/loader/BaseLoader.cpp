#include "BaseLoader.h"

#include <mutex>
#include <fstream>
#include <regex>

#include <fmt/format.h>

#include "util/Log.h"
#include "ki/yaml.h"
#include "ki/uuid.h"

#include "util/Util.h"

#include "engine/AsyncLoader.h"

namespace {
    std::mutex uuid_lock{};

    std::regex UUID_RE = std::regex("[0-9]{8}-[0-9]{4}-[0-9]{4}-[0-9]{4}-[0-9]{8}");
}

namespace loader
{
    //static const float DEF_ALPHA = 1.0;

    //static const std::string AUTO_UUID{ "AUTO" };
    //static const std::string ROOT_UUID{ "ROOT" };
    //static const std::string VOLUME_UUID{ "VOLUME" };
    //static const std::string CUBE_MAP_UUID{ "CUBE_MAP" };

    //static const std::string MACRO_STEP_X{ "X" };
    //static const std::string MACRO_STEP_Y{ "Y" };
    //static const std::string MACRO_STEP_Z{ "Z" };

    BaseLoader::BaseLoader(
        Context ctx)
        : m_assets(ctx.m_assets),
        m_ctx(ctx)
    {
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

    glm::quat BaseLoader::readRotation(const YAML::Node& node) const
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

            glm::vec3 v{ a[0], a[1], a[2] };
            return glm::quat(glm::radians(v));
        }

        auto a = readFloat(node);
        glm::vec3 v{ 0, a, 0 };
        return glm::quat(glm::radians(v));
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

    uuids::uuid BaseLoader::resolveUUID(
        const BaseUUID& parts,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        if (parts.empty()) {
            return {};
        }

        std::string key = parts[0];
        key = util::toUpper(key);

        if (key.empty()) return {};

        if (key == AUTO_UUID) {
            return resolveAutoUUID(parts, cloneIndex, tile, 1);
        }
        else if (key == ROOT_UUID) {
            return m_ctx.m_assets.rootUUID;
        }
        else if (key == VOLUME_UUID) {
            return m_ctx.m_assets.volumeUUID;
        }
        else if (key == CUBE_MAP_UUID) {
            return m_ctx.m_assets.cubeMapUUID;
        }
        else if (std::regex_match(key, UUID_RE)) {
            return KI_UUID(key);
        }
        else {
            return resolveAutoUUID(parts, cloneIndex, tile, 0);
        }
    }

    uuids::uuid BaseLoader::resolveAutoUUID(
        const BaseUUID& parts,
        const int cloneIndex,
        const glm::uvec3& tile,
        int index)
    {
        uuids::uuid uuid;
        if (parts.size() > index) {
            std::string name = expandMacros(parts[index], cloneIndex, tile);

            {
                std::lock_guard<std::mutex> lock(uuid_lock);

                if (const auto& it = m_ctx.m_autoIds->find(name);
                    it == m_ctx.m_autoIds->end())
                {
                    uuid = uuids::uuid_system_generator{}();
                    (*m_ctx.m_autoIds)[name] = uuid;
                }
                else {
                    uuid = it->second;
                }
            }
        }
        if (uuid.is_nil()) {
            uuid = uuids::uuid_system_generator{}();
        }
        return uuid;
    }

    std::string BaseLoader::expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        std::string out{ str };

        {
            const auto pos = out.find("{x}");
            if (pos != std::string::npos) {
                out.replace(pos, 3, fmt::format("{}", tile.x));
            }
        }
        {
            const auto pos = out.find("{y}");
            if (pos != std::string::npos) {
                out.replace(pos, 3, fmt::format("{}", tile.y));
            }
        }
        {
            const auto pos = out.find("{z}");
            if (pos != std::string::npos) {
                out.replace(pos, 3, fmt::format("{}", tile.z));
            }
        }

        return out;
    }

    BaseUUID BaseLoader::readUUID(const YAML::Node& node) const
    {
        BaseUUID parts;

        if (node.IsSequence()) {
            for (const auto& e : node) {
                parts.push_back(e.as<std::string>());
            }
        }
        else {
            parts.push_back(node.as<std::string>());
        }

        return parts;
    }

    const std::string BaseLoader::resolveTexturePath(std::string_view path) const
    {
        return std::string{ path };
    }

    std::string BaseLoader::readFile(std::string_view filename) const
    {
        std::stringstream buffer;

        std::string filePath = util::joinPath(
            m_ctx.m_dirname,
            filename);

        if (!util::fileExists(filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
        }

        try {
            std::ifstream t(filePath);
            t.exceptions(std::ifstream::badbit);
            //t.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            buffer << t.rdbuf();
        }
        catch (std::ifstream::failure e) {
            std::string what{ e.what() };
            const auto msg = fmt::format(
                "SCENE::FILE_NOT_SUCCESFULLY_READ: {}\n{}",
                filePath, what);

            throw std::runtime_error{ msg };
        }
        return buffer.str();
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
