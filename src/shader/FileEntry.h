#pragma once

#include <string>
#include <filesystem>

struct FileEntry {
    std::filesystem::path m_path;
    std::filesystem::file_time_type m_modifiedAt;

    void mark();
    bool modified() const;
    bool exists() const;
};
