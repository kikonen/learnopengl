#include "FileEntryCache.h"

#include <cassert>

namespace {
    static FileEntryCache* s_instance{ nullptr };
}

void FileEntryCache::init() noexcept
{
    assert(!s_instance);
    s_instance= new FileEntryCache();
}

void FileEntryCache::release() noexcept
{
    auto* s = s_instance;
    s_instance = nullptr;
    delete s;
}

FileEntryCache& FileEntryCache::get() noexcept
{
    assert(s_instance);
    return *s_instance;
}

FileEntryCache::FileEntryCache()
{
    clear();
}

FileEntryCache::~FileEntryCache()
{
    clear();
}

void FileEntryCache::clear()
{
    m_files.clear();
    m_files.emplace_back();
}

FileEntry* FileEntryCache::getEntry(const std::string& path)
{
    if (path.empty()) return nullptr;

    std::lock_guard lock(m_lock);

    const auto& it = m_pathToId.find(path);
    if (it != m_pathToId.end()) {
        return m_files[it->second].get();
    }

    auto fileEntry = std::make_unique<FileEntry>(path);

    if (!fileEntry->exists()) {
        return nullptr;
    }
    fileEntry->mark();

    int fileId = static_cast<int>(m_files.size());
    {
        fileEntry->m_id = fileId;

        m_files.push_back(std::move(fileEntry));
        m_pathToId.insert({ path, fileId });
    }

    return m_files[fileId].get();
}

void FileEntryCache::markAllModified()
{
    std::lock_guard lock(m_lock);

    for (const auto& fileEntry : m_files) {
        if (fileEntry && fileEntry->exists()) {
            fileEntry->mark();
            m_modified.push_back(fileEntry->m_id);
        }
    }
}

void FileEntryCache::checkModified()
{
    std::lock_guard lock(m_lock);

    for (const auto& fileEntry : m_files) {
        //fileEntry->exists()
        if (fileEntry && fileEntry->modified()) {
            fileEntry->mark();
            m_modified.push_back(fileEntry->m_id);
        }
    }
}

void FileEntryCache::clearModified()
{
    m_modified.clear();
}

bool FileEntryCache::isModified(int fileId)
{
    if (!fileId) return false;

    const auto& it = std::find(m_modified.begin(), m_modified.end(), fileId);
    return it != m_modified.end();
}
