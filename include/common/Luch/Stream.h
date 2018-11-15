#pragma once

#include <Luch/Types.h>

namespace Luch
{

enum class SeekOrigin
{
    Begin,
    Current,
    End
};

class Stream
{
public:
    virtual int64 Read(void* buffer, int64 count, int64 elementSize) = 0;

    virtual void Write(const void* buffer, int64 count, int64 elementSize) = 0;

    template<typename T>
    int64 Read(T* buffer, int64 count)
    {
        return Read(buffer, count, sizeof(T));
    }

    template<typename T>
    void Write(T* buffer, int64 count)
    {
        Write(buffer, count, sizeof(T));
    }

    template<typename T>
    std::vector<T> ReadAll()
    {
        std::vector<T> result;
        auto size = GetSize();
        auto elementCount = size / sizeof(T);
        result.resize(elementCount);
        Read(result.data(), elementCount, sizeof(T));
        return result;
    }

    virtual int64 GetSize() const = 0;
    virtual int64 GetPosition() const = 0;

    virtual int64 Seek(int64 offset, SeekOrigin origin) = 0;

    virtual bool IsReadable() const = 0;
    virtual bool IsWriteable() const = 0;
    virtual void Close() = 0;

    virtual ~Stream() {};
protected:
    Stream() = default;
private:
    Stream(const Stream& other) = delete;
    Stream& operator=(const Stream& other) = delete;
};

}
