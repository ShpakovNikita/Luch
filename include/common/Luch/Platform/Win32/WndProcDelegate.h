#pragma once

#define NOMINMAX
#include <windows.h>

namespace Luch::Platform::Win32
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
