#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

#include "FileEntry.h"

class FileEntryCache {
public:
    static FileEntryCache& get();

    FileEntryCache();
    ~FileEntryCache();

    // @return file, null if not existing
    FileEntry* getEntry(const std::string& path);

    void markModified();
    void clearModified();

    bool isModified(int fileId);

private:
    std::mutex m_lock{};

    std::vector<std::unique_ptr<FileEntry>> m_files;
    std::unordered_map<std::string, int> m_pathToId;

    std::vector<int> m_modified;
};

