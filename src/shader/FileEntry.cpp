#include "FileEntry.h"

#include "windows.h"
#include "fileapi.h"

#include "util/util.h"

namespace
{
    // NOTE KI std::filesystem::last_write_time(path) is a bit slow
    //
    // https://www.cppstories.com/2024/cpp-query-file-attribs-faster/
    // https://stackoverflow.com/questions/514199/example-of-using-findfirstfileex-with-specific-search-criteria
    //
    size_t getModifiedAt(const std::filesystem::path& path) {
        //return std::filesystem::last_write_time(path);

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFileEx(
            path.c_str(),
            FindExInfoStandard,
            &findFileData,
            FindExSearchNameMatch, NULL, 0);

        if (hFind == INVALID_HANDLE_VALUE) {
            return 0;
        }

        return (((size_t)findFileData.ftLastWriteTime.dwHighDateTime) << 32) + findFileData.ftLastWriteTime.dwLowDateTime;
    }
}

FileEntry::FileEntry()
{}

FileEntry::FileEntry(const std::string& path)
    : m_path{ path }
{}

FileEntry::~FileEntry() = default;

void FileEntry::mark()
{
    m_modifiedAt = getModifiedAt(m_path);
}

bool FileEntry::modified() const
{
    const auto ts = getModifiedAt(m_path);
    return m_modifiedAt != ts;
}

bool FileEntry::exists() const
{
    return std::filesystem::exists(m_path);
}
