#pragma once

#include <windows.h>

namespace Husky::Platform::Win32
{

class WndProcDelegate
{
public:
    virtual LRESULT WndProc(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    ) = 0;
};

}
