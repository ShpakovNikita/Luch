#pragma once

#include <Husky/Types.h>
#include <Husky/Version.h>

namespace Husky
{

class BaseApplication
{
public:
    virtual ~BaseApplication() = default;
    virtual void Initialize(const Vector<String>& args) = 0;
    virtual void Deinitialize() = 0;
    virtual void Run() = 0;
protected:
    virtual const String& GetApplicationName() = 0;
    virtual const Version& GetApplicationVersion() = 0;

    BaseApplication() = default;
};

}
