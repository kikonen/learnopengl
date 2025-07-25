#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"
#include "ki/size.h"

#include "ShaderSource.h"

namespace uniform {
    class Uniform;
    class Subroutine;
}

struct ProgramUniforms;

class Program final
{
    friend class uniform::Uniform;
    friend class uniform::Subroutine;
    friend struct ShaderSource;

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

public:
    inline bool isPrepared() const {
        return m_prepared;
    }

    inline bool isReady() const {
        return m_prepared && m_programId > 0;
    }

    bool isLoaded() const noexcept
    {
        return m_loaded;
    }

    bool isModified() const noexcept;

    void load();
    void reload();

    ki::program_id prepareRT();

    void bind() const noexcept;

    int prepared() noexcept { return m_prepared; }

    void setInt(const std::string& name, int value) noexcept;
    void setFloat(const std::string& name, float value) noexcept;
    void setVec3(const std::string& name, glm::vec3 value) noexcept;
    void setMat4(const std::string& name, const glm::mat4& value) noexcept;
    void setVec3Array(
        const std::string& name,
        const std::vector<glm::vec3>& value) noexcept;

    operator int() const { return m_programId; }

    const std::map<std::string, std::string, std::less<> >& getDefines() const
    {
        return m_defines;
    }

    // NOTE KI apply "geom mod" for debugging (optional per shader)
    // @return true if program needs re-compile
    bool setDebugGeometryType(const std::string& geometryType);

private:
    // @return shaderId
    int compileSource(
        GLenum shaderType,
        const ShaderSource& source);

    void createProgram();

    void initProgram(int programId) const;
    void validateProgram(int programId) const;

    void validateUBO(
        int programId,
        const char* name,
        unsigned int UBO,
        unsigned int expectedSize) const;


    //void prepareTextureUniform();
    //void prepareTextureUniforms();

    GLint getUniformLoc(const std::string& name);
    GLint getUniformSubroutineLoc(const std::string& name, GLenum shadertype);
    GLint getSubroutineIndex(const std::string& name, GLenum shadertype);

public:
    const ki::program_id m_id;

    const std::string m_programName;
    const std::string m_key;

    const bool m_compute;

    const std::string m_geometryType;

    GLuint m_programId{ 0 };

    std::unique_ptr<ProgramUniforms> m_uniforms;

private:
    bool m_loaded{ false };
    bool m_prepared{ false };

    std::map<std::string, std::string, std::less<> > m_defines;

    std::string m_basePath;
    std::unordered_map<GLenum, ShaderSource> m_sources;

    std::map<std::string, GLint, std::less<> > m_uniformLocations;
    std::unordered_map<GLenum, std::map<std::string, GLuint, std::less<>> > m_subroutineIndeces;
    std::unordered_map<GLenum, std::map<std::string, GLuint, std::less<>> > m_subroutineLocations;
};
