#pragma once

#include <string>
#include <vector>
#include <map>

#include "FileEntry.h"

class Program;

struct ShaderSource {
    ShaderSource();

    ShaderSource(
        bool required,
        std::string path);

    bool m_required;
    std::string m_path;

    std::string m_source;

private:
    std::vector<FileEntry*> m_files;

public:
    bool empty() const {
        return m_source.empty();
    }

    bool exists() const;
    bool modified() const;

    void clear();

    void load(const Program& program);

    std::string loadSource(
        const std::string& path,
        bool optional,
        const Program& program);

    std::vector<std::string> loadSourceLines(
        const std::string& path,
        bool optional,
        const Program& program);

    std::vector<std::string> processInclude(
        std::string_view includePath,
        int lineNumber,
        const Program& program);

    void appendDefines(
        std::vector<std::string>& lines,
        const Program& program);
};
