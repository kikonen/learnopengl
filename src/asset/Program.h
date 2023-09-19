#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "Assets.h"


class GLState;

namespace uniform {
    class Uniform;
    class Subroutine;
    class Float;
    class Int;
    class UInt;
    class Bool;
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

    void setInt(const std::string& name, int value) noexcept;
    void setFloat(const std::string& name, float value) noexcept;
    void setMat4(const std::string& name, const glm::mat4& value) noexcept;

    void setupUBO(
        const char* name,
        unsigned int UBO,
        unsigned int expectedSize);

    operator int() const { return m_programId; }

public:
    // public due to shared_ptr
    Program(
        const Assets& assets,
        const std::string& key,
        const std::string& name,
        const bool compute,
        const std::string& geometryType,
        const std::map<std::string, std::string>& defines);

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

    std::string loadSource(const std::string& filename, bool optional);
    std::vector<std::string> loadSourceLines(const std::string& path, bool optional);
    std::vector<std::string> processInclude(const std::string& includePath, int lineNumber);

    //void prepareTextureUniform();
    //void prepareTextureUniforms();

    GLint getUniformLoc(const std::string& name);
    GLint getUniformSubroutineLoc(const std::string& name, GLenum shadertype);
    GLint getSubroutineIndex(const std::string& name, GLenum shadertype);

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

    std::unique_ptr<uniform::Int> u_stencilMode;

private:
    const Assets& m_assets;

    int m_prepareResult = -1;
    bool m_prepared = false;

    mutable std::map<std::string, std::string> m_defines;

    std::unordered_map<GLenum, std::string> m_paths;
    std::unordered_map<GLenum, bool> m_required;
    std::unordered_map<GLenum, std::string> m_sources;

    std::unordered_map<std::string, GLint> m_uniformLocations;
    std::unordered_map<GLenum, std::unordered_map<std::string, GLuint>> m_subroutineIndeces;
    std::unordered_map<GLenum, std::unordered_map<std::string, GLuint>> m_subroutineLocations;
};
