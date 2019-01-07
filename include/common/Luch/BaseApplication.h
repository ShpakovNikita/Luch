#pragma once

#include <Luch/Types.h>

namespace Luch
{

class BaseApplication
{
public:
    virtual ~BaseApplication() = default;
    virtual bool Initialize(const Vector<String>& args) = 0;
    virtual bool Deinitialize() = 0;
    virtual bool ShouldQuit() const = 0;

    virtual const String& GetApplicationName() const = 0;
    virtual const String& GetMainWindowTitle() const = 0;
protected:
    BaseApplication() = default;
};

}
