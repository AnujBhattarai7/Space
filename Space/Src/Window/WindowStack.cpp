#include "PCH.h"
#include "WindowStack.h"

namespace Space
{
    WindowStack *WindowStack::Get()
    {
        if (_Stack == NULL)
            SP_CORE_ERROR("Init not called!!");
        return _Stack;
    } // namespace Space

    int WindowStack::AddWindows(Window *_W)
    {
        SP_CORE_PRINT("[WINDOWSTACK]: Added Windows: " << _W->GetName())

        // Used when we dont have to replace any Window*
        if (_Stack->_Replace_Index == -1)
        {
            _Stack->_Windows.push_back(_W);
            _Stack->_Active_Window = Size() - 1;

            return Size() - 1;
        }

        // Replaces the index present in the _Replace_Index
        // Sets the Window* to _W
        _Stack->_Windows[_Stack->_Replace_Index] = std::move(_W);

        // Updates the _Index
        _Stack->_Active_Window = _Stack->_Replace_Index;
        _Stack->_Replace_Index = -1;

        return _Stack->_Active_Window;
    }

    void WindowStack::PopWindows(int i)
    {
        // Deletes the i index in _Windows
        SP_CORE_PRINT("[WINDOWSTACK]: Window: " << _Stack->_Windows[i]->GetName() << " Deleted!!");
        delete _Stack->_Windows[i];
        _Stack->_Windows.erase(_Stack->_Windows.begin() + i);
        _Stack->_Replace_Index = i;

        if(_Stack->_RenderIndex == i)
            _Stack->_RenderIndex = 0;
    }

    void WindowStack::Flush()
    {
        for (int i = 0; i < _Stack->_Windows.size(); i++)
        {
            SP_CORE_PRINT("[WINDOWSTACK]: Window: " << _Stack->_Windows[i]->GetName() << " Deleted!!");
            delete _Stack->_Windows[i];
            _Stack->_Windows.erase(_Stack->_Windows.begin() + i);
        }
    }

    Window &WindowStack::GetWindows(int i)
    {
        if (i == _Stack->_Replace_Index)
            SP_CORE_ERROR("The given index has experinced Pop_Windows")

        if (i > _Stack->_Windows.size())
            SP_CORE_ERROR("[WINDOWSTACK]: Index out of range!!")

        return *_Stack->_Windows[i];
    }

    int WindowStack::GetWindowIndex(Window *_W)
    {
        for (int i = 0; i < Size(); i++)
        {
            if (_Stack->GetWindows(i).GetID() == _W->GetID())
                return i;
        }

        return -1;
    }

    void WindowStack::ShutDown()
    {
        Flush();
        SP_CORE_PRINT("[WINDOWSTACK]: ShutDown!!")
        _SHUT_GLFW();
        delete _Stack;
    }
}