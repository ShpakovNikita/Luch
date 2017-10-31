#include <Husky/FileStream.h>
#include <Husky/Assert.h>

namespace Husky
{

FileStream::FileStream(const String& filename, FileOpenModes aMode)
    : mode(mode)
{
    fstream.open(filename, ToStdOpenMode(mode));
}

int64 FileStream::Read(void* buffer, int64 count, int64 elementSize)
{
    auto positionBefore = fstream.tellg();
    fstream.read(reinterpret_cast<char*>(buffer), count*elementSize);
    auto positionAfter = fstream.tellg();
    return positionAfter - positionBefore;
}

void FileStream::Write(const void* buffer, int64 count, int64 elementSize)
{
    fstream.write(reinterpret_cast<const char*>(buffer), count*elementSize);
}

int64 FileStream::GetSize() const
{
    auto positionBefore = fstream.tellg();
    fstream.seekg(0, std::ios::end);
    auto result = fstream.tellg();
    fstream.seekg(positionBefore, std::ios::beg);
    return result;
}

int64 FileStream::GetPosition() const
{
    return fstream.tellg();
}

int64 FileStream::Seek(int64 offset, SeekOrigin origin)
{
    fstream.seekg(offset, ToStdSeekDir(origin));
}

bool FileStream::IsReadable() const
{
    return (mode & FileOpenModes::Read) == FileOpenModes::Read;
}

bool FileStream::IsWriteable() const
{
    return (mode & FileOpenModes::Write) == FileOpenModes::Write;
}

void FileStream::Close()
{
    fstream.close();
}

FileStream::~FileStream()
{
    fstream.close();
}

std::ios::openmode FileStream::ToStdOpenMode(FileOpenModes mode)
{
    std::ios::openmode result = std::ios::binary;

    if((mode & FileOpenModes::Append) == FileOpenModes::Append)
    {
        result |= std::ios::app;
    }
    if ((mode & FileOpenModes::AtEnd) == FileOpenModes::AtEnd)
    {
        result |= std::ios::ate;
    }
    if ((mode & FileOpenModes::Read) == FileOpenModes::Read)
    {
        result |= std::ios::in;
    }
    if ((mode & FileOpenModes::Truncate) == FileOpenModes::Truncate)
    {
        result |= std::ios::trunc;
    }
    if ((mode & FileOpenModes::Write) == FileOpenModes::Write)
    {
        result |= std::ios::out;
    }

    return result;
}

std::ios::seekdir FileStream::ToStdSeekDir(SeekOrigin origin)
{
    switch (origin)
    {
        case SeekOrigin::Begin:
            return std::ios::beg;
        case SeekOrigin::Current:
            return std::ios::cur;
        case SeekOrigin::End:
            return std::ios::end;
        default:
            HUSKY_ASSERT(false);
    }
}

}
