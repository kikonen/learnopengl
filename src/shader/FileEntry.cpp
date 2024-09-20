#include "FileEntry.h"

#include "util/Util.h"

void FileEntry::mark()
{
    m_modifiedAt = std::filesystem::last_write_time(m_path);
}

bool FileEntry::modified() const
{
    const auto& ts = std::filesystem::last_write_time(m_path);
    if (ts != m_modifiedAt)
        int x = 0;
    return m_modifiedAt != ts;
}

bool FileEntry::exists() const
{
    return std::filesystem::exists(m_path);
}
