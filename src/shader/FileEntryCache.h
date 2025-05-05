#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

#include "FileEntry.h"

class FileEntryCache {
public:
    static void init() noexcept;
    static void release() noexcept;
    static FileEntryCache& get() noexcept;

    FileEntryCache();
    FileEntryCache& operator=(const FileEntryCache&) = delete;

    ~FileEntryCache();

    void clear();

    // @return file, null if not existing
    FileEntry* getEntry(const std::string& path);

    // Tag all files as modified
    void markAllModified();

    void checkModified();
    void clearModified();

    bool isModified(int fileId);

    size_t getSize() const noexcept
    {
        return m_files.size();
    }

    size_t getModifiedCount() const noexcept
    {
        return m_modified.size();
    }

private:
    std::mutex m_lock{};

    std::vector<std::unique_ptr<FileEntry>> m_files;
    std::unordered_map<std::string, int> m_pathToId;

    std::vector<int> m_modified;
};

