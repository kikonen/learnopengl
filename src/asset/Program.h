#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"
#include "Assets.h"


class GLState;

namespace uniform {
    class Uniform;
    class Subroutine;
    class Float;
    class Int;
    class UInt;
    class Bool;
    class Mat4;
}

class Program final
{
    friend uniform::Uniform;
    friend uniform::Subroutine;

public:
    void load();

    int prepare(const Assets& assets);

    void bind(GLState& state) const noexcept;

    int prepared() noexcept { return m_prepared; }

    void setInt(std::string_view name, int value) noexcept;
    void setFloat(std::string_view name, float value) noexcept;
    void setMat4(std::string_view name, const glm::mat4& value) noexcept;

    void setupUBO(
        const char* name,
        unsigned int UBO,
        unsigned int expectedSize);

    operator int() const { return m_programId; }

public:
    // public due to shared_ptr
    Program(
        const Assets& assets,
        std::string_view key,
        std::string_view name,
        const bool compute,
        std::string_view geometryType,
        const std::map<std::string, std::string, std::less<>>& defines);

    // https://stackoverflow.com/questions/7823845/disable-compiler-generated-copy-assignment-operator
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    // public due to shared_ptr
    ~Program();

    void validateProgram() const;

private:
    // @return shaderId
    int compileSource(
        GLenum shaderType,
        const std::string& shaderPath,
        const std::string& source);

    int createProgram();
    int initProgram();

    void appendDefines(std::vector<std::string>& lines);

    std::string loadSource(std::string_view filename, bool optional);
    std::vector<std::string> loadSourceLines(std::string_view path, bool optional);
    std::vector<std::string> processInclude(std::string_view includePath, int lineNumber);

    //void prepareTextureUniform();
    //void prepareTextureUniforms();

    GLint getUniformLoc(std::string_view name);
    GLint getUniformSubroutineLoc(std::string_view name, GLenum shadertype);
    GLint getSubroutineIndex(std::string_view name, GLenum shadertype);

public:
    const int m_objectID;

    const std::string m_programName;
    const std::string m_key;

    const bool m_compute;

    const std::string m_geometryType;

    int m_programId = -1;

    std::unique_ptr<uniform::UInt> u_shadowIndex;
    std::unique_ptr<uniform::Subroutine> u_effect;

    std::unique_ptr<uniform::Float> u_nearPlane;
    std::unique_ptr<uniform::Float> u_farPlane;

    std::unique_ptr<uniform::UInt> u_drawParametersIndex;

    std::unique_ptr<uniform::UInt> u_effectBloomIteration;

    std::unique_ptr<uniform::Bool> u_toneHdri;
    std::unique_ptr<uniform::Bool> u_gammaCorrect;
    std::unique_ptr<uniform::Mat4> u_viewportTransform;

    std::unique_ptr<uniform::Int> u_stencilMode;

private:
    const Assets& m_assets;

    int m_prepareResult = -1;
    bool m_prepared = false;

    mutable std::map<std::string, std::string, std::less<> > m_defines;

    std::unordered_map<GLenum, std::string> m_paths;
    std::unordered_map<GLenum, bool> m_required;
    std::unordered_map<GLenum, std::string> m_sources;

    std::map<std::string, GLint, std::less<> > m_uniformLocations;
    std::unordered_map<GLenum, std::map<std::string, GLuint, std::less<>> > m_subroutineIndeces;
    std::unordered_map<GLenum, std::map<std::string, GLuint, std::less<>> > m_subroutineLocations;
};
