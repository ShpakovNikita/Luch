#pragma once

#include <fstream>
#include <Husky/Stream.h>
#include <Husky/Flags.h>

namespace Husky
{

enum class FileOpenModes
{
    Read,
    Write,
    Append,
    Truncate,
    AtEnd
};

DEFINE_OPERATORS_FOR_FLAGS_ENUM(FileOpenModes);

class FileStream : public Stream
{
public:
    FileStream(const FilePath& filename, FileOpenModes mode);
    int64 Read(void* buffer, int64 count, int64 elementSize) override;
    void Write(const void* buffer, int64 count, int64 elementSize) override;
    int64 GetSize() const override;
    int64 GetPosition() const override;
    int64 Seek(int64 offset, SeekOrigin origin) override;
    bool IsReadable() const override;
    bool IsWriteable() const override;
    void Close() override;
    ~FileStream();
private:
    static std::ios::openmode ToStdOpenMode(FileOpenModes mode);
    static std::ios::seekdir ToStdSeekDir(SeekOrigin origin);
    mutable std::fstream fstream;
    FilePath canonicalPath;
    FileOpenModes mode;
};

}
