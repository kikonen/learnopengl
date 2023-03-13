#pragma once

#include <map>
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

private:
    const Assets& m_assets;

    int m_prepareResult = -1;
    bool m_prepared = false;

    mutable std::map<std::string, std::string> m_defines;

    std::map<GLenum, std::string> m_paths;
    std::map<GLenum, bool> m_required;
    std::map<GLenum, std::string> m_sources;

    std::map<const std::string, GLint> m_uniformLocations;
    std::map<GLenum, std::map<const std::string, GLuint>> m_subroutineIndeces;
    std::map<GLenum, std::map<const std::string, GLuint>> m_subroutineLocations;
};
