#pragma once

#include <string>
#include <filesystem>

struct FileEntry {
    FileEntry();
    FileEntry(const std::string& path);
    ~FileEntry();

    int m_id{ 0 };

    std::filesystem::path m_path;
    //std::filesystem::file_time_type m_modifiedAt;
    size_t m_modifiedAt{ 0 };

    void mark();
    bool modified() const;
    bool exists() const;
};
