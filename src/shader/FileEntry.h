#pragma once

#include <string>
#include <filesystem>

struct FileEntry {
    FileEntry();
    FileEntry(const std::string& path);
    ~FileEntry();

    int m_id{ 0 };

    std::filesystem::path m_path;
    std::filesystem::file_time_type m_modifiedAt;

    void mark();
    bool modified() const;
    bool exists() const;
};
