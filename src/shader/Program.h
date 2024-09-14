#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"
#include "ki/size.h"

namespace uniform {
    class Uniform;
    class Subroutine;
}

struct ProgramUniforms;

class Program final
{
    friend uniform::Uniform;
    friend uniform::Subroutine;

public:
    static Program* get(ki::program_id id);

    // public due to shared_ptr
    Program(
        ki::program_id id,
        std::string_view key,
        std::string_view name,
        const bool compute,
        std::string_view geometryType,
        const std::map<std::string, std::string, std::less<>>& defines);

    // TODO KI implement move
    Program(Program&& o) = delete;

    // https://stackoverflow.com/questions/7823845/disable-compiler-generated-copy-assignment-operator
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    Program& operator=(Program&& o) = delete;

    // public due to shared_ptr
    ~Program();

    void validateProgram() const;

public:
    inline bool isReady() const { return m_prepared; }

    void load();

    ki::program_id prepareRT();

    void bind() const noexcept;

    int prepared() noexcept { return m_prepared; }

    void setInt(std::string_view name, int value) noexcept;
    void setFloat(std::string_view name, float value) noexcept;
    void setMat4(std::string_view name, const glm::mat4& value) noexcept;

    void setupUBO(
        const char* name,
        unsigned int UBO,
        unsigned int expectedSize);

    operator int() const { return m_programId; }

private:
    // @return shaderId
    int compileSource(
        GLenum shaderType,
        const std::string& shaderPath,
        const std::string& source);

    void createProgram();
    void initProgram();

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
    const ki::program_id m_id;

    const std::string m_programName;
    const std::string m_key;

    const bool m_compute;

    const std::string m_geometryType;

    int m_programId = -1;

    std::unique_ptr<ProgramUniforms> m_uniforms;

private:
    bool m_prepared = false;

    std::map<std::string, std::string, std::less<> > m_defines;

    std::unordered_map<GLenum, std::string> m_paths;
    std::unordered_map<GLenum, bool> m_required;
    std::unordered_map<GLenum, std::string> m_sources;

    std::map<std::string, GLint, std::less<> > m_uniformLocations;
    std::unordered_map<GLenum, std::map<std::string, GLuint, std::less<>> > m_subroutineIndeces;
    std::unordered_map<GLenum, std::map<std::string, GLuint, std::less<>> > m_subroutineLocations;
};
