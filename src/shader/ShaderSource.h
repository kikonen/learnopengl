#pragma once

#include <string>
#include <vector>
#include <map>

#include "kigl/kigl.h"

#include "FileEntry.h"

class Program;

struct ShaderSource {
    ShaderSource();

    ShaderSource(
        bool required,
        std::string path);

    ShaderSource(
        bool debug,
        bool required,
        std::string path);

    bool m_debug;
    bool m_required;
    std::string m_path;

    std::string m_source;

private:
    std::vector<FileEntry*> m_files;

public:
    bool empty() const {
        return m_source.empty();
    }

    bool pathExists() const;
    bool exists() const;
    bool modified() const;

    void clear();

    void load(
        GLenum shaderType,
        const Program& program);

    std::string loadSource(
        GLenum shaderType,
        const std::string& path,
        bool optional,
        const Program& program);

    std::vector<std::string> loadSourceLines(
        GLenum shaderType,
        const std::string& path,
        bool optional,
        const Program& program);

    std::vector<std::string> processInclude(
        GLenum shaderType,
        std::string_view includePath,
        int lineNumber,
        const Program& program);

    void appendDefines(
        GLenum shaderType,
        std::vector<std::string>& lines,
        const Program& program);
};
